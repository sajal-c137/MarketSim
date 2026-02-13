#pragma once

#include "../threads/generation_thread.h"
#include "../models/generation_parameters.h"
#include "io_handler/io_context.h"
#include <memory>

namespace marketsim::traffic_generator::main {

/**
 * @brief Main entry point for TrafficGenerator component
 * 
 * Orchestrates initialization, thread management, and lifecycle.
 */
class TrafficGeneratorMain {
public:
    /**
     * @brief Construct traffic generator
     * @param endpoint Exchange endpoint to send orders
     */
    explicit TrafficGeneratorMain(const std::string& endpoint = "tcp://localhost:5555");
    
    ~TrafficGeneratorMain();
    
    /**
     * @brief Initialize the traffic generator with parameters
     */
    void initialize(const models::GenerationParameters& params);
    
    /**
     * @brief Start generating traffic
     */
    void start();
    
    /**
     * @brief Stop traffic generation
     */
    void stop();
    
    /**
     * @brief Wait for generation to complete
     */
    void wait_for_completion();
    
    /**
     * @brief Check if generator is running
     */
    bool is_running() const;
    
private:
    std::string endpoint_;
    io_handler::IOContext io_context_;
    std::unique_ptr<threads::GenerationThread> generation_thread_;
};

} // namespace marketsim::traffic_generator::main
