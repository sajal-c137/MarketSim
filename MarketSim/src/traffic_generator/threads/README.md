# TrafficGenerator Threads

Thread management and background processing for the TrafficGenerator component.

## Responsibility

Orchestrate mathematical models (from operations/) with I/O and threading, but **do not contain math logic itself**.

The threads:
1. Call mathematical models (operations/) with current market state
2. Receive calculated results
3. Serialize and publish via IOHandler
4. Coordinate timing and thread lifecycle

## Key Components

### GenerationThread
Main loop for continuous market data generation:
- Calls PriceMovementCalculator (operations/) to compute next price
- Calls OrderFlowGenerator (operations/) to create synthetic orders
- Reads current market state from Exchange (via IOHandler)
- Publishes generated data via ZeroMQ (IOHandler)
- Manages generation timing and thread lifecycle
- Tracks metrics (messages/sec, orders generated, etc.)
- **Does not contain math** - delegates to operations/

### OHLCVAggregatorThread
Aggregates real-time ticks into candlesticks:
- Calls OHLCVBuilder (operations/) with price ticks
- Receives completed OHLCV bars
- Publishes completed bars to subscribers via IOHandler
- Handles bar transitions and timing
- Manages thread lifecycle
- **Does not compute OHLCV** - delegates to operations/

### PublisherThread (optional)
Batches and publishes market data:
- Accumulates generated data in queue
- Batches updates for publishing efficiency
- Publishes via ZeroMQ PUB socket (IOHandler)
- Tracks publishing latency
- Manages thread lifecycle

## Data Flow

```
Operations (pure math)
    ?
  Models output (price, orders, etc.)
    ?
  GenerationThread/OHLCVAggregatorThread (coordinate & publish)
    ?
  IOHandler (serialize & send via ZeroMQ)
    ?
  Exchange (receives market data)
```

## Files

- `generation_thread.cpp/h` - Main generation loop
- `ohlcv_aggregator_thread.cpp/h` - Candlestick aggregation orchestration
- `publisher_thread.cpp/h` - Message publishing coordination

## Thread Synchronization

- Thread-safe queues for data passing
- Lock-free data structures where possible
- Coordination via Monitor (status tracking, health checks)
- Graceful shutdown signaling

## Dependencies

- **Operations**: Core mathematical models (calls into these)
- **IOHandler**: ZeroMQ socket management
- **Monitor**: Thread state tracking and metrics
- **Models**: Data structures
