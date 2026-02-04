# Trader

Executes trading strategies by reading market data and sending orders to the Exchange.

## Responsibilities

- Read market data from Exchange (via IOHandler)
- Execute trading strategies (abstract strategy pattern)
- Make buy/sell decisions based on strategy logic
- Send orders to Exchange
- Track positions and P&L
- Risk management and position sizing

## Strategy Abstraction

The Trader component uses an abstract strategy interface allowing:
- Multiple strategy implementations
- Hot-swapping strategies at runtime
- Strategy combination and portfolio management
- Backtesting capabilities

## Dependencies

- IOHandler: For reading market data from Exchange
- Monitor: For logging trading decisions and performance metrics
- Exchange: Receives trading orders

## Communication

- **Input**: Market data from Exchange (via IOHandler)
- **Output**: Trading orders to Exchange (via ZeroMQ)
