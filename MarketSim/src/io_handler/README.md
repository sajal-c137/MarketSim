# IOHandler

Unified I/O abstraction layer for reading and writing market data across the system.

## Responsibilities

- Abstract ZeroMQ socket operations
- Serialize/deserialize messages using Protobuf
- Provide consistent API for market data read/write
- Handle connection management and reconnection logic
- Buffer management for high-throughput scenarios
- Thread-safe I/O operations

## Cross-Language & Distributed Support

Designed to enable services in **different languages** on **different devices**:

- **Language-Agnostic Protocol**: Uses Protobuf + ZeroMQ
  - Any language with ZeroMQ and Protobuf bindings can communicate
  - No shared memory or language-specific serialization
  
- **Network-Transparent**: Services can run:
  - On the same machine (localhost)
  - Across different machines (TCP/IP)
  - In containers or VMs
  
- **Service Independence**: Each component has its own IOHandler implementation
  - Exchange in C++ communicates with Trader in Python
  - No tight coupling between service implementations

## Design Goals

- Single point of I/O logic (DRY principle)
- Decouples communication details from business logic
- Simplifies testing with mock implementations
- Type-safe message handling via Protobuf
- Language and platform independence

## Used By

- **traffic_generator**: Reading from and writing to Exchange
- **trader**: Reading market data from Exchange

## Communication Patterns

Supports multiple ZeroMQ patterns:
- **PUB-SUB**: Market data broadcasts (one-to-many)
- **REQ-REP**: Order submissions (request-response)
- **PUSH-PULL**: Work distribution (load balancing)

Each pattern works identically across languages and platforms.
