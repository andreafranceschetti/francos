#include <francos/francos.hpp>
#include <gtest/gtest.h>

using namespace francos;
using namespace std::chrono_literals;

constexpr int MAX_LATENCIES = 100000;

Topic<Clock::time_point> number_topic_ab("ab");

class NodeA : public Node{
public:
    NodeA(Thread* thread, std::string const& name): Node(thread, name) {
        publisher = this->create_publisher(&number_topic_ab);
        timer = this->create_timer(thread, std::bind(&NodeA::send_time, this), 10ms);
        timer->start();
    }

    void send_time(){
        publisher->publish(Clock::now());
        if(calls++ == MAX_LATENCIES){
            LOG_WARN("Timer stopped");
            timer->stop();
        }
    }

private:
    size_t calls = 0;
    Publisher<Clock::time_point>::SharedPtr publisher;
    Timer::SharedPtr timer;
};

class NodeB : public Node{
public:
    NodeB(Thread* thread, std::string const& name): Node(thread, name) {
        subscriber = this->create_subscriber<Clock::time_point>(thread, &number_topic_ab, std::bind(&NodeB::on_msg_received, this, std::placeholders::_1));
        latencies.reserve(100000);
    }

    void on_msg_received(Clock::time_point const& time){
        double latency;
        if (time.time_since_epoch().count() == 0) {
            LOG_ERROR("Received invalid time value.");
            latency = 0;
        }else{
            latency = (std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - time).count());
        }
        // LOG_INFO("%.8f", latency);
        latencies.push_back(latency);
    }

    ~NodeB(){
        LOG_INFO("SIZE: %d", latencies.size());
        double sum = 0;
        for(auto l : latencies){
            sum += l;
        }
        LOG_INFO("Average latency: %.8f microseconds", sum/static_cast<double>(MAX_LATENCIES));
    }

private:
    std::vector<double> latencies{};
    Subscriber<Clock::time_point>::SharedPtr subscriber;
};


void test_latency_betwenn_threads(){
    LOG_INFO("[test latency between 2 threads]");
    Thread t1("thread_1");
    Thread t2("thread_2");

    NodeA a(&t1, "node_a");
    NodeB b(&t2, "node_b");

    spin_for(1s);
}

void test_latency_same_thread(){
    LOG_INFO("[test latency same thread]");
    Thread t1("single_thread");

    NodeA a(&t1, "node_a");
    NodeB b(&t1, "node_b");

    spin_for(1s);
}

int main(){

    test_latency_betwenn_threads();
    // std::this_thread::sleep_for(1s);
    test_latency_same_thread();

    return 0;
    
}