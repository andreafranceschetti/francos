#include <cstdarg>
#include <cstdio>
#include <unistd.h>
#include <climits>
#include <mutex>
#include <iostream>
#include <thread>

#include <francos/logging.hpp>
#include <francos/queue.hpp>


namespace francos {

static constexpr size_t LOG_QUEUE_SIZE = 1024;

static Queue<std::string, LOG_QUEUE_SIZE> logging_queue;
static std::mutex m;
static LogLevel current_log_level = LogLevel::DEBUG;
    
void set_log_level(LogLevel const& level){
    std::lock_guard<std::mutex> lock(m);
    current_log_level = level;
}


void log(LogLevel const& level, const char* fmt,  ...) {

    if(static_cast<int>(level) < static_cast<int>(current_log_level)){
        return;
    }

    static char buffer[PIPE_BUF-5];  // Buffer for formatted message + newline

    va_list args;
    va_start(args, fmt);
    // Format into buffer (leave space for newline & null terminator)
    int len = vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);

    // Handle errors and add newline
    if (len < 0) return;  // Formatting error
    
    if (len >= sizeof(buffer) - 1) {  // Message truncated
        // Overwrite last character with newline
        buffer[sizeof(buffer) - 2] = '\n';
        len = sizeof(buffer) - 1;     // Write full buffer (minus null terminator)
    } else {                          // Message fits
        buffer[len++] = '\n';         // Append newline
    }

    logging_queue.push(std::string(buffer, len));
}


std::atomic<bool> stop_logging_flag{false};
void flush_logging_queue() {
    static std::string out(PIPE_BUF * LOG_QUEUE_SIZE, '\0');
    size_t out_size = 0;  

    while (!stop_logging_flag || !logging_queue.is_empty()) {
        while (!logging_queue.is_empty()) {
            std::string msg;
            if (!logging_queue.pop(msg)) break;

            if (out_size + msg.size() <= out.size()) {
                std::copy(msg.begin(), msg.end(), out.begin() + out_size);
                out_size += msg.size();
            } else {
                write(STDOUT_FILENO, out.c_str(), out_size);
                out_size = 0;
            }
        }

        if (out_size > 0) {
            write(STDOUT_FILENO, out.c_str(), out_size);
            out_size = 0;  // Reset buffer position
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}



}