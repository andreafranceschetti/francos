#include <francos/francos.hpp>

using namespace std::chrono_literals;

namespace francos {

void spin(){

    Thread::start_all();

    while(true){
        std::this_thread::sleep_for(10s);
    }

}

}