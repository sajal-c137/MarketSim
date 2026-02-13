#include "exchange/operations/matching_engine.h"
#include "monitor/status_monitor.h"
#include <iostream>
#include <iomanip>

using namespace marketsim::exchange::operations;
using namespace marketsim::monitor;

// Protobuf types
using marketsim::exchange::Order;
using marketsim::exchange::OrderSide;
using marketsim::exchange::OrderType;

void print_order_book(const OrderBook& book) {
    std::cout << "\n=== Order Book: " << book.get_symbol() << " ===\n";
    
    auto sell_side = book.get_sell_side(5);
    std::cout << "ASK Side (Sellers):\n";
    std::cout << std::setw(10) << "Price" << std::setw(15) << "Quantity\n";
    std::cout << std::string(25, '-') << "\n";
    for (const auto& level : sell_side) {
        std::cout << std::fixed << std::setprecision(2)
                  << std::setw(10) << level.price
                  << std::setw(15) << level.total_quantity() << "\n";
    }
    
    auto bid_price = 0.0, bid_qty = 0.0;
    auto ask_price = 0.0, ask_qty = 0.0;
    book.get_best_bid(bid_price, bid_qty);
    book.get_best_ask(ask_price, ask_qty);
    
    std::cout << "\nSpread: " << std::fixed << std::setprecision(2)
              << ask_price - bid_price << " (bid: " << bid_price << ", ask: " << ask_price << ")\n";
    
    auto buy_side = book.get_buy_side(5);
    std::cout << "\nBID Side (Buyers):\n";
    std::cout << std::setw(10) << "Price" << std::setw(15) << "Quantity\n";
    std::cout << std::string(25, '-') << "\n";
    for (const auto& level : buy_side) {
        std::cout << std::fixed << std::setprecision(2)
                  << std::setw(10) << level.price
                  << std::setw(15) << level.total_quantity() << "\n";
    }
}

