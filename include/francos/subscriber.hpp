#pragma once

#include <functional>
#include <memory>
#include <deque>
#include <mutex>

#include "logging.hpp"

namespace francos {

class Thread;

template<typename Message>
class Topic;


template<typename Message>
class Subscriber {

    friend class Topic<Message>;
    static constexpr uint32_t BUFFER_SIZE = 32;

public:

    using Callback = std::function<void(Message const&)>;
    using SharedPtr = std::shared_ptr<Subscriber<Message>>;


    Subscriber(Thread* thread, Topic<Message>* topic, Callback const& callback) : thread(thread), topic(topic), callback(callback) {
        topic->add_subscriber(this);
    }

    ~Subscriber(){
        topic->remove_subscriber(this);
    }

    void execute() { 
        if(buffer.empty()){
            LOG_WARN("Queue empty!");
            return;
        }
        this->callback(buffer.front()); 
        buffer.pop_front();
    }

    void push(Message const& msg) {
        std::lock_guard<std::mutex> lock(queue_mtx);
        if(buffer.size() >= BUFFER_SIZE) {
            LOG_INFO("Subscriber queue full");
            buffer.pop_front();
        }

        buffer.push_back(msg);
    }

private:
    Thread * thread;
    Topic<Message>* topic;
    Callback callback;
    std::deque<Message> buffer;
    std::mutex queue_mtx;
};


}