#include <francos/francos.hpp>
#include <gtest/gtest.h>

using namespace francos;
using namespace std::chrono_literals;

Topic<int> number_topic_ab("a--->b");
Topic<int> number_topic_ba("a<---b");

int a = 0;

class NodeA : public Node{
public:
    NodeA(Thread* thread): Node(thread, "NodeA") {
        publisher = this->create_publisher(&number_topic_ab);
        subscriber = this->create_subscriber<int>(thread, &number_topic_ba, std::bind(&NodeA::on_msg_received, this, std::placeholders::_1));
    }

    void on_msg_received(int const& data){
        if(a++ > 1e6){
            LOG_INFO("Reached");
        }
        publisher->publish(data+1);
    }

private:
    Publisher<int>::SharedPtr publisher;
    Subscriber<int>::SharedPtr subscriber;
};

class NodeB : public Node{
public:
    NodeB(Thread* thread): Node(thread, "NodeB") {
        publisher = this->create_publisher(&number_topic_ba);
        subscriber = this->create_subscriber<int>(thread, &number_topic_ab, std::bind(&NodeB::on_msg_received, this, std::placeholders::_1));
    }

    void on_msg_received(int const& data){
        static const int MAX_MESSAGES = 1000;
        static int count = 0;
        if (++count > MAX_MESSAGES) {
            LOG_WARN("Terminating ping-pong loop after %d messages", MAX_MESSAGES);
            return;
        }
        publisher->publish(data+1);
    }
private:
    Publisher<int>::SharedPtr publisher;
    Subscriber<int>::SharedPtr subscriber;
};



void test_same_thread(){
    Thread t1("1");

    NodeA a(&t1);
    NodeB b(&t1);

    number_topic_ab.write(1);

    spin_for(10s);

}


void test_different_thread(){
    Thread t1("1");
    Thread t2("1");

    NodeA a(&t1);
    NodeB b(&t2);

    number_topic_ab.write(1);

    spin_for(1s);

}

int main(){
    // a = 0;
    // test_different_thread();
    // a = 0;
    test_same_thread();
}