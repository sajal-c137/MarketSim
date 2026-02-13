#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <memory>

namespace marketsim::exchange::operations {

    /**
     * @brief Represents a single order in the order book
     */
    struct OrderEntry {
        std::string order_id;
        std::string client_id;
        double price;
        double quantity;
        double filled_quantity;
        int64_t timestamp;

        OrderEntry(const std::string& id, const std::string& client, double p, double q, int64_t ts)
            : order_id(id), client_id(client), price(p), quantity(q),
            filled_quantity(0), timestamp(ts) {
        }

        double remaining_quantity() const {
            return quantity - filled_quantity;
        }
    };

    /**
     * @brief Single price level in order book
     */
    struct PriceLevel {
        double price;
        std::vector<OrderEntry> orders;  // FIFO queue at this price

        explicit PriceLevel(double p) : price(p) {}

        double total_quantity() const {
            double total = 0;
            for (const auto& order : orders) {
                total += order.remaining_quantity();
            }
            return total;
        }
    };

    /**
     * @brief Order Book for a single symbol
     * Maintains separate buy and sell sides
     */
    class OrderBook {
    public:
        explicit OrderBook(const std::string& symbol);

        // Add order
        void add_order(const OrderEntry& order, bool is_buy);

        // Cancel order
        bool cancel_order(const std::string& order_id, bool is_buy);

        // Get best bid/ask
        bool get_best_bid(double& price, double& quantity) const;
        bool get_best_ask(double& price, double& quantity) const;

        // Get order book snapshot
        std::vector<PriceLevel> get_buy_side(int depth = 10) const;
        std::vector<PriceLevel> get_sell_side(int depth = 10) const;

        // Statistics
        size_t total_buy_orders() const;
        size_t total_sell_orders() const;
        double total_buy_quantity() const;
        double total_sell_quantity() const;

        const std::string& get_symbol() const { return symbol_; }

        // Display order book in market depth format
        void print_depth(int depth = 10) const;

    private:
        std::string symbol_;

        // Buy side: price in descending order (highest bid first)
        std::map<double, PriceLevel, std::greater<double>> buy_side_;

        // Sell side: price in ascending order (lowest ask first)
        std::map<double, PriceLevel, std::less<double>> sell_side_;

        // Quick lookup for order cancellation
        std::unordered_map<std::string, std::pair<double, bool>> order_price_map_;
    };

}