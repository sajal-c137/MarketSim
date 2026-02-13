#include "price_movement_calculator.h"

namespace marketsim::traffic_generator::operations {

PriceMovementCalculator::PriceMovementCalculator(double base_price, double rate_per_second)
    : base_price_(base_price)
    , rate_per_second_(rate_per_second)
{
}

double PriceMovementCalculator::calculate_price(double time_seconds) const {
    // Simple linear model: price = base_price + rate * time
    return base_price_ + (rate_per_second_ * time_seconds);
}

void PriceMovementCalculator::set_base_price(double base_price) {
    base_price_ = base_price;
}

void PriceMovementCalculator::set_rate(double rate_per_second) {
    rate_per_second_ = rate_per_second;
}

} // namespace marketsim::traffic_generator::operations
