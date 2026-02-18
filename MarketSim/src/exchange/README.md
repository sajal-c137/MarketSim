# Exchange

Order matching engine with price-time priority.

## Features

- Price-time priority (FIFO) matching
- Multi-symbol order books
- Trade and price history tracking
- ZeroMQ REQ-REP interface
- Maintain trade history and market state
- Handle order cancellations and modifications

```
exchange/
├── operations/     # MatchingEngine, OrderBook
├── main/          # ExchangeService
└── data/          # PriceHistory, Tick
```

## Endpoints

- `tcp://*:5555` - Order submission (REQ-REP)
- `tcp://*:5557` - Status queries (REQ-REP)

- **Input**: 
  - Orders from Trader (buy/sell/cancel)
  - Market data from TrafficGenerator
- **Output**: 
  - Market data feeds to Trader and TrafficGenerator
  - Trade confirmations
  - Order book updates
