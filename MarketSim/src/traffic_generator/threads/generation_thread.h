#pragma once

#include "../operations/price_movement_calculator.h"
#include "../operations/order_flow_generator.h"
#include "../models/generation_parameters.h"
#include "io_handler/zmq_requester.h"
#include "io_handler/io_context.h"
#include <thread>
#include <atomic>
#include <memory>

namespace marketsim::traffic_generator::threads {

/**
 * @brief Main generation thread that orchestrates order generation
 * 
 * Responsibilities:
 * - Call operations/ to calculate price and generate orders
 * - Serialize and publish orders via IOHandler
 * - Manage timing (0.1s intervals)
 * - Track generation state
 */
class GenerationThread {
public:
    /**
     * @brief Construct generation thread
     * @param params Generation parameters (symbol, timing, etc.)
     * @param io_context ZeroMQ context for publishing
     * @param endpoint Endpoint to publish orders (e.g., "tcp://localhost:5555")
     */
    GenerationThread(
        const models::GenerationParameters& params,
        io_handler::IOContext& io_context,
        const std::string& endpoint
    );
    
    ~GenerationThread();
    
    /**
     * @brief Start the generation thread
     */
    void start();
    
    /**
     * @brief Stop the generation thread
     */
    void stop();
    
    /**
     * @brief Check if thread is running
     */
    bool is_running() const;
    
    /**
     * @brief Get current generation state
     */
    models::GenerationState get_state() const;
    
private:
    void run();
    void send_orders_at_time(double time_seconds);
    
    models::GenerationParameters params_;
    models::GenerationState state_;
    
    // Operations (pure math)
    operations::PriceMovementCalculator price_calculator_;
    operations::OrderFlowGenerator order_generator_;
    
    // I/O
    std::unique_ptr<io_handler::ZmqRequester> requester_;
    
    // Threading
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_;
};

} // namespace marketsim::traffic_generator::threads
