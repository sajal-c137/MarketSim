#include "time_utils.h"
#include <sstream>
#include <iomanip>
#include <iostream>

namespace marketsim::exchange::utils {

int64_t TimeUtils::now_millis() {
    auto now = Clock::now();
    return std::chrono::duration_cast<Milliseconds>(now.time_since_epoch()).count();
}

int64_t TimeUtils::now_micros() {
    auto now = Clock::now();
    return std::chrono::duration_cast<Microseconds>(now.time_since_epoch()).count();
}

int64_t TimeUtils::now_nanos() {
    auto now = Clock::now();
    return std::chrono::duration_cast<Nanoseconds>(now.time_since_epoch()).count();
}

TimeUtils::TimePoint TimeUtils::now() {
    return Clock::now();
}

int64_t TimeUtils::to_millis(const TimePoint& tp) {
    return std::chrono::duration_cast<Milliseconds>(tp.time_since_epoch()).count();
}

int64_t TimeUtils::to_micros(const TimePoint& tp) {
    return std::chrono::duration_cast<Microseconds>(tp.time_since_epoch()).count();
}

int64_t TimeUtils::to_nanos(const TimePoint& tp) {
    return std::chrono::duration_cast<Nanoseconds>(tp.time_since_epoch()).count();
}

TimeUtils::TimePoint TimeUtils::from_millis(int64_t millis) {
    return TimePoint(Milliseconds(millis));
}

TimeUtils::TimePoint TimeUtils::from_micros(int64_t micros) {
    return TimePoint(Microseconds(micros));
}

TimeUtils::TimePoint TimeUtils::from_nanos(int64_t nanos) {
    return TimePoint(Nanoseconds(nanos));
}

std::string TimeUtils::to_iso8601(int64_t millis) {
    auto tp = from_millis(millis);
    return to_iso8601(tp);
}

std::string TimeUtils::to_iso8601(const TimePoint& tp) {
    auto time_t_val = Clock::to_time_t(tp);
    auto millis = to_millis(tp) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << millis << 'Z';
    return oss.str();
}

int64_t TimeUtils::duration_millis(const TimePoint& start, const TimePoint& end) {
    return std::chrono::duration_cast<Milliseconds>(end - start).count();
}

int64_t TimeUtils::duration_micros(const TimePoint& start, const TimePoint& end) {
    return std::chrono::duration_cast<Microseconds>(end - start).count();
}

int64_t TimeUtils::duration_nanos(const TimePoint& start, const TimePoint& end) {
    return std::chrono::duration_cast<Nanoseconds>(end - start).count();
}

ScopedTimer::ScopedTimer(const char* name)
    : name_(name)
    , start_(TimeUtils::now())
{}

ScopedTimer::~ScopedTimer() {
    auto elapsed = elapsed_micros();
    std::cout << "[TIMER] " << name_ << ": " << elapsed << " ?s" << std::endl;
}

int64_t ScopedTimer::elapsed_micros() const {
    return TimeUtils::duration_micros(start_, TimeUtils::now());
}

}
