#include "price_generation_thread.h"
#include "../utils/time_utils.h"
#include "../models/price_models/hawkes_microstructure_model.h"
#include <iostream>

namespace marketsim::traffic_generator::threads {

PriceGenerationThread::PriceGenerationThread(
    const std::string& symbol,
    std::unique_ptr<models::price_models::IPriceModel> price_model,
    int64_t step_interval_ms,
    double duration_seconds,
    std::queue<Order>& queue,
    std::mutex& queue_mutex,
    std::condition_variable& queue_cv)
    : symbol_(symbol)
    , price_model_(std::move(price_model))
    , step_interval_ms_(step_interval_ms)
    , duration_seconds_(duration_seconds)
    , queue_(queue)
    , queue_mutex_(queue_mutex)
    , queue_cv_(queue_cv)
    , orders_generated_(0)
    , next_order_id_(1)
    , running_(false)
{
}

PriceGenerationThread::~PriceGenerationThread() {
    stop();
}

void PriceGenerationThread::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    thread_ = std::make_unique<std::thread>(&PriceGenerationThread::run, this);
}

void PriceGenerationThread::stop() {
    running_ = false;
    
    // Wake up consumer if blocked
    queue_cv_.notify_all();
    
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

std::string PriceGenerationThread::model_name() const {
    return price_model_ ? price_model_->model_name() : "unknown";
}

void PriceGenerationThread::run() {
    std::cout << "[OrderGenerator] Starting order generation...\n";
    std::cout << "  Model: " << price_model_->model_name() << "\n";
    std::cout << "  Description: " << price_model_->description() << "\n";
    std::cout << "  Initial Price: " << price_model_->current_price() << "\n";
    std::cout << "  Interval: " << step_interval_ms_ << " ms\n";
    std::cout << "  Duration: " << duration_seconds_ << " seconds\n";
    
    double t = 0.0;
    double step_seconds = step_interval_ms_ / 1000.0;
    
    while (running_ && t <= duration_seconds_) {
        // Step model forward
        double new_price = price_model_->next_price();
        
        // Check if model generates orders (e.g., Hawkes)
        auto* hawkes_model = dynamic_cast<models::price_models::HawkesMicrostructureModel*>(price_model_.get());
        
        if (hawkes_model && !hawkes_model->current_orders().empty()) {
            // Hawkes model: use generated order clouds
            for (const auto& hawkes_order : hawkes_model->current_orders()) {
                Order order{
                    .order_id = next_order_id_++,
                    .symbol = symbol_,
                    .is_buy = hawkes_order.is_buy,
                    .price = hawkes_order.price,
                    .volume = hawkes_order.volume,
                    .timestamp_seconds = t
                };
                
                // Push to queue (thread-safe)
                {
                    std::lock_guard<std::mutex> lock(queue_mutex_);
                    queue_.push(order);
                }
                
                orders_generated_++;
            }
            
            // Notify consumer once per batch
            queue_cv_.notify_one();
        } else {
            // Simple models (linear, GBM): generate buy+sell at mid-price
            Order buy_order{
                .order_id = next_order_id_++,
                .symbol = symbol_,
                .is_buy = true,
                .price = new_price,
                .volume = 1.0,
                .timestamp_seconds = t
            };
            
            Order sell_order{
                .order_id = next_order_id_++,
                .symbol = symbol_,
                .is_buy = false,
                .price = new_price,
                .volume = 1.0,
                .timestamp_seconds = t
            };
            
            // Push to queue (thread-safe)
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                queue_.push(buy_order);
                queue_.push(sell_order);
            }
            
            orders_generated_ += 2;
            queue_cv_.notify_one();
        }
        
        // Log every 10 steps
        if (static_cast<int>(t / step_seconds) % 10 == 0) {
            std::cout << "[OrderGenerator] t=" << t 
                      << "s, price=" << new_price 
                      << ", orders_generated=" << orders_generated_ << "\n";
        }
        
        // Sleep for interval
        utils::TimeUtils::sleep_ms(step_interval_ms_);
        
        // Advance time
        t += step_seconds;
    }
    
    std::cout << "[OrderGenerator] Generation complete. Total orders: " 
              << orders_generated_ << "\n";
    running_ = false;
}

} // namespace marketsim::traffic_generator::threads
