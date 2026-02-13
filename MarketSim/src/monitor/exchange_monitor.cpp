#include "exchange_monitor.h"
#include "exchange_logger.h"
#include <iostream>
#include <chrono>

namespace marketsim::monitor {

ExchangeMonitor::ExchangeMonitor(const std::string& status_endpoint)
    : status_endpoint_(status_endpoint)
    , io_context_(1)
    , running_(false)
{
}

ExchangeMonitor::~ExchangeMonitor() {
    stop();
}

void ExchangeMonitor::start() {
    if (running_) {
        return;
    }
    
    std::cout << "[MONITOR] Starting Exchange Monitor...\n";
    std::cout << "[MONITOR] Connecting to: " << status_endpoint_ << "\n\n";
    
    // Create status requester
    status_requester_ = std::make_unique<io_handler::ZmqRequester>(
        io_context_,
        "Monitor_Status",
        status_endpoint_
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
    
    // Start monitoring thread
    running_ = true;
    monitor_thread_ = std::make_unique<std::thread>(
        &ExchangeMonitor::run_monitor_loop, 
        this
    );
}

void ExchangeMonitor::stop() {
    running_ = false;
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
    std::cout << "[MONITOR] Monitoring started. Querying Exchange every 100ms...\n\n";
    
    while (running_) {
        // Query Exchange for status and display
        query_and_display_status();
        
        // Sleep before next query
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "[MONITOR] Monitoring stopped\n";
}

void ExchangeMonitor::query_and_display_status() {
    // Create status request
    exchange::StatusRequest request;
    request.set_request_type("full");
    request.set_symbol("AAPL");
    
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
    
    // Display orderbook snapshot
    if (response.has_current_orderbook()) {
        // Convert protobuf OrderBook to operations::OrderBook for logging
        // For now, we'll create a temporary OrderBook and populate it
        
        // Create a minimal orderbook representation for display
        const auto& pb_orderbook = response.current_orderbook();
        
        // Create a real OrderBook object
        exchange::operations::OrderBook display_book(pb_orderbook.symbol());
        
        // Add orders to the book from the protobuf levels
        // Note: This is a simplified approach - we're creating synthetic orders
        // just for display purposes
        for (const auto& bid : pb_orderbook.bids()) {
            if (bid.order_count() > 0) {
                // Create a synthetic order entry for this price level
                exchange::operations::OrderEntry entry(
                    "display",  // dummy order id
                    "monitor",  // dummy client id
                    bid.price(),
                    bid.quantity(),
                    0  // timestamp
                );
                display_book.add_order(entry, true);  // true = buy side
            }
        }
        
        for (const auto& ask : pb_orderbook.asks()) {
            if (ask.order_count() > 0) {
                // Create a synthetic order entry for this price level
                exchange::operations::OrderEntry entry(
                    "display",  // dummy order id
                    "monitor",  // dummy client id
                    ask.price(),
                    ask.quantity(),
                    0  // timestamp
                );
                display_book.add_order(entry, false);  // false = sell side
            }
        }
        
        ExchangeLogger::log_orderbook(display_book, 5);
    }
}

} // namespace marketsim::monitor
