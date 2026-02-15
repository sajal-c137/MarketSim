#include "gbm_price_model.h"

namespace marketsim::traffic_generator::models::price_models {

GBMPriceModel::GBMPriceModel(
    double initial_price,
    double drift,
    double volatility,
    double dt,
    uint64_t seed)
{
    generator_ = std::make_unique<operations::GBMPriceGenerator>(
        initial_price,
        drift,
        volatility,
        dt,
        seed
    );
}

double GBMPriceModel::next_price() {
    return generator_->next_price();
}

double GBMPriceModel::current_price() const {
    return generator_->current_price();
}

void GBMPriceModel::reset() {
    generator_->reset();
}

} // namespace marketsim::traffic_generator::models::price_models
