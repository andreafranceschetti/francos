#include <francos/thread.hpp>
#include <chrono>

using namespace std::chrono_literals;

namespace francos
{

    static std::vector<Thread*> threads;

    Thread::Thread(std::string const &name) : name(name)
    {
        threads.push_back(this);
    }

    Thread::~Thread()
    {
        if (worker.joinable())
            worker.join();
    }

    void Thread::start()
    {
        running_ = true;
        worker = std::thread(&Thread::spin, this);
        // worker.detach();
    }

    void Thread::stop()
    {
        running_ = false;
    }

    void Thread::start_all(void)
    {
        for (Thread *t : threads)
        {
            t->start();
        }
    }

    void Thread::stop_all(void)
    {
        for (Thread *t : threads)
        {
            t->stop();
        }
    }
    // void schedule(Task const& task, Clock::time_point const& t){
    //     std::lock_guard<std::mutex> lock(thread_mtx);
    //     tasks.push({task, t});
    // }

    void Thread::schedule(Task const &task, Clock::time_point const &t)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks.push({task, t});
        cv_.notify_one(); // Wake up the worker thread
    }

    void Thread::spin()
    {
        while (running_)
        {
            if (tasks.empty())
            {
                std::this_thread::sleep_for(100us); // Prevent busy-waiting
                continue;
            }

            auto now = Clock::now();
            auto scheduled = tasks.top();

            // Sleep until the scheduled time
            if (scheduled.time > now)
            {
                std::this_thread::sleep_until(scheduled.time);
            }

            tasks.pop();
            scheduled.task();
        }
    }

    // void spin() {
    //         while (true) {
    //             std::unique_lock<std::mutex> lock(mutex_);
    //             // Wait until there's a task or the thread is stopped
    //             cv_.wait(lock, [this]() { return !tasks.empty() || !running_; });

    //             if (!running_) break;  // Exit if stopped

    //             auto now = Clock::now();
    //             auto scheduled = tasks.top();

    //             if (scheduled.time > now) {
    //                 // Wait until the scheduled time or a new task arrives
    //                 cv_.wait_until(lock, scheduled.time);
    //                 continue;  // Re-check the queue after waking up
    //             }

    //             tasks.pop();
    //             lock.unlock();  // Release lock before executing the task
    //             scheduled.task();  // Execute the task
    //         }
    //     }

}
