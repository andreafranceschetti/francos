#pragma once


#include <francos/node.hpp>
#include <francos/topic.hpp>
#include <francos/publisher.hpp>
#include <francos/subscriber.hpp>
#include <francos/timer.hpp>
#include <francos/logging.hpp>
#include <francos/thread.hpp>

namespace francos {

// spins the os forever until ctrl+c
void spin(void);

// spins the os for the given time
void spin_for(std::chrono::seconds const& duration);

}

