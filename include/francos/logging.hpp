#pragma once

namespace francos
{

    void log(const char *fmt, ...);

#define LOG_INFO(fmt, ...) log("[INFO] " fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log("[ERROR] " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) log("[WARN] " fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log("[DEBUG] " fmt, ##__VA_ARGS__)

}