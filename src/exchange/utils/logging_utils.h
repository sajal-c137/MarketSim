#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <mutex>

namespace marketsim::exchange::utils {

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

class Logger {
public:
    static Logger& instance();
    
    void set_level(LogLevel level);
    LogLevel get_level() const;
    
    void log(LogLevel level, const char* file, int line, const std::string& message);
    
    template<typename... Args>
    void trace(const char* file, int line, Args&&... args) {
        log_formatted(LogLevel::TRACE, file, line, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(const char* file, int line, Args&&... args) {
        log_formatted(LogLevel::DEBUG, file, line, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const char* file, int line, Args&&... args) {
        log_formatted(LogLevel::INFO, file, line, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const char* file, int line, Args&&... args) {
        log_formatted(LogLevel::WARN, file, line, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const char* file, int line, Args&&... args) {
        log_formatted(LogLevel::ERROR, file, line, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void fatal(const char* file, int line, Args&&... args) {
        log_formatted(LogLevel::FATAL, file, line, std::forward<Args>(args)...);
    }
    
private:
    Logger();
    
    template<typename... Args>
    void log_formatted(LogLevel level, const char* file, int line, Args&&... args) {
        if (level < current_level_) {
            return;
        }
        
        std::ostringstream oss;
        format_message(oss, std::forward<Args>(args)...);
        log(level, file, line, oss.str());
    }
    
    template<typename T>
    void format_message(std::ostringstream& oss, T&& arg) {
        oss << std::forward<T>(arg);
    }
    
    template<typename T, typename... Args>
    void format_message(std::ostringstream& oss, T&& first, Args&&... rest) {
        oss << std::forward<T>(first);
        format_message(oss, std::forward<Args>(rest)...);
    }
    
    const char* level_to_string(LogLevel level) const;
    
    LogLevel current_level_;
    mutable std::mutex mutex_;
};

}

#define LOG_TRACE(...) marketsim::exchange::utils::Logger::instance().trace(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) marketsim::exchange::utils::Logger::instance().debug(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) marketsim::exchange::utils::Logger::instance().info(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) marketsim::exchange::utils::Logger::instance().warn(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) marketsim::exchange::utils::Logger::instance().error(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) marketsim::exchange::utils::Logger::instance().fatal(__FILE__, __LINE__, __VA_ARGS__)
