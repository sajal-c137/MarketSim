#include "monitor/status_monitor.h"
#include "monitor/monitor_helpers.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace marketsim::monitor;

void worker_thread(const std::string& name, int iterations) {
    MonitoredThread monitor(name);
    
    for (int i = 0; i < iterations; ++i) {
        monitor.update_state(ThreadState::RUNNING);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        monitor.increment_tasks();
        
        if (i % 3 == 0) {
            monitor.update_state(ThreadState::IDLE);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    
    monitor.update_state(ThreadState::TERMINATED);
}

void socket_activity(const std::string& name) {
    StatusMonitor::instance().register_socket(name, SocketType::PUB, "tcp://localhost:5555");
    StatusMonitor::instance().update_socket_state(name, SocketState::CONNECTED);
    
    for (int i = 0; i < 10; ++i) {
        StatusMonitor::instance().record_socket_send(name, 256);
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    
    StatusMonitor::instance().update_socket_state(name, SocketState::DISCONNECTED);
}

int main() {
    std::cout << "=== Logging Example ===\n\n";
    std::cout << "Configuration:\n";
    std::cout << "- Output Mode: SILENT (no console spam)\n";
    std::cout << "- File Logging: logs/demo.log (all details preserved)\n";
    std::cout << "- Check 'logs/demo.log' for output\n\n";
    
    // Configure monitoring
    StatusMonitor::instance().set_output_mode(OutputMode::SILENT);
    StatusMonitor::instance().enable_file_logging("logs/demo.log");
    StatusMonitor::instance().start_periodic_monitoring(std::chrono::seconds(2));
    
    std::cout << "Running simulation for 6 seconds...\n";
    
    std::vector<std::thread> threads;
    threads.emplace_back(worker_thread, "Worker1", 5);
    threads.emplace_back(worker_thread, "Worker2", 5);
    threads.emplace_back(socket_activity, "PubSocket1");
    
    std::this_thread::sleep_for(std::chrono::seconds(6));
    
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    
    StatusMonitor::instance().stop_periodic_monitoring();
    
    std::cout << "\n=== Done ===\n";
    std::cout << "Logs written to 'logs/demo.log'\n\n";
    std::cout << "Try these commands:\n";
    std::cout << "  tail -f logs/demo.log              # Watch logs\n";
    std::cout << "  grep 'Worker1' logs/demo.log       # Find Worker1 activity\n";
    std::cout << "  grep 'RUNNING' logs/demo.log       # Find running states\n";
    std::cout << "  grep -c 'record_socket_send' logs/demo.log  # Count sends\n";
    
    return 0;
}
