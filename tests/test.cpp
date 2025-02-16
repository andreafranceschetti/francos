#include <francos/francos.hpp>

using namespace francos;
using namespace std::chrono_literals;

Topic<std::string> hello_world{"/hello_world"};

class NodeA : public Node {
public:

    NodeA(Thread * thread) : Node(thread) {

        publisher = this->create_publisher<std::string>(&hello_world);
        timer = this->create_timer<NodeA>(thread, {&NodeA::run, this}, 1ms);
        timer->start();
    }

    void run(){
        publisher->publish("Hello World" + std::to_string(i++));
        LOG_INFO("[Node A] I say Hello World on thread %u", std::this_thread::get_id());
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
        LOG_INFO("[Node B] I heard %s on thead %u", msg.c_str(), std::this_thread::get_id());
    }

private:

    Subscriber<NodeB, std::string>::SharedPtr subscriber;
};


// int main(){


//     Thread t1("cpu1");
//     Thread t2("cpu2");
//     Thread t3("cpu3");

//     NodeA node_a(&t1);
//     NodeB node_b(&t2);
//     NodeB node_c(&t3);

//     francos::spin();

//     return 0;
// }