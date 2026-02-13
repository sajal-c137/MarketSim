# TrafficGenerator Configs

Configuration management for the TrafficGenerator component.

## Responsibilities

- Load configuration from files or environment
- Provide validated configuration access
- Handle default values
- Configuration validation and error handling
- Configuration schema documentation

## Key Configuration Areas

### Exchange Connection
- Exchange publisher endpoint (for receiving market data)
- Exchange listener endpoint (for sending orders/data)
- Reconnection strategy and timeouts

### Generation Parameters
- Base price for each symbol
- Volatility parameters (normal, volatile modes)
- Order frequency (orders per second)
- Order size distribution
- Price movement thresholds

### Market Conditions
- Initial market condition (Normal, Volatile, Trending, etc.)
- Condition transition probabilities
- Condition duration ranges
- Volatility levels per condition

### Publishing
- Market data publishing interval (microseconds)
- OHLCV bar intervals (1m, 5m, 15m)
- Batch size for publishing
- Message buffering strategy

### Monitoring
- Thread names and priorities
- Metrics collection frequency
- Logging levels
- Health check intervals

### Symbols and Markets
- Tradeable symbols
- Tick size per symbol
- Initial bid/ask spreads
- Price ranges (min/max)
- Market hours (if applicable)

## Files

- `traffic_generator_config.cpp/h` - Main configuration class

## Configuration Format

Configuration can be loaded from:
- YAML/JSON files
- Environment variables
- Command-line arguments
- Code defaults

## Example Configuration Structure

```yaml
exchange:
  publisher_endpoint: "tcp://localhost:5556"
  listener_endpoint: "tcp://localhost:5557"

generation:
  symbols:
    - symbol: "AAPL"
      base_price: 150.0
      tick_size: 0.01
      normal_volatility: 0.02
      volatile_volatility: 0.05
  
  orders_per_second: 100
  order_size_mean: 100.0
  order_size_std_dev: 50.0

market_conditions:
  initial: "NORMAL"
  transition_probability: 0.01

ohlcv:
  intervals_seconds: [60, 300, 900]  # 1m, 5m, 15m

monitoring:
  metrics_interval_ms: 1000
  log_level: "INFO"
```

## Dependencies

- No external config library (keep lightweight)
- Uses standard C++20 features
