# MarketSim

## ?? **FOR RECRUITERS: WORK IN PROGRESS**

### Expected Completion: **February 24th**

A high-performance market simulation system written in C++20.

## Architecture Principles

- **Language-Agnostic Interfaces**: Protobuf message definitions allow components in different languages
- **Distributed-Ready**: ZeroMQ enables components to run on different machines/containers
- **Service Independence**: Each component can be implemented and deployed separately
- **Cross-Platform**: Works on Windows, Linux, macOS

## Project Structure

```
MarketSim/
??? MarketSim/
    ??? src/
    ?   ??? traffic_generator/    # Market data generation
    ?   ??? exchange/             # Order matching and market operations
    ?   ??? trader/               # Trading strategies and execution
    ?   ??? monitor/              # System observability (logging, metrics, alerts)
    ?   ??? io_handler/           # Unified I/O operations for market data
    ?   ??? common/               # Shared types, interfaces, and utilities
    ??? MarketSim.cpp             # Main entry point
    ??? MarketSim.h               # Main header
```

## Components

- **traffic_generator**: Generates realistic market traffic and price movements
- **exchange**: Core matching engine and order book management
- **trader**: Strategy execution and order management
- **monitor**: Logging, metrics collection, and system monitoring
- **io_handler**: Abstraction layer for reading/writing market data
- **common**: Shared types, interfaces, and utilities

## Dependencies

- cppzmq (ZeroMQ C++ bindings) - Inter-component communication
- protobuf - Message serialization
- CMake 3.10+ - Build system
- C++20 - Language standard

## Building

```bash
cmake --preset x64-debug
cmake --build out/build/x64-debug
```

## Running

```bash
./out/build/x64-debug/MarketSim/MarketSim.exe
```
