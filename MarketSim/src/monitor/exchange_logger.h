#pragma once

#include "exchange/operations/matching_engine.h"
#include "exchange/operations/order_book.h"
#include "exchange.pb.h"
#include <string>

namespace marketsim::monitor {

/**
 * @brief Centralized logging for Exchange operations
 * 
 * All log formatting and patterns are defined here.
 * Services just call these functions with data.
 */
class ExchangeLogger {
public:
    // Log received order
    static void log_order_received(
        int order_count,
        const exchange::Order& order
    );
    
    // Log matching results
    static void log_matching_result(
        const std::string& order_id,
        const exchange::operations::MatchResult& result
    );
    
    // Log price update from trades
    static void log_price_update(
        double last_price,
        size_t total_trades,
        double total_volume
    );
    
    // Log orderbook snapshot
    static void log_orderbook(
        const exchange::operations::OrderBook& order_book,
        int depth = 3
    );
    
    // Print header on startup
    static void print_startup_header();
};

} // namespace marketsim::monitor
