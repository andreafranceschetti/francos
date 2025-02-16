#pragma once

#include <cstdint>
#include <functional>

#define TASK_FIXED_SIZE 1
namespace francos {

#if !TASK_FIXED_SIZE

using Task = std::function<void()>;

#else 

constexpr std::size_t BufferSize = 64;
class Task {
public:
    ~Task();
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

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

    Task(Task&& other) noexcept;

    void operator()();

private:
    typename std::aligned_storage<BufferSize>::type storage;
    void (*call_)(void*);
    void (*destroy_)(void*);
    void (*move_)(void*, void*); // Function to move the stored callable
};

#endif

}


