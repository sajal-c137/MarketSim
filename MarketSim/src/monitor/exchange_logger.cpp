#include "exchange_logger.h"
#include <iostream>
#include <iomanip>

namespace marketsim::monitor {

void ExchangeLogger::log_order_received(
    int order_count,
    const exchange::Order& order)
{
    std::cout << "[ORDER_RCV] #" << order_count << " "
              << order.order_id() << " "
              << (order.side() == 1 ? "BUY " : "SELL") << " "
              << order.quantity() << "@"
              << std::fixed << std::setprecision(2) << order.price() << "\n";
}

void ExchangeLogger::log_matching_result(
    const std::string& order_id,
    const exchange::operations::MatchResult& result)
{
    if (result.success) {
        if (result.trades.empty()) {
            std::cout << "[MATCHING] " << order_id << " -> ADDED (no match)\n";
        } else {
            std::cout << "[MATCHING] " << order_id << " -> MATCHED "
                      << result.trades.size() << " trades, "
                      << result.executed_quantity << "@"
                      << std::fixed << std::setprecision(2) << result.execution_price << "\n";
        }
    } else {
        std::cout << "[MATCHING] " << order_id << " -> ERROR: "
                  << result.error_message << "\n";
    }
}

void ExchangeLogger::log_price_update(
    double last_price,
    size_t total_trades,
    double total_volume)
{
    std::cout << "[PRICE] Last=$" << std::fixed << std::setprecision(2)
              << last_price
              << " Trades=" << total_trades
              << " Volume=" << total_volume << "\n";
}

void ExchangeLogger::log_orderbook(
    const exchange::operations::OrderBook& order_book,
    int depth)
{
    auto buy_levels = order_book.get_buy_side(depth);
    auto sell_levels = order_book.get_sell_side(depth);
    
    std::cout << "[BOOK] BUYS=";
    if (buy_levels.empty()) {
        std::cout << "empty";
    } else {
        for (size_t i = 0; i < buy_levels.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << buy_levels[i].price << ":" << buy_levels[i].total_quantity();
        }
    }
    
    std::cout << " | SELLS=";
    if (sell_levels.empty()) {
        std::cout << "empty";
    } else {
        for (size_t i = 0; i < sell_levels.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << sell_levels[i].price << ":" << sell_levels[i].total_quantity();
        }
    }
    std::cout << "\n\n";
}

void ExchangeLogger::print_startup_header() {
    std::cout << "========================================\n";
    std::cout << "EXCHANGE SERVICE\n";
    std::cout << "========================================\n\n";
    std::cout << "Log prefixes:\n";
    std::cout << "  [ORDER_RCV] - Orders received\n";
    std::cout << "  [MATCHING]  - Matching results\n";
    std::cout << "  [PRICE]     - Price updates\n";
    std::cout << "  [BOOK]      - Orderbook state\n\n";
}

} // namespace marketsim::monitor
