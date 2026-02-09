#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <deque>

namespace marketsim::exchange::models {

struct Order;

struct PriceLevel {
    double price;
    double total_quantity;
    uint32_t order_count;
    
    std::unordered_map<std::string, Order*> orders;
    std::deque<std::string> order_sequence;
    
    PriceLevel()
        : price(0.0)
        , total_quantity(0.0)
        , order_count(0)
    {}
    
    explicit PriceLevel(double p)
        : price(p)
        , total_quantity(0.0)
        , order_count(0)
    {}
    
    void add_order(Order* order);
    void remove_order(const std::string& order_id);
    Order* get_order(const std::string& order_id) const;
    Order* get_front_order() const;
    
    bool is_empty() const {
        return orders.empty() || order_count == 0 || total_quantity <= 0.0;
    }
    
    double average_order_size() const {
        return order_count > 0 ? total_quantity / order_count : 0.0;
    }
};

struct OrderBookLevel {
    double price;
    double quantity;
    uint32_t order_count;
    
    OrderBookLevel()
        : price(0.0)
        , quantity(0.0)
        , order_count(0)
    {}
    
    OrderBookLevel(double p, double q, uint32_t count = 1)
        : price(p)
        , quantity(q)
        , order_count(count)
    {}
    
    void add_quantity(double qty) {
        quantity += qty;
        order_count++;
    }
    
    void remove_quantity(double qty) {
        quantity -= qty;
        if (order_count > 0) {
            order_count--;
        }
    }
    
    bool is_empty() const {
        return quantity <= 0.0 || order_count == 0;
    }
    
    double average_order_size() const {
        return order_count > 0 ? quantity / order_count : 0.0;
    }
};

}
