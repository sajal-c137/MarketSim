#include "order_submission_thread.h"
#include "exchange.pb.h"
#include <iostream>

namespace marketsim::traffic_generator::threads {

OrderSubmissionThread::OrderSubmissionThread(
    io_handler::IOContext& io_context,
    const std::string& endpoint,
    std::queue<PriceGenerationThread::Order>& queue,
    std::mutex& queue_mutex,
    std::condition_variable& queue_cv)
    : queue_(queue)
    , queue_mutex_(queue_mutex)
    , queue_cv_(queue_cv)
    , orders_sent_(0)
    , running_(false)
{
    // Create ZeroMQ requester for sending orders to Exchange
    requester_ = std::make_unique<io_handler::ZmqRequester>(
        io_context,
        "TrafficGenerator",
        endpoint
    );
    
    // Connect to Exchange
    requester_->connect();
    std::cout << "[OrderSubmitter] Connected to Exchange at " << endpoint << "\n";
}

OrderSubmissionThread::~OrderSubmissionThread() {
    stop();
}

void OrderSubmissionThread::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    thread_ = std::make_unique<std::thread>(&OrderSubmissionThread::run, this);
}

void OrderSubmissionThread::stop() {
    running_ = false;
    
    // Wake up if blocked
    queue_cv_.notify_all();
    
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

void OrderSubmissionThread::run() {
    std::cout << "[OrderSubmitter] Starting order submission...\n";
    
    while (running_) {
        PriceGenerationThread::Order order;
        
        // Pull order from queue (blocking)
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            queue_cv_.wait(lock, [this] { 
                return !queue_.empty() || !running_; 
            });
            
            if (!running_) {
                break;
            }
            
            if (!queue_.empty()) {
                order = queue_.front();
                queue_.pop();
            } else {
                continue;
            }
        }
        
        // Submit the order (outside lock)
        submit_order(order);
    }
    
    std::cout << "[OrderSubmitter] Submission complete. Total orders sent: " 
              << orders_sent_ << "\n";
}

void OrderSubmissionThread::submit_order(const PriceGenerationThread::Order& order) {
    // Create protobuf Order message
    marketsim::exchange::Order proto_order;
    proto_order.set_order_id(std::to_string(order.order_id));
    proto_order.set_symbol(order.symbol);
    proto_order.set_side(order.is_buy ? 
                         marketsim::exchange::OrderSide::BUY : 
                         marketsim::exchange::OrderSide::SELL);
    proto_order.set_type(marketsim::exchange::OrderType::LIMIT);
    proto_order.set_price(order.price);
    proto_order.set_quantity(order.volume);
    proto_order.set_timestamp(static_cast<int64_t>(order.timestamp_seconds * 1000));  // Convert to ms
    proto_order.set_client_id("TrafficGenerator");
    
    // Send to Exchange and wait for acknowledgement
    marketsim::exchange::OrderAck ack;
    bool success = requester_->request(proto_order, ack);
    
    if (success) {
        orders_sent_++;
        
        // Log every 10 orders
        if (orders_sent_ % 10 == 0) {
            std::cout << "[OrderSubmitter] Sent " << orders_sent_ << " orders. "
                      << "Latest: " << (order.is_buy ? "BUY" : "SELL") 
                      << " " << order.symbol 
                      << " @ $" << order.price 
                      << " qty=" << order.volume << "\n";
        }
    } else {
        std::cerr << "[OrderSubmitter] Failed to send order " << order.order_id << "\n";
    }
}

} // namespace marketsim::traffic_generator::threads
