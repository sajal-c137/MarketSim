#pragma once

#include <cstdint>
#include <deque>
#include <chrono>

namespace marketsim::exchange::data {

/**
 * @brief Single price data point with timestamp
 */
struct PriceTick {
    double price;
    int64_t timestamp_ms;  // Milliseconds since epoch
    
    PriceTick() : price(0.0), timestamp_ms(0) {}
    
    PriceTick(double p, int64_t ts) : price(p), timestamp_ms(ts) {}
    
    static int64_t now_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
};

/**
 * @brief Tracks price history with configurable size
 */
class PriceHistory {
public:
    explicit PriceHistory(size_t max_size = 100)
        : max_size_(max_size)
    {}
    
    // Add new price tick
    void add(double price, int64_t timestamp_ms) {
        // Ensure timestamps are monotonically increasing
        if (!history_.empty() && timestamp_ms < history_.back().timestamp_ms) {
            // Use the last timestamp + 1ms if clock went backwards
            timestamp_ms = history_.back().timestamp_ms + 1;
        }
        
        history_.emplace_back(price, timestamp_ms);
        if (history_.size() > max_size_) {
            history_.pop_front();
        }
    }
    
    // Get last price (most recent)
    bool get_last(PriceTick& tick) const {
        if (history_.empty()) return false;
        tick = history_.back();
        return true;
    }
    
    // Get all history (oldest to newest)
    const std::deque<PriceTick>& get_all() const {
        return history_;
    }
    
    // Get last N ticks
    std::deque<PriceTick> get_last_n(size_t n) const {
        if (n >= history_.size()) {
            return history_;
        }
        return std::deque<PriceTick>(
            history_.end() - n,
            history_.end()
        );
    }
    
    size_t size() const { return history_.size(); }
    bool empty() const { return history_.empty(); }
    void clear() { history_.clear(); }
    
private:
    size_t max_size_;
    std::deque<PriceTick> history_;
};

} // namespace marketsim::exchange::data
