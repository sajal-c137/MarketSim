#pragma once

#include "common/math/random.h"
#include <vector>

namespace marketsim::traffic_generator::operations {

/**
 * @brief Generates prices using Geometric Brownian Motion (GBM)
 * 
 * Pure math implementation of the stochastic differential equation:
 * dS/S = ?*dt + ?*dW
 * 
 * Discrete form:
 * S(t+?t) = S(t) * exp((? - ?²/2)*?t + ?*??t*Z)
 * where Z ~ N(0,1)
 * 
 * No I/O, threading, or side effects.
 */
class GBMPriceGenerator {
public:
    /**
     * @brief Construct GBM price generator
     * @param initial_price Starting price (S?)
     * @param drift Annual drift rate (?), e.g., 0.05 = 5% per year
     * @param volatility Annual volatility (?), e.g., 0.20 = 20% per year
     * @param dt Time step in seconds
     * @param seed Random seed for reproducibility (optional)
     */
    GBMPriceGenerator(
        double initial_price,
        double drift,
        double volatility,
        double dt,
        uint64_t seed = 0
    );
    
    /**
     * @brief Generate next price using GBM
     * @return Next price value
     */
    double next_price();
    
    /**
     * @brief Get current price without advancing
     */
    double current_price() const { return current_price_; }
    
    /**
     * @brief Reset to initial conditions
     */
    void reset();
    
    /**
     * @brief Generate N future prices (for testing/visualization)
     * @param n Number of prices to generate
     * @return Vector of future prices
     */
    std::vector<double> generate_path(int n);
    
private:
    // GBM parameters
    double initial_price_;     // S?
    double drift_;             // ? (annual)
    double volatility_;        // ? (annual)
    double dt_;                // ?t (seconds)
    
    // Convert annual parameters to per-step
    double drift_per_step_;    // ? * dt
    double vol_per_step_;      // ? * ?dt
    double drift_adjustment_;  // (? - ?²/2) * dt
    
    // State
    double current_price_;
    
    // Random number generator
    common::math::RandomGenerator rng_;
};

} // namespace marketsim::traffic_generator::operations
