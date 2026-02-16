#pragma once

#include "price_generation_thread.h"
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
 * @brief Thread that consumes orders and submits them to Exchange
 * 
 * Consumer thread in producer-consumer pattern.
 * Sole responsibility: Pull order from queue, send to Exchange via ZeroMQ.
 * 
 * NO order generation - just network I/O!
 */
class OrderSubmissionThread {
public:
    /**
     * @brief Construct order submission thread
     * @param io_context ZeroMQ context
     * @param endpoint Exchange endpoint (e.g., "tcp://localhost:5555")
     * @param queue Shared queue to pull ORDERS from
     * @param queue_mutex Mutex protecting the queue
     * @param queue_cv Condition variable for signaling
     */
    OrderSubmissionThread(
        io_handler::IOContext& io_context,
        const std::string& endpoint,
        std::queue<PriceGenerationThread::Order>& queue,
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
    void submit_order(const PriceGenerationThread::Order& order);
    
    // I/O
    std::unique_ptr<io_handler::ZmqRequester> requester_;
    
    // Shared queue (not owned by this thread)
    std::queue<PriceGenerationThread::Order>& queue_;
    std::mutex& queue_mutex_;
    std::condition_variable& queue_cv_;
    
    // State
    std::atomic<uint64_t> orders_sent_;
    
    // Threading
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> running_;
};

} // namespace marketsim::traffic_generator::threads
