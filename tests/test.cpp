#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <chrono>
#include <queue>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <cstdarg>
#include <cstdio>

using namespace std::chrono_literals;
using Clock = std::chrono::high_resolution_clock;


std::mutex mtx;

static inline void log(const char* fmt, ...) {
    char buffer[256];  // Fixed-size buffer (can be dynamically allocated for larger logs)
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::lock_guard<std::mutex> lock(mtx);
    std::cout << buffer << std::endl;


}

#define LOG_INFO(...) log(__VA_ARGS__)


class Thread;

template<typename Message>
class SubscriberBase {
public:
    SubscriberBase(Thread * thread) : thread(thread){}
    virtual ~SubscriberBase() = default;
    virtual void execute() = 0;
    virtual void push(Message const& msg) = 0;
    Thread * thread;
};

template<typename Message>
class RingBuffer {
public:

    struct Slot{
        Message msg;
        std::atomic<uint32_t> ref_counter;
    };

    Message* push(Message const& msg, uint32_t subscribers){
        slots[head] = {msg, subscribers};
    }

    bool try_pop(Message &msg){

    }




private:
    std::atomic<uint32_t> head = 0;
    std::atomic<uint32_t> tail = 0;
    std::array<Slot, 16> slots;
};

template<typename Message, size_t BufferSize = 1023>
class Topic {

public:

    Topic(std::string const& name): name_(name){}


    void write(Message const& msg) {
        // Message* m = buffer.push(msg, static_cast<uint32_t>(subscribers.size()));

        for(auto sub: subscribers){
            sub->push(msg);
            sub->thread->schedule([sub](){sub->execute();}, Clock::now()); 
        }
    }

    const std::string& name() const {return name_;}


    void add_subscriber(SubscriberBase<Message>* subscriber){
        subscribers.push_back(subscriber);
    }

private:

    std::string name_;
    std::vector<SubscriberBase<Message>*> subscribers;
    // RingBuffer<Message> buffer;
};


template<typename Message>
class Publisher{
public:
    using SharedPtr = std::shared_ptr<Publisher<Message>>;

    Publisher(Topic<Message> * topic) : topic(topic) {}

    void publish(Message const& msg) {
        topic->write(msg);
    }


    const std::string& topic_name() const {
        return topic->name();
    }

private:

    Topic<Message> * topic;

};

template<typename T>
struct TimedTask {
    using Function = void(T::*)(void);
    Function f;
    T* instance;

    void operator()(){
        (instance->*f)();
    }
};


template<typename Class, typename Message>
class Subscriber : public SubscriberBase<Message>{
public:

    static constexpr uint32_t BUFFER_SIZE = 32;

    using SharedPtr = std::shared_ptr<Subscriber<Class, Message>>;

    struct Callback {
        using Function = void(Class::*)(Message const& msg);
        Function callback;
        Class* instance;
    
        void operator()(Message const& msg){
            (instance->*callback)(msg);
        }
    };

    Subscriber(Thread* thread, Topic<Message>* topic, Callback callback) : SubscriberBase<Message>(thread), topic(topic), callback(callback) {
        topic->add_subscriber(this);
    }

    void execute() override { 
        callback(buffer.front()); 
        buffer.pop_front();
    }

    void push(Message const& msg) override {
        if(buffer.size() >= BUFFER_SIZE) {
            LOG_INFO("Subscriber queue full");
            buffer.pop_front();
        }

        buffer.push_back(msg);
    }

private:
    Topic<Message>* topic;
    Callback callback;
    std::deque<Message> buffer;
};


static std::vector<Thread*> threads;

class Thread {

public:

    Thread(std::string const& name) : name(name) {
        threads.push_back(this);
    }

    using Task = std::function<void()>;

    void schedule(Task const& task, Clock::time_point const& t){
        tasks.push({task, t});
    }

    void start(){
        running_ = true;
        worker = std::thread(&Thread::spin, this);
        worker.detach();
    }

    void stop(){
        running_ = false;
    }

    static void start_all(void){
        for(Thread* t: threads){
            t->start();
        }
    }


private:
    void spin() {
        while (running_) {  
            if (tasks.empty()) {
                std::this_thread::sleep_for(1ms);  // Prevent busy-waiting
                continue;
            }

            auto now = Clock::now();
            auto scheduled = tasks.top();

            // Sleep until the scheduled time
            if (scheduled.time > now) {
                std::this_thread::sleep_until(scheduled.time);
            }

            tasks.pop();
            scheduled.task();
        }
    }

    struct ScheduledTask {
        Task task;
        Clock::time_point time;

        bool operator<(const ScheduledTask& other) const {
            return time > other.time;
        }
    };

    std::string name;
    std::thread worker;
    bool running_ = false;
    std::priority_queue<ScheduledTask> tasks;
};


template<typename T>
class Timer {
public:
    using SharedPtr = std::shared_ptr<Timer>;

    Timer(Thread * thread, TimedTask<T> const& timed_task, std::chrono::milliseconds const& interval) : thread(thread), timed_task(timed_task), interval(interval){}
    
    void start(){
        running_ = true;
        thread->schedule([this] (){this->tick();}, Clock::now());
    }

    void stop(){
        running_ = false;
    }


private:
    void tick(){
        timed_task();
        if(running_){
            auto t = Clock::now() + interval;
            thread->schedule([this] (){this->tick();}, t);
        }
    }


    Thread * thread;
    bool running_ = false;
    TimedTask<T> timed_task;
    std::chrono::milliseconds interval;
    
};

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
        return std::make_shared<Subscriber<Class, Message>>(thread, topic, callback);
    }

    template<typename Class>
    typename Timer<Class>::SharedPtr create_timer(Thread * thread, TimedTask<Class> const& task, std::chrono::milliseconds const& interval){
        return std::make_shared<Timer<Class>>(thread, task, interval);
    }

private:
    Thread const* thread;
};

// APPLICATION CODE --------------------------------------------------------

Topic<std::string> hello_world{"/hello_world"};

class NodeA : public Node {
public:


    NodeA(Thread * thread) : Node(thread) {

        publisher = this->create_publisher<std::string>(&hello_world);
        timer = this->create_timer<NodeA>(thread, {&NodeA::run, this}, 500ms);
        timer->start();
    }

    void run(){
        publisher->publish("Hello World" + std::to_string(i++));
        LOG_INFO("[Node A] I say Hello World on topic", publisher->topic_name());
    }


private:
    int i = 0;

    Publisher<std::string>::SharedPtr publisher;
    Timer<NodeA>::SharedPtr timer;

};

class NodeB : public Node {

public:

    NodeB(Thread * thread) : Node(thread) {
        subscriber = this->create_subscriber<NodeB, std::string>(thread, &hello_world, {&NodeB::on_msg_receive, this});
    }
 

    void on_msg_receive(std::string const& msg){
        LOG_INFO("[Node A] I heard Hello World on topic", hello_world.name(), msg);
    }

private:

    Subscriber<NodeB, std::string>::SharedPtr subscriber;
};


void spin(){

}


int main(){


    Thread t1("cpu1");
    Thread t2("cpu2");
    Thread t3("cpu3");

    NodeA node_a(&t1);
    NodeB node_b(&t2);
    NodeB node_c(&t3);

    Thread::start_all();

    std::this_thread::sleep_for(10s);

}
