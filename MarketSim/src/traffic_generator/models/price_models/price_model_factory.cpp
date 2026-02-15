#include "price_model_factory.h"
#include "linear_price_model.h"
#include "gbm_price_model.h"
#include <algorithm>

namespace marketsim::traffic_generator::models::price_models {

std::unique_ptr<IPriceModel> PriceModelFactory::create(
    const std::string& model_name,
    const GenerationParameters& config,
    double dt)
{
    // Convert to lowercase for case-insensitive comparison
    std::string name_lower = model_name;
    std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    if (name_lower == "linear") {
        // For linear model, rate_per_step can be derived from config
        double rate_per_step = config.price_rate * dt;
        return std::make_unique<LinearPriceModel>(
            config.base_price,
            rate_per_step
        );
    }
    else if (name_lower == "gbm") {
        // Convert percentage to decimal
        double drift_decimal = config.drift / 100.0;
        double volatility_decimal = config.volatility / 100.0;
        
        return std::make_unique<GBMPriceModel>(
            config.base_price,
            drift_decimal,
            volatility_decimal,
            dt,
            0  // seed = 0 for random
        );
    }
    else {
        throw std::invalid_argument(
            "Unknown price model: '" + model_name + "'. " +
            "Available models: " + available_models()
        );
    }
}

std::string PriceModelFactory::available_models() {
    return "linear, gbm";
}

} // namespace marketsim::traffic_generator::models::price_models
