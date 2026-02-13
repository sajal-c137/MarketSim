#include "exchange_service.h"
#include <iostream>

namespace marketsim::exchange::main {

ExchangeService::ExchangeService(const config::ExchangeConfig& config)
    : config_(config)
    , running_(false)
{
}

ExchangeService::ExchangeService(
    const std::string& order_port,
    const std::string& status_port)
    : running_(false)
{
    config_.order_port = order_port;
    config_.status_port = status_port;
}

ExchangeService::~ExchangeService() {
    stop();
}

ExchangeService::SymbolData& ExchangeService::get_or_create_symbol(const std::string& symbol) {
    auto it = symbols_.find(symbol);
    if (it == symbols_.end()) {
        auto result = symbols_.emplace(
            symbol,
            std::make_unique<SymbolData>(symbol, config_.price_history_size)
        );
        return *result.first->second;
    }
    return *it->second;
}

void ExchangeService::run() {
    std::cout << "[EXCHANGE] Starting...\n";
    
    try {
        io_handler::IOContext io_context(1);
        
        // Socket for receiving orders
        io_handler::ZmqReplier order_replier(io_context, "Exchange_Orders", config_.order_port);
        order_replier.bind();
        std::cout << "[EXCHANGE] Order receiver: " << config_.order_port << "\n";
        
        // Socket for status queries
        io_handler::ZmqReplier status_replier(io_context, "Exchange_Status", config_.status_port);
        status_replier.bind();
        std::cout << "[EXCHANGE] Status endpoint: " << config_.status_port << "\n";
        std::cout << "[EXCHANGE] Price history size: " << config_.price_history_size << "\n";
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
        
        // No need to track last_trade_price separately - it's in the history now

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
            
            // Set last trade price from history
            data::PriceTick last_trade;
            if (symbol_data.engine->get_last_trade_price(last_trade)) {
                resp.set_last_trade_price(last_trade.price);
                resp.set_last_trade_timestamp(last_trade.timestamp_ms);
            } else {
                resp.set_last_trade_price(0.0);
                resp.set_last_trade_timestamp(0);
            }
            
            // Set mid price from history
            data::PriceTick last_mid;
            if (symbol_data.engine->get_last_mid_price(last_mid)) {
                resp.set_mid_price(last_mid.price);
                resp.set_mid_price_timestamp(last_mid.timestamp_ms);
            } else {
                resp.set_mid_price(0.0);
                resp.set_mid_price_timestamp(0);
            }
            
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

