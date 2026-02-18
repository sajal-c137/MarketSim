# OHLCV Feature Implementation

## Summary

**OHLCV (Open-High-Low-Close-Volume) candlestick generation** has been implemented as a reusable component in the `io_handler` layer.

---

## Architecture Decision

### **Location: `src/io_handler/`**

**Rationale:**
- ✅ **Reusable across components**: Monitor, Trader, Backtester can all use it
- ✅ **Data processing utility**: Logically belongs with data handling
- ✅ **Separation of concerns**: Pure data transformation, no business logic

### **Namespace: `marketsim::io_handler`**

```cpp
#include "io_handler/ohlcv_builder.h"

// Usage in Monitor
auto builder = std::make_unique<io_handler::OHLCVBuilder>("AAPL", 60);

// Future: Usage in Trader
auto builder = std::make_unique<io_handler::OHLCVBuilder>("BTC-USD", 300);
```

---

## Files Created/Modified

### **New Files:**
1. `MarketSim/src/io_handler/ohlcv_builder.h` - OHLCV builder interface
2. `MarketSim/src/io_handler/ohlcv_builder.cpp` - Implementation

### **Modified Files:**
1. `proto/src/common/exchange.proto` - Added `tick_size` field to OHLCV message
2. `MarketSim/CMakeLists.txt` - Added ohlcv_builder.cpp to io_handler_lib
3. `MarketSim/src/monitor/monitor_config.h` - Added OHLCV configuration options
4. `MarketSim/src/monitor/exchange_monitor.h` - Integrated OHLCVBuilder
5. `MarketSim/src/monitor/exchange_monitor.cpp` - Tick processing and bar generation
6. `MarketSim/src/monitor/exchange_logger.h` - Added `log_ohlcv()` method
7. `MarketSim/src/monitor/exchange_logger.cpp` - Implemented OHLCV display

---

## How It Works

### **1. Data Flow:**

```
Exchange → StatusResponse (trade_price_history[]) 
         ↓
    Monitor polls (via io_handler::ZmqRequester)
         ↓
    io_handler::OHLCVBuilder processes ticks
         ↓
    Completed bars emitted
         ↓
    Monitor displays / Trader uses for signals
```

### **2. Tick Processing:**

```cpp
// Monitor receives ticks from Exchange
for (const auto& tick : response.trade_price_history()) {
    if (tick.timestamp_ms() > last_processed_tick_timestamp_) {
        ohlcv_builder_->process_tick(
            tick.price(), 
            tick.timestamp_ms(), 
            1.0  // volume
        );
        last_processed_tick_timestamp_ = tick.timestamp_ms();
    }
}

// Check for completed bars
if (ohlcv_builder_->has_completed_bar()) {
    auto bar = ohlcv_builder_->get_completed_bar();
    ExchangeLogger::log_ohlcv(bar);
}
```

### **3. Bar Alignment:**

- Timestamps are **aligned to interval boundaries**
- Example: For 60-second bars, `12:34:45.123` → `12:34:00.000`
- When a tick arrives in a new interval, the current bar closes automatically

---

## Configuration

### **Enable OHLCV in Monitor:**

```cpp
MonitorConfig config;
config.enable_ohlcv = true;            // Enable OHLCV generation
config.ohlcv_interval_seconds = 60;    // 1-minute bars
config.show_ohlcv = true;              // Display in console
config.polling_interval_ms = 250;      // Poll more frequently for timely updates
```

### **Example Output:**

```
[OHLCV] 60s Bar - AAPL
┌──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬────────┐
│   Time   │   Open   │   High   │   Low    │  Close   │  Volume  │ Ticks  │
├──────────┼──────────┼──────────┼──────────┼──────────┼──────────┼────────┤
│ 14:23:00 │  100.50  │  100.75  │  100.45  │  100.60  │   42.50  │   18   │
└──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴────────┘
```

---

## Protobuf Message

```protobuf
message OHLCV {
  string symbol = 1;
  int64 timestamp = 2;           // Bar opening time (milliseconds)
  double open = 3;
  double high = 4;
  double low = 5;
  double close = 6;
  double volume = 7;
  int32 interval_seconds = 8;    // Bar interval (60 = 1-minute)
  int32 tick_size = 9;           // Number of trades in this bar
}
```

---

## Future Usage: Trader Component

When the Trader component is implemented, it can use OHLCV builder for strategy signals:

```cpp
// Example: Moving Average Crossover Strategy
class MovingAverageStrategy {
private:
    io_handler::OHLCVBuilder ohlcv_builder_;
    
public:
    void on_tick(double price, int64_t timestamp) {
        ohlcv_builder_.process_tick(price, timestamp, 1.0);
        
        if (ohlcv_builder_.has_completed_bar()) {
            auto bar = ohlcv_builder_.get_completed_bar();
            
            // Calculate indicators from OHLCV
            double sma_fast = calculate_sma(bars_, 10);
            double sma_slow = calculate_sma(bars_, 50);
            
            // Generate signals
            if (sma_fast > sma_slow) {
                place_buy_order();
            }
        }
    }
};
```

---

## Benefits

1. **Separation of Concerns**: Data processing is independent of display/strategy logic
2. **Reusability**: Any component can build OHLCV bars from tick data
3. **Flexibility**: Configurable intervals (1s, 60s, 300s, etc.)
4. **Performance**: Efficient incremental updates, no reprocessing
5. **Testability**: Pure function - easy to unit test

---

## Next Steps

1. **Build and test** the implementation
2. **Enable OHLCV in monitor** configuration file
3. **Run the system** and verify bar generation
4. **Future**: Integrate with Trader component for strategy signals
