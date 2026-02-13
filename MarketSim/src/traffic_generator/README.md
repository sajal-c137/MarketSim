# TrafficGenerator

Generates realistic market traffic data by continuously calculating and producing price movements using mathematical models (jump diffusion, etc.).

## Architecture Principle

**Math ? Networking/Threading**: Mathematical models (price movements, order generation) are isolated from I/O and threading infrastructure. This separation enables:
- Pure mathematical models testable independently
- Clean, maintainable market dynamics code
- Easy model swapping and upgrades
- Clear data flow from models ? threading ? I/O

## Responsibilities

- Calculate next price data using mathematical models (jump diffusion, etc.)
- Generate realistic order flow patterns using statistical models
- Simulate various market conditions (normal, volatile, trending, etc.)
- Read current market state from Exchange (via IOHandler)
- Send generated market data to Exchange
- Coordinate generation, aggregation, and publishing threads

## Module Organization

- **operations/**: Pure mathematical models (isolated from I/O)
  - Price movement calculators (jump diffusion, mean reversion, etc.)
  - Order flow generators (statistical models)
  - Market condition simulators
- **models/**: Data structures for market state and generation parameters
- **threads/**: Threading and coordination logic
  - Orchestrates models via operations/
  - Manages I/O via IOHandler
- **handlers/**: Message handling and market state updates
- **utils/**: Mathematical utilities (random numbers, statistics)
- **configs/**: Configuration management
- **main/**: High-level orchestration and initialization

## Dependencies

- IOHandler: For reading from and writing to Exchange
- Monitor: For logging traffic generation metrics
- Exchange: Receives generated market data

## Communication

- **Input**: Market data from Exchange (via IOHandler)
- **Output**: Generated price updates and orders to Exchange (via ZeroMQ)
