#pragma once

#include <string>
#include <chrono>
#include <thread>

namespace marketsim::monitor {

enum class ThreadState {
    RUNNING,
    IDLE,
    BLOCKED,
    TERMINATED,
    UNKNOWN
};

struct ThreadInfo {
    std::thread::id thread_id;
    std::string name;
    ThreadState state;
    std::chrono::milliseconds cpu_time;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_activity;
    size_t tasks_processed;
    
    ThreadInfo()
        : thread_id()
        , name("unknown")
        , state(ThreadState::UNKNOWN)
        , cpu_time(0)
        , created_at(std::chrono::system_clock::now())
        , last_activity(std::chrono::system_clock::now())
        , tasks_processed(0)
    {}
    
    ThreadInfo(std::thread::id id, const std::string& thread_name)
        : thread_id(id)
        , name(thread_name)
        , state(ThreadState::RUNNING)
        , cpu_time(0)
        , created_at(std::chrono::system_clock::now())
        , last_activity(std::chrono::system_clock::now())
        , tasks_processed(0)
    {}
};

inline std::string to_string(ThreadState state) {
    switch (state) {
        case ThreadState::RUNNING: return "RUNNING";
        case ThreadState::IDLE: return "IDLE";
        case ThreadState::BLOCKED: return "BLOCKED";
        case ThreadState::TERMINATED: return "TERMINATED";
        case ThreadState::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

}
