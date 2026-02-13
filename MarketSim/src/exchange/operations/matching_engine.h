#pragma once

#include "order_book.h"
#include "exchange/data/price_history.h"
#include "exchange.pb.h"
#include <vector>
#include <string>
#include <cstdint>

namespace marketsim::exchange::operations {

    /**
     * @brief Result of matching a single order
     */
    struct MatchResult {
        std::string trade_id;
        double executed_quantity;
        double execution_price;
        std::vector<marketsim::exchange::Trade> trades;
        std::string error_message;
        bool success;

        MatchResult() : executed_quantity(0), execution_price(0), success(false) {}
    };

    /**
     * @brief Core matching engine
     * Implements price-time priority matching (FIFO at same price)
     */
    class MatchingEngine {
    public:
        explicit MatchingEngine(const std::string& symbol, size_t price_history_size = 100);

        // Submit order for matching
        MatchResult match_order(const marketsim::exchange::Order& order);

        // Cancel existing order
        bool cancel_order(const std::string& order_id, const std::string& symbol);

        // Get order book
        const OrderBook& get_order_book() const { return order_book_; }

        // Statistics
        size_t total_trades() const { return trade_count_; }
        double total_volume() const { return total_volume_; }
        
        // Price tracking
        const data::PriceHistory& get_trade_price_history() const { return trade_price_history_; }
        const data::PriceHistory& get_mid_price_history() const { return mid_price_history_; }
        
        bool get_last_trade_price(data::PriceTick& tick) const {
            return trade_price_history_.get_last(tick);
        }
        
        bool get_last_mid_price(data::PriceTick& tick) const {
            return mid_price_history_.get_last(tick);
        }

    private:
        struct TradeExecutionContext {
            std::vector<marketsim::exchange::Trade> trades;
            double remaining_quantity;
            double average_price;
        };

        // Match buy order against sell side
        TradeExecutionContext match_buy_order(const marketsim::exchange::Order& buy_order);

        // Match sell order against buy side
        TradeExecutionContext match_sell_order(const marketsim::exchange::Order& sell_order);

        // Generate unique trade ID
        std::string generate_trade_id();
        
        // Update mid price after order book changes
        void update_mid_price();

        OrderBook order_book_;
        size_t trade_count_;
        double total_volume_;
        int64_t trade_id_counter_;
        
        // Price tracking
        data::PriceHistory trade_price_history_;
        data::PriceHistory mid_price_history_;
    };

}