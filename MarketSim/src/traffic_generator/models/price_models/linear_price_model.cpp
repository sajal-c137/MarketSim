#include "linear_price_model.h"

namespace marketsim::traffic_generator::models::price_models {

LinearPriceModel::LinearPriceModel(double base_price, double rate_per_step)
    : base_price_(base_price)
    , rate_per_step_(rate_per_step)
    , current_price_(base_price)
    , step_count_(0)
{
}

double LinearPriceModel::next_price() {
    step_count_++;
    current_price_ = base_price_ + (rate_per_step_ * step_count_);
    return current_price_;
}

double LinearPriceModel::current_price() const {
    return current_price_;
}

void LinearPriceModel::reset() {
    current_price_ = base_price_;
    step_count_ = 0;
}

} // namespace marketsim::traffic_generator::models::price_models
