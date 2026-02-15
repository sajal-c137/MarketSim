#include "gbm_price_generator.h"
#include <cmath>

namespace marketsim::traffic_generator::operations {

GBMPriceGenerator::GBMPriceGenerator(
    double initial_price,
    double drift,
    double volatility,
    double dt,
    uint64_t seed)
    : initial_price_(initial_price)
    , drift_(drift)
    , volatility_(volatility)
    , dt_(dt)
    , current_price_(initial_price)
    , rng_(seed == 0 ? common::math::RandomGenerator() : common::math::RandomGenerator(seed))
{
    // dt is the simulated time step (fraction of a year)
    // drift and volatility are annualized parameters
    drift_per_step_ = drift * dt;
    vol_per_step_ = volatility * std::sqrt(dt);
    drift_adjustment_ = (drift - 0.5 * volatility * volatility) * dt;
}

double GBMPriceGenerator::next_price() {
    // GBM formula (discrete):
    // S(t+?t) = S(t) * exp((? - ?²/2)*?t + ?*??t*Z)
    // where Z ~ N(0,1)
    
    double z = rng_.standard_normal();
    double drift_term = drift_adjustment_;
    double diffusion_term = vol_per_step_ * z;
    
    // Update price
    current_price_ *= std::exp(drift_term + diffusion_term);
    
    return current_price_;
}

void GBMPriceGenerator::reset() {
    current_price_ = initial_price_;
}

std::vector<double> GBMPriceGenerator::generate_path(int n) {
    std::vector<double> path;
    path.reserve(n);
    
    // Save current state
    double saved_price = current_price_;
    
    // Generate path
    for (int i = 0; i < n; ++i) {
        path.push_back(next_price());
    }
    
    // Restore state
    current_price_ = saved_price;
    
    return path;
}

} // namespace marketsim::traffic_generator::operations
