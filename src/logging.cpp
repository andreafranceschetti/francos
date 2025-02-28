#include <cstdarg>
#include <cstdio>
#include <unistd.h>
#include <climits>
#include <mutex>
#include <iostream>

#include <francos/logging.hpp>

namespace francos {

static std::mutex m;
static LogLevel current_log_level = LogLevel::INFO;
    
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

    // // Atomic single write (POSIX guarantees atomicity for <= PIPE_BUF bytes)
    write(STDOUT_FILENO, buffer, len);
    // fflush(stdout);
}



}