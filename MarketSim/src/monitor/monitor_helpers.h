#pragma once

#include "status_monitor.h"
#include <thread>

namespace marketsim::monitor {

// RAII wrapper for automatic thread registration/unregistration
class MonitoredThread {
public:
    MonitoredThread(const std::string& name) 
        : name_(name) 
    {
        StatusMonitor::instance().register_thread(std::this_thread::get_id(), name);
    }
    
    ~MonitoredThread() {
        StatusMonitor::instance().unregister_thread(std::this_thread::get_id());
    }
    
    void update_state(ThreadState state) {
        StatusMonitor::instance().update_thread_state(std::this_thread::get_id(), state);
    }
    
    void update_activity() {
        StatusMonitor::instance().update_thread_activity(std::this_thread::get_id());
    }
    
    void increment_tasks() {
        StatusMonitor::instance().increment_thread_tasks(std::this_thread::get_id());
    }
    
private:
    std::string name_;
};

// RAII wrapper for automatic socket registration/unregistration
class MonitoredSocket {
public:
    MonitoredSocket(const std::string& name, SocketType type, const std::string& endpoint)
        : name_(name)
    {
        StatusMonitor::instance().register_socket(name, type, endpoint);
    }
    
    ~MonitoredSocket() {
        StatusMonitor::instance().unregister_socket(name_);
    }
    
    void update_state(SocketState state) {
        StatusMonitor::instance().update_socket_state(name_, state);
    }
    
    void record_send(size_t bytes) {
        StatusMonitor::instance().record_socket_send(name_, bytes);
    }
    
    void record_receive(size_t bytes) {
        StatusMonitor::instance().record_socket_receive(name_, bytes);
    }
    
    void record_error(const std::string& error) {
        StatusMonitor::instance().record_socket_error(name_, error);
    }
    
private:
    std::string name_;
};

}
