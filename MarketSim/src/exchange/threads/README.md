# Exchange Threads

Thread implementations for the 5 main responsibilities of the exchange.

## Threads

1. **OrderBook Updater Thread** - Updates the order book based on incoming orders
2. **Matching Engine Thread** - Matches orders and updates current price
3. **Order Ingest Thread** - Receives and validates incoming orders from clients
4. **Order Response Thread** - Sends acknowledgements and responses to clients
5. **Market Data Server Thread** - Serves client requests for price and order book snapshots

## Threading Model

Each thread runs independently with lock-free queues for inter-thread communication where possible.
