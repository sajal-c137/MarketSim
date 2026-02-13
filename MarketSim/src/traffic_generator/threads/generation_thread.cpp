#include "generation_thread.h"
#include "../utils/time_utils.h"
#include "exchange.pb.h"
#include <iostream>

namespace marketsim::traffic_generator::threads {

GenerationThread::GenerationThread(
    const models::GenerationParameters& params,
    io_handler::IOContext& io_context,
    const std::string& endpoint)
    : params_(params)
    , price_calculator_(params.base_price, params.price_rate)
    , running_(false)
{
    // Create requester to send orders to Exchange
    requester_ = std::make_unique<io_handler::ZmqRequester>(
        io_context,
        "TrafficGenerator",
        endpoint
    );
}

GenerationThread::~GenerationThread() {
    stop();
}

void GenerationThread::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    thread_ = std::make_unique<std::thread>(&GenerationThread::run, this);
}

void GenerationThread::stop() {
    running_ = false;
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

bool GenerationThread::is_running() const {
    return running_;
}

models::GenerationState GenerationThread::get_state() const {
    return state_;
}

void GenerationThread::run() {
    std::cout << "[TrafficGenerator] Starting generation...\n";
    std::cout << "  Symbol: " << params_.symbol << "\n";
    std::cout << "  Base Price: " << params_.base_price << "\n";
    std::cout << "  Price Rate: " << params_.price_rate << " per second\n";
    std::cout << "  Interval: " << params_.step_interval_ms << " ms\n";
    std::cout << "  Duration: " << params_.duration_seconds << " seconds\n";
    
    // Connect requester to Exchange
    try {
        requester_->connect();
        std::cout << "[TrafficGenerator] Connected to Exchange successfully\n";
    } catch (const std::exception& e) {
        std::cerr << "[TrafficGenerator] Failed to connect to Exchange: " << e.what() << "\n";
        running_ = false;
        return;
    }
    
    // Initialize state
    state_.start_timestamp_ms = utils::TimeUtils::current_timestamp_ms();
    state_.is_running = true;
    state_.orders_sent = 0;
    
    // Generation loop: t = 0.0 to duration_seconds, step by step_interval
    double t = 0.0;
    double step_seconds = params_.step_interval_ms / 1000.0;
    
    while (running_ && t <= params_.duration_seconds) {
        // Send orders at current time
        send_orders_at_time(t);
        
        // Update state
        state_.elapsed_seconds = t;
        
        // Sleep for interval
        utils::TimeUtils::sleep_ms(static_cast<int64_t>(params_.step_interval_ms));
        
        // Advance time
        t += step_seconds;
    }
    
    state_.is_running = false;
    running_ = false;  // Signal that thread has completed
    std::cout << "[TrafficGenerator] Generation complete. Total orders sent: " 
              << state_.orders_sent << "\n";
    std::cout.flush();  // Ensure message is printed immediately
}

void GenerationThread::send_orders_at_time(double time_seconds) {
    // Calculate price using operations (pure math)
    double price = price_calculator_.calculate_price(time_seconds);
    
    // Generate orders using operations (pure math)
    auto orders = order_generator_.generate_orders(
        params_.symbol,
        price,
        params_.order_quantity
    );
    
    // Send each order via IOHandler
    int64_t timestamp = utils::TimeUtils::current_timestamp_ms();
    
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
        proto_order.set_client_id("TrafficGenerator");
        
        // Send order and receive acknowledgement
        marketsim::exchange::OrderAck ack;
        try {
            if (requester_->request(proto_order, ack)) {
                state_.orders_sent++;
                std::cout << "[TrafficGenerator] t=" << time_seconds 
                          << "s, price=" << price 
                          << ", order=" << order.order_id
                          << " (" << (order.is_buy ? "BUY" : "SELL") << ")\n";
            } else {
                std::cerr << "[TrafficGenerator] Failed to send order: " 
                          << order.order_id << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[TrafficGenerator] Error sending order: " 
                      << order.order_id << " - " << e.what() << "\n";
        }
    }
}

} // namespace marketsim::traffic_generator::threads
