#include <francos/francos.hpp>
#include <csignal>

using namespace std::chrono_literals;

namespace francos {

static std::atomic<bool> running{true};

static void signal_handler(int) {
    running.store(false);
}

void spin(){
    std::signal(SIGINT, signal_handler);

    Thread::start_all();

    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    Thread::stop_all();
}

void spin_for(std::chrono::seconds const& duration){
    std::signal(SIGINT, signal_handler);

    Thread::start_all();

    auto end_time = std::chrono::steady_clock::now() + duration;
    while (running.load() && std::chrono::steady_clock::now() < end_time ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Thread::stop_all();
}

}