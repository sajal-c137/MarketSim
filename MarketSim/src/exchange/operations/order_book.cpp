#include "order_book.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace marketsim::exchange::operations {

    OrderBook::OrderBook(const std::string& symbol)
        : symbol_(symbol)
    {
    }

    void OrderBook::add_order(const OrderEntry& order, bool is_buy) {
        if (is_buy) {
            auto it = buy_side_.find(order.price);
            if (it == buy_side_.end()) {
                buy_side_.emplace(order.price, PriceLevel(order.price));
            }
            buy_side_.at(order.price).orders.push_back(order);
        }
        else {
            auto it = sell_side_.find(order.price);
            if (it == sell_side_.end()) {
                sell_side_.emplace(order.price, PriceLevel(order.price));
            }
            sell_side_.at(order.price).orders.push_back(order);
        }

        order_price_map_[order.order_id] = { order.price, is_buy };
    }

    bool OrderBook::cancel_order(const std::string& order_id, bool is_buy) {
        auto it = order_price_map_.find(order_id);
        if (it == order_price_map_.end()) {
            return false;
        }

        double price = it->second.first;

        if (is_buy) {
            auto level_it = buy_side_.find(price);
            if (level_it != buy_side_.end()) {
                auto& orders = level_it->second.orders;
                auto order_it = std::find_if(orders.begin(), orders.end(),
                    [&order_id](const OrderEntry& o) { return o.order_id == order_id; });

                if (order_it != orders.end()) {
                    orders.erase(order_it);
                    order_price_map_.erase(it);

                    if (orders.empty()) {
                        buy_side_.erase(level_it);
                    }
                    return true;
                }
            }
        }
        else {
            auto level_it = sell_side_.find(price);
            if (level_it != sell_side_.end()) {
                auto& orders = level_it->second.orders;
                auto order_it = std::find_if(orders.begin(), orders.end(),
                    [&order_id](const OrderEntry& o) { return o.order_id == order_id; });

                if (order_it != orders.end()) {
                    orders.erase(order_it);
                    order_price_map_.erase(it);

                    if (orders.empty()) {
                        sell_side_.erase(level_it);
                    }
                    return true;
                }
            }
        }

        return false;
    }

    bool OrderBook::get_best_bid(double& price, double& quantity) const {
        if (buy_side_.empty()) {
            return false;
        }

        const auto& level = buy_side_.begin()->second;
        price = level.price;
        quantity = level.total_quantity();
        return true;
    }

    bool OrderBook::get_best_ask(double& price, double& quantity) const {
        if (sell_side_.empty()) {
            return false;
        }

        const auto& level = sell_side_.begin()->second;
        price = level.price;
        quantity = level.total_quantity();
        return true;
    }

    std::vector<PriceLevel> OrderBook::get_buy_side(int depth) const {
        std::vector<PriceLevel> result;
        int count = 0;

        for (const auto& [price, level] : buy_side_) {
            if (count++ >= depth) break;
            result.push_back(level);
        }

        return result;
    }

    std::vector<PriceLevel> OrderBook::get_sell_side(int depth) const {
        std::vector<PriceLevel> result;
        int count = 0;

        for (const auto& [price, level] : sell_side_) {
            if (count++ >= depth) break;
            result.push_back(level);
        }

        return result;
    }

    size_t OrderBook::total_buy_orders() const {
        size_t total = 0;
        for (const auto& [price, level] : buy_side_) {
            total += level.orders.size();
        }
        return total;
    }

    size_t OrderBook::total_sell_orders() const {
        size_t total = 0;
        for (const auto& [price, level] : sell_side_) {
            total += level.orders.size();
        }
        return total;
    }

    double OrderBook::total_buy_quantity() const {
        double total = 0;
        for (const auto& [price, level] : buy_side_) {
            total += level.total_quantity();
        }
        return total;
    }

    double OrderBook::total_sell_quantity() const {
        double total = 0;
        for (const auto& [price, level] : sell_side_) {
            total += level.total_quantity();
        }
        return total;
    }

    void OrderBook::print_depth(int depth) const {
        // Get both sides
        auto buy_levels = get_buy_side(depth);
        auto sell_levels = get_sell_side(depth);

        // Print header
        std::cout << symbol_ << " Order Book\n";
        std::cout << "Bid\t\t\tAsk\n";
        std::cout << std::fixed << std::setprecision(2);

        size_t max_levels = std::max(buy_levels.size(), sell_levels.size());

        // Print each level
        for (size_t i = 0; i < max_levels; ++i) {
            // Bid side
            if (i < buy_levels.size()) {
                std::cout << buy_levels[i].price << "\t" << static_cast<int>(buy_levels[i].total_quantity());
            }
            else {
                std::cout << "\t";
            }

            // Spacing between bid and ask
            std::cout << "\t\t";

            // Ask side
            if (i < sell_levels.size()) {
                std::cout << sell_levels[i].price << "\t" << static_cast<int>(sell_levels[i].total_quantity());
            }

            std::cout << "\n";
        }
        std::cout << std::endl;
    }

}