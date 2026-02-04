# Exchange Mappers

Convert between protobuf messages and internal C++ objects.

## Contents

- **Order Mapper**: Maps between protobuf Order messages and internal Order objects
- **Trade Mapper**: Maps between protobuf Trade messages and internal Trade objects
- **Market Data Mapper**: Maps between protobuf market data messages and internal structures
- **Order Book Mapper**: Maps internal order book to protobuf OrderBook message

## Purpose

Keeps protobuf dependencies isolated from core business logic.
Allows internal data structures to be optimized for performance without being constrained by protobuf.
