#include <francos/francos.hpp>

using namespace francos;
using namespace std::chrono_literals;

static constexpr int N = 10000;

Topic<Clock::time_point> number_topic_ab("ab");

class NodeA : public Node
{
public:
    NodeA(Thread *thread, std::string const &name) : Node(thread, name)
    {
        publisher = this->create_publisher(&number_topic_ab);
        timer = this->create_timer(thread, std::bind(&NodeA::send_time, this), 1ms);
        timer->start();
    }

    void send_time()
    {
        if (++calls == N)
        {
            LOG_WARN("Timer stopped");
            timer->stop();
            return;
        }

        publisher->publish(Clock::now());
    }

private:
    size_t calls = 0;
    Publisher<Clock::time_point>::SharedPtr publisher;
    Timer::SharedPtr timer;
};

class NodeB : public Node
{
public:
    NodeB(Thread *thread, std::string const &name) : Node(thread, name)
    {
        subscriber = this->create_subscriber<Clock::time_point>(thread, &number_topic_ab, std::bind(&NodeB::on_msg_received, this, std::placeholders::_1));
        latencies.reserve(N);
    }

    void on_msg_received(Clock::time_point const &time)
    {
        double latency;
        if (time.time_since_epoch().count() == 0)
        {
            LOG_ERROR("Received invalid time value.");
            latency = 0;
        }
        else
        {
            latency = (std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - time).count());
        }
        latencies.push_back(latency);
    }

    ~NodeB()
    {
        double sum = 0;
        for (auto l : latencies)
        {
            sum += l;
        }
        LOG_INFO("Average latency: %.8f microseconds (%d samples)", sum / static_cast<double>(latencies.size()), latencies.size());
    }

private:
    std::vector<double> latencies{};
    Subscriber<Clock::time_point>::SharedPtr subscriber;
};

void test_latency_betwenn_many_threads()
{
    LOG_INFO("[test latency between multiple threads]");

    // sender
    Thread sender_thread("sender_thread", 1);
    NodeA sender(&sender_thread, "sender");

    Thread receiver_thread_1("receiver_thread1", 1);
    NodeB receiver_1(&receiver_thread_1, "receiver1");
    Thread receiver_thread_2("receiver_thread2", 1);
    NodeB receiver_2(&receiver_thread_2, "receiver2");
    Thread receiver_thread_3("receiver_thread3", 1);
    NodeB receiver_3(&receiver_thread_3, "receiver3");
    Thread receiver_thread_4("receiver_thread4", 1);
    NodeB receiver_4(&receiver_thread_4, "receiver4");

    spin_for(11s);
}

int main()
{

    test_latency_betwenn_many_threads();
    return 0;
}