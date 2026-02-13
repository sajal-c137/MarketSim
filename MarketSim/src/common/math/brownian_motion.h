#pragma once

#include "random.h"
#include <cmath>
#include <vector>

namespace marketsim::common::math {

/**
 * @brief Brownian Motion and Geometric Brownian Motion simulation
 * 
 * Standard Brownian Motion: dW_t ~ N(0, dt)
 * Geometric Brownian Motion: dS_t = ?*S_t*dt + ?*S_t*dW_t
 */
class BrownianMotion {
public:
    explicit BrownianMotion(RandomGenerator& rng) : rng_(rng) {}
    
    /**
     * @brief Generate standard Brownian motion increment
     * @param dt Time step
     * @return dW_t ~ N(0, dt)
     */
    double standard_increment(double dt) {
        return rng_.standard_normal() * std::sqrt(dt);
    }
    
    /**
     * @brief Generate Brownian motion path
     * @param n_steps Number of time steps
     * @param dt Time increment
     * @param initial_value Starting value (default 0)
     * @return Vector of W_t values
     */
    std::vector<double> generate_path(int n_steps, double dt, double initial_value = 0.0) {
        std::vector<double> path;
        path.reserve(n_steps + 1);
        path.push_back(initial_value);
        
        double current = initial_value;
        for (int i = 0; i < n_steps; ++i) {
            current += standard_increment(dt);
            path.push_back(current);
        }
        
        return path;
    }
    
private:
    RandomGenerator& rng_;
};

/**
 * @brief Geometric Brownian Motion (GBM)
 * 
 * Used for stock price simulation:
 * S_t = S_0 * exp((? - ?²/2)*t + ?*W_t)
 */
class GeometricBrownianMotion {
public:
    GeometricBrownianMotion(RandomGenerator& rng, double drift, double volatility)
        : rng_(rng)
        , drift_(drift)
        , volatility_(volatility)
    {}
    
    /**
     * @brief Generate single GBM increment
     * @param current_price Current stock price
     * @param dt Time step
     * @return New stock price
     */
    double step(double current_price, double dt) {
        double dW = rng_.standard_normal() * std::sqrt(dt);
        double drift_term = (drift_ - 0.5 * volatility_ * volatility_) * dt;
        double diffusion_term = volatility_ * dW;
        
        return current_price * std::exp(drift_term + diffusion_term);
    }
    
    /**
     * @brief Generate full GBM price path
     * @param initial_price Starting price (S_0)
     * @param n_steps Number of time steps
     * @param dt Time increment
     * @return Vector of S_t values
     */
    std::vector<double> generate_path(double initial_price, int n_steps, double dt) {
        std::vector<double> path;
        path.reserve(n_steps + 1);
        path.push_back(initial_price);
        
        double current = initial_price;
        for (int i = 0; i < n_steps; ++i) {
            current = step(current, dt);
            path.push_back(current);
        }
        
        return path;
    }
    
    /**
     * @brief Generate terminal price directly (efficient for Monte Carlo)
     * @param initial_price Starting price
     * @param time_to_maturity Total time T
     * @return S_T
     */
    double terminal_price(double initial_price, double time_to_maturity) {
        double z = rng_.standard_normal();
        double drift_term = (drift_ - 0.5 * volatility_ * volatility_) * time_to_maturity;
        double diffusion_term = volatility_ * std::sqrt(time_to_maturity) * z;
        
        return initial_price * std::exp(drift_term + diffusion_term);
    }
    
    void set_drift(double drift) { drift_ = drift; }
    void set_volatility(double volatility) { volatility_ = volatility; }
    
    double get_drift() const { return drift_; }
    double get_volatility() const { return volatility_; }
    
private:
    RandomGenerator& rng_;
    double drift_;       // ? (expected return)
    double volatility_;  // ? (volatility)
};

} // namespace marketsim::common::math
