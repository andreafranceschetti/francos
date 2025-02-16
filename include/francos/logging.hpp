#pragma once

namespace francos {

void log(const char* fmt, ...);
#define LOG_INFO(...) log(__VA_ARGS__)

}