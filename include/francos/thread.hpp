#pragma once

#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <chrono>

#include "task.hpp"

using Clock = std::chrono::steady_clock;

namespace francos {

class Thread {

public:
    using Task = std::function<void()>;

    Thread(std::string const& name);
    virtual ~Thread();

    void start();

    void stop();

    static void start_all(void);

    static void stop_all(void);

    void schedule(Task const& task, Clock::time_point const& t);

private:
    void spin() ;

    struct ScheduledTask {
        Task task;
        Clock::time_point time;

        bool operator<(const ScheduledTask& other) const {
            return time > other.time;
        }
    };

    std::string name;
    std::thread worker;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_ = false;
    std::priority_queue<ScheduledTask> tasks;
};

// class Thread {
// public:


//     using Task = std::function<void()>;


//     Thread(const std::string& name) : name_(name), running_(false) {
//         std::lock_guard<std::mutex> lock(global_mutex_);
//         all_threads_.push_back(this);
//     }

//     ~Thread() {
//         std::lock_guard<std::mutex> lock(global_mutex_);
//         all_threads_.erase(std::remove(all_threads_.begin(), all_threads_.end(), this), all_threads_.end());
//     }

//     void start() {
//         if (running_) return;
//         running_ = true;
//         worker_ = std::thread(&Thread::run, this);
//     }

//     void stop() {
//         {
//             std::lock_guard<std::mutex> lock(mutex_);
//             running_ = false;
//         }
//         cv_.notify_all();
//         if (worker_.joinable()) worker_.join();
//     }

//     void schedule(Task task) {
//         {
//             std::lock_guard<std::mutex> lock(mutex_);
//             tasks_.push(task);
//         }
//         cv_.notify_one();
//     }

//     static void stop_all() {
//         std::lock_guard<std::mutex> lock(global_mutex_);
//         for (auto* thread : all_threads_) {
//             thread->stop();
//         }
//     }

// private:
//     void run() {
//         while (true) {
//             Task task;
//             {
//                 std::unique_lock<std::mutex> lock(mutex_);
//                 cv_.wait(lock, [&]{ return !tasks_.empty() || !running_; });
//                 if (!running_ && tasks_.empty()) break;
//                 task = tasks_.front();
//                 tasks_.pop();
//             }
//             if (task) task();
//         }
//     }

//     std::string name_;
//     std::atomic<bool> running_;
//     std::thread worker_;
//     std::queue<Task> tasks_;
//     std::mutex mutex_;
//     std::condition_variable cv_;

//     static std::vector<Thread*> all_threads_;
//     static std::mutex global_mutex_;
// };
}