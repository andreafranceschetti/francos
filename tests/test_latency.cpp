#include <francos/francos.hpp>
#include <gtest/gtest.h>

using namespace francos;
using namespace std::chrono_literals;

Topic<Clock::time_point> number_topic_ab("ab");

class NodeA : public Node{
public:
    NodeA(Thread* thread): Node(thread) {
        publisher = this->create_publisher(&number_topic_ab);
        timer = this->create_timer<NodeA>(thread, {&NodeA::send_time, this}, 500ms);
        timer->start();
    }

    void send_time(){
        publisher->publish(Clock::now());
    }

private:

    Publisher<Clock::time_point>::SharedPtr publisher;
    Timer<NodeA>::SharedPtr timer;
};

class NodeB : public Node{
public:
    NodeB(Thread* thread): Node(thread) {
        subscriber = this->create_subscriber<NodeB, Clock::time_point>(thread, &number_topic_ab, {&NodeB::on_msg_received, this});
    }

    void on_msg_received(Clock::time_point const& time){
        LOG_INFO("Latency: %d us", std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - time).count());
    }

    Subscriber<NodeB, Clock::time_point>::SharedPtr subscriber;
};

// TEST(FrancosTest, TestPingPong){
//     Thread t("1");

//     NodeA a(&t);
//     NodeB b(&t);

//     t.schedule([&a](){a.on_msg_received(2);} ,Clock::now());

//     Thread::start_all();

//     std::this_thread::sleep_for(5s);
    
// }
int main(){
    Thread t1("1");
    Thread t2("2");

    NodeA a(&t1);
    NodeB b(&t2);

    Thread::start_all();

    std::this_thread::sleep_for(5s);

    Thread::stop_all();
    
}