#pragma once

#include <string>
#include <chrono>

namespace marketsim::monitor {

enum class SocketType {
    REQ,      // Request
    REP,      // Reply
    PUB,      // Publish
    SUB,      // Subscribe
    PUSH,     // Push
    PULL,     // Pull
    DEALER,   // Dealer
    ROUTER,   // Router
    UNKNOWN
};

enum class SocketState {
    CONNECTED,
    DISCONNECTED,
    LISTENING,
    CONNECTING,
    ERROR,
    UNKNOWN
};

struct SocketInfo {
    std::string socket_name;
    SocketType type;
    SocketState state;
    std::string endpoint;
    size_t messages_sent;
    size_t messages_received;
    size_t bytes_sent;
    size_t bytes_received;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_activity;
    size_t error_count;
    std::string last_error;
    
    SocketInfo()
        : socket_name("unknown")
        , type(SocketType::UNKNOWN)
        , state(SocketState::UNKNOWN)
        , endpoint("")
        , messages_sent(0)
        , messages_received(0)
        , bytes_sent(0)
        , bytes_received(0)
        , created_at(std::chrono::system_clock::now())
        , last_activity(std::chrono::system_clock::now())
        , error_count(0)
        , last_error("")
    {}
    
    SocketInfo(const std::string& name, SocketType sock_type, const std::string& ep)
        : socket_name(name)
        , type(sock_type)
        , state(SocketState::CONNECTING)
        , endpoint(ep)
        , messages_sent(0)
        , messages_received(0)
        , bytes_sent(0)
        , bytes_received(0)
        , created_at(std::chrono::system_clock::now())
        , last_activity(std::chrono::system_clock::now())
        , error_count(0)
        , last_error("")
    {}
};

inline std::string to_string(SocketType type) {
    switch (type) {
        case SocketType::REQ: return "REQ";
        case SocketType::REP: return "REP";
        case SocketType::PUB: return "PUB";
        case SocketType::SUB: return "SUB";
        case SocketType::PUSH: return "PUSH";
        case SocketType::PULL: return "PULL";
        case SocketType::DEALER: return "DEALER";
        case SocketType::ROUTER: return "ROUTER";
        case SocketType::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

inline std::string to_string(SocketState state) {
    switch (state) {
        case SocketState::CONNECTED: return "CONNECTED";
        case SocketState::DISCONNECTED: return "DISCONNECTED";
        case SocketState::LISTENING: return "LISTENING";
        case SocketState::CONNECTING: return "CONNECTING";
        case SocketState::ERROR: return "ERROR";
        case SocketState::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

}
