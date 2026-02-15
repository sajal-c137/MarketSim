#include "order_submission_thread.h"
#include "../utils/time_utils.h"
#include "exchange.pb.h"
#include <iostream>
#include <chrono>

namespace marketsim::traffic_generator::threads {

OrderSubmissionThread::OrderSubmissionThread(
    const std::string& symbol,
    int64_t order_quantity,
    io_handler::IOContext& io_context,
    const std::string& endpoint,
    std::queue<PriceGenerationThread::PricePoint>& queue,
    std::mutex& queue_mutex,
    std::condition_variable& queue_cv)
    : symbol_(symbol)
    , order_quantity_(order_quantity)
    , queue_(queue)
    , queue_mutex_(queue_mutex)
    , queue_cv_(queue_cv)
    , orders_sent_(0)
    , running_(false)
{
    // Create requester to send orders to Exchange
    requester_ = std::make_unique<io_handler::ZmqRequester>(
        io_context,
        "OrderSubmitter",
        endpoint
    );
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
    
    // Wake up if blocked on queue
    queue_cv_.notify_all();
    
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

void OrderSubmissionThread::run() {
    std::cout << "[OrderSubmitter] Starting order submission...\n";
    std::cout << "  Symbol: " << symbol_ << "\n";
    std::cout << "  Quantity: " << order_quantity_ << "\n";
    
    // Connect to Exchange
    try {
        requester_->connect();
        std::cout << "[OrderSubmitter] Connected to Exchange successfully\n";
    } catch (const std::exception& e) {
        std::cerr << "[OrderSubmitter] Failed to connect to Exchange: " << e.what() << "\n";
        running_ = false;
        return;
    }
    
    while (running_) {
        PriceGenerationThread::PricePoint point;
        
        // Wait for price from queue
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // Wait until queue has data or thread is stopped
            queue_cv_.wait(lock, [this] { 
                return !queue_.empty() || !running_; 
            });
            
            if (!running_) {
                break;
            }
            
            if (!queue_.empty()) {
                point = queue_.front();
                queue_.pop();
            } else {
                continue;
            }
        }
        
        // Process the price point
        process_price(point);
    }
    
    std::cout << "[OrderSubmitter] Submission complete. Total orders sent: " 
              << orders_sent_ << "\n";
}

void OrderSubmissionThread::process_price(const PriceGenerationThread::PricePoint& point) {
    // Generate orders at this price (buy + sell, qty 1 each)
    auto orders = order_generator_.generate_orders(
        symbol_,
        point.price,
        order_quantity_
    );
    
    int64_t timestamp = utils::TimeUtils::current_timestamp_ms();
    
    // Send each order to Exchange
    for (const auto& order : orders) {
        // Create protobuf Order message
        marketsim::exchange::Order proto_order;
        proto_order.set_order_id(order.order_id);
        proto_order.set_symbol(order.symbol);
        proto_order.set_side(order.is_buy ? 
            marketsim::exchange::OrderSide::BUY : 
            marketsim::exchange::OrderSide::SELL);
        proto_order.set_type(marketsim::exchange::OrderType::LIMIT);
        proto_order.set_price(order.price);
        proto_order.set_quantity(order.quantity);
        proto_order.set_timestamp(timestamp);
        proto_order.set_client_id("TrafficGenerator_GBM");
        
        // Send order and receive acknowledgement
        marketsim::exchange::OrderAck ack;
        try {
            if (requester_->request(proto_order, ack)) {
                orders_sent_++;
                std::cout << "[OrderSubmitter] seq=" << point.sequence_number
                          << ", t=" << point.timestamp_seconds 
                          << "s, price=" << point.price 
                          << ", order=" << order.order_id
                          << " (" << (order.is_buy ? "BUY" : "SELL") << ")\n";
            } else {
                std::cerr << "[OrderSubmitter] Failed to send order: " 
                          << order.order_id << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[OrderSubmitter] Error sending order: " 
                      << order.order_id << " - " << e.what() << "\n";
        }
    }
}

} // namespace marketsim::traffic_generator::threads
