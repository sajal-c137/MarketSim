# Exchange

Core matching engine responsible for order matching, order book management, and market data distribution.

## Responsibilities

- Maintain order books for all traded instruments
- Perform order matching (price-time priority)
- Process incoming orders from Traders and TrafficGenerator
- Generate and distribute market data feeds
- Maintain trade history and market state
- Handle order cancellations and modifications

## Dependencies

- Monitor: For logging exchange operations and metrics
- IOHandler: For distributing market data

## Communication

- **Input**: 
  - Orders from Trader (buy/sell/cancel)
  - Market data from TrafficGenerator
- **Output**: 
  - Market data feeds to Trader and TrafficGenerator
  - Trade confirmations
  - Order book updates
