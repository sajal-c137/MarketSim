#include "exchange_logger.h"
#include <tabulate/table.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace marketsim::monitor {

void ExchangeLogger::clear_screen() {
    std::cout << "\033[2J\033[H" << std::flush;  // Clear screen + move cursor to home
}

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
    
    using namespace tabulate;
    
    // Clear screen and move cursor to top-left for refresh effect
    clear_screen();
    
    std::cout << "[ORDERBOOK] " << order_book.get_symbol() << " - Live Update\n";
    
    Table table;
    
    // Set table format
    table.format()
        .border_top("?")
        .border_bottom("?")
        .border_left("?")
        .border_right("?")
        .corner_top_left("?")
        .corner_top_right("?")
        .corner_bottom_left("?")
        .corner_bottom_right("?")
        .corner("?");
    
    // Add header row with styling
    table.add_row({"BIDS (Buy)", "", "", "ASKS (Sell)", "", ""});
    table[0].format()
        .font_color(Color::green)
        .font_align(FontAlign::center)
        .font_style({FontStyle::bold});
    
    // Add column headers
    table.add_row({"Price", "Quantity", "Orders", "Price", "Quantity", "Orders"});
    table[1].format()
        .font_color(Color::yellow)
        .font_align(FontAlign::center);
    
    // Determine max rows
    size_t max_rows = std::max(buy_levels.size(), sell_levels.size());
    
    // Add data rows
    for (size_t i = 0; i < max_rows; ++i) {
        std::string buy_price = "", buy_qty = "", buy_orders = "";
        std::string sell_price = "", sell_qty = "", sell_orders = "";
        
        // Buy side (left 3 columns)
        if (i < buy_levels.size()) {
            const auto& buy = buy_levels[i];
            std::ostringstream ps, qs, os;
            ps << std::fixed << std::setprecision(2) << "$" << buy.price;
            qs << std::fixed << std::setprecision(2) << buy.total_quantity();
            os << buy.orders.size();
            buy_price = ps.str();
            buy_qty = qs.str();
            buy_orders = os.str();
        }
        
        // Sell side (right 3 columns)
        if (i < sell_levels.size()) {
            const auto& sell = sell_levels[i];
            std::ostringstream ps, qs, os;
            ps << std::fixed << std::setprecision(2) << "$" << sell.price;
            qs << std::fixed << std::setprecision(2) << sell.total_quantity();
            os << sell.orders.size();
            sell_price = ps.str();
            sell_qty = qs.str();
            sell_orders = os.str();
        }
        
        table.add_row({buy_price, buy_qty, buy_orders, sell_price, sell_qty, sell_orders});
        
        // Color buy side green
        if (i < buy_levels.size()) {
            table[i + 2][0].format().font_color(Color::green);
            table[i + 2][1].format().font_color(Color::green);
            table[i + 2][2].format().font_color(Color::green);
        }
        
        // Color sell side red
        if (i < sell_levels.size()) {
            table[i + 2][3].format().font_color(Color::red);
            table[i + 2][4].format().font_color(Color::red);
            table[i + 2][5].format().font_color(Color::red);
        }
    }
    
    // Set column alignments
    for (size_t i = 0; i < table.size(); ++i) {
        if (i >= 2) {  // Data rows
            table[i][0].format().font_align(FontAlign::right);
            table[i][1].format().font_align(FontAlign::right);
            table[i][2].format().font_align(FontAlign::center);
            table[i][3].format().font_align(FontAlign::right);
            table[i][4].format().font_align(FontAlign::right);
            table[i][5].format().font_align(FontAlign::center);
        }
    }
    
    std::cout << table << "\n\n";
}

