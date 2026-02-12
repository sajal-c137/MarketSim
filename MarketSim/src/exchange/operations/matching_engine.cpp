#include "matching_engine.h"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace marketsim::exchange::operations {

    MatchingEngine::MatchingEngine(const std::string& symbol)
        : order_book_(symbol)
        , trade_count_(0)
        , total_volume_(0)
        , trade_id_counter_(0)
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

        auto sell_side = order_book_.get_sell_side(10);

        // Match against sell side (lowest ask first)
        for (auto& sell_level : sell_side) {
            if (buy_order.type() == marketsim::exchange::OrderType::LIMIT &&
                sell_level.price > buy_order.price()) {
                break;  // No match at this price
            }

            // Match against orders at this price level (FIFO)
            for (auto& sell_order : sell_level.orders) {
                if (ctx.remaining_quantity <= 0) break;

                double fill_qty = std::min(ctx.remaining_quantity, sell_order.remaining_quantity());
                execute_fill(sell_order, buy_order, fill_qty, sell_level.price, ctx.trades);

                ctx.remaining_quantity -= fill_qty;
                total_filled_value += fill_qty * sell_level.price;
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

        auto buy_side = order_book_.get_buy_side(10);

        // Match against buy side (highest bid first)
        for (auto& buy_level : buy_side) {
            if (sell_order.type() == marketsim::exchange::OrderType::LIMIT &&
                buy_level.price < sell_order.price()) {
                break;  // No match at this price
            }

            // Match against orders at this price level (FIFO)
            for (auto& buy_order : buy_level.orders) {
                if (ctx.remaining_quantity <= 0) break;

                double fill_qty = std::min(ctx.remaining_quantity, buy_order.remaining_quantity());
                execute_fill(buy_order, sell_order, fill_qty, buy_level.price, ctx.trades);

                ctx.remaining_quantity -= fill_qty;
                total_filled_value += fill_qty * buy_level.price;
            }
        }

        if (sell_order.quantity() - ctx.remaining_quantity > 0) {
            ctx.average_price = total_filled_value / (sell_order.quantity() - ctx.remaining_quantity);
        }

        return ctx;
    }

    void MatchingEngine::execute_fill(OrderEntry& existing_order, const marketsim::exchange::Order& incoming,
        double fill_quantity, double fill_price,
        std::vector<marketsim::exchange::Trade>& trades) {
        existing_order.filled_quantity += fill_quantity;

        // Create trade record
        marketsim::exchange::Trade trade;
        trade.set_trade_id(generate_trade_id());
        trade.set_symbol(order_book_.get_symbol());
        trade.set_price(fill_price);
        trade.set_quantity(fill_quantity);
        trade.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());

        if (incoming.side() == marketsim::exchange::OrderSide::BUY) {
            trade.set_aggressor_side(marketsim::exchange::OrderSide::BUY);
            trade.set_buyer_order_id(incoming.order_id());
            trade.set_seller_order_id(existing_order.order_id);
        }
        else {
            trade.set_aggressor_side(marketsim::exchange::OrderSide::SELL);
            trade.set_buyer_order_id(existing_order.order_id);
            trade.set_seller_order_id(incoming.order_id());
        }

        trades.push_back(trade);
    }

    std::string MatchingEngine::generate_trade_id() {
        std::ostringstream oss;
        oss << "TRD_" << std::setfill('0') << std::setw(10) << (++trade_id_counter_);
        return oss.str();
    }

}