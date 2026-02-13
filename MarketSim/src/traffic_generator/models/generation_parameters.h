#pragma once

#include <string>
#include <cstdint>

namespace marketsim::traffic_generator::models {

/**
 * @brief Configuration parameters for traffic generation
 */
struct GenerationParameters {
    std::string symbol;              // Trading symbol (e.g., "AAPL")
    double base_price;               // Starting price (e.g., 100.0)
    double price_rate;               // Price increase per second (e.g., 10.0)
    double order_quantity;           // Order size (e.g., 1.0)
    double step_interval_ms;         // Time between orders in milliseconds (e.g., 100 for 0.1s)
    double duration_seconds;         // Total duration (e.g., 10.0 seconds)
    
    GenerationParameters()
        : symbol("AAPL")
        , base_price(100.0)
        , price_rate(10.0)
        , order_quantity(1.0)
        , step_interval_ms(100.0)  // 0.1 seconds
        , duration_seconds(10.0)
    {}
};

/**
 * @brief Current state of traffic generation
 */
struct GenerationState {
    double elapsed_seconds;          // Time elapsed since start
    int64_t orders_sent;             // Total orders sent
    int64_t start_timestamp_ms;      // Start time in milliseconds
    bool is_running;                 // Whether generation is active
    
    GenerationState()
        : elapsed_seconds(0.0)
        , orders_sent(0)
        , start_timestamp_ms(0)
        , is_running(false)
    {}
};

} // namespace marketsim::traffic_generator::models
