#pragma once

namespace marketsim::traffic_generator::operations {

/**
 * @brief Calculates price movements using mathematical models
 * 
 * Pure math - no I/O, threading, or side effects.
 * Simple linear model: price = base_price + rate * time_elapsed
 */
class PriceMovementCalculator {
public:
    PriceMovementCalculator(double base_price = 100.0, double rate_per_second = 10.0);
    
    /**
     * @brief Calculate price at given time
     * @param time_seconds Time elapsed in seconds
     * @return Calculated price
     */
    double calculate_price(double time_seconds) const;
    
    // Setters for testing/configuration
    void set_base_price(double base_price);
    void set_rate(double rate_per_second);
    
private:
    double base_price_;      // Initial price (e.g., 100)
    double rate_per_second_; // Price increase per second (e.g., 10)
};

} // namespace marketsim::traffic_generator::operations
