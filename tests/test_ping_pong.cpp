#include <francos/francos.hpp>
#include <gtest/gtest.h>

using namespace francos;
using namespace std::chrono_literals;

Topic<int> number_topic_ab("ab");
Topic<int> number_topic_ba("ba");


class NodeA : public Node{
public:
    NodeA(Thread* thread): Node(thread, "NodeA") {
        publisher = this->create_publisher(&number_topic_ab);
        subscriber = this->create_subscriber<int>(thread, &number_topic_ba, std::bind(&NodeA::on_msg_received, this, std::placeholders::_1));
    }

    void on_msg_received(int const& data){
        // LOG_INFO("A received %d", data);
        // printf("A received %d", data);
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
        // printf("[DEBUG] data: %d\n", data);

        // LOG_INFO("B received %d", data);
        publisher->publish(data +1);
    }

    Publisher<int>::SharedPtr publisher;
    Subscriber<int>::SharedPtr subscriber;
};

TEST(FrancosTest, TestPingPong){
    Thread t("1");

    NodeA a(&t);
    NodeB b(&t);

    number_topic_ab.write(1);

    spin_for(1s);
}

#if 0
int main(){
    Thread t1("1");
    Thread t2("1");

    NodeA a(&t1);
    NodeB b(&t2);

    t1.schedule([&a](){a.on_msg_received(2);} ,Clock::now());

    Thread::start_all();

    std::this_thread::sleep_for(5s);

    Thread::stop_all();
    
}
#endif