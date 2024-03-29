#include "utils.h"

#include "common.h"

std::string ComputeTime(){
    auto now = std::chrono::high_resolution_clock::now();
    auto tt_now = std::chrono::high_resolution_clock::to_time_t(now);
    auto lt_now = std::localtime(&tt_now);
    std::string now_time_str;
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt_now);
    now_time_str += buf;
    auto mt_now = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    std::snprintf(buf, sizeof(buf), ".%06ld", mt_now.count() % 1000000);
    now_time_str += buf;
    return now_time_str;
}