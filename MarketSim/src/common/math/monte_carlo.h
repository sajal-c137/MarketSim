#pragma once

#include "random.h"
#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>

namespace marketsim::common::math {

/**
 * @brief Monte Carlo simulation framework
 * 
 * Generic Monte Carlo engine for pricing, risk analysis, etc.
 */
class MonteCarlo {
public:
    explicit MonteCarlo(RandomGenerator& rng) : rng_(rng) {}
    
    /**
     * @brief Run Monte Carlo simulation
     * @param n_simulations Number of paths to simulate
     * @param payoff_function Function that computes payoff for each path
     * @return Mean payoff value
     */
    double simulate(int n_simulations, std::function<double()> payoff_function) {
        double sum = 0.0;
        for (int i = 0; i < n_simulations; ++i) {
            sum += payoff_function();
        }
        return sum / n_simulations;
    }
    
    /**
     * @brief Run Monte Carlo with confidence interval
     * @param n_simulations Number of paths
     * @param payoff_function Payoff computation
     * @return {mean, std_error, lower_95%, upper_95%}
     */
    struct Result {
        double mean;
        double std_error;
        double confidence_lower;
        double confidence_upper;
    };
    
    Result simulate_with_confidence(int n_simulations, std::function<double()> payoff_function) {
        std::vector<double> payoffs;
        payoffs.reserve(n_simulations);
        
        for (int i = 0; i < n_simulations; ++i) {
            payoffs.push_back(payoff_function());
        }
        
        // Calculate mean
        double mean = std::accumulate(payoffs.begin(), payoffs.end(), 0.0) / n_simulations;
        
        // Calculate variance
        double variance = 0.0;
        for (double payoff : payoffs) {
            double diff = payoff - mean;
            variance += diff * diff;
        }
        variance /= (n_simulations - 1);
        
        // Standard error
        double std_error = std::sqrt(variance / n_simulations);
        
        // 95% confidence interval (z = 1.96)
        double margin = 1.96 * std_error;
        
        return {
            mean,
            std_error,
            mean - margin,
            mean + margin
        };
    }
    
    /**
     * @brief Antithetic variance reduction
     * @param n_pairs Number of paired simulations
     * @param payoff_function Function that takes a random value
     * @return Mean of antithetic pairs
     */
    double simulate_antithetic(int n_pairs, std::function<double(double)> payoff_function) {
        double sum = 0.0;
        for (int i = 0; i < n_pairs; ++i) {
            double z = rng_.standard_normal();
            sum += payoff_function(z);
            sum += payoff_function(-z);  // Antithetic variate
        }
        return sum / (2 * n_pairs);
    }
    
private:
    RandomGenerator& rng_;
};

/**
 * @brief Online statistics accumulator
 * 
 * Computes mean, variance, min, max in a single pass
 */
class Statistics {
public:
    Statistics() : count_(0), mean_(0.0), m2_(0.0), min_(0.0), max_(0.0) {}
    
    void add(double value) {
        count_++;
        
        if (count_ == 1) {
            min_ = max_ = value;
        } else {
            min_ = std::min(min_, value);
            max_ = std::max(max_, value);
        }
        
        // Welford's online algorithm for variance
        double delta = value - mean_;
        mean_ += delta / count_;
        double delta2 = value - mean_;
        m2_ += delta * delta2;
    }
    
    int count() const { return count_; }
    double mean() const { return mean_; }
    double variance() const { return count_ > 1 ? m2_ / (count_ - 1) : 0.0; }
    double stddev() const { return std::sqrt(variance()); }
    double min() const { return min_; }
    double max() const { return max_; }
    
    void reset() {
        count_ = 0;
        mean_ = 0.0;
        m2_ = 0.0;
        min_ = 0.0;
        max_ = 0.0;
    }
    
private:
    int count_;
    double mean_;
    double m2_;
    double min_;
    double max_;
};

} // namespace marketsim::common::math
