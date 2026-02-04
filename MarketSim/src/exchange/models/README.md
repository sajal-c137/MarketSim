# Exchange Models

Internal data models and domain objects.

## Contents

- **Order Model**: Internal order representation (optimized for performance)
- **Trade Model**: Internal trade record
- **Order Book Level**: Internal representation of price levels
- **Market Stats**: Internal market statistics

## Design

These are the internal C++ objects used by the exchange.
Separate from protobuf messages for performance optimization.
Memory-efficient, cache-friendly layouts.
