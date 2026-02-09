#pragma once

#include <unordered_map>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include "order_model.h"
#include "orderbook_level_model.h"

namespace marketsim::exchange::models {

class OrderBookModel {
public:
    OrderBookModel() 
        : best_bid_price_(0.0)
        , best_ask_price_(0.0)
    {}
    
    ~OrderBookModel() = default;
    
    OrderBookModel(const OrderBookModel&) = delete;
    OrderBookModel& operator=(const OrderBookModel&) = delete;
    OrderBookModel(OrderBookModel&&) = default;
    OrderBookModel& operator=(OrderBookModel&&) = default;
    
    void add_order(Order* order);
    void remove_order(const std::string& order_id, double price, bool is_bid);
    void update_order_quantity(const std::string& order_id, double price, bool is_bid, double new_quantity);
    
    Order* find_order(const std::string& order_id, double price, bool is_bid) const;
    Order* find_order_any_price(const std::string& order_id) const;
    
    PriceLevel* get_level(double price, bool is_bid);
    const PriceLevel* get_level(double price, bool is_bid) const;
    
    PriceLevel* get_best_bid_level();
    PriceLevel* get_best_ask_level();
    
    double get_best_bid_price() const { return best_bid_price_; }
    double get_best_ask_price() const { return best_ask_price_; }
    double get_spread() const { return best_ask_price_ - best_bid_price_; }
    double get_mid_price() const { return (best_bid_price_ + best_ask_price_) / 2.0; }
    
    std::vector<OrderBookLevel> get_bids(size_t depth) const;
    std::vector<OrderBookLevel> get_asks(size_t depth) const;
    
    size_t bid_level_count() const { return bid_levels_.size(); }
    size_t ask_level_count() const { return ask_levels_.size(); }
    size_t total_order_count() const;
    
private:
    void update_best_bid();
    void update_best_ask();
    void remove_level_if_empty(double price, bool is_bid);
    
    std::unordered_map<double, PriceLevel> bid_levels_;
    std::unordered_map<double, PriceLevel> ask_levels_;
    
    std::set<double, std::greater<double>> active_bid_prices_;
    std::set<double, std::less<double>> active_ask_prices_;
    
    double best_bid_price_;
    double best_ask_price_;
    
    std::unordered_map<std::string, std::pair<double, bool>> order_price_index_;
};

}
