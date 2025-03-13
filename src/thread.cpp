#include <francos/thread.hpp>
#include <francos/logging.hpp>
#include <chrono>

#include <linux/futex.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <atomic>
#include <iostream>




using namespace std::chrono_literals;

#define USE_FUTEX

namespace francos
{

    static std::vector<Thread *> threads;
    
    // logging stuff
    std::thread logging_thread;
    extern void flush_logging_queue(void);
    extern std::atomic<bool> stop_logging_flag;
    //    

    static int pin_thread_to_core(std::thread &t, int core_id) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);             
        CPU_SET(core_id, &cpuset);     

        return pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
    }
    
    void start_logging(void){
        logging_thread = std::thread(flush_logging_queue);
        pthread_setname_np(logging_thread.native_handle(), "logging");
        sched_param param{.sched_priority = 50}; // low priority
        pthread_setschedparam(logging_thread.native_handle(), SCHED_RR, &param);
        int result = pin_thread_to_core(logging_thread, 0);
    }

    void stop_logging(void){
        stop_logging_flag.store(true);
        if(logging_thread.joinable()){
            logging_thread.join();
        }
        std::cerr << "Logging thread stopped" << std::endl;
    }

    Thread::Thread(std::string const &name, int core) : name(name), core_id_(core)
    {

        threads.push_back(this);
    }

    Thread::~Thread()
    {
    }

    void Thread::start()
    {
        running_ = true;
        worker = std::thread(&Thread::spin, this);
        pthread_setname_np(worker.native_handle(), name.c_str());
        sched_param param{.sched_priority = 99}; // Requires root
        pthread_setschedparam(worker.native_handle(), SCHED_RR, &param);
        int result = pin_thread_to_core(worker, core_id_);
        if (result != 0) {
            LOG_ERROR("Failed to pin thread %s to core %d", name.c_str(), core_id_ );
        } else {
            LOG_INFO("Thread %s pinned to core %d", name.c_str(), core_id_);
        }
    }

    void Thread::stop()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            running_ = false;
            futex_wake();
        }

        if (worker.joinable())
            worker.join();
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
        LOG_DEBUG("Stopping all threads...");
        for (Thread *t : threads)
        {
            t->stop();
        }

        threads.clear();
    }


    void Thread::schedule(Task const &task, Clock::time_point const &t)
    {


        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks.push({task, t});
        }
        futex_flag.store(1, std::memory_order_release); // Set flag before wake-up
        futex_wake();

    }

    std::thread::id Thread::id(){
        return worker.get_id();
    }

#if defined(USE_FUTEX)

void Thread::futex_wake() {
    syscall(SYS_futex, &futex_flag, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

void Thread::futex_wait() {
    int expected = 0;
    syscall(SYS_futex, &futex_flag, FUTEX_WAIT_PRIVATE, expected, nullptr, nullptr, 0);
}

void Thread::spin(){

    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);

        while (tasks.empty() && running_) {
            futex_flag.store(0, std::memory_order_relaxed); // Reset before sleep
            lock.unlock();
            futex_wait();  // Sleep until woken up
            lock.lock();
        }

        if (!running_ ) {
            break; // Exit if stopped and no tasks are left
        }

        auto now = Clock::now();
        auto scheduled = tasks.top();

        if (scheduled.time > now) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;
        }

        tasks.pop();
        lock.unlock(); // Release lock before executing the task
        scheduled.task();
    }
}

#else 

    void Thread::spin()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Wait until there's a task or the thread is stopped
            cv_.wait(lock, [this]() { return !tasks.empty() || !running_; });

            if (!running_){
                break; // Exit if stopped
            }

            auto now = Clock::now();
            auto scheduled = tasks.top();

            if (scheduled.time > now)
            {
                // Wait until the next task is ready or a new task arrives
                cv_.wait_until(lock, scheduled.time, [this, now] {
                    return !tasks.empty() && tasks.top().time <= Clock::now() || !running_;
                });
            }

            tasks.pop();
            lock.unlock();    // Release lock before executing the task
            scheduled.task(); // Execute the task
        }
        LOG_DEBUG("Thread %u died\n", std::this_thread::get_id());
    }
#endif

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
