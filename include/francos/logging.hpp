#pragma once

namespace francos
{
    enum class LogLevel{
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    void log(LogLevel const& level, const char *fmt, ...);
    void set_log_level(LogLevel const& level);

#define LOG_INFO(fmt, ...) log(LogLevel::INFO, "[INFO] " fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log(LogLevel::ERROR, "[ERROR] " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) log(LogLevel::WARN, "[WARN] " fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log(LogLevel::DEBUG, "[DEBUG] " fmt, ##__VA_ARGS__)

}