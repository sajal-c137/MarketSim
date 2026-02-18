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
    double price_rate;               // Price increase per second (e.g., 10.0) - for linear model
    double order_quantity;           // Order size (e.g., 1.0)
    double step_interval_ms;         // Time between orders in milliseconds (e.g., 100 for 0.1s)
    double duration_seconds;         // Total duration (e.g., 10.0 seconds)
    
    // GBM parameters
    double drift;                    // Annual drift (μ) in percentage (e.g., 5.0 = 5%)
    double volatility;               // Annual volatility (σ) in percentage (e.g., 3.0 = 3%)
    
    // Hawkes Microstructure Model parameters
    double hawkes_mu;                // μ - Baseline order rate (events per second)
    double hawkes_alpha;             // α - Excitation coefficient (self-reinforcement)
    double hawkes_beta;              // β - Decay rate (how fast excitement dies)
    double momentum_k;               // k - Momentum sensitivity (+ trend, - mean-revert)
    double price_offset_L;           // L - Minimum price offset (tick size)
    double price_offset_alpha;       // α - Power law tail index (1.5-2.5)
    double price_offset_max;         // Maximum price offset (truncation)
    double volume_mu;                // μ_v - Log-normal volume mean (log-scale)
    double volume_sigma;             // σ_v - Log-normal volume std dev (log-scale)
    int orders_per_event;            // N - Number of orders per Hawkes event
    
    GenerationParameters()
        : symbol("AAPL")
        , base_price(100.0)
        , price_rate(10.0)
        , order_quantity(1.0)
        , step_interval_ms(10.0)   // 0.01 seconds (10x faster!)
        , duration_seconds(300.0)  // 5 minutes simulation (30x longer!)
        , drift(5.0)               // 5% annual drift
        , volatility(3.0)          // 3% annual volatility
        // Hawkes defaults (WIDER SPREAD = MORE LIMIT ORDERS ON BOOK)
        , hawkes_mu(10.0)          // 10 background orders/second
        , hawkes_alpha(2.0)        // VERY HIGH self-excitation
        , hawkes_beta(5.0)         // Fast decay but high rate
        , momentum_k(2.0)          // REDUCED trend-following (less aggressive)
        , price_offset_L(0.10)     // 10 cent minimum offset (10x wider!)
        , price_offset_alpha(3.5)  // HEAVY tail = more orders far from mid
        , price_offset_max(5.0)    // Max $5 from mid-price (5x wider!)
        , volume_mu(0.0)           // Median volume = exp(0) = 1
        , volume_sigma(0.5)        // Moderate volume variability
        , orders_per_event(5)      // 5 orders per cluster
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
