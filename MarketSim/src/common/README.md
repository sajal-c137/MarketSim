# Common

Shared types, interfaces, data structures, and utilities used across all components.

## Contents

- **Interfaces**: Abstract base classes for strategies, I/O handlers, etc.
- **Message Definitions**: Protobuf `.proto` files for all message types
- **Data Structures**: Order, Trade, MarketData, OrderBook types
- **Constants**: Configuration values, magic numbers, enumerations
- **Utilities**: Time handling, string formatting, math helpers

## Cross-Language & Cross-Platform Design

This module ensures services can be developed in **different languages** and run on **different devices**:

- **Protobuf Message Definitions**: Language-agnostic interface contracts
  - Can generate bindings for C++, Python, Java, Go, Rust, etc.
  - Guarantees wire compatibility across implementations
  
- **ZeroMQ Communication Protocol**: Platform-independent messaging
  - Works across network boundaries
  - No shared memory requirements
  - Services can run on separate machines

## Interface Contracts

All inter-service communication is defined through:
- `.proto` files (no language-specific types in interfaces)
- Well-defined message schemas
- Version-compatible protocol evolution

This allows:
- Exchange written in C++, Trader in Python
- Components distributed across multiple machines
- Mix and match implementations per component

## Examples

- `messages.proto`: All message definitions (Orders, Trades, MarketData)
- `IStrategy`: Abstract strategy interface for Trader
- `Config`: System-wide configuration constants

## Dependencies

- No dependencies on other MarketSim components
- All other components depend on Common for message definitions
