#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <functional>



namespace francos {

class RAII_Timer {
public:
    explicit RAII_Timer(
        const std::string& name = "Timer",
        std::function<void(double, const std::string&)> callback = nullptr
    ) : name_(name), callback_(callback) {
        start_ = std::chrono::high_resolution_clock::now();
    }

    ~RAII_Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::micro>(end - start_).count();
        
        if(callback_) {
            callback_(duration, name_);
        } else {
            std::cout << name_ << " took: " << duration << " us\n";
        }
    }

    RAII_Timer(const RAII_Timer&) = delete;
    RAII_Timer& operator=(const RAII_Timer&) = delete;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    std::string name_;
    std::function<void(double, const std::string&)> callback_;
};

}