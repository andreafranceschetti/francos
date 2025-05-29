#pragma once

#include <array>
#include <vector>
#include <atomic>
#include <semaphore>
#include <thread>
#include <string>
#include <unordered_set>

#include "subscriber.hpp"
#include "clock.hpp"
#include "thread.hpp"
#include "slot.hpp"

namespace francos {

template<typename Message>
class Topic {
    
public:

    Topic(std::string const& name): name_(name), queue_([](Slot<Message> const& msg){return msg.ref_count <= 0;}) { }


    void write(Message const& msg) {

        std::unordered_set<Thread*> threads_to_notify;
        std::uint32_t ref_count = 0;

        for(Subscriber<Message>*  sub: subscribers_){

            if(std::this_thread::get_id() == sub->thread->id()) {  
                sub->callback(msg);
            } else { 
                ref_count++;
                threads_to_notify.insert(sub->thread);
            }
        }

        if(threads_to_notify.empty())
            return;

        if(!queue_.push(Slot<Message>{ref_count, msg})){
            LOG_DEBUG("Failed to push message in topic %s", name_.c_str());
            return;
        }

        Slot<Message>* slot = &(queue_.front());

        Clock::time_point const now = Clock::now();
        // notify all the other threads to process this
        for(Thread* thread: threads_to_notify){
            thread->schedule([this, slot]() {this->read(slot);}, now);
        }
    }

    const std::string& name() const {return name_;}

    void add_subscriber(Subscriber<Message>* subscriber){
        LOG_DEBUG("[topic %s] Adding subscriber %p", name_.c_str(), subscriber);
        subscribers_.push_back(subscriber);
    }

    void remove_subscriber(Subscriber<Message>* subscriber){
        LOG_DEBUG("[topic %s] Removing subscriber %p", name_.c_str(), subscriber);
        subscribers_.erase(std::remove(subscribers_.begin(), subscribers_.end(), subscriber), subscribers_.end());
    }

    const std::vector<Subscriber<Message>*>& subscribers() const noexcept {
        return subscribers_;
    }

private:

    void read(Slot<Message>* slot){

        std::thread::id this_thread_id = std::this_thread::get_id();
        Slot<Message> curr;

        for(Subscriber<Message>*  sub: subscribers_){
            if(this_thread_id != sub->thread->id()) {  
                continue; // subscriber in another thread
            }

            // LOG_DEBUG("Executing from thread %u, current sub thread: %u, slot: %p", std::this_thread::get_id(), sub->thread->id(), slot);

            if (slot->ref_count.fetch_sub(1, std::memory_order_acq_rel) > 0) {  
                sub->callback(slot->msg);
            }else {
                queue_.pop(curr);
            }
        }

    }


    std::string name_;
    Queue<Slot<Message>, 16> queue_;
    std::vector<Subscriber<Message>*> subscribers_;
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