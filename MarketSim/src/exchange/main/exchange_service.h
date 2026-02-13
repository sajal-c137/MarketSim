#pragma once

#include "exchange/operations/matching_engine.h"
#include "exchange/config/exchange_config.h"
#include "io_handler/io_context.h"
#include "io_handler/zmq_replier.h"
#include "exchange.pb.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace marketsim::exchange::main {

/**
 * @brief Exchange Service - handles orders and status queries
 * 
 * All Exchange logic is here. Test files just instantiate and run.
 * Supports multiple ticker symbols with separate matching engines.
 */
class ExchangeService {
public:
    /**
     * @brief Construct Exchange service with config
     */
    explicit ExchangeService(const config::ExchangeConfig& config = config::ExchangeConfig());
    
    /**
     * @brief Construct Exchange service with ports (legacy)
     */
    ExchangeService(
        const std::string& order_port,
        const std::string& status_port
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
    // Order tracking per symbol
    struct SymbolData {
        std::unique_ptr<operations::MatchingEngine> engine;
        int order_count;
        Order last_received_order;
        
        SymbolData(const std::string& symbol, size_t price_history_size)
            : engine(std::make_unique<operations::MatchingEngine>(symbol, price_history_size))
            , order_count(0)
        {}
    };
    
    SymbolData& get_or_create_symbol(const std::string& symbol);
    
    void handle_order_request(io_handler::ZmqReplier& order_replier);
    
    void handle_status_request(io_handler::ZmqReplier& status_replier);
    
    config::ExchangeConfig config_;
    bool running_;
    
    // Map of symbol -> matching engine and data
    std::unordered_map<std::string, std::unique_ptr<SymbolData>> symbols_;
};

} // namespace marketsim::exchange::main
