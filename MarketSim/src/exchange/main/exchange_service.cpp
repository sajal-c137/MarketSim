#include "exchange_service.h"
#include <iostream>

namespace marketsim::exchange::main {

ExchangeService::ExchangeService(
    const std::string& order_port,
    const std::string& status_port)
    : order_port_(order_port)
    , status_port_(status_port)
    , running_(false)
{
}

ExchangeService::~ExchangeService() {
    stop();
}

void ExchangeService::run() {
    std::cout << "[EXCHANGE] Starting...\n";
    
    try {
        io_handler::IOContext io_context(1);
        operations::MatchingEngine matching_engine("AAPL");
        
        // Socket for receiving orders from TrafficGenerator
        io_handler::ZmqReplier order_replier(io_context, "Exchange_Orders", order_port_);
        order_replier.bind();
        std::cout << "[EXCHANGE] Order receiver: " << order_port_ << "\n";
        
        // Socket for status queries from Monitor
        io_handler::ZmqReplier status_replier(io_context, "Exchange_Status", status_port_);
        status_replier.bind();
        std::cout << "[EXCHANGE] Status endpoint: " << status_port_ << "\n";
        std::cout << "[EXCHANGE] Ready (silent mode - no logging)\n\n";
        
        int order_count = 0;
        double last_trade_price = 0.0;
        Order last_received_order;
        
        running_ = true;
        
        while (running_) {
            // Handle order requests
            handle_order_request(
                order_replier,
                matching_engine,
                order_count,
                last_trade_price,
                last_received_order
            );
            
            // Handle status requests from Monitor
            handle_status_request(
                status_replier,
                matching_engine,
                order_count,
                last_trade_price,
                last_received_order
            );
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[EXCHANGE] FATAL: " << e.what() << "\n";
        throw;
    }
}

void ExchangeService::stop() {
    running_ = false;
}

void ExchangeService::handle_order_request(
    io_handler::ZmqReplier& order_replier,
    operations::MatchingEngine& matching_engine,
    int& order_count,
    double& last_trade_price,
    Order& last_received_order)
{
    Order order;
    if (order_replier.receive_request(order, 10)) {
        order_count++;
        last_received_order = order;
        
        // Process order
        auto match_result = matching_engine.match_order(order);
        
        if (!match_result.trades.empty()) {
            last_trade_price = match_result.execution_price;
        }
        
        // Send acknowledgement
        OrderAck ack;
        ack.set_order_id(order.order_id());
        ack.set_status(match_result.success ? 
            OrderStatus::ACCEPTED : 
            OrderStatus::REJECTED);
        ack.set_message(match_result.success ? "OK" : match_result.error_message);
        ack.set_timestamp(order.timestamp());
        
        order_replier.send_response(ack);
    }
}

void ExchangeService::handle_status_request(
    io_handler::ZmqReplier& status_replier,
    const operations::MatchingEngine& matching_engine,
    int order_count,
    double last_trade_price,
    const Order& last_received_order)
{
    StatusRequest status_req;
    if (status_replier.receive_request(status_req, 10)) {
        // Build status response
        StatusResponse resp;
        resp.set_total_orders_received(order_count);
        resp.set_total_trades(matching_engine.total_trades());
        resp.set_total_volume(matching_engine.total_volume());
        resp.set_last_trade_price(last_trade_price);
        
        // Add last received order if available
        if (order_count > 0) {
            auto* last_order = resp.mutable_last_received_order();
            last_order->CopyFrom(last_received_order);
        }
        
        // Add orderbook snapshot
        const auto& order_book = matching_engine.get_order_book();
        auto buy_levels = order_book.get_buy_side(5);
        auto sell_levels = order_book.get_sell_side(5);
        
        auto* ob = resp.mutable_current_orderbook();
        ob->set_symbol("AAPL");
        ob->set_timestamp(0);
        
        // Add buy side
        for (const auto& level : buy_levels) {
            auto* bid = ob->add_bids();
            bid->set_price(level.price);
            bid->set_quantity(level.total_quantity());
            bid->set_order_count(static_cast<int>(level.orders.size()));
        }
        
        // Add sell side
        for (const auto& level : sell_levels) {
            auto* ask = ob->add_asks();
            ask->set_price(level.price);
            ask->set_quantity(level.total_quantity());
            ask->set_order_count(static_cast<int>(level.orders.size()));
        }
        
        status_replier.send_response(resp);
    }
}

} // namespace marketsim::exchange::main
