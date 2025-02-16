#pragma once

#include <cstdint>
#include <functional>

#define TASK_FIXED_SIZE 0

namespace francos {

#if !TASK_FIXED_SIZE

using Task = std::function<void()>;

#else 

template<std::size_t BufferSize = 64>
class Task {
public:

    template<typename F>
    Task(F&& f) {
        static_assert(sizeof(F) <= BufferSize, "Task too large for FixedTask");
        new (&storage) F(std::forward<F>(f));
        call_ = [](void* ptr) { (*reinterpret_cast<F*>(ptr))(); };
        destroy_ = [](void* ptr) { reinterpret_cast<F*>(ptr)->~F(); };
        move_ = [](void* dest, void* src) { 
            new (dest) F(std::move(*reinterpret_cast<F*>(src))); 
        };
    }

    Task(Task&& other) noexcept {
        call_ = other.call_;
        destroy_ = other.destroy_;
        move_ = other.move_;

        // Move the stored callable properly
        if (other.call_) {
            move_(&storage, &other.storage);
            other.destroy_(&other.storage); // Destroy the old object
            other.call_ = nullptr;
            other.destroy_ = nullptr;
            other.move_ = nullptr;
        }
    }

    void operator()() {
        std::cout << "Calling the task" << std::endl;
        call_(&storage);
    }

    ~Task() {
        if (destroy_) {
            destroy_(&storage);
        }
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

private:
    typename std::aligned_storage<BufferSize>::type storage;
    void (*call_)(void*);
    void (*destroy_)(void*);
    void (*move_)(void*, void*); // Function to move the stored callable
};


#endif

}


