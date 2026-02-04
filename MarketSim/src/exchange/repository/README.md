# Exchange Repository

Data persistence layer for orders, trades, and market data.

## Contents

- **Order Repository**: Stores and retrieves order records
- **Trade Repository**: Stores and retrieves trade history
- **Market Data Repository**: Stores historical OHLCV and market stats
- **Order Book Snapshot Repository**: Stores order book snapshots for recovery

## Storage

- In-memory for hot data (current orders, recent trades)
- Redis for cold storage (historical data, snapshots)
- Configurable retention policies