int main() {
    std::cout << "=== Matching Engine Test ===\n\n";
    
    // Initialize monitoring (silent mode for clean output)
    StatusMonitor::instance().set_output_mode(OutputMode::SILENT);
    StatusMonitor::instance().enable_file_logging("logs/matching_engine_test.log");
    StatusMonitor::instance().start_periodic_monitoring(std::chrono::seconds(10));
    
    MatchingEngine engine("AAPL");
    
    // Test 1: Add orders without matching
    std::cout << "Test 1: Adding orders without matches\n";
    {
        Order sell_order;
        sell_order.set_order_id("S1");
        sell_order.set_symbol("AAPL");
        sell_order.set_side(OrderSide::SELL);
        sell_order.set_type(OrderType::LIMIT);
        sell_order.set_price(105.0);
        sell_order.set_quantity(100);
        sell_order.set_timestamp(1);
        sell_order.set_client_id("SELLER1");
        
        auto result = engine.match_order(sell_order);
        std::cout << "  Sell 100 @ 105.00: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
    }
    
    {
        Order sell_order;
        sell_order.set_order_id("S2");
        sell_order.set_symbol("AAPL");
        sell_order.set_side(OrderSide::SELL);
        sell_order.set_type(OrderType::LIMIT);
        sell_order.set_price(105.50);
        sell_order.set_quantity(150);
        sell_order.set_timestamp(2);
        sell_order.set_client_id("SELLER2");
        
        auto result = engine.match_order(sell_order);
        std::cout << "  Sell 150 @ 105.50: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
    }
    
    {
        Order buy_order;
        buy_order.set_order_id("B1");
        buy_order.set_symbol("AAPL");
        buy_order.set_side(OrderSide::BUY);
        buy_order.set_type(OrderType::LIMIT);
        buy_order.set_price(104.0);
        buy_order.set_quantity(50);
        buy_order.set_timestamp(3);
        buy_order.set_client_id("BUYER1");
        
        auto result = engine.match_order(buy_order);
        std::cout << "  Buy 50 @ 104.00: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
    }
    
    print_order_book(engine.get_order_book());
    
    // Test 2: Execute matching orders
    std::cout << "\n\nTest 2: Executing matching orders\n";
    {
        Order buy_order;
        buy_order.set_order_id("B2");
        buy_order.set_symbol("AAPL");
        buy_order.set_side(OrderSide::BUY);
        buy_order.set_type(OrderType::LIMIT);
        buy_order.set_price(105.50);
        buy_order.set_quantity(75);
        buy_order.set_timestamp(4);
        buy_order.set_client_id("BUYER2");
        
        auto result = engine.match_order(buy_order);
        std::cout << "  Buy 75 @ 105.50: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
        std::cout << "  Executed: " << result.executed_quantity << " @ " << std::fixed 
                  << std::setprecision(2) << result.execution_price << "\n";
        std::cout << "  Trades: " << result.trades.size() << "\n";
        
        for (const auto& trade : result.trades) {
            std::cout << "    - " << trade.trade_id() << ": "
                      << trade.quantity() << " @ " << trade.price() << "\n";
        }
    }
    
    print_order_book(engine.get_order_book());
    
    // Test 3: Partial fill
    std::cout << "\n\nTest 3: Partial fill test\n";
    {
        Order buy_order;
        buy_order.set_order_id("B3");
        buy_order.set_symbol("AAPL");
        buy_order.set_side(OrderSide::BUY);
        buy_order.set_type(OrderType::LIMIT);
        buy_order.set_price(105.0);
        buy_order.set_quantity(200);
        buy_order.set_timestamp(5);
        buy_order.set_client_id("BUYER3");
        
        auto result = engine.match_order(buy_order);
        std::cout << "  Buy 200 @ 105.00: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
        std::cout << "  Executed: " << result.executed_quantity << " (remaining: " 
                  << (200 - result.executed_quantity) << ")\n";
        std::cout << "  Trades: " << result.trades.size() << "\n";
    }
    
    print_order_book(engine.get_order_book());
    
    // Test 4: Cancel order
    std::cout << "\n\nTest 4: Order cancellation\n";
    {
        bool cancelled = engine.cancel_order("B3", "AAPL");
        std::cout << "  Cancel B3: " << (cancelled ? "SUCCESS" : "FAILED") << "\n";
    }
    
    print_order_book(engine.get_order_book());
    
    // Test 5: Market order
    std::cout << "\n\nTest 5: Market order (buy all available)\n";
    {
        Order market_buy;
        market_buy.set_order_id("B4");
        market_buy.set_symbol("AAPL");
        market_buy.set_side(OrderSide::BUY);
        market_buy.set_type(OrderType::MARKET);
        market_buy.set_price(0);
        market_buy.set_quantity(300);
        market_buy.set_timestamp(6);
        market_buy.set_client_id("BUYER4");
        
        auto result = engine.match_order(market_buy);
        std::cout << "  Market Buy 300: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
        std::cout << "  Executed: " << result.executed_quantity << " @ " << std::fixed 
                  << std::setprecision(2) << result.execution_price << "\n";
        std::cout << "  Trades: " << result.trades.size() << "\n";
    }
    
    print_order_book(engine.get_order_book());
    
    // Statistics
    std::cout << "\n\n=== Statistics ===\n";
    std::cout << "Total Trades Executed: " << engine.total_trades() << "\n";
    std::cout << "Total Volume: " << std::fixed << std::setprecision(2) 
              << engine.total_volume() << "\n";
    std::cout << "Order Book - Buys: " << engine.get_order_book().total_buy_orders() << " orders, "
              << engine.get_order_book().total_buy_quantity() << " qty\n";
    std::cout << "Order Book - Sells: " << engine.get_order_book().total_sell_orders() << " orders, "
              << engine.get_order_book().total_sell_quantity() << " qty\n";
    
    StatusMonitor::instance().stop_periodic_monitoring();
    
    std::cout << "\n? Test Complete!\n";
    std::cout << "Check logs/matching_engine_test.log for detailed logging\n";
    
    return 0;
}
