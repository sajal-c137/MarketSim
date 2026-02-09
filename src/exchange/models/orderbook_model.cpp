#include "orderbook_model.h"
#include <algorithm>

namespace marketsim::exchange::models {

void OrderBookModel::add_order(Order* order) {
    if (!order || order->is_market()) {
        return;
    }
    
    bool is_bid = order->is_buy();
    auto& levels = is_bid ? bid_levels_ : ask_levels_;
    
    auto [it, inserted] = levels.try_emplace(order->price, order->price);
    it->second.add_order(order);
    
    if (inserted) {
        if (is_bid) {
            active_bid_prices_.insert(order->price);
            update_best_bid();
        } else {
            active_ask_prices_.insert(order->price);
            update_best_ask();
        }
    }
    
    order_price_index_[order->order_id] = {order->price, is_bid};
}

void OrderBookModel::remove_order(const std::string& order_id, double price, bool is_bid) {
    auto& levels = is_bid ? bid_levels_ : ask_levels_;
    auto it = levels.find(price);
    
    if (it != levels.end()) {
        it->second.remove_order(order_id);
        
        if (it->second.is_empty()) {
            remove_level_if_empty(price, is_bid);
        }
    }
    
    order_price_index_.erase(order_id);
}

void OrderBookModel::update_order_quantity(const std::string& order_id, double price, bool is_bid, double new_quantity) {
    Order* order = find_order(order_id, price, is_bid);
    if (!order) {
        return;
    }
    
    auto& levels = is_bid ? bid_levels_ : ask_levels_;
    auto it = levels.find(price);
    
    if (it != levels.end()) {
        double quantity_delta = new_quantity - order->quantity;
        order->quantity = new_quantity;
        it->second.total_quantity += quantity_delta;
        
        if (it->second.is_empty()) {
            remove_level_if_empty(price, is_bid);
        }
    }
}

Order* OrderBookModel::find_order(const std::string& order_id, double price, bool is_bid) const {
    const auto& levels = is_bid ? bid_levels_ : ask_levels_;
    auto it = levels.find(price);
    
    if (it != levels.end()) {
        return it->second.get_order(order_id);
    }
    
    return nullptr;
}

Order* OrderBookModel::find_order_any_price(const std::string& order_id) const {
    auto it = order_price_index_.find(order_id);
    if (it == order_price_index_.end()) {
        return nullptr;
    }
    
    return find_order(order_id, it->second.first, it->second.second);
}

PriceLevel* OrderBookModel::get_level(double price, bool is_bid) {
    auto& levels = is_bid ? bid_levels_ : ask_levels_;
    auto it = levels.find(price);
    return it != levels.end() ? &it->second : nullptr;
}

const PriceLevel* OrderBookModel::get_level(double price, bool is_bid) const {
    const auto& levels = is_bid ? bid_levels_ : ask_levels_;
    auto it = levels.find(price);
    return it != levels.end() ? &it->second : nullptr;
}

PriceLevel* OrderBookModel::get_best_bid_level() {
    return best_bid_price_ > 0.0 ? get_level(best_bid_price_, true) : nullptr;
}

PriceLevel* OrderBookModel::get_best_ask_level() {
    return best_ask_price_ > 0.0 ? get_level(best_ask_price_, false) : nullptr;
}

std::vector<OrderBookLevel> OrderBookModel::get_bids(size_t depth) const {
    std::vector<OrderBookLevel> result;
    result.reserve(std::min(depth, active_bid_prices_.size()));
    
    size_t count = 0;
    for (double price : active_bid_prices_) {
        if (count >= depth) break;
        
        auto it = bid_levels_.find(price);
        if (it != bid_levels_.end()) {
            result.emplace_back(price, it->second.total_quantity, it->second.order_count);
        }
        count++;
    }
    
    return result;
}

std::vector<OrderBookLevel> OrderBookModel::get_asks(size_t depth) const {
    std::vector<OrderBookLevel> result;
    result.reserve(std::min(depth, active_ask_prices_.size()));
    
    size_t count = 0;
    for (double price : active_ask_prices_) {
        if (count >= depth) break;
        
        auto it = ask_levels_.find(price);
        if (it != ask_levels_.end()) {
            result.emplace_back(price, it->second.total_quantity, it->second.order_count);
        }
        count++;
    }
    
    return result;
}

size_t OrderBookModel::total_order_count() const {
    return order_price_index_.size();
}

void OrderBookModel::update_best_bid() {
    if (!active_bid_prices_.empty()) {
        best_bid_price_ = *active_bid_prices_.begin();
    } else {
        best_bid_price_ = 0.0;
    }
}

void OrderBookModel::update_best_ask() {
    if (!active_ask_prices_.empty()) {
        best_ask_price_ = *active_ask_prices_.begin();
    } else {
        best_ask_price_ = 0.0;
    }
}

void OrderBookModel::remove_level_if_empty(double price, bool is_bid) {
    auto& levels = is_bid ? bid_levels_ : ask_levels_;
    
    auto it = levels.find(price);
    if (it != levels.end() && it->second.is_empty()) {
        levels.erase(it);
        
        if (is_bid) {
            active_bid_prices_.erase(price);
            update_best_bid();
        } else {
            active_ask_prices_.erase(price);
            update_best_ask();
        }
    }
}

}
