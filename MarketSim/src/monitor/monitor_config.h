#pragma once

#include <string>

namespace marketsim::monitor {

// Forward declare to avoid circular dependency
struct HistoryRecorderConfig;

/**
 * @brief Configuration for History Recorder
 */
struct HistoryRecorderConfig {
    std::string output_directory;      // Directory for history files
    int write_interval_seconds;        // How often to write to file (seconds)
    bool record_trade_prices;          // Record trade price history
    bool record_mid_prices;            // Record mid price history
    bool record_orderbook_snapshots;   // Record orderbook state
    
    HistoryRecorderConfig()
        : output_directory("./market_history")
        , write_interval_seconds(5)  // Write every 5 seconds
        , record_trade_prices(true)
        , record_mid_prices(false)     // Disabled by default
        , record_orderbook_snapshots(false)  // Disabled by default
    {}
};

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
    bool enable_history_recording;         // Enable history recording to file
    HistoryRecorderConfig history_config;  // History recorder configuration
    
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
        , enable_history_recording(true)
        , history_config()
    {}
};

} // namespace marketsim::monitor
