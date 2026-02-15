#pragma once

#include "price_generation_thread.h"
#include "../operations/order_flow_generator.h"
#include "io_handler/zmq_requester.h"
#include "io_handler/io_context.h"
#include <thread>
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace marketsim::traffic_generator::threads {

/**
 * @brief Thread that consumes prices and submits orders to Exchange
 * 
 * Consumer thread in producer-consumer pattern.
 * Sole responsibility: Pull price from queue, create orders, send to Exchange.
 */
class OrderSubmissionThread {
public:
    /**
     * @brief Construct order submission thread
     * @param symbol Trading symbol
     * @param order_quantity Quantity per order
     * @param io_context ZeroMQ context
     * @param endpoint Exchange endpoint (e.g., "tcp://localhost:5555")
     * @param queue Shared queue to pull prices from
     * @param queue_mutex Mutex protecting the queue
     * @param queue_cv Condition variable for signaling
     */
    OrderSubmissionThread(
        const std::string& symbol,
        int64_t order_quantity,
        io_handler::IOContext& io_context,
        const std::string& endpoint,
        std::queue<PriceGenerationThread::PricePoint>& queue,
        std::mutex& queue_mutex,
        std::condition_variable& queue_cv
    );
    
    ~OrderSubmissionThread();
    
    /**
     * @brief Start submitting orders
     */
    void start();
    
    /**
     * @brief Stop order submission
     */
    void stop();
    
    /**
     * @brief Check if thread is running
     */
    bool is_running() const { return running_; }
    
    /**
     * @brief Get number of orders sent
     */
    uint64_t orders_sent() const { return orders_sent_; }
    
private:
    void run();
    void process_price(const PriceGenerationThread::PricePoint& point);
    
    // Configuration
    std::string symbol_;
    int64_t order_quantity_;
    
    // Order generator (pure math)
    operations::OrderFlowGenerator order_generator_;
    
    // I/O
    std::unique_ptr<io_handler::ZmqRequester> requester_;
    
    // Shared queue (not owned by this thread)
    std::queue<PriceGenerationThread::PricePoint>& queue_;
    std::mutex& queue_mutex_;
    std::condition_variable& queue_cv_;
    
    // State
    std::atomic<uint64_t> orders_sent_;
    
    // Threading
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_;
};

} // namespace marketsim::traffic_generator::threads
