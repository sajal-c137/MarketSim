# Exchange Operations

Core business logic for the exchange.

## Contents

- **Order Book**: Maintains bid/ask levels, order priority
- **Matching Engine**: Matches buy and sell orders based on price-time priority
- **Price Calculator**: Calculates current price, OHLCV data, statistics
- **Order Validator**: Validates incoming orders (price limits, quantities, etc.)
- **Trade Generator**: Creates trade records from matched orders

## Design

These are the core domain objects that implement the business rules of the exchange.
Thread-safe where necessary for concurrent access.
