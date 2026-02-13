#pragma once

#include <random>
#include <memory>
#include <cmath>

namespace marketsim::common::math {

/**
 * @brief High-quality random number generator wrapper
 * 
 * Uses std::mt19937_64 (Mersenne Twister) with proper seeding
 */
class RandomGenerator {
public:
    RandomGenerator() : rng_(std::random_device{}()) {}
    
    explicit RandomGenerator(uint64_t seed) : rng_(seed) {}
    
    // Get standard normal (mean=0, stddev=1)
    double standard_normal() {
        return normal_dist_(rng_);
    }
    
    // Get normal with specified mean and stddev
    double normal(double mean, double stddev) {
        return mean + stddev * standard_normal();
    }
    
    // Get uniform in [min, max)
    double uniform(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng_);
    }
    
    // Get uniform in [0, 1)
    double uniform_01() {
        return uniform_dist_(rng_);
    }
    
    // Get underlying generator
    std::mt19937_64& generator() { return rng_; }
    
private:
    std::mt19937_64 rng_;
    std::normal_distribution<double> normal_dist_{0.0, 1.0};
    std::uniform_real_distribution<double> uniform_dist_{0.0, 1.0};
};

} // namespace marketsim::common::math
