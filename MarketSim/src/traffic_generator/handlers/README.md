# TrafficGenerator Handlers

Message and data handling for the TrafficGenerator component.

## Responsibilities

- Parse market data received from Exchange
- Validate incoming messages
- Handle subscription requests (if any)
- Error handling and recovery
- Message deserialization from Protobuf

## Key Components

### MarketDataHandler
Handles incoming market data from Exchange:
- Deserializes Protobuf messages
- Validates message format and data integrity
- Updates internal market state
- Detects anomalies or gaps

### SubscriptionHandler (optional)
Manages subscription requests from consumers:
- Tracks active subscribers
- Filters based on symbol/data type
- Manages subscription lifecycle

## Files

- `market_data_handler.cpp/h` - Incoming market data processing
- `subscription_handler.cpp/h` - Subscription management (optional)

## Message Flow

```
Exchange (MarketData) 
  ? [Protobuf message]
IOHandler (ZeroMQ subscriber)
  ? [Deserialization]
MessageSerializer
  ? [C++ object]
MarketDataHandler
  ? [Validation]
Internal MarketState
```

## Dependencies

- IOHandler: ZeroMQ integration and message serialization
- Models: C++ data structures
- Monitors: Error logging
