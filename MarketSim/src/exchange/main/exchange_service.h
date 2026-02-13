#pragma once

#include "exchange/operations/matching_engine.h"
#include "io_handler/io_context.h"
#include "io_handler/zmq_replier.h"
#include "exchange.pb.h"
#include <memory>
#include <string>

namespace marketsim::exchange::main {

/**
 * @brief Exchange Service - handles orders and status queries
 * 
 * All Exchange logic is here. Test files just instantiate and run.
 */
class ExchangeService {
public:
    /**
     * @brief Construct Exchange service
     * @param order_port Port for receiving orders (e.g., "tcp://*:5555")
     * @param status_port Port for status queries (e.g., "tcp://*:5557")
     */
    ExchangeService(
        const std::string& order_port = "tcp://*:5555",
        const std::string& status_port = "tcp://*:5557"
    );
    
    ~ExchangeService();
    
    /**
     * @brief Start the Exchange service (blocking)
     */
    void run();
    
    /**
     * @brief Stop the Exchange service
     */
    void stop();
    
private:
    void handle_order_request(
        io_handler::ZmqReplier& order_replier,
        operations::MatchingEngine& matching_engine,
        int& order_count,
        double& last_trade_price,
        Order& last_received_order
    );
    
    void handle_status_request(
        io_handler::ZmqReplier& status_replier,
        const operations::MatchingEngine& matching_engine,
        int order_count,
        double last_trade_price,
        const Order& last_received_order
    );
    
    std::string order_port_;
    std::string status_port_;
    bool running_;
};

} // namespace marketsim::exchange::main
