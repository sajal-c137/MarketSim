#pragma once

#include "random.h"
#include <cmath>

namespace marketsim::common::math {

/**
 * @brief Utility functions for sampling from various probability distributions
 * 
 * All functions are pure math - no side effects, threading, or I/O.
 * Each function documents the mathematical formula used.
 * 
 * Part of common/math library for use across the entire project.
 */
class DistributionUtils {
public:
    /**
     * @brief Sample from Exponential distribution
     * 
     * Formula: X = -ln(U) / ?
     * where U ~ Uniform(0,1), ? is the rate parameter
     * 
     * Used for: Inter-arrival times in Poisson/Hawkes processes
     * 
     * @param lambda Rate parameter (? > 0)
     * @param rng Random number generator
     * @return Sample from Exp(?)
     */
    static double sample_exponential(double lambda, RandomGenerator& rng);
    
    /**
     * @brief Compute logistic (sigmoid) function
     * 
     * Formula: ?(x) = 1 / (1 + exp(-x))
     * 
     * Maps real numbers to (0, 1) probability range.
     * Used for: Converting momentum to buy/sell probability
     * 
     * @param x Input value (can be any real number)
     * @return Probability in range (0, 1)
     */
    static double logistic(double x);
    
    /**
     * @brief Sample from Bernoulli distribution
     * 
     * Formula: X = { 1 with probability p
     *              { 0 with probability 1-p
     * 
     * Implementation: return (U < p) where U ~ Uniform(0,1)
     * 
     * Used for: Binary decisions (buy vs sell)
     * 
     * @param p Success probability (0 ? p ? 1)
     * @param rng Random number generator
     * @return true with probability p, false otherwise
     */
    static bool sample_bernoulli(double p, RandomGenerator& rng);
    
    /**
     * @brief Sample from Truncated Power Law (Pareto) distribution
     * 
     * Formula: f(x) = (? * L^?) / (x^(?+1))  for L ? x ? x_max
     * 
     * Inverse CDF method:
     *   X = L * (1 - U * (1 - (L/x_max)^?))^(-1/?)
     * where U ~ Uniform(0,1)
     * 
     * Properties:
     *   - Heavy-tailed: P(X > x) ~ x^(-?)
     *   - Lower ? ? heavier tails (more extreme values)
     *   - Typical range: ? ? [1.5, 2.5]
     * 
     * Used for: Price offsets from mid-price (order book depth)
     * 
     * @param L Minimum value (scale parameter, L > 0)
     * @param alpha Tail index (? > 0, typically 1.5-2.5)
     * @param x_max Maximum value (truncation point)
     * @param rng Random number generator
     * @return Sample from truncated Pareto distribution
     */
    static double sample_truncated_power_law(
        double L, 
        double alpha, 
        double x_max, 
        RandomGenerator& rng
    );
    
    /**
     * @brief Sample from Log-Normal distribution
     * 
     * Formula: if Y ~ Normal(?, ?²), then X = exp(Y) ~ LogNormal(?, ?)
     * 
     * Properties:
     *   - Always positive: X > 0
     *   - Right-skewed (long tail to the right)
     *   - E[X] = exp(? + ?²/2)
     *   - Var[X] = (exp(?²) - 1) * exp(2? + ?²)
     * 
     * Used for: Trading volumes (positive, skewed distribution)
     * 
     * @param mu Mean of underlying normal (log-scale mean)
     * @param sigma Std dev of underlying normal (log-scale std)
     * @param rng Random number generator
     * @return Sample from LogNormal(?, ?)
     */
    static double sample_lognormal(
        double mu, 
        double sigma, 
        RandomGenerator& rng
    );
};

} // namespace marketsim::common::math
