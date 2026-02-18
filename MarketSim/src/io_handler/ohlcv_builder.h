#pragma once

#include "exchange.pb.h"
#include <vector>
#include <cstdint>
#include <string>

namespace marketsim::io_handler {

/**
 * @brief Builds OHLCV (candlestick) bars from streaming tick data
 * 
 * Reusable utility for Monitor, Trader, Backtester, etc.
 * Thread-safety: Not thread-safe. Use from single thread per instance.
 */
class OHLCVBuilder {
public:
    explicit OHLCVBuilder(const std::string& symbol, int32_t interval_seconds);
    
    void process_tick(double price, int64_t timestamp_ms, double volume);
    bool has_completed_bar() const;
    exchange::OHLCV get_completed_bar();
    std::vector<exchange::OHLCV> get_all_completed_bars();
    exchange::OHLCV get_current_bar() const;
    void reset();
    
private:
    struct BarState {
        int64_t bar_start_time_ms = 0;
        double open = 0.0;
        double high = 0.0;
        double low = 0.0;
        double close = 0.0;
        double volume = 0.0;
        int32_t tick_count = 0;
        bool is_initialized = false;
        
        void reset() {
            bar_start_time_ms = 0;
            open = high = low = close = volume = 0.0;
            tick_count = 0;
            is_initialized = false;
        }
    };
    
    int64_t get_bar_start_time(int64_t timestamp_ms) const;
    void close_current_bar();
    exchange::OHLCV bar_state_to_proto(const BarState& state) const;
    
    std::string symbol_;
    int32_t interval_seconds_;
    int64_t interval_ms_;
    
    BarState current_bar_;
    std::vector<exchange::OHLCV> completed_bars_;
};

} // namespace marketsim::io_handler
