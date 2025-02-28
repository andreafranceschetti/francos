#pragma once

#include <atomic>
#include <array>


namespace francos {


// thread safe 
// lock free
// fixed size
template<typename T, std::size_t Size = 64>
class Queue {

public:
    Queue(bool override_last_msg = false): override_last_msg_{override_last_msg} { }

    bool push(const T& item) {
        std::size_t current_tail = tail_.load(std::memory_order_relaxed);
        std::size_t next_tail = (current_tail + 1) % Size;

        if(!override_last_msg_){
            if (next_tail == head_.load(std::memory_order_acquire)) { // Queue is full
                return false;
            }
        }

        buffer_[current_tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        std::size_t current_head = head_.load(std::memory_order_relaxed);

        if (current_head == tail_.load(std::memory_order_acquire)) { // Queue is empty
            return false;
        }

        item = buffer_[current_head];
        head_.store((current_head + 1) % Size, std::memory_order_release);
        return true;
    }

    bool is_empty() const noexcept {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

private:

    std::atomic_uint32_t head_ = 0;
    std::atomic_uint32_t tail_ = 0;
    std::array<T, Size> buffer_;
    bool override_last_msg_ = false;

};


}