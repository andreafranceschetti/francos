#pragma once

#include "thread.hpp"
#include "publisher.hpp"
#include "subscriber.hpp"
#include "timer.hpp"


namespace francos {


class Node {
public:
    Node(Thread const* thread) : thread(thread) {};
    virtual ~Node() = default;

protected:

    template<typename Message>
    typename Publisher<Message>::SharedPtr create_publisher(Topic<Message>* topic){
        return std::make_shared<Publisher<Message>>(topic);
    }

    template<typename Class, typename Message>
    typename Subscriber<Class, Message>::SharedPtr create_subscriber(Thread* thread, Topic<Message>* topic, typename Subscriber<Class, Message>::Callback callback){
        auto sub = std::make_shared<Subscriber<Class, Message>>(thread, topic, callback);
        topic->add_subscriber(sub);
        return sub;
    }

    template<typename Class>
    typename Timer<Class>::SharedPtr create_timer(Thread * thread, TimedTask<Class> const& task, std::chrono::milliseconds const& interval){
        return std::make_shared<Timer<Class>>(thread, task, interval);
    }

private:
    Thread const* thread;
};



}