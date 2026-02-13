#include "exchange_monitor.h"
#include "exchange_logger.h"
#include "history_recorder.h"
#include <iostream>
#include <chrono>

namespace marketsim::monitor {

ExchangeMonitor::ExchangeMonitor(const MonitorConfig& config)
    : config_(config)
    , io_context_(1)
    , running_(false)
{
}

ExchangeMonitor::ExchangeMonitor(const std::string& status_endpoint)
    : io_context_(1)
    , running_(false)
{
    config_.exchange_status_endpoint = status_endpoint;
}

ExchangeMonitor::~ExchangeMonitor() {
    stop();
}

void ExchangeMonitor::start() {
    if (running_) {
        return;
    }
    
    std::cout << "[MONITOR] Starting Exchange Monitor...\n";
    std::cout << "[MONITOR] Config:\n";
    std::cout << "[MONITOR]   Ticker: " << config_.ticker << "\n";
    std::cout << "[MONITOR]   Endpoint: " << config_.exchange_status_endpoint << "\n";
    std::cout << "[MONITOR]   Polling Interval: " << config_.polling_interval_ms << " ms\n\n";
    
    // Create status requester
    status_requester_ = std::make_unique<io_handler::ZmqRequester>(
        io_context_,
        "Monitor_Status",
        config_.exchange_status_endpoint
    );
    
    try {
        status_requester_->connect();
        std::cout << "[MONITOR] Connected successfully\n";
    } catch (const std::exception& e) {
        std::cerr << "[MONITOR] Failed to connect: " << e.what() << "\n";
        return;
    }
    
    // Print header
    ExchangeLogger::print_startup_header();
    
    // Start history recording if enabled
    if (config_.enable_history_recording) {
        history_recorder_ = std::make_unique<HistoryRecorder>(config_.history_config);
        history_recorder_->start_session(config_.ticker);
    }
    
    // Start monitoring thread
    running_ = true;
    monitor_thread_ = std::make_unique<std::thread>(
        &ExchangeMonitor::run_monitor_loop, 
        this
    );
}

void ExchangeMonitor::stop() {
    running_ = false;
    
    // Stop history recording
    if (history_recorder_) {
        history_recorder_->end_session();
    }
    
    if (monitor_thread_ && monitor_thread_->joinable()) {
        monitor_thread_->join();
    }
}

bool ExchangeMonitor::is_running() const {
    return running_;
}

void ExchangeMonitor::wait() {
    if (monitor_thread_ && monitor_thread_->joinable()) {
        monitor_thread_->join();
    }
}

void ExchangeMonitor::run_monitor_loop() {
    std::cout << "[MONITOR] Monitoring started. Querying Exchange every " 
              << config_.polling_interval_ms << "ms...\n\n";
    
    while (running_) {
        // Query Exchange for status and display
        query_and_display_status();
        
        // Sleep before next query (using config value)
        std::this_thread::sleep_for(
            std::chrono::milliseconds(config_.polling_interval_ms)
        );
    }
    
    std::cout << "[MONITOR] Monitoring stopped\n";
}

void ExchangeMonitor::query_and_display_status() {
// Create status request
exchange::StatusRequest request;
request.set_request_type("full");
request.set_symbol(config_.ticker);  // Use ticker from config
    
// Send request and receive response
exchange::StatusResponse response;
    
    try {
        if (!status_requester_->request(request, response)) {
            return;  // Failed to get response, skip this cycle
        }
    } catch (const std::exception& e) {
        // Silently skip if Exchange not responding yet
        return;
    }
    
    // Record to history if enabled
    if (history_recorder_ && history_recorder_->is_recording()) {
        history_recorder_->record_status(response);
    }
    
    // Only display if we have activity (orders received)
    if (response.total_orders_received() == 0) {
        return;  // No activity yet
    }
    
    // Display last received order
    if (response.has_last_received_order()) {
        ExchangeLogger::log_order_received(
            response.total_orders_received(),
            response.last_received_order()
        );
    }
    
    // Display price update if trades occurred
    if (response.total_trades() > 0) {
        ExchangeLogger::log_price_update(
            response.last_trade_price(),
            response.total_trades(),
            response.total_volume()
        );
    }
    
    // Display orderbook snapshot - PASS FULL STATUS RESPONSE
    if (response.has_current_orderbook()) {
        ExchangeLogger::log_orderbook_pb(response);
    }
}

} // namespace marketsim::monitor
