#include <francos/thread.hpp>
#include <francos/logging.hpp>
#include <chrono>

using namespace std::chrono_literals;

namespace francos
{

    static std::vector<Thread *> threads;

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
        pthread_setname_np(worker.native_handle(), name.c_str());
        sched_param param{.sched_priority = 99}; // Requires root
        pthread_setschedparam(worker.native_handle(), SCHED_FIFO, &param);

        // worker.detach();
    }

    void Thread::stop()
    {
        running_ = false;
        cv_.notify_one();
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

        threads.clear();
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

    std::thread::id Thread::id(){
        return worker.get_id();
    }



    void Thread::spin()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Wait until there's a task or the thread is stopped
            cv_.wait(lock, [this]() { return !tasks.empty() || !running_; });

            if (!running_){
                LOG_DEBUG("Thread %u died\n", std::this_thread::get_id());
                break; // Exit if stopped
            }

            if(tasks.empty()) continue;

            auto now = Clock::now();
            auto scheduled = tasks.top();

            if (scheduled.time > now)
            {
                continue; // Re-check the queue after waking up
            }

            tasks.pop();
            lock.unlock();    // Release lock before executing the task
            scheduled.task(); // Execute the task
        }
    }
    

    // void Thread::spin()
    // {
    //     while (running_)
    //     {
    //         if (tasks.empty())
    //         {
    //             std::this_thread::yield(); // Prevent busy-waiting
    //             continue;
    //         }

    //         auto now = Clock::now();
    //         auto scheduled = tasks.top();

    //         // Sleep until the scheduled time
    //         if (scheduled.time > now)
    //         {
    //             std::this_thread::sleep_until(scheduled.time);
    //         }

    //         tasks.pop();
    //         scheduled.task();
    //     }
    // }


// void Thread::spin() {
//     while (running_) {
//         auto now = Clock::now();
//         {
//             std::lock_guard lock(mutex_); // Minimize lock scope
//             if (!tasks.empty() && tasks.top().time <= now) {
//                 auto task = tasks.top().task;
//                 tasks.pop();
//                 // lock.unlock();
//                 task(); // Execute inline (no thread pool)
//                 continue;
//             }
//         }
//         // Busy-wait with yield for 1µs precision
//         std::this_thread::yield();
//     }
// }
// #endif

}
