#pragma once

#include "monitor_config.h"
#include "io_handler/io_context.h"
#include "io_handler/zmq_requester.h"
#include "exchange/operations/matching_engine.h"
#include <memory>
#include <atomic>
#include <thread>

namespace marketsim::monitor {

/**
 * @brief Exchange Monitor Service
 * 
 * Connects to Exchange, queries for status, and displays formatted logs.
 * This is the reusable monitoring logic.
 */
class ExchangeMonitor {
public:
    /**
     * @brief Construct monitor with config
     * @param config Monitor configuration
     */
    explicit ExchangeMonitor(const MonitorConfig& config);
    
    /**
     * @brief Construct monitor with endpoint (uses default config)
     * @param status_endpoint Exchange status endpoint (e.g., "tcp://localhost:5557")
     */
    explicit ExchangeMonitor(const std::string& status_endpoint);
    
    ~ExchangeMonitor();
    
    /**
     * @brief Start monitoring (runs in background thread)
     */
    void start();
    
    /**
     * @brief Stop monitoring
     */
    void stop();
    
    /**
     * @brief Check if monitoring is running
     */
    bool is_running() const;
    
    /**
     * @brief Wait for monitor to finish (blocking)
     */
    void wait();
    
private:
    void run_monitor_loop();
    void query_and_display_status();
    
    MonitorConfig config_;
    io_handler::IOContext io_context_;
    std::unique_ptr<io_handler::ZmqRequester> status_requester_;
    
    std::unique_ptr<std::thread> monitor_thread_;
    std::atomic<bool> running_;
};

} // namespace marketsim::monitor
