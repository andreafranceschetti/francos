#include <francos/timer.hpp>
#include <francos/thread.hpp>

namespace francos
{

    Timer::Timer(Thread *thread, TimerTask const &timed_task, std::chrono::milliseconds const &interval) : thread(thread), timed_task(timed_task), interval(interval) {}

    void Timer::start()
    {
        running_ = true;
        thread->schedule([this]()
                         { this->tick(); }, Clock::now());
    }

    void Timer::stop()
    {
        running_ = false;
    }

    void Timer::tick()
    {
        if (running_)
        {
            auto t = Clock::now() + interval;
            thread->schedule([this]()
                             { this->tick(); }, t);
        }
        timed_task();
    }
}