#pragma once

#include "i_price_model.h"

namespace marketsim::traffic_generator::models::price_models {

/**
 * @brief Linear price model: price = base_price + rate * time_elapsed
 * 
 * Simple deterministic model where price increases linearly with time.
 * No randomness - predictable price path.
 */
class LinearPriceModel : public IPriceModel {
public:
    /**
     * @brief Construct linear price model
     * @param base_price Starting price
     * @param rate_per_step Price increase per step
     */
    LinearPriceModel(double base_price, double rate_per_step);
    
    double next_price() override;
    double current_price() const override;
    void reset() override;
    std::string model_name() const override { return "linear"; }
    std::string description() const override {
        return "Linear price model: P(t) = P0 + rate * t";
    }
    
private:
    double base_price_;
    double rate_per_step_;
    double current_price_;
    int step_count_;
};

} // namespace marketsim::traffic_generator::models::price_models
