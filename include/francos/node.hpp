#pragma once


#include "publisher.hpp"
#include "subscriber.hpp"
#include "timer.hpp"


namespace francos {

class Thread;

class Node {
public:
    explicit Node(Thread const* thread, std::string const& name) : thread(thread), name(name) {};
    virtual ~Node() {
        LOG_DEBUG("Node %s died", name.c_str());
    };

protected:

    template<typename Message>
    typename Publisher<Message>::SharedPtr create_publisher(Topic<Message>* topic){
        return std::make_shared<Publisher<Message>>(topic);
    }

    template<typename Message>
    typename Subscriber<Message>::SharedPtr create_subscriber(Thread* thread, Topic<Message>* topic, typename Subscriber<Message>::Callback callback){
        return std::make_shared<Subscriber<Message>>(thread, topic, callback);
    }

    typename Timer::SharedPtr create_timer(Thread * thread, Timer::TimerTask const& task, std::chrono::milliseconds const& interval){
        return std::make_shared<Timer>(thread, task, interval);
    }

    // Prevent accidental copying
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

private:
    Thread const* thread;
    std::string name;
};



}