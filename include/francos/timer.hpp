#pragma once

#include <functional>
#include <chrono>
#include <memory>


namespace francos {

class Thread;


// template<typename T>
class Timer {
public:
    using TimerTask = std::function<void(void)>;
    using SharedPtr = std::shared_ptr<Timer>;

    Timer(Thread * thread, TimerTask const& timed_task, std::chrono::milliseconds const& interval);
    
    void start();
    void stop();

private:
    void tick();

    Thread * thread;
    bool running_ = false;
    TimerTask timed_task;
    std::chrono::milliseconds interval;
    
};



}