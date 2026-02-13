#pragma once

#include <string>

namespace marketsim::monitor {

/**
 * @brief Configuration for Monitor service
 */
struct MonitorConfig {
    std::string exchange_status_endpoint;  // Exchange status endpoint
    std::string ticker;                    // Ticker symbol to monitor (e.g., "AAPL")
    int polling_interval_ms;               // How often to poll Exchange (milliseconds)
    bool show_order_received;              // Display order received logs
    bool show_matching;                    // Display matching logs
    bool show_price_updates;               // Display price update logs
    bool show_orderbook;                   // Display orderbook logs
    int orderbook_depth;                   // Orderbook depth to display
    
    // Default constructor with sensible defaults
    MonitorConfig()
        : exchange_status_endpoint("tcp://localhost:5557")
        , ticker("AAPL")
        , polling_interval_ms(1000)  // Poll every 1 second
        , show_order_received(true)
        , show_matching(true)
        , show_price_updates(true)
        , show_orderbook(true)
        , orderbook_depth(5)
    {}
};

} // namespace marketsim::monitor
