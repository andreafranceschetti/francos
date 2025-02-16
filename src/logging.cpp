#include <cstdarg>
#include <cstdio>
#include <unistd.h>

namespace francos {


void log(const char* fmt, ...) {
    char buffer[256];  // Buffer for formatted message + newline

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

    // Atomic single write (POSIX guarantees atomicity for <= PIPE_BUF bytes)
    write(STDOUT_FILENO, buffer, len);
}



}