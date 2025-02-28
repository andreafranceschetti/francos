#include <francos/task.hpp>

namespace francos
{


    // Task::Task(Task &&other) noexcept
    // {
    //     call_ = other.call_;
    //     destroy_ = other.destroy_;
    //     move_ = other.move_;

    //     // Move the stored callable properly
    //     if (other.call_)
    //     {
    //         move_(&storage, &other.storage);
    //         other.destroy_(&other.storage); // Destroy the old object
    //         other.call_ = nullptr;
    //         other.destroy_ = nullptr;
    //         other.move_ = nullptr;
    //     }
    // }

    // void Task::operator()()
    // {
    //     call_(&storage);
    // }

    // Task::~Task()
    // {
    //     if (destroy_)
    //     {
    //         destroy_(&storage);
    //     }
    // }

}