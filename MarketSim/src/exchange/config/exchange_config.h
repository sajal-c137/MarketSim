#pragma once

#include <string>

namespace marketsim::exchange::config {

/**
 * @brief Configuration for Exchange service
 */
struct ExchangeConfig {
    std::string order_port;            // Order receiving port (e.g., "tcp://*:5555")
    std::string status_port;           // Status query port (e.g., "tcp://*:5557")
    int price_history_size;            // Number of historical price ticks to keep
    
    // Default constructor
    ExchangeConfig()
        : order_port("tcp://*:5555")
        , status_port("tcp://*:5557")
        , price_history_size(100)  // Keep last 100 price points
    {}
};

} // namespace marketsim::exchange::config
