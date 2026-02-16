#pragma once

#include "../models/price_models/i_price_model.h"
#include <thread>
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace marketsim::traffic_generator::threads {

/**
 * @brief Thread that runs price models and generates orders
 * 
 * Producer thread in producer-consumer pattern.
 * Responsibility: Run model, generate orders, push to queue.
 * 
 * For simple models (linear, GBM): generates buy+sell at mid-price
 * For Hawkes: generates order clouds with distributed prices
 */
class PriceGenerationThread {
public:
    /**
     * @brief Order ready for submission
     */
    struct Order {
        uint64_t order_id;
        std::string symbol;
        bool is_buy;
        double price;
        double volume;
        double timestamp_seconds;
    };
    
    /**
     * @brief Construct price generation thread with a price model
     * @param symbol Trading symbol
     * @param price_model Unique pointer to price model (ownership transferred)
     * @param step_interval_ms Time between price updates (milliseconds)
     * @param duration_seconds Total duration to generate prices
     * @param queue Shared queue to push ORDERS to
     * @param queue_mutex Mutex protecting the queue
     * @param queue_cv Condition variable for signaling
     */
    PriceGenerationThread(
        const std::string& symbol,
        std::unique_ptr<models::price_models::IPriceModel> price_model,
        int64_t step_interval_ms,
        double duration_seconds,
        std::queue<Order>& queue,
        std::mutex& queue_mutex,
        std::condition_variable& queue_cv
    );
    
    ~PriceGenerationThread();
    
    /**
     * @brief Start generating orders
     */
    void start();
    
    /**
     * @brief Stop order generation
     */
    void stop();
    
    /**
     * @brief Check if thread is running
     */
    bool is_running() const { return running_; }
    
    /**
     * @brief Get number of orders generated
     */
    uint64_t orders_generated() const { return orders_generated_; }
    
    /**
     * @brief Get model name
     */
    std::string model_name() const;
    
private:
    void run();
    
    // Symbol
    std::string symbol_;
    
    // Price model (pure math, pluggable)
    std::unique_ptr<models::price_models::IPriceModel> price_model_;
    
    // Configuration
    int64_t step_interval_ms_;
    double duration_seconds_;
    
    // Shared queue (not owned by this thread)
    std::queue<Order>& queue_;
    std::mutex& queue_mutex_;
    std::condition_variable& queue_cv_;
    
    // State
    std::atomic<uint64_t> orders_generated_;
    uint64_t next_order_id_;
    
    // Threading
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_;
};

} // namespace marketsim::traffic_generator::threads
