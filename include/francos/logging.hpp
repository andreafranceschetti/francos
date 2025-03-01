#pragma once

namespace francos
{
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    void log(LogLevel const &level, const char *fmt, ...);
    void set_log_level(LogLevel const &level);


#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"


#if !defined(DISABLE_LOGGING)

#define LOG_INFO(fmt, ...) log(LogLevel::INFO, GREEN "[INFO] " fmt RESET, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log(LogLevel::ERROR, RED "[ERROR] " fmt RESET, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) log(LogLevel::WARN, YELLOW, "[WARN] " fmt RESET, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log(LogLevel::DEBUG, "[DEBUG] " fmt RESET, ##__VA_ARGS__)

#else

#define LOG_INFO(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_DEBUG(fmt, ...)

#endif

}