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

ExchangeService::SymbolData& ExchangeService::get_or_create_symbol(const std::string& symbol) {
    auto it = symbols_.find(symbol);
    if (it == symbols_.end()) {
        auto result = symbols_.emplace(symbol, std::make_unique<SymbolData>(symbol));
        return *result.first->second;
    }
    return *it->second;
}

void ExchangeService::run() {
    std::cout << "[EXCHANGE] Starting...\n";
    
    try {
        io_handler::IOContext io_context(1);
        
        // Socket for receiving orders
        io_handler::ZmqReplier order_replier(io_context, "Exchange_Orders", order_port_);
        order_replier.bind();
        std::cout << "[EXCHANGE] Order receiver: " << order_port_ << "\n";
        
        // Socket for status queries
        io_handler::ZmqReplier status_replier(io_context, "Exchange_Status", status_port_);
        status_replier.bind();
        std::cout << "[EXCHANGE] Status endpoint: " << status_port_ << "\n";
        std::cout << "[EXCHANGE] Ready (silent mode - no logging)\n\n";
        
        running_ = true;
        
        while (running_) {
            handle_order_request(order_replier);
            handle_status_request(status_replier);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[EXCHANGE] FATAL: " << e.what() << "\n";
        throw;
    }
}

void ExchangeService::stop() {
    running_ = false;
}

void ExchangeService::handle_order_request(io_handler::ZmqReplier& order_replier) {
    Order order;
    if (order_replier.receive_request(order, 10)) {
        // Get or create symbol data
        auto& symbol_data = get_or_create_symbol(order.symbol());
        
        symbol_data.order_count++;
        symbol_data.last_received_order = order;
        
        // Process order
        auto match_result = symbol_data.engine->match_order(order);
        
        if (!match_result.trades.empty()) {
            symbol_data.last_trade_price = match_result.execution_price;
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

void ExchangeService::handle_status_request(io_handler::ZmqReplier& status_replier) {
    StatusRequest status_req;
    if (status_replier.receive_request(status_req, 10)) {
        const std::string& requested_symbol = status_req.symbol();
        
        // Build status response - FILTER BY REQUESTED SYMBOL
        StatusResponse resp;
        
        auto it = symbols_.find(requested_symbol);
        if (it != symbols_.end()) {
            // Symbol exists - return its data
            const auto& symbol_data = *it->second;
            
            resp.set_total_orders_received(symbol_data.order_count);
            resp.set_total_trades(symbol_data.engine->total_trades());
            resp.set_total_volume(symbol_data.engine->total_volume());
            resp.set_last_trade_price(symbol_data.last_trade_price);
            
            // Add last received order if available
            if (symbol_data.order_count > 0) {
                auto* last_order = resp.mutable_last_received_order();
                last_order->CopyFrom(symbol_data.last_received_order);
            }
            
            // Add orderbook snapshot for THIS SYMBOL ONLY
            const auto& order_book = symbol_data.engine->get_order_book();
            auto buy_levels = order_book.get_buy_side(5);
            auto sell_levels = order_book.get_sell_side(5);
            
            auto* ob = resp.mutable_current_orderbook();
            ob->set_symbol(requested_symbol);
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
        } else {
            // Symbol doesn't exist yet - return empty response
            resp.set_total_orders_received(0);
            resp.set_total_trades(0);
            resp.set_total_volume(0.0);
            resp.set_last_trade_price(0.0);
            
            auto* ob = resp.mutable_current_orderbook();
            ob->set_symbol(requested_symbol);
        }
        
        status_replier.send_response(resp);
    }
}

} // namespace marketsim::exchange::main

