# TrafficGenerator Operations

Core mathematical models for market data generation and simulation. **This module contains pure market dynamics models with NO I/O, threading, or networking code.**

## Design Principle

Keep mathematical models 100% isolated from:
- ? ZeroMQ or any networking
- ? Threading or concurrency
- ? File I/O or system calls
- ? Pure math, algorithms, statistical models

This separation enables:
- Unit testing models without I/O infrastructure
- Swapping models without touching threading code
- Clear data dependencies between components
- Easy model validation and verification

## Responsibilities

- Calculate realistic price movements using mathematical models:
  - Jump diffusion processes
  - Mean reversion models
  - Momentum/trend following
  - Random walks with drift
- Generate synthetic order flow based on statistical distributions
- Simulate various market conditions (normal, volatile, trending, etc.)
- Calculate OHLCV (candlestick) data from price ticks
- All calculations are **stateless or depend only on input parameters**

## Key Components

### PriceMovementCalculator
Pure mathematical price dynamics:
- Jump diffusion: price jumps + continuous diffusion
- Mean reversion: tendency to revert to long-term mean
- Momentum: trending behavior
- Volatility: market regime impact on movement size
- Input: current price, volatility, trend parameters
- Output: next price (no side effects)

### OrderFlowGenerator
Statistical order generation:
- Size distributions (normal, log-normal)
- Price level clustering (realistic order book formation)
- Buy/sell imbalance patterns
- Cancellation probabilities
- Input: symbol, mid-price, order count target
- Output: list of synthetic orders (no I/O)

### MarketConditionSimulator
Market regime simulation:
- **Normal**: Low volatility, mean-reverting
- **Volatile**: High volatility, larger swings
- **Trending**: Directional momentum with continuity
- **Ranging**: Oscillating within bounds
- **Gapped**: Sudden jumps (gaps)
- Regime transitions with configurable probabilities
- Input: current condition, parameters
- Output: next condition, volatility level

### OHLCVBuilder
Candlestick aggregation (pure math):
- Accumulates ticks into OHLCV bars
- Tracks open, high, low, close, volume
- Time-based bar alignment
- No dependency on external time services
- Input: ticks with price/volume
- Output: completed OHLCV bars

## Dependencies

- **utils/**: Random number generation, statistical helpers
- **models/**: Data structures (MarketState, GenerationParameters)
- ? No IOHandler, threading, or external I/O

## Testing Strategy

Each operation module is testable in isolation:
```cpp
// Example: Pure math test, no setup needed
PriceMovementCalculator calc;
double next_price = calc.calculate_next_price(100.0, 0.02, 0.1);
// Verify result without any I/O infrastructure
```

## Files

- `price_movement_calculator.cpp/h` - Price dynamics calculation
- `order_flow_generator.cpp/h` - Synthetic order generation
- `market_condition_simulator.cpp/h` - Market regime simulation
- `ohlcv_builder.cpp/h` - Candlestick data aggregation
