#include "ohlcv_builder.h"
#include <stdexcept>
#include <algorithm>

namespace marketsim::io_handler {

OHLCVBuilder::OHLCVBuilder(const std::string& symbol, int32_t interval_seconds)
    : symbol_(symbol)
    , interval_seconds_(interval_seconds)
    , interval_ms_(static_cast<int64_t>(interval_seconds) * 1000)
{
    if (interval_seconds <= 0) {
        throw std::invalid_argument("Interval must be positive");
    }
}

void OHLCVBuilder::process_tick(double price, int64_t timestamp_ms, double volume) {
    int64_t bar_start = get_bar_start_time(timestamp_ms);
    
    if (!current_bar_.is_initialized) {
        current_bar_.bar_start_time_ms = bar_start;
        current_bar_.open = price;
        current_bar_.high = price;
        current_bar_.low = price;
        current_bar_.close = price;
        current_bar_.volume = volume;
        current_bar_.tick_count = 1;
        current_bar_.is_initialized = true;
    } else if (bar_start != current_bar_.bar_start_time_ms) {
        close_current_bar();
        
        current_bar_.bar_start_time_ms = bar_start;
        current_bar_.open = price;
        current_bar_.high = price;
        current_bar_.low = price;
        current_bar_.close = price;
        current_bar_.volume = volume;
        current_bar_.tick_count = 1;
        current_bar_.is_initialized = true;
    } else {
        current_bar_.high = std::max(current_bar_.high, price);
        current_bar_.low = std::min(current_bar_.low, price);
        current_bar_.close = price;
        current_bar_.volume += volume;
        current_bar_.tick_count++;
    }
}

bool OHLCVBuilder::has_completed_bar() const {
    return !completed_bars_.empty();
}

exchange::OHLCV OHLCVBuilder::get_completed_bar() {
    if (completed_bars_.empty()) {
        throw std::runtime_error("No completed bars available");
    }
    
    auto bar = completed_bars_.back();
    completed_bars_.pop_back();
    return bar;
}

std::vector<exchange::OHLCV> OHLCVBuilder::get_all_completed_bars() {
    std::vector<exchange::OHLCV> bars;
    bars.swap(completed_bars_);
    return bars;
}

exchange::OHLCV OHLCVBuilder::get_current_bar() const {
    return bar_state_to_proto(current_bar_);
}

void OHLCVBuilder::reset() {
    current_bar_.reset();
    completed_bars_.clear();
}

int64_t OHLCVBuilder::get_bar_start_time(int64_t timestamp_ms) const {
    return (timestamp_ms / interval_ms_) * interval_ms_;
}

void OHLCVBuilder::close_current_bar() {
    if (!current_bar_.is_initialized) {
        return;
    }
    
    completed_bars_.push_back(bar_state_to_proto(current_bar_));
}

exchange::OHLCV OHLCVBuilder::bar_state_to_proto(const BarState& state) const {
    exchange::OHLCV bar;
    
    if (!state.is_initialized) {
        bar.set_symbol(symbol_);
        bar.set_interval_seconds(interval_seconds_);
        return bar;
    }
    
    bar.set_symbol(symbol_);
    bar.set_timestamp(state.bar_start_time_ms);
    bar.set_open(state.open);
    bar.set_high(state.high);
    bar.set_low(state.low);
    bar.set_close(state.close);
    bar.set_volume(state.volume);
    bar.set_interval_seconds(interval_seconds_);
    
    return bar;
}

} // namespace marketsim::io_handler
