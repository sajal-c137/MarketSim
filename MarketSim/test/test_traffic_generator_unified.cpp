#include "traffic_generator/threads/price_generation_thread.h"
#include "traffic_generator/threads/order_submission_thread.h"
#include "traffic_generator/models/generation_parameters.h"
#include "traffic_generator/models/price_models/price_model_factory.h"
#include "io_handler/io_context.h"
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace marketsim;

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <model_name>\n";
    std::cout << "\nAvailable models:\n";
    std::cout << "  " << traffic_generator::models::price_models::PriceModelFactory::available_models() << "\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " linear\n";
    std::cout << "  " << program_name << " gbm\n";
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    std::string model_name = "gbm";  // Default model
    
    if (argc > 1) {
        model_name = argv[1];
        
        // Check for help flag
        if (model_name == "-h" || model_name == "--help" || model_name == "help") {
            print_usage(argv[0]);
            return 0;
        }
    } else {
        std::cout << "No model specified, using default: " << model_name << "\n";
        std::cout << "Use --help to see available models\n\n";
    }
    
    std::cout << "=== Traffic Generator with " << model_name << " Model ===\n\n";
    
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
    config.drift = 5.0;             // 5% annual drift (for GBM)
    config.volatility = 3.0;        // 3% annual volatility (for GBM)
    config.price_rate = 0.1;        // $0.10 per second (for linear)
    
    std::string exchange_endpoint = "tcp://localhost:5555";
    
    // Create IOContext for order submission
    io_handler::IOContext io_context;
    
    // Calculate simulated time step (fraction of year per step)
    double total_steps = config.duration_seconds / (config.step_interval_ms / 1000.0);
    double dt = 1.0 / total_steps;  // Simulate 1 year over the duration
    
    std::cout << "Configuration:\n";
    std::cout << "  Symbol: " << config.symbol << "\n";
    std::cout << "  Initial Price: $" << config.base_price << "\n";
    std::cout << "  Model: " << model_name << "\n";
    if (model_name == "gbm") {
        std::cout << "  Drift: " << config.drift << "%\n";
        std::cout << "  Volatility: " << config.volatility << "%\n";
    } else if (model_name == "linear") {
        std::cout << "  Rate: $" << config.price_rate << " per second\n";
    }
    std::cout << "  Interval: " << config.step_interval_ms << " ms\n";
    std::cout << "  Duration: " << config.duration_seconds << " seconds\n";
    std::cout << "  Total Steps: " << total_steps << "\n";
    std::cout << "  Simulated Time Per Step: " << dt << " years\n\n";
    
    // Create price model using factory
    std::unique_ptr<traffic_generator::models::price_models::IPriceModel> price_model;
    try {
        price_model = traffic_generator::models::price_models::PriceModelFactory::create(
            model_name,
            config,
            dt
        );
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        print_usage(argv[0]);
        return 1;
    }
    
    std::cout << "Model Description: " << price_model->description() << "\n\n";
    
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
    std::cout << "Model: " << model_name << "\n";
    std::cout << "Prices Generated: " << price_thread.prices_generated() << "\n";
    std::cout << "Orders Sent: " << order_thread.orders_sent() << "\n";
    std::cout << "Queue Size: " << price_queue.size() << " (should be 0)\n";
    
    return 0;
}
