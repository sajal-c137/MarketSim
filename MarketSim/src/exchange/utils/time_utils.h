#pragma once

#include <cstdint>
#include <chrono>
#include <string>

namespace marketsim::exchange::utils {

class TimeUtils {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Nanoseconds = std::chrono::nanoseconds;
    using Microseconds = std::chrono::microseconds;
    using Milliseconds = std::chrono::milliseconds;
    
    static int64_t now_millis();
    static int64_t now_micros();
    static int64_t now_nanos();
    
    static TimePoint now();
    
    static int64_t to_millis(const TimePoint& tp);
    static int64_t to_micros(const TimePoint& tp);
    static int64_t to_nanos(const TimePoint& tp);
    
    static TimePoint from_millis(int64_t millis);
    static TimePoint from_micros(int64_t micros);
    static TimePoint from_nanos(int64_t nanos);
    
    static std::string to_iso8601(int64_t millis);
    static std::string to_iso8601(const TimePoint& tp);
    
    static int64_t duration_millis(const TimePoint& start, const TimePoint& end);
    static int64_t duration_micros(const TimePoint& start, const TimePoint& end);
    static int64_t duration_nanos(const TimePoint& start, const TimePoint& end);
};

class ScopedTimer {
public:
    explicit ScopedTimer(const char* name);
    ~ScopedTimer();
    
    int64_t elapsed_micros() const;
    
private:
    const char* name_;
    TimeUtils::TimePoint start_;
};

}
