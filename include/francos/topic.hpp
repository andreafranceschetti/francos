#pragma once

#include <array>
#include <vector>
#include <atomic>
#include <semaphore>
#include <thread>

#include "subscriber.hpp"
#include "clock.hpp"

namespace francos {


template<typename Message>
class Topic {

public:

    Topic(std::string const& name): name_(name){}


    void write(Message const& msg) {

        for(Subscriber<Message>*  sub: subscribers){
            if(std::this_thread::get_id() == sub->thread->id()) {  // fast as fuck boi
                sub->callback(msg);
            } else { // safe general method
                sub->push(msg);
                sub->thread->schedule([sub](){sub->execute();}, Clock::now()); 
            }
        }
    }

    const std::string& name() const {return name_;}


    void add_subscriber(Subscriber<Message>* subscriber){
        LOG_DEBUG("[topic %s] Adding subscriber %p", name_.c_str(), subscriber);
        subscribers.push_back(subscriber);
    }

    void remove_subscriber(Subscriber<Message>* subscriber){
        LOG_DEBUG("[topic %s] Removing subscriber %p", name_.c_str(), subscriber);
        subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriber), subscribers.end());
    }

private:

    std::string name_;
    std::vector<Subscriber<Message>*> subscribers;
};

// template<typename T, size_t BufferSize = 1024>
// class Topic {
//     struct MessageSlot {
//         std::atomic<bool> ready{false};
//         T data;
//     };

//     std::array<MessageSlot, BufferSize> buffer_;
//     std::atomic<size_t> write_idx_{0};
//     std::counting_semaphore<BufferSize> sem_{0};

// public:
//     bool publish(const T& msg) {
//         size_t idx = write_idx_.fetch_add(1, std::memory_order_relaxed);
//         size_t slot_idx = idx % BufferSize;
        
//         buffer_[slot_idx].data = msg;
//         buffer_[slot_idx].ready.store(true, std::memory_order_release);
//         sem_.release(); // Notify subscribers
//         return true;
//     }

//     bool try_consume(T& msg, size_t& read_idx) {
//         size_t slot_idx = read_idx % BufferSize;
//         if (buffer_[slot_idx].ready.load(std::memory_order_acquire)) {
//             msg = buffer_[slot_idx].data;
//             buffer_[slot_idx].ready.store(false, std::memory_order_release);
//             read_idx++;
//             return true;
//         }
//         return false;
//     }

//     void wait_consume(T& msg, size_t& read_idx) {
//         sem_.acquire(); // Wait for signal
//         while (!try_consume(msg, read_idx)) {
//             std::this_thread::yield();
//         }
//     }
// };
 
 
 }