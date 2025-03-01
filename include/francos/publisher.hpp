#pragma once

#include "topic.hpp"

namespace francos {

template<typename Message>
class Publisher{
public:
    using SharedPtr = std::shared_ptr<Publisher<Message>>;

    Publisher(Topic<Message> * topic) : topic(topic) {
        LOG_DEBUG("Publisher created at %p", this);
    }

    ~Publisher(){
        LOG_DEBUG("Publisher died");
    }

    void publish(Message const& msg) {
        topic->write(msg);
    }


    const std::string& topic_name() const {
        return topic->name();
    }

private:

    Topic<Message> * topic;

};

}