# Exchange Models

Internal data models and domain objects for the exchange.

## Contents

- **`order_model.h`**: Order representation with status tracking
- **`trade_model.h`**: Trade execution records
- **`orderbook_level_model.h`**: Price level structures
- **`orderbook_model.h`**: Complete order book with O(1) lookup
- **`market_stats_model.h`**: Market statistics and OHLCV
- **`price_cache.h`**: Thread-safe atomic price cache

## Design

**Order Book**: Hash map per price level + FIFO deque for time priority
- O(1) order lookup by (price, order_id)
- Sparse representation (no gaps)
- Partial fills stay at front of queue

**Separation from Protobuf**: These are internal C++ structs optimized for performance. Mappers handle conversion at I/O boundaries.
