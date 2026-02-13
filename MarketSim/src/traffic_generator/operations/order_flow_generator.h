#pragma once

#include <string>
#include <vector>

namespace marketsim::traffic_generator::operations {

/**
 * @brief Represents a synthetic order to be sent to Exchange
 */
struct SyntheticOrder {
    std::string order_id;
    std::string symbol;
    bool is_buy;          // true = BUY, false = SELL
    double quantity;
    double price;
    
    SyntheticOrder(const std::string& id, const std::string& sym, 
                   bool buy, double qty, double prc)
        : order_id(id), symbol(sym), is_buy(buy), quantity(qty), price(prc)
    {}
};

/**
 * @brief Generates synthetic orders based on current price
 * 
 * Pure math - no I/O, threading, or side effects.
 * Simply creates order structures with given parameters.
 */
class OrderFlowGenerator {
public:
    OrderFlowGenerator();
    
    /**
     * @brief Generate one buy and one sell order at given price
     * @param symbol Trading symbol (e.g., "AAPL")
     * @param price Price for both orders
     * @param quantity Quantity for both orders
     * @return Vector containing buy and sell orders
     */
    std::vector<SyntheticOrder> generate_orders(
        const std::string& symbol,
        double price,
        double quantity = 1.0
    );
    
private:
    int order_counter_;
    
    std::string generate_order_id();
};

} // namespace marketsim::traffic_generator::operations
