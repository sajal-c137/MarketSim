#pragma once

#include "thread_info.h"
#include "socket_info.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

namespace marketsim::monitor {

class StatusMonitor {
public:
    static StatusMonitor& instance();
    
    // Thread monitoring
    void register_thread(std::thread::id thread_id, const std::string& name);
    void update_thread_state(std::thread::id thread_id, ThreadState state);
    void update_thread_activity(std::thread::id thread_id);
    void increment_thread_tasks(std::thread::id thread_id);
    void unregister_thread(std::thread::id thread_id);
    
    // Socket monitoring
    void register_socket(const std::string& name, SocketType type, const std::string& endpoint);
    void update_socket_state(const std::string& name, SocketState state);
    void record_socket_send(const std::string& name, size_t bytes);
    void record_socket_receive(const std::string& name, size_t bytes);
    void record_socket_error(const std::string& name, const std::string& error);
    void unregister_socket(const std::string& name);
    
    // Status retrieval
    std::vector<ThreadInfo> get_thread_status() const;
    std::vector<SocketInfo> get_socket_status() const;
    ThreadInfo get_thread_info(std::thread::id thread_id) const;
    SocketInfo get_socket_info(const std::string& name) const;
    
    // Periodic monitoring control
    void start_periodic_monitoring(std::chrono::milliseconds interval = std::chrono::seconds(5));
    void stop_periodic_monitoring();
    void set_status_callback(std::function<void(const std::vector<ThreadInfo>&, const std::vector<SocketInfo>&)> callback);
    
    // Statistics
    size_t active_thread_count() const;
    size_t active_socket_count() const;
    size_t total_messages_sent() const;
    size_t total_messages_received() const;
    
    // Print status to console
    void print_status() const;
    
private:
    StatusMonitor();
    ~StatusMonitor();
    
    StatusMonitor(const StatusMonitor&) = delete;
    StatusMonitor& operator=(const StatusMonitor&) = delete;
    
    void monitoring_loop();
    void check_thread_health();
    void check_socket_health();
    
    mutable std::mutex threads_mutex_;
    mutable std::mutex sockets_mutex_;
    
    std::unordered_map<std::thread::id, ThreadInfo> threads_;
    std::unordered_map<std::string, SocketInfo> sockets_;
    
    std::atomic<bool> monitoring_running_;
    std::unique_ptr<std::thread> monitoring_thread_;
    std::chrono::milliseconds monitoring_interval_;
    
    std::function<void(const std::vector<ThreadInfo>&, const std::vector<SocketInfo>&)> status_callback_;
};

}
