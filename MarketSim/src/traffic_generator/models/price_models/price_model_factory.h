#pragma once

#include "i_price_model.h"
#include "../generation_parameters.h"
#include <memory>
#include <string>
#include <stdexcept>

namespace marketsim::traffic_generator::models::price_models {

/**
 * @brief Factory for creating price models by name
 * 
 * Centralizes model instantiation and parameter configuration.
 */
class PriceModelFactory {
public:
    /**
     * @brief Create price model from configuration
     * @param model_name Name of model ("linear", "gbm", etc.)
     * @param config Generation parameters
     * @param dt Simulated time step (fraction of year per step)
     * @return Unique pointer to price model
     * @throws std::invalid_argument if model name is unknown
     */
    static std::unique_ptr<IPriceModel> create(
        const std::string& model_name,
        const GenerationParameters& config,
        double dt
    );
    
    /**
     * @brief Get list of available model names
     * @return Comma-separated list of model names
     */
    static std::string available_models();
};

} // namespace marketsim::traffic_generator::models::price_models
