#include "traffic_generator/threads/price_generation_thread.h"
#include "traffic_generator/threads/order_submission_thread.h"
#include "traffic_generator/models/generation_parameters.h"
#include "traffic_generator/models/price_models/gbm_price_model.h"
#include "io_handler/io_context.h"
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace marketsim;

int main() {
    std::cout << "=== GBM-Based Traffic Generator ===\n\n";
    
    // Shared queue for producer-consumer pattern
    std::queue<traffic_generator::threads::PriceGenerationThread::PricePoint> price_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    
    // Configuration
    traffic_generator::models::GenerationParameters config;
    config.symbol = "AAPL";
    config.base_price = 100.0;
    config.order_quantity = 1;
    config.step_interval_ms = 100;  // Generate price every 100ms
    config.duration_seconds = 30.0; // Run for 30 seconds
    config.drift = 5.0;             // 5% annual drift
    config.volatility = 3.0;        // 3% annual volatility
    
    std::string exchange_endpoint = "tcp://localhost:5555";
    
    // Create IOContext for order submission
    io_handler::IOContext io_context;
    
    // Convert percentage to decimal for GBM formula
    double drift_decimal = config.drift / 100.0;       // 5.0 -> 0.05
    double volatility_decimal = config.volatility / 100.0; // 3.0 -> 0.03
    
    // Calculate simulated time step (fraction of year per step)
    double total_steps = config.duration_seconds / (config.step_interval_ms / 1000.0);
    double dt = 1.0 / total_steps;  // Simulate 1 year over the duration
    
    std::cout << "Configuration:\n";
    std::cout << "  Symbol: " << config.symbol << "\n";
    std::cout << "  Initial Price: $" << config.base_price << "\n";
    std::cout << "  Drift: " << config.drift << "% (" << drift_decimal << ")\n";
    std::cout << "  Volatility: " << config.volatility << "% (" << volatility_decimal << ")\n";
    std::cout << "  Interval: " << config.step_interval_ms << " ms\n";
    std::cout << "  Duration: " << config.duration_seconds << " seconds\n";
    std::cout << "  Simulated Time Per Step: " << dt << " years\n\n";
    
    // Create GBM price model
    auto price_model = std::make_unique<traffic_generator::models::price_models::GBMPriceModel>(
        config.base_price,
        drift_decimal,
        volatility_decimal,
        dt,
        0  // seed = 0 for random
    );
    
    // Create producer thread (generates prices)
    traffic_generator::threads::PriceGenerationThread price_thread(
        std::move(price_model),
        static_cast<int64_t>(config.step_interval_ms),
        config.duration_seconds,
        price_queue,
        queue_mutex,
        queue_cv
    );
    
    // Create consumer thread (submits orders)
    traffic_generator::threads::OrderSubmissionThread order_thread(
        config.symbol,
        static_cast<int64_t>(config.order_quantity),
        io_context,
        exchange_endpoint,
        price_queue,
        queue_mutex,
        queue_cv
    );
    
    // Start both threads
    std::cout << "Starting threads...\n\n";
    price_thread.start();
    order_thread.start();
    
    // Wait for completion
    while (price_thread.is_running() || order_thread.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Stop threads
    price_thread.stop();
    order_thread.stop();
    
    // Summary
    std::cout << "\n=== Summary ===\n";
    std::cout << "Prices Generated: " << price_thread.prices_generated() << "\n";
    std::cout << "Orders Sent: " << order_thread.orders_sent() << "\n";
    std::cout << "Queue Size: " << price_queue.size() << " (should be 0)\n";
    
    return 0;
}
