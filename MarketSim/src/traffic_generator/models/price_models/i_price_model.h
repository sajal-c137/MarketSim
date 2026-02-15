#pragma once

#include <string>
#include <memory>

namespace marketsim::traffic_generator::models::price_models {

/**
 * @brief Interface for all price generation models
 * 
 * Defines the contract that all price models must implement.
 * Each model encapsulates a different mathematical approach to price generation.
 */
class IPriceModel {
public:
    virtual ~IPriceModel() = default;
    
    /**
     * @brief Generate the next price
     * @return Next price value
     */
    virtual double next_price() = 0;
    
    /**
     * @brief Get current price without advancing
     * @return Current price
     */
    virtual double current_price() const = 0;
    
    /**
     * @brief Reset model to initial state
     */
    virtual void reset() = 0;
    
    /**
     * @brief Get model name/identifier
     * @return Model name (e.g., "linear", "gbm", "jump-diffusion")
     */
    virtual std::string model_name() const = 0;
    
    /**
     * @brief Get model description
     * @return Human-readable description
     */
    virtual std::string description() const = 0;
};

} // namespace marketsim::traffic_generator::models::price_models
