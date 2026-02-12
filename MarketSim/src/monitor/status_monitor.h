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
#include <fstream>

namespace marketsim::monitor {

enum class OutputMode {
    CONSOLE,        // Print to console (default)
    SUMMARY,        // Compact summary only
    SILENT,         // No console output
    CHANGES_ONLY    // Only show changes
};

class StatusMonitor {
public:
    static StatusMonitor& instance();
    
    // Configuration
    void set_output_mode(OutputMode mode);
    void set_console_verbosity(int level); // 0=minimal, 1=normal, 2=verbose
    void enable_file_logging(const std::string& filename);
    
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
    
    // Health checks
    std::vector<std::string> get_dead_threads() const;
    std::vector<std::string> get_stuck_threads(int idle_threshold_seconds = 30) const;
    std::vector<std::string> get_disconnected_sockets() const;
    std::vector<std::string> get_error_sockets() const;
    bool has_dead_components() const;

    
    // Output methods
    void print_status() const;
    void print_summary() const;  // Compact one-line summary
    void print_changes() const;  // Only show changes
    
private:
    StatusMonitor();
    ~StatusMonitor();
    
    StatusMonitor(const StatusMonitor&) = delete;
    StatusMonitor& operator=(const StatusMonitor&) = delete;
    
    void monitoring_loop();
    void check_thread_health();
    void check_socket_health();
    void log_to_file(const std::string& message);
    
    mutable std::mutex threads_mutex_;
    mutable std::mutex sockets_mutex_;
    mutable std::mutex file_mutex_;
    
    std::unordered_map<std::thread::id, ThreadInfo> threads_;
    std::unordered_map<std::string, SocketInfo> sockets_;
    
    // Previous state for change detection
    mutable std::unordered_map<std::thread::id, ThreadState> prev_thread_states_;
    mutable std::unordered_map<std::string, SocketState> prev_socket_states_;
    mutable size_t prev_total_sent_;
    mutable size_t prev_total_received_;
    
    std::atomic<bool> monitoring_running_;
    std::unique_ptr<std::thread> monitoring_thread_;
    std::chrono::milliseconds monitoring_interval_;
    
    std::function<void(const std::vector<ThreadInfo>&, const std::vector<SocketInfo>&)> status_callback_;
    
    // Configuration
    OutputMode output_mode_;
    int console_verbosity_;
    std::unique_ptr<std::ofstream> log_file_;
    mutable size_t report_count_;
};

}

