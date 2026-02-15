#pragma once

#include "i_price_model.h"
#include "../../operations/gbm_price_generator.h"
#include <memory>

namespace marketsim::traffic_generator::models::price_models {

/**
 * @brief Geometric Brownian Motion (GBM) price model
 * 
 * Stochastic model: dS/S = ?*dt + ?*dW
 * Discrete: S(t+?t) = S(t) * exp((? - ?²/2)*?t + ?*??t*Z)
 * 
 * Produces realistic random price paths with configurable drift and volatility.
 */
class GBMPriceModel : public IPriceModel {
public:
    /**
     * @brief Construct GBM price model
     * @param initial_price Starting price
     * @param drift Annual drift (?), e.g., 0.05 = 5%
     * @param volatility Annual volatility (?), e.g., 0.03 = 3%
     * @param dt Simulated time step (fraction of year)
     * @param seed Random seed (0 = random)
     */
    GBMPriceModel(
        double initial_price,
        double drift,
        double volatility,
        double dt,
        uint64_t seed = 0
    );
    
    double next_price() override;
    double current_price() const override;
    void reset() override;
    std::string model_name() const override { return "gbm"; }
    std::string description() const override {
        return "Geometric Brownian Motion: dS/S = ?*dt + ?*dW";
    }
    
private:
    std::unique_ptr<operations::GBMPriceGenerator> generator_;
};

} // namespace marketsim::traffic_generator::models::price_models