void ExchangeLogger::log_orderbook_pb(
    const marketsim::exchange::StatusResponse& status_response)
{
    using namespace tabulate;
    
    const auto& pb_orderbook = status_response.current_orderbook();
    
    // Clear screen and move cursor to top-left for refresh effect
    clear_screen();
    
    // Display header with symbol
    std::cout << "[ORDERBOOK] " << pb_orderbook.symbol() << " - Live Update\n";
    
    // Display price information at the top
    std::cout << "\n";
    std::cout << "???????????????????????????????????????????????????????????????\n";
    std::cout << "?                     MARKET PRICES                           ?\n";
    std::cout << "???????????????????????????????????????????????????????????????\n";
    
    // Last Traded Price
    std::cout << "? Last Traded:  ";
    if (status_response.last_trade_price() > 0) {
        std::cout << "$" << std::fixed << std::setprecision(2) 
                  << std::setw(10) << status_response.last_trade_price();
    } else {
        std::cout << std::setw(11) << "N/A";
    }
    std::cout << "                                   ?\n";
    
    // Mid Price
    std::cout << "? Mid Price:    ";
    if (status_response.mid_price() > 0) {
        std::cout << "$" << std::fixed << std::setprecision(2) 
                  << std::setw(10) << status_response.mid_price();
    } else {
        std::cout << std::setw(11) << "N/A";
    }
    std::cout << "                                   ?\n";
    
    // Spread (if both bid and ask exist)
    double best_bid = pb_orderbook.bids_size() > 0 ? pb_orderbook.bids(0).price() : 0;
    double best_ask = pb_orderbook.asks_size() > 0 ? pb_orderbook.asks(0).price() : 0;
    if (best_bid > 0 && best_ask > 0) {
        double spread = best_ask - best_bid;
        std::cout << "? Spread:       $" << std::fixed << std::setprecision(2) 
                  << std::setw(10) << spread;
        std::cout << "                                   ?\n";
    }
    
    std::cout << "???????????????????????????????????????????????????????????????\n\n";
    
    Table table;
    
    // Set table format
    table.format()
        .border_top("?")
        .border_bottom("?")
        .border_left("?")
        .border_right("?")
        .corner_top_left("?")
        .corner_top_right("?")
        .corner_bottom_left("?")
        .corner_bottom_right("?")
        .corner("?");
    
    // Add header row with styling
    table.add_row({"BIDS (Buy)", "", "", "ASKS (Sell)", "", ""});
    table[0].format()
        .font_color(Color::green)
        .font_align(FontAlign::center)
        .font_style({FontStyle::bold});
    
    // Add column headers
    table.add_row({"Price", "Quantity", "Orders", "Price", "Quantity", "Orders"});
    table[1].format()
        .font_color(Color::yellow)
        .font_align(FontAlign::center);
    
    // Determine max rows
    size_t max_rows = std::max(pb_orderbook.bids_size(), pb_orderbook.asks_size());
    
    // Add data rows - DIRECTLY FROM PROTOBUF
    for (size_t i = 0; i < max_rows; ++i) {
        std::string buy_price = "", buy_qty = "", buy_orders = "";
        std::string sell_price = "", sell_qty = "", sell_orders = "";
        
        // Buy side
        if (i < static_cast<size_t>(pb_orderbook.bids_size())) {
            const auto& bid = pb_orderbook.bids(i);
            std::ostringstream ps, qs, os;
            ps << std::fixed << std::setprecision(2) << "$" << bid.price();
            qs << std::fixed << std::setprecision(2) << bid.quantity();
            os << bid.order_count();  // This is the ACTUAL order count from Exchange
            buy_price = ps.str();
            buy_qty = qs.str();
            buy_orders = os.str();
        }
        
        // Sell side
        if (i < static_cast<size_t>(pb_orderbook.asks_size())) {
            const auto& ask = pb_orderbook.asks(i);
            std::ostringstream ps, qs, os;
            ps << std::fixed << std::setprecision(2) << "$" << ask.price();
            qs << std::fixed << std::setprecision(2) << ask.quantity();
            os << ask.order_count();  // This is the ACTUAL order count from Exchange
            sell_price = ps.str();
            sell_qty = qs.str();
            sell_orders = os.str();
        }
        
        table.add_row({buy_price, buy_qty, buy_orders, sell_price, sell_qty, sell_orders});
        
        // Color buy side green
        if (!buy_price.empty()) {
            table[i + 2][0].format().font_color(Color::green);
            table[i + 2][1].format().font_color(Color::green);
            table[i + 2][2].format().font_color(Color::green);
        }
        
        // Color sell side red
        if (!sell_price.empty()) {
            table[i + 2][3].format().font_color(Color::red);
            table[i + 2][4].format().font_color(Color::red);
            table[i + 2][5].format().font_color(Color::red);
        }
    }
    
    // Set column alignments
    for (size_t i = 0; i < table.size(); ++i) {
        if (i >= 2) {  // Data rows
            table[i][0].format().font_align(FontAlign::right);
            table[i][1].format().font_align(FontAlign::right);
            table[i][2].format().font_align(FontAlign::center);
            table[i][3].format().font_align(FontAlign::right);
            table[i][4].format().font_align(FontAlign::right);
            table[i][5].format().font_align(FontAlign::center);
        }
    }
    
    std::cout << table << "\n\n";
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
