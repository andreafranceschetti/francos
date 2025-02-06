

#include <string>
#include <iostream>
#include <functional>

#include <francos/francos.hpp>

francos::Topic<std::string> hello_world{"hello_world"};

class HelloWorldSender : public francos::Node
{

public:
    HelloWorldSender(const francos::Thread* thread) : francos::Node(thread)
    {
        hello_world_publisher_ = francos::create_publisher<std::string>(&hello_world);
        hello_world_timer_ = francos::create_timer(std::chrono::milliseconds(500), std::bind(&HelloWorldSender::run, this));
    }

    void run(void)
    {
        std::string msg = "Hello World";
        std::cout << "I sent " << msg;
        hello_world_publisher_->publish(msg);
    }

private:
    francos::Publisher<std::string>::SharedPtr hello_world_publisher_;
    francos::Timer::SharedPtr hello_world_timer_;
};

class HelloWorldReceiver : public francos::Node
{

public:
    HelloWorldReceiver(const francos::Thread* thread) : francos::Node(thread)
    {
        hello_world_subscriber_ = francos::create_subscriber<std::string>(&hello_world, std::bind(&HelloWorldReceiver::on_hello_world_receive, this))
    }

    void on_hello_world_receive(const std::string &msg)
    {
        std::cout << "I heard " << msg;
    }

private:
    francos::Subscriber<std::string>::SharedPtr hello_world_subscriber_;
};

int main()
{
    francos::Thread thread_1{"thread_1"};

    HelloWorldSender hello_world_sender(&thread_1);
    HelloWorldReceiver hello_world_receiver(&thread_1);

    francos::spin();
}