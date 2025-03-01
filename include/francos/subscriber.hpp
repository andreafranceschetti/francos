#pragma once

#include <functional>
#include <memory>
#include <mutex>

// #include <deque>
#include "queue.hpp"

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
        LOG_DEBUG("Subscriber created at %p", this);
    }

    ~Subscriber(){
        LOG_DEBUG("Subscriber destroyed at %p", this);
        topic->remove_subscriber(this);
    }

    void execute() {
        Message msg;
        if(!buffer.pop(msg)){
            LOG_WARN("Queue empty!");
            return;
        }
        this->callback(msg); 
    }

    void push(Message const& msg) {

        if(!buffer.push(msg)){
            LOG_WARN("Subscriber queue full");
            return;
        }
    }

private:
    Thread * thread;
    Topic<Message>* topic;
    Callback callback;
    Queue<Message> buffer;
    std::mutex queue_mtx;
};


}