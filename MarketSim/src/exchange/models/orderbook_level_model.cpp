#include "orderbook_level_model.h"
#include "order_model.h"
#include <algorithm>

namespace marketsim::exchange::models {

void PriceLevel::add_order(Order* order) {
    if (!order) {
        return;
    }
    
    orders[order->order_id] = order;
    order_sequence.push_back(order->order_id);
    
    total_quantity += order->remaining_quantity();
    order_count++;
}

void PriceLevel::remove_order(const std::string& order_id) {
    auto it = orders.find(order_id);
    if (it == orders.end()) {
        return;
    }
    
    Order* order = it->second;
    total_quantity -= order->remaining_quantity();
    order_count--;
    
    orders.erase(it);
    
    auto seq_it = std::find(order_sequence.begin(), order_sequence.end(), order_id);
    if (seq_it != order_sequence.end()) {
        order_sequence.erase(seq_it);
    }
}

Order* PriceLevel::get_order(const std::string& order_id) const {
    auto it = orders.find(order_id);
    return it != orders.end() ? it->second : nullptr;
}

Order* PriceLevel::get_front_order() const {
    while (!order_sequence.empty()) {
        const std::string& front_id = order_sequence.front();
        auto it = orders.find(front_id);
        
        if (it != orders.end()) {
            return it->second;
        }
        
        const_cast<std::deque<std::string>&>(order_sequence).pop_front();
    }
    
    return nullptr;
}

}
