# TrafficGenerator

Generates realistic market traffic data by continuously calculating and producing price movements.

## Responsibilities

- Read current market state from Exchange (via IOHandler)
- Calculate next price data based on market dynamics
- Generate realistic order flow patterns
- Send generated market data to Exchange
- Simulate various market conditions (normal, volatile, trending, etc.)

## Dependencies

- IOHandler: For reading from and writing to Exchange
- Monitor: For logging traffic generation metrics
- Exchange: Receives generated market data

## Communication

- **Input**: Market data from Exchange (via IOHandler)
- **Output**: Generated price updates and orders to Exchange (via ZeroMQ)
