#include "time_utils.h"
#include <chrono>
#include <thread>

namespace marketsim::traffic_generator::utils {

int64_t TimeUtils::current_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int64_t TimeUtils::current_timestamp_us() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

double TimeUtils::elapsed_seconds(int64_t start_ms) {
    int64_t now_ms = current_timestamp_ms();
    return (now_ms - start_ms) / 1000.0;
}

void TimeUtils::sleep_ms(int64_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

} // namespace marketsim::traffic_generator::utils
