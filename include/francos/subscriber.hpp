#pragma once

#include "logging.hpp"

namespace francos {

class Thread;

template<typename Message>
class Topic;

template<typename Message>
class SubscriberBase {
public:
    SubscriberBase(Thread * thread) : thread(thread){}
    virtual ~SubscriberBase() = default;
    virtual void execute() = 0;
    virtual void push(Message const& msg) = 0;
    Thread * thread;
};


template<typename Class, typename Message>
class Subscriber : public SubscriberBase<Message>{


public:

    static constexpr uint32_t BUFFER_SIZE = 32;

    using SharedPtr = std::shared_ptr<Subscriber<Class, Message>>;

    struct Callback {
        using Function = void(Class::*)(Message const& msg);
        Function callback;
        Class* instance;
    
        void operator()(Message const& msg){
            (instance->*callback)(msg);
        }
    };

    Subscriber(Thread* thread, Topic<Message>* topic, Callback callback) : SubscriberBase<Message>(thread), topic(topic), callback(callback) {
        // topic->add_subscriber(this);
    }

    void execute() override { 
        callback(buffer.front()); 
        buffer.pop_front();
    }

    void push(Message const& msg) override {
        std::lock_guard<std::mutex> lock(queue_mtx);
        if(buffer.size() >= BUFFER_SIZE) {
            LOG_INFO("Subscriber queue full");
            buffer.pop_front();
        }

        buffer.push_back(msg);
    }

private:
    Topic<Message>* topic;
    Callback callback;
    std::deque<Message> buffer;
    std::mutex queue_mtx;
};


}