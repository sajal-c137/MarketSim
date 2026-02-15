#include "price_generation_thread.h"
#include "../utils/time_utils.h"
#include <iostream>

namespace marketsim::traffic_generator::threads {

PriceGenerationThread::PriceGenerationThread(
    std::unique_ptr<models::price_models::IPriceModel> price_model,
    int64_t step_interval_ms,
    double duration_seconds,
    std::queue<PricePoint>& queue,
    std::mutex& queue_mutex,
    std::condition_variable& queue_cv)
    : price_model_(std::move(price_model))
    , step_interval_ms_(step_interval_ms)
    , duration_seconds_(duration_seconds)
    , queue_(queue)
    , queue_mutex_(queue_mutex)
    , queue_cv_(queue_cv)
    , prices_generated_(0)
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
    std::cout << "[PriceGenerator] Starting price generation...\n";
    std::cout << "  Model: " << price_model_->model_name() << "\n";
    std::cout << "  Description: " << price_model_->description() << "\n";
    std::cout << "  Initial Price: " << price_model_->current_price() << "\n";
    std::cout << "  Interval: " << step_interval_ms_ << " ms\n";
    std::cout << "  Duration: " << duration_seconds_ << " seconds\n";
    
    double t = 0.0;
    double step_seconds = step_interval_ms_ / 1000.0;
    uint64_t sequence = 0;
    
    while (running_ && t <= duration_seconds_) {
        // Generate next price using model
        double new_price = price_model_->next_price();
        
        // Create price point
        PricePoint point{
            .price = new_price,
            .timestamp_seconds = t,
            .sequence_number = sequence++
        };
        
        // Push to queue (thread-safe)
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            queue_.push(point);
        }
        
        // Notify consumer
        queue_cv_.notify_one();
        
        // Update counters
        prices_generated_++;
        
        // Log every 10 prices
        if (sequence % 10 == 0) {
            std::cout << "[PriceGenerator] t=" << t 
                      << "s, price=" << new_price 
                      << ", seq=" << sequence << "\n";
        }
        
        // Sleep for interval
        utils::TimeUtils::sleep_ms(step_interval_ms_);
        
        // Advance time
        t += step_seconds;
    }
    
    std::cout << "[PriceGenerator] Generation complete. Total prices: " 
              << prices_generated_ << "\n";
    running_ = false;
}

} // namespace marketsim::traffic_generator::threads
