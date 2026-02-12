#include "monitor/status_monitor.h"
#include "monitor/monitor_helpers.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>

using namespace marketsim::monitor;

// Simulate a worker thread
void worker_thread(const std::string& name, int task_count) {
    MonitoredThread monitor(name);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay(100, 500);
    
    for (int i = 0; i < task_count; ++i) {
        monitor.update_state(ThreadState::RUNNING);
        
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
        
        monitor.increment_tasks();
        monitor.update_activity();
        
        // Occasionally go idle
        if (i % 3 == 0) {
            monitor.update_state(ThreadState::IDLE);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    
    monitor.update_state(ThreadState::TERMINATED);
}

// Simulate a socket communication
void socket_thread(const std::string& name, SocketType type) {
    MonitoredSocket socket(name, type, "tcp://localhost:5555");
    MonitoredThread thread(name + "_thread");
    
    socket.update_state(SocketState::CONNECTING);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    socket.update_state(SocketState::CONNECTED);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> msg_size(64, 1024);
    std::uniform_int_distribution<> delay(50, 200);
    
    for (int i = 0; i < 20; ++i) {
        // Send messages
        size_t bytes = msg_size(gen);
        socket.record_send(bytes);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
        
        // Receive messages
        bytes = msg_size(gen);
        socket.record_receive(bytes);
        
        // Occasional error
        if (i == 10) {
            socket.record_error("Timeout on message receive");
            socket.update_state(SocketState::ERROR);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            socket.update_state(SocketState::CONNECTED);
        }
        
        thread.increment_tasks();
    }
    
    socket.update_state(SocketState::DISCONNECTED);
    thread.update_state(ThreadState::TERMINATED);
}

int main() {
    std::cout << "=== Status Monitor Test ===" << std::endl;
    std::cout << "Starting periodic monitoring...\n" << std::endl;
    
    // Start the status monitor with 3-second intervals
    StatusMonitor::instance().start_periodic_monitoring(std::chrono::seconds(3));
    
    // Launch worker threads
    std::vector<std::thread> threads;
    
    threads.emplace_back(worker_thread, "OrderProcessor", 10);
    threads.emplace_back(worker_thread, "TradeExecutor", 8);
    threads.emplace_back(worker_thread, "DataAggregator", 12);
    
    // Launch socket threads
    threads.emplace_back(socket_thread, "OrderSocket", SocketType::REQ);
    threads.emplace_back(socket_thread, "MarketDataSocket", SocketType::SUB);
    threads.emplace_back(socket_thread, "TradeSocket", SocketType::PUB);
    
    std::cout << "Threads and sockets running... Monitor will print status every 3 seconds.\n" << std::endl;
    
    // Let them run for a while
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cout << "\nAll threads completed. Printing final status..." << std::endl;
    StatusMonitor::instance().print_status();
    
    std::cout << "\nStopping monitor..." << std::endl;
    StatusMonitor::instance().stop_periodic_monitoring();
    
    std::cout << "Test complete!" << std::endl;
    
    return 0;
}
