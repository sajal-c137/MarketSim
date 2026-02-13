#include "matching_engine.h"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace marketsim::exchange::operations {

MatchingEngine::MatchingEngine(const std::string& symbol, size_t price_history_size)
    : order_book_(symbol)
    , trade_count_(0)
    , total_volume_(0)
    , trade_id_counter_(0)
    , trade_price_history_(price_history_size)
    , mid_price_history_(price_history_size)
{
}

    MatchResult MatchingEngine::match_order(const marketsim::exchange::Order& order) {
        MatchResult result;
        result.success = false;

        if (order.symbol() != order_book_.get_symbol()) {
            result.error_message = "Symbol mismatch";
            return result;
        }

        if (order.type() == marketsim::exchange::OrderType::MARKET) {
            // Market order: execute immediately against best prices
            TradeExecutionContext ctx;

            if (order.side() == marketsim::exchange::OrderSide::BUY) {
                ctx = match_buy_order(order);
            }
            else {
                ctx = match_sell_order(order);
            }

            if (ctx.remaining_quantity > 0) {
                // Partial fill - add remaining as limit order
                OrderEntry limit_order(order.order_id(), order.client_id(),
                    order.price() == 0 ? ctx.average_price : order.price(),
                    order.quantity(), order.timestamp());
                limit_order.filled_quantity = order.quantity() - ctx.remaining_quantity;

                bool is_buy = (order.side() == marketsim::exchange::OrderSide::BUY);
                order_book_.add_order(limit_order, is_buy);
            }

            result.trades = ctx.trades;
            result.executed_quantity = order.quantity() - ctx.remaining_quantity;
            result.execution_price = ctx.average_price;

        }
        else {
            // Limit order: match what we can, add rest to book
            TradeExecutionContext ctx;

            if (order.side() == marketsim::exchange::OrderSide::BUY) {
                ctx = match_buy_order(order);
            }
            else {
                ctx = match_sell_order(order);
            }

            // Add remaining quantity to order book
            if (ctx.remaining_quantity > 0) {
                OrderEntry limit_order(order.order_id(), order.client_id(),
                    order.price(), order.quantity(), order.timestamp());
                limit_order.filled_quantity = order.quantity() - ctx.remaining_quantity;

                bool is_buy = (order.side() == marketsim::exchange::OrderSide::BUY);
                order_book_.add_order(limit_order, is_buy);
            }

            result.trades = ctx.trades;
            result.executed_quantity = order.quantity() - ctx.remaining_quantity;
            result.execution_price = ctx.average_price;
        }

        // Update statistics
        trade_count_ += result.trades.size();
        total_volume_ += result.executed_quantity;
        
        // Update mid price after order book changes
        update_mid_price();

        result.success = true;
        return result;
    }

    bool MatchingEngine::cancel_order(const std::string& order_id, const std::string& symbol) {
        if (symbol != order_book_.get_symbol()) {
            return false;
        }

        // Try to cancel from buy side
        if (order_book_.cancel_order(order_id, true)) {
            return true;
        }

        // Try to cancel from sell side
        return order_book_.cancel_order(order_id, false);
    }

    MatchingEngine::TradeExecutionContext MatchingEngine::match_buy_order(const marketsim::exchange::Order& buy_order) {
        TradeExecutionContext ctx;
        ctx.remaining_quantity = buy_order.quantity();
        ctx.average_price = 0;
        double total_filled_value = 0;

        // Match against sell side (lowest ask first) - DIRECT ACCESS, not copies
        while (ctx.remaining_quantity > 0) {
            double best_ask_price = 0;
            double best_ask_qty = 0;
            
            if (!order_book_.get_best_ask(best_ask_price, best_ask_qty)) {
                break;  // No more sellers
            }

            // Check if price is acceptable for limit order
            if (buy_order.type() == marketsim::exchange::OrderType::LIMIT &&
                best_ask_price > buy_order.price()) {
                break;  // Price too high
            }

            // Get the actual sell-side map and modify it directly
            auto& sell_side_map = order_book_.get_sell_side_map();
            auto level_it = sell_side_map.find(best_ask_price);
            
            if (level_it == sell_side_map.end()) {
                break;  // Should not happen
            }

            auto& orders = level_it->second.orders;
            
            // Process orders at this price level (FIFO)
            while (!orders.empty() && ctx.remaining_quantity > 0) {
                auto& sell_order = orders.front();
                
                double fill_qty = std::min(ctx.remaining_quantity, sell_order.remaining_quantity());
                
                // Create trade
                marketsim::exchange::Trade trade;
                trade.set_trade_id(generate_trade_id());
                trade.set_symbol(order_book_.get_symbol());
                trade.set_price(best_ask_price);
                trade.set_quantity(fill_qty);
                trade.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
                trade.set_aggressor_side(marketsim::exchange::OrderSide::BUY);
                trade.set_buyer_order_id(buy_order.order_id());
                trade.set_seller_order_id(sell_order.order_id);
                ctx.trades.push_back(trade);

                // Record trade price in history
                int64_t now = data::PriceTick::now_ms();
                trade_price_history_.add(best_ask_price, now);

                // Update filled quantity
                sell_order.filled_quantity += fill_qty;
                ctx.remaining_quantity -= fill_qty;
                total_filled_value += fill_qty * best_ask_price;

                // Remove if fully filled
                if (sell_order.remaining_quantity() <= 0) {
                    order_book_.remove_order_from_map(sell_order.order_id);
                    orders.erase(orders.begin());
                }
            }

            // Remove empty price level
            if (orders.empty()) {
                sell_side_map.erase(level_it);
            }
        }

        if (buy_order.quantity() - ctx.remaining_quantity > 0) {
            ctx.average_price = total_filled_value / (buy_order.quantity() - ctx.remaining_quantity);
        }

        return ctx;
    }

    MatchingEngine::TradeExecutionContext MatchingEngine::match_sell_order(const marketsim::exchange::Order& sell_order) {
        TradeExecutionContext ctx;
        ctx.remaining_quantity = sell_order.quantity();
        ctx.average_price = 0;
        double total_filled_value = 0;

        // Match against buy side (highest bid first) - DIRECT ACCESS, not copies
        while (ctx.remaining_quantity > 0) {
            double best_bid_price = 0;
            double best_bid_qty = 0;
            
            if (!order_book_.get_best_bid(best_bid_price, best_bid_qty)) {
                break;  // No more buyers
            }

            // Check if price is acceptable for limit order
            if (sell_order.type() == marketsim::exchange::OrderType::LIMIT &&
                best_bid_price < sell_order.price()) {
                break;  // Price too low
            }

            // Get the actual buy-side map and modify it directly
            auto& buy_side_map = order_book_.get_buy_side_map();
            auto level_it = buy_side_map.find(best_bid_price);
            
            if (level_it == buy_side_map.end()) {
                break;  // Should not happen
            }

            auto& orders = level_it->second.orders;
            
            // Process orders at this price level (FIFO)
            while (!orders.empty() && ctx.remaining_quantity > 0) {
                auto& buy_order = orders.front();
                
                double fill_qty = std::min(ctx.remaining_quantity, buy_order.remaining_quantity());
                
                // Create trade
                marketsim::exchange::Trade trade;
                trade.set_trade_id(generate_trade_id());
                trade.set_symbol(order_book_.get_symbol());
                trade.set_price(best_bid_price);
                trade.set_quantity(fill_qty);
                trade.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
                trade.set_aggressor_side(marketsim::exchange::OrderSide::SELL);
                trade.set_buyer_order_id(buy_order.order_id);
                trade.set_seller_order_id(sell_order.order_id());
                ctx.trades.push_back(trade);

                // Record trade price in history
                int64_t now = data::PriceTick::now_ms();
                trade_price_history_.add(best_bid_price, now);

                // Update filled quantity
                buy_order.filled_quantity += fill_qty;
                ctx.remaining_quantity -= fill_qty;
                total_filled_value += fill_qty * best_bid_price;

                // Remove if fully filled
                if (buy_order.remaining_quantity() <= 0) {
                    order_book_.remove_order_from_map(buy_order.order_id);
                    orders.erase(orders.begin());
                }
            }

            // Remove empty price level
            if (orders.empty()) {
                buy_side_map.erase(level_it);
            }
        }

        if (sell_order.quantity() - ctx.remaining_quantity > 0) {
            ctx.average_price = total_filled_value / (sell_order.quantity() - ctx.remaining_quantity);
        }

        return ctx;
    }

    std::string MatchingEngine::generate_trade_id() {
        std::ostringstream oss;
        oss << "TRD_" << std::setfill('0') << std::setw(10) << (++trade_id_counter_);
        return oss.str();
    }
    
    void MatchingEngine::update_mid_price() {
        double best_bid_price = 0, best_bid_qty = 0;
        double best_ask_price = 0, best_ask_qty = 0;
        
        bool has_bid = order_book_.get_best_bid(best_bid_price, best_bid_qty);
        bool has_ask = order_book_.get_best_ask(best_ask_price, best_ask_qty);
        
        if (has_bid && has_ask) {
            // Both sides exist - calculate mid price
            double mid_price = (best_bid_price + best_ask_price) / 2.0;
            int64_t now = data::PriceTick::now_ms();
            mid_price_history_.add(mid_price, now);
        } else if (has_bid) {
            // Only bid side - use bid as mid
            int64_t now = data::PriceTick::now_ms();
            mid_price_history_.add(best_bid_price, now);
        } else if (has_ask) {
            // Only ask side - use ask as mid
            int64_t now = data::PriceTick::now_ms();
            mid_price_history_.add(best_ask_price, now);
        }
        // If neither exists, don't add anything
    }

} // namespace marketsim::exchange::operations

