#pragma once

#include <functional>
#include <chrono>


namespace francos {

// template<typename T>
// struct TimedTask {
//     using Function = void(T::*)(void);
//     Function f;
//     T* instance;

//     void operator()(){
//         (instance->*f)();
//     }
// };

using TimedTask = std::function<void(void)>;

// template<typename T>
class Timer {
public:
    using SharedPtr = std::shared_ptr<Timer>;

    Timer(Thread * thread, TimedTask const& timed_task, std::chrono::milliseconds const& interval) : thread(thread), timed_task(timed_task), interval(interval){}
    
    void start(){
        running_ = true;
        thread->schedule([this] (){this->tick();}, Clock::now());
    }

    void stop(){
        running_ = false;
    }


private:
    void tick(){
        timed_task();
        if(running_){
            auto t = Clock::now() + interval;
            thread->schedule([this] (){this->tick();}, t);
        }
    }


    Thread * thread;
    bool running_ = false;
    TimedTask timed_task;
    std::chrono::milliseconds interval;
    
};



}