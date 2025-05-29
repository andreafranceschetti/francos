#pragma once

#include <atomic>
#include <array>
#include <functional>


namespace francos {


// thread safe 
// lock free
// fixed size
template<typename T, std::size_t Size = 64>
class Queue {

public:
    using OverwriteRule = std::function<bool(T const&)>;
    Queue(OverwriteRule is_overwritable = [](T const&) {return true;}) : is_overwritable_(is_overwritable) { }

    bool push(const T& item) {
        std::size_t current_tail = tail_.load(std::memory_order_acquire);
        std::size_t current_head = head_.load(std::memory_order_acquire);
        std::size_t next_tail = next(current_tail);

        if (next_tail == current_head) { // Queue is full
            
            if(is_overwritable_(buffer_[current_head])){ 
                // advance head too
                head_.store(next(current_head), std::memory_order_release);
            } else{
                return false; 
            }
        }

        buffer_[current_tail] = std::move(item);
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        std::size_t current_head = head_.load(std::memory_order_acquire);
        std::size_t current_tail = tail_.load(std::memory_order_acquire);

        if (current_head == current_tail) { // Queue is empty
            return false;
        }

        item = buffer_[current_head];
        head_.store(next(current_head), std::memory_order_release);
        return true;
    }

    bool is_empty() const noexcept {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

    size_t size() const {
        std::size_t current_head = head_.load(std::memory_order_relaxed);
        std::size_t current_tail = tail_.load(std::memory_order_relaxed);
        return (current_tail - current_head + buffer_.size()) % buffer_.size();
    }

    T& back() {
        return buffer_[tail_.load(std::memory_order_relaxed)];
    }

    T& front() {
        return buffer_[head_.load(std::memory_order_relaxed)];
    }

private:

    std::size_t next(std::size_t const& n){ return (n+1) %Size;}

    std::atomic_uint32_t head_ = 0;
    std::atomic_uint32_t tail_ = 0;
    std::array<T, Size> buffer_;
    OverwriteRule is_overwritable_;

};


}