#pragma once

#include <atomic>

namespace francos
{

    template <typename Message>
    struct Slot
    {
        std::atomic<std::int32_t> ref_count;
        Message msg;

        Slot() : ref_count(0), msg() {}

        Slot(std::uint32_t count, Message message)
            : ref_count(count), msg(std::move(message)) {}

        Slot(const Slot &other)
            : ref_count(other.ref_count.load(std::memory_order_relaxed)), msg(other.msg) {}

        Slot(Slot &&other) noexcept
            : ref_count(other.ref_count.load()), msg(std::move(other.msg)) {}

        Slot &operator=(const Slot &other)
        {
            if (this != &other)
            {
                ref_count.store(other.ref_count.load());
                msg = other.msg;
            }
            return *this;
        }

        Slot &operator=(Slot &&other) noexcept
        {
            if (this != &other)
            {
                ref_count.store(other.ref_count.load());
                msg = std::move(other.msg); // Use std::move for Message
            }
            return *this;
        }
    };

    template <typename T, std::size_t Size = 64>
    class SlotQueue
    {
    public:
        // Here T is expected to be a MessageSlot<U> for some U.
        using OverwriteRule = std::function<bool(const T &)>;

        // subscriber_count: number of subscribers that must each read a message.
        // Overwrite rule defaults to always allow overwriting.
        SlotQueue(uint32_t subscriber_count, OverwriteRule is_overwritable = [](const T &)
                                             { return true; }) : subscriber_count_(subscriber_count), is_overwritable_(is_overwritable)
        {
            // Initialize the buffer’s ref_count to zero.
            for (std::size_t i = 0; i < Size; ++i)
            {
                buffer_[i].ref_count.store(0, std::memory_order_relaxed);
            }
            head_.store(0, std::memory_order_relaxed);
            tail_.store(0, std::memory_order_relaxed);
        }

        // MPMC-safe push. Returns false if the queue is full and cannot be overwritten.
        bool push(const T &item)
        {
            std::size_t pos, next_tail, current_head;
            while (true)
            {
                pos = tail_.load(std::memory_order_acquire);
                next_tail = next(pos);
                current_head = head_.load(std::memory_order_acquire);

                // If the next slot equals head, the buffer is full.
                if (next_tail == current_head)
                {
                    // Check the oldest slot (at current_head).
                    // Only allow overwriting if the rule permits and if its ref_count is zero
                    // (i.e. all subscribers have consumed the previous message).
                    if (is_overwritable_(buffer_[current_head]) &&
                        buffer_[current_head].ref_count.load(std::memory_order_acquire) == 0)
                    {
                        // Try to advance head to free the slot.
                        if (!head_.compare_exchange_weak(current_head, next(current_head),
                                                         std::memory_order_release,
                                                         std::memory_order_relaxed))
                        {
                            continue; // Retry if CAS fails.
                        }
                    }
                    else
                    {
                        return false; // Cannot overwrite.
                    }
                }
                // Reserve the slot by advancing tail.
                if (tail_.compare_exchange_weak(pos, next_tail,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed))
                {
                    break; // Successfully reserved slot at index pos.
                }
                // Otherwise, retry.
            }
            // Write the new message into the reserved slot.
            buffer_[pos] = item;
            // Set the slot's ref_count to the number of subscribers.
            buffer_[pos].ref_count.store(subscriber_count_, std::memory_order_release);
            return true;
        }

        // Instead of a pop() that removes the message immediately, provide a read() function.
        // Each subscriber should call read() to get the message and decrement the slot's ref_count.
        // The last subscriber to read the message will advance the head pointer.
        bool read(T &out)
        {
            std::size_t pos = head_.load(std::memory_order_acquire);
            std::size_t current_tail = tail_.load(std::memory_order_acquire);
            if (pos == current_tail)
            {
                return false; // Queue is empty.
            }
            // Reference to the slot at the head.
            T &slot = buffer_[pos];
            // Atomically decrement the ref_count.
            uint32_t old = slot.ref_count.fetch_sub(1, std::memory_order_acq_rel);
            if (old == 0)
            {
                // Should not happen: reading a message that’s already been fully consumed.
                return false;
            }
            // Provide the message to the subscriber.
            out = slot.msg;
            // If this was the last subscriber, advance the head pointer.
            if (old == 1)
            {
                std::size_t expected = pos;
                head_.compare_exchange_strong(expected, next(pos),
                                              std::memory_order_release,
                                              std::memory_order_relaxed);
            }
            return true;
        }

        bool is_empty() const noexcept
        {
            return head_.load(std::memory_order_acquire) ==
                   tail_.load(std::memory_order_acquire);
        }

        size_t size() const
        {
            std::size_t h = head_.load(std::memory_order_acquire);
            std::size_t t = tail_.load(std::memory_order_acquire);
            return (t + Size - h) % Size;
        }

        T &front()
        {
            return buffer_[head_.load(std::memory_order_acquire)];
        }

        T &back()
        {
            return buffer_[tail_.load(std::memory_order_acquire)];
        }

    private:
        // Compute the next index in the circular buffer.
        std::size_t next(std::size_t n) const
        {
            return (n + 1) % Size;
        }

        std::atomic<std::size_t> head_{0};
        std::atomic<std::size_t> tail_{0};
        std::array<T, Size> buffer_;
        OverwriteRule is_overwritable_;
        uint32_t subscriber_count_;
    };

}