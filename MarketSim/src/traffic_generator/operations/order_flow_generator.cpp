#include "order_flow_generator.h"
#include <sstream>

namespace marketsim::traffic_generator::operations {

OrderFlowGenerator::OrderFlowGenerator()
    : order_counter_(0)
{
}

std::vector<SyntheticOrder> OrderFlowGenerator::generate_orders(
    const std::string& symbol,
    double price,
    double quantity)
{
    std::vector<SyntheticOrder> orders;
    
    // Generate BUY order
    orders.emplace_back(
        generate_order_id(),
        symbol,
        true,  // is_buy
        quantity,
        price
    );
    
    // Generate SELL order
    orders.emplace_back(
        generate_order_id(),
        symbol,
        false, // is_sell
        quantity,
        price
    );
    
    return orders;
}

std::string OrderFlowGenerator::generate_order_id() {
    std::ostringstream oss;
    oss << "TG-" << ++order_counter_;
    return oss.str();
}

} // namespace marketsim::traffic_generator::operations
