# TrafficGenerator Models

C++ data structures and models used by the TrafficGenerator component.

## Responsibilities

- Define market state representations
- Define market condition enumerations
- Define generation parameters and configurations
- Provide model utilities and helpers

## Key Models

### MarketState
Represents current market snapshot:
- Current price (bid, mid, ask)
- Volatility estimate
- Trend direction and strength
- Order book depth
- Recent trade activity

### MarketCondition
Enumeration of market regimes:
- Normal
- Volatile
- Trending
- Ranging
- Gapped

### GenerationParameters
Configuration for generation behavior:
- Order size distribution (mean, std dev)
- Order frequency (orders per second)
- Spread ranges (tick-based offsets)
- Price jump probabilities
- Cancellation rates
- Volume profiles

### PriceMovement
Single price movement data:
- Previous close/mid price
- New price (bid, mid, ask)
- Movement size (ticks, percentage)
- Volatility reading
- Movement direction

### SyntheticOrder
Generated order representation:
- Order ID
- Symbol
- Side (Buy/Sell)
- Quantity
- Price level (bid, mid, ask offset)
- Lifetime/expiration
- Cancellation probability

## Files

- `market_state_model.h` - Market state structures
- `market_condition_model.h` - Condition enumeration
- `generation_parameters.h` - Generation configuration
- `price_movement_model.h` - Price movement data
- `synthetic_order_model.h` - Synthetic order structure

## Cross-Component Compatibility

Models are designed to map cleanly to Protobuf messages defined in:
- `proto/src/common/exchange.proto` (Order, OrderBook, OHLCV, Trade)

This enables seamless serialization/deserialization via the IOHandler MessageSerializer.
