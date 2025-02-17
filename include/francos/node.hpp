#pragma once


#include "publisher.hpp"
#include "subscriber.hpp"
#include "timer.hpp"


namespace francos {

class Thread;

class Node {
public:
    Node(Thread const* thread) : thread(thread) {};
    virtual ~Node() = default;

protected:

    template<typename Message>
    typename Publisher<Message>::SharedPtr create_publisher(Topic<Message>* topic){
        return std::make_shared<Publisher<Message>>(topic);
    }

    template<typename Message>
    typename Subscriber<Message>::SharedPtr create_subscriber(Thread* thread, Topic<Message>* topic, typename Subscriber<Message>::Callback callback){
        auto sub = std::make_shared<Subscriber<Message>>(thread, topic, callback);
        topic->add_subscriber(sub);
        return sub;
    }

    typename Timer::SharedPtr create_timer(Thread * thread, Timer::TimerTask const& task, std::chrono::milliseconds const& interval){
        return std::make_shared<Timer>(thread, task, interval);
    }

private:
    Thread const* thread;
};



}