#include "traffic_generator/main/traffic_generator_main.h"
#include "traffic_generator/models/generation_parameters.h"
#include <iostream>

using namespace marketsim::traffic_generator;

int main() {
    std::cout << "========================================\n";
    std::cout << "Traffic Generator Test\n";
    std::cout << "========================================\n\n";
    
    // Create traffic generator pointing to Exchange
    // (Adjust endpoint as needed - Exchange should be listening here)
    main::TrafficGeneratorMain generator("tcp://localhost:5555");
    
    // Set up generation parameters
    models::GenerationParameters params;
    params.symbol = "AAPL";
    params.base_price = 100.0;           // Starting price
    params.price_rate = 10.0;            // +10 per second
    params.order_quantity = 1.0;         // 1 unit per order
    params.step_interval_ms = 100.0;     // 0.1 seconds (100ms)
    params.duration_seconds = 10.0;      // Run for 10 seconds
    
    std::cout << "Configuration:\n";
    std::cout << "  Formula: price = " << params.base_price 
              << " + " << params.price_rate << " * t\n";
    std::cout << "  Time range: t = 0 to " << params.duration_seconds << " seconds\n";
    std::cout << "  Step: " << params.step_interval_ms << " ms\n";
    std::cout << "  Expected orders: ~" 
              << static_cast<int>((params.duration_seconds * 1000.0 / params.step_interval_ms) * 2)
              << " (100 BUY + 100 SELL)\n";
    std::cout << "  Final price: " 
              << (params.base_price + params.price_rate * params.duration_seconds) << "\n\n";
    
    // Initialize and start
    generator.initialize(params);
    generator.start();
    
    // Wait for completion
    std::cout << "Waiting for generation to complete...\n\n";
    generator.wait_for_completion();
    
    std::cout << "\n========================================\n";
    std::cout << "Test completed!\n";
    std::cout << "========================================\n";
    
    return 0;
}
