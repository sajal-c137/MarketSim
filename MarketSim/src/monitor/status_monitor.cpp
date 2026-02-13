#include "status_monitor.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

namespace marketsim::monitor {

StatusMonitor& StatusMonitor::instance() {
    static StatusMonitor instance;
    return instance;
}

StatusMonitor::StatusMonitor()
    : monitoring_running_(false)
    , monitoring_interval_(std::chrono::seconds(5))
    , status_callback_(nullptr)
    , output_mode_(OutputMode::CONSOLE)
    , console_verbosity_(1)
    , prev_total_sent_(0)
    , prev_total_received_(0)
    , report_count_(0)
{}

StatusMonitor::~StatusMonitor() {
    stop_periodic_monitoring();
}

// Thread monitoring implementation
void StatusMonitor::register_thread(std::thread::id thread_id, const std::string& name) {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    threads_[thread_id] = ThreadInfo(thread_id, name);
}

void StatusMonitor::update_thread_state(std::thread::id thread_id, ThreadState state) {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    auto it = threads_.find(thread_id);
    if (it != threads_.end()) {
        it->second.state = state;
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

void StatusMonitor::update_thread_activity(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    auto it = threads_.find(thread_id);
    if (it != threads_.end()) {
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

void StatusMonitor::increment_thread_tasks(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    auto it = threads_.find(thread_id);
    if (it != threads_.end()) {
        it->second.tasks_processed++;
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

void StatusMonitor::unregister_thread(std::thread::id thread_id) {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    auto it = threads_.find(thread_id);
    if (it != threads_.end()) {
        it->second.state = ThreadState::TERMINATED;
    }
}

// Socket monitoring implementation
void StatusMonitor::register_socket(const std::string& name, SocketType type, const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    sockets_[name] = SocketInfo(name, type, endpoint);
}

void StatusMonitor::update_socket_state(const std::string& name, SocketState state) {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    auto it = sockets_.find(name);
    if (it != sockets_.end()) {
        it->second.state = state;
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

void StatusMonitor::record_socket_send(const std::string& name, size_t bytes) {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    auto it = sockets_.find(name);
    if (it != sockets_.end()) {
        it->second.messages_sent++;
        it->second.bytes_sent += bytes;
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

void StatusMonitor::record_socket_receive(const std::string& name, size_t bytes) {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    auto it = sockets_.find(name);
    if (it != sockets_.end()) {
        it->second.messages_received++;
        it->second.bytes_received += bytes;
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

void StatusMonitor::record_socket_error(const std::string& name, const std::string& error) {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    auto it = sockets_.find(name);
    if (it != sockets_.end()) {
        it->second.error_count++;
        it->second.last_error = error;
        it->second.state = SocketState::ERROR;
        it->second.last_activity = std::chrono::system_clock::now();
    }
}

void StatusMonitor::unregister_socket(const std::string& name) {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    auto it = sockets_.find(name);
    if (it != sockets_.end()) {
        it->second.state = SocketState::DISCONNECTED;
    }
}

// Status retrieval
std::vector<ThreadInfo> StatusMonitor::get_thread_status() const {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    std::vector<ThreadInfo> result;
    result.reserve(threads_.size());
    for (const auto& [id, info] : threads_) {
        result.push_back(info);
    }
    return result;
}

std::vector<SocketInfo> StatusMonitor::get_socket_status() const {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    std::vector<SocketInfo> result;
    result.reserve(sockets_.size());
    for (const auto& [name, info] : sockets_) {
        result.push_back(info);
    }
    return result;
}

ThreadInfo StatusMonitor::get_thread_info(std::thread::id thread_id) const {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    auto it = threads_.find(thread_id);
    return (it != threads_.end()) ? it->second : ThreadInfo();
}

SocketInfo StatusMonitor::get_socket_info(const std::string& name) const {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    auto it = sockets_.find(name);
    return (it != sockets_.end()) ? it->second : SocketInfo();
}

// Periodic monitoring
void StatusMonitor::start_periodic_monitoring(std::chrono::milliseconds interval) {
    if (monitoring_running_) {
        return;
    }
    
    monitoring_interval_ = interval;
    monitoring_running_ = true;
    monitoring_thread_ = std::make_unique<std::thread>(&StatusMonitor::monitoring_loop, this);
}

void StatusMonitor::stop_periodic_monitoring() {
    monitoring_running_ = false;
    if (monitoring_thread_ && monitoring_thread_->joinable()) {
        monitoring_thread_->join();
    }
    monitoring_thread_.reset();
}

void StatusMonitor::set_status_callback(std::function<void(const std::vector<ThreadInfo>&, const std::vector<SocketInfo>&)> callback) {
    status_callback_ = callback;
}

void StatusMonitor::monitoring_loop() {
    while (monitoring_running_) {
        check_thread_health();
        check_socket_health();
        
        if (status_callback_) {
            auto threads = get_thread_status();
            auto sockets = get_socket_status();
            status_callback_(threads, sockets);
        } else {
            print_status();
        }
        
        std::this_thread::sleep_for(monitoring_interval_);
    }
}

void StatusMonitor::check_thread_health() {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    auto now = std::chrono::system_clock::now();
    
    for (auto& [id, info] : threads_) {
        auto idle_duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - info.last_activity
        );
        
        if (info.state == ThreadState::RUNNING && idle_duration.count() > 30) {
            info.state = ThreadState::IDLE;
        }
    }
}

void StatusMonitor::check_socket_health() {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    auto now = std::chrono::system_clock::now();
    
    for (auto& [name, info] : sockets_) {
        auto idle_duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - info.last_activity
        );
        
        if (info.state == SocketState::CONNECTED && idle_duration.count() > 60) {
            // Socket might be stale - log warning
            std::ostringstream oss;
            oss << "[WARNING] Socket " << name << " idle for " << idle_duration.count() << " seconds";
            log_to_file(oss.str());
        }
    }
}

// Health check methods
std::vector<std::string> StatusMonitor::get_dead_threads() const {
std::vector<std::string> dead_threads;
std::lock_guard<std::mutex> lock(threads_mutex_);
    
for (const auto& [id, info] : threads_) {
    if (info.state == ThreadState::TERMINATED || info.state == ThreadState::UNKNOWN) {
        dead_threads.push_back(info.name);
    }
}
    
return dead_threads;
}

std::vector<std::string> StatusMonitor::get_stuck_threads(int idle_threshold_seconds) const {
    std::vector<std::string> stuck_threads;
    std::lock_guard<std::mutex> lock(threads_mutex_);
    auto now = std::chrono::system_clock::now();
    
    for (const auto& [id, info] : threads_) {
        if (info.state == ThreadState::RUNNING) {
            auto idle_duration = std::chrono::duration_cast<std::chrono::seconds>(
                now - info.last_activity
            );
            
            if (idle_duration.count() > idle_threshold_seconds) {
                stuck_threads.push_back(info.name + " (idle " + std::to_string(idle_duration.count()) + "s)");
            }
        }
    }
    
    return stuck_threads;
}

std::vector<std::string> StatusMonitor::get_disconnected_sockets() const {
    std::vector<std::string> disconnected;
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    
    for (const auto& [name, info] : sockets_) {
        if (info.state == SocketState::DISCONNECTED) {
            disconnected.push_back(name);
        }
    }
    
    return disconnected;
}

std::vector<std::string> StatusMonitor::get_error_sockets() const {
    std::vector<std::string> error_sockets;
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    
    for (const auto& [name, info] : sockets_) {
        if (info.state == SocketState::ERROR || !info.last_error.empty()) {
            error_sockets.push_back(name + ": " + info.last_error);
        }
    }
    
    return error_sockets;
}

bool StatusMonitor::has_dead_components() const {
    return !get_dead_threads().empty() || 
           !get_error_sockets().empty();
}


// Statistics
size_t StatusMonitor::active_thread_count() const {
    std::lock_guard<std::mutex> lock(threads_mutex_);
    return std::count_if(threads_.begin(), threads_.end(), 
        [](const auto& pair) { 
            return pair.second.state == ThreadState::RUNNING || 
                   pair.second.state == ThreadState::IDLE;
        });
}

size_t StatusMonitor::active_socket_count() const {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    return std::count_if(sockets_.begin(), sockets_.end(),
        [](const auto& pair) {
            return pair.second.state == SocketState::CONNECTED ||
                   pair.second.state == SocketState::LISTENING;
        });
}

size_t StatusMonitor::total_messages_sent() const {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    size_t total = 0;
    for (const auto& [name, info] : sockets_) {
        total += info.messages_sent;
    }
    return total;
}

size_t StatusMonitor::total_messages_received() const {
    std::lock_guard<std::mutex> lock(sockets_mutex_);
    size_t total = 0;
    for (const auto& [name, info] : sockets_) {
        total += info.messages_received;
    }
    return total;
}

// Print status
void StatusMonitor::print_status() const {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "  SYSTEM STATUS MONITOR" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Timestamp: " << std::ctime(&now_c);
    
    // Thread Status
    std::cout << "\n--- THREAD STATUS ---" << std::endl;
    auto threads = get_thread_status();
    
    if (threads.empty()) {
        std::cout << "No threads registered" << std::endl;
    } else {
        std::cout << std::left
                  << std::setw(25) << "Thread Name"
                  << std::setw(15) << "State"
                  << std::setw(15) << "Tasks"
                  << std::setw(20) << "Idle Time (s)" << std::endl;
        std::cout << std::string(80, '-') << std::endl;
        
        for (const auto& thread : threads) {
            auto idle_time = std::chrono::duration_cast<std::chrono::seconds>(
                now - thread.last_activity
            ).count();
            
            std::cout << std::left
                      << std::setw(25) << thread.name
                      << std::setw(15) << to_string(thread.state)
                      << std::setw(15) << thread.tasks_processed
                      << std::setw(20) << idle_time << std::endl;
        }
    }
    
    // Socket Status
    std::cout << "\n--- SOCKET STATUS ---" << std::endl;
    auto sockets = get_socket_status();
    
    if (sockets.empty()) {
        std::cout << "No sockets registered" << std::endl;
    } else {
        std::cout << std::left
                  << std::setw(20) << "Socket Name"
                  << std::setw(10) << "Type"
                  << std::setw(15) << "State"
                  << std::setw(10) << "Sent"
                  << std::setw(10) << "Recv"
                  << std::setw(10) << "Errors" << std::endl;
        std::cout << std::string(80, '-') << std::endl;
        
        for (const auto& socket : sockets) {
            std::cout << std::left
                      << std::setw(20) << socket.socket_name
                      << std::setw(10) << to_string(socket.type)
                      << std::setw(15) << to_string(socket.state)
                      << std::setw(10) << socket.messages_sent
                      << std::setw(10) << socket.messages_received
                      << std::setw(10) << socket.error_count << std::endl;
            
            if (!socket.last_error.empty()) {
                std::cout << "    Last error: " << socket.last_error << std::endl;
            }
        }
    }
    
    // Summary
    std::cout << "\n--- SUMMARY ---" << std::endl;
    std::cout << "Active Threads: " << active_thread_count() << " / " << threads.size() << std::endl;
    std::cout << "Active Sockets: " << active_socket_count() << " / " << sockets.size() << std::endl;
    std::cout << "Total Messages Sent: " << total_messages_sent() << std::endl;
    std::cout << "Total Messages Received: " << total_messages_received() << std::endl;
    
    std::cout << std::string(80, '=') << std::endl << std::endl;
}

// Configuration methods
void StatusMonitor::set_output_mode(OutputMode mode) {
    output_mode_ = mode;
}

void StatusMonitor::set_console_verbosity(int level) {
    console_verbosity_ = level;
}

void StatusMonitor::enable_file_logging(const std::string& filename) {
    std::lock_guard<std::mutex> lock(file_mutex_);
    log_file_ = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (log_file_->is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        
        *log_file_ << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        *log_file_ << " [System] Logging started to " << filename << std::endl;
    }
}

void StatusMonitor::log_to_file(const std::string& message) {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (log_file_ && log_file_->is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ) % 1000;
        
        *log_file_ << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        *log_file_ << "." << std::setfill('0') << std::setw(3) << ms.count();
        *log_file_ << " " << message << std::endl;
        log_file_->flush();
    }
}

void StatusMonitor::print_summary() const {
    auto threads = get_thread_status();
    auto sockets = get_socket_status();
    
    size_t running_threads = std::count_if(threads.begin(), threads.end(),
        [](const ThreadInfo& t) { return t.state == ThreadState::RUNNING; });
    
    size_t connected_sockets = std::count_if(sockets.begin(), sockets.end(),
        [](const SocketInfo& s) { 
            return s.state == SocketState::CONNECTED || s.state == SocketState::LISTENING; 
        });
    
    std::ostringstream oss;
    oss << "[Monitor #" << report_count_++ << "] "
        << "Threads: " << running_threads << "/" << threads.size() << " active | "
        << "Sockets: " << connected_sockets << "/" << sockets.size() << " connected | "
        << "Msgs: ?" << total_messages_sent() << " ?" << total_messages_received();
    
    std::string summary = oss.str();
    std::cout << summary << std::endl;
    const_cast<StatusMonitor*>(this)->log_to_file(summary);
}

void StatusMonitor::print_changes() const {
    bool has_changes = false;
    std::ostringstream changes;
    
    // Check thread state changes
    {
        std::lock_guard<std::mutex> lock(threads_mutex_);
        for (const auto& [id, info] : threads_) {
            auto prev_it = prev_thread_states_.find(id);
            if (prev_it == prev_thread_states_.end() || prev_it->second != info.state) {
                changes << "[" << info.name << "] state changed: " 
                        << to_string(info.state) << "\n";
                has_changes = true;
            }
        }
    }
    
    // Check socket state changes
    {
        std::lock_guard<std::mutex> lock(sockets_mutex_);
        for (const auto& [name, info] : sockets_) {
            auto prev_it = prev_socket_states_.find(name);
            if (prev_it == prev_socket_states_.end() || prev_it->second != info.state) {
                changes << "[" << name << "] state: " 
                        << to_string(info.state) << "\n";
                has_changes = true;
            }
        }
    }
    
    // Check message count changes
    size_t current_sent = total_messages_sent();
    size_t current_received = total_messages_received();
    
    if (current_sent != prev_total_sent_ || current_received != prev_total_received_) {
        changes << "[Traffic] Sent: +" << (current_sent - prev_total_sent_) 
                << " Recv: +" << (current_received - prev_total_received_) << "\n";
        has_changes = true;
    }
    
    if (has_changes) {
        std::string output = changes.str();
        std::cout << output;
        const_cast<StatusMonitor*>(this)->log_to_file(output);
    }
}

}

