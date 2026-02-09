#include "logging_utils.h"
#include "time_utils.h"
#include <iostream>
#include <iomanip>

namespace marketsim::exchange::utils {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::Logger() : current_level_(LogLevel::INFO) {}

void Logger::set_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

LogLevel Logger::get_level() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_level_;
}

void Logger::log(LogLevel level, const char* file, int line, const std::string& message) {
    if (level < current_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto timestamp = TimeUtils::now_millis();
    auto time_str = TimeUtils::to_iso8601(timestamp);
    
    std::cout << "[" << time_str << "] "
              << "[" << level_to_string(level) << "] "
              << "[" << file << ":" << line << "] "
              << message << std::endl;
}

const char* Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

}
