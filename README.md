# **FOR RECRUITERS: This Project is WIP**

## Expected Completion: **February 20th**


# MarketSim

> **A high-performance, distributed market simulation system demonstrating advanced C++20, concurrent programming, and quantitative finance concepts.**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.10+-064F8C.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## Project Overview

MarketSim is a **real-time market exchange simulator** that models realistic financial market microstructure using stochastic processes. The system implements a **distributed architecture** with separate components for order matching, traffic generation, and monitoring—all communicating via ZeroMQ.

**Key Highlights:**
- **Price-Time Priority Matching Engine** with FIFO order book
- **Advanced Price Models**: Linear, GBM, and Hawkes self-exciting processes
- **Distributed Architecture**: Language-agnostic components via Protobuf
- **Real-Time Monitoring**: Live order book visualization and metrics
- **Mathematical Rigor**: Statistical distributions, stochastic calculus

---

## Architecture

### **Design Principles**

| Principle | Implementation |
|-----------|----------------|
| **Separation of Concerns** | Pure mathematical models isolated from I/O and threading |
| **Distributed-Ready** | Components communicate via ZeroMQ sockets (REQ-REP, PUB-SUB) |
| **Language-Agnostic** | Protobuf message definitions enable polyglot systems |
| **Testability** | Unit-testable components with mock-friendly interfaces |
| **Performance** | Lock-free data structures, C++20 optimizations, zero-copy messaging |

### **System Components**

```
┌─────────────────────────────────────────────────────────────┐
│                     MarketSim System                         │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────┐      ┌──────────────┐      ┌──────────┐ │
│  │   Traffic    │─────▶│   Exchange   │◀─────│ Monitor  │ │
│  │  Generator   │ REQ  │   Service    │ REQ  │ Service  │ │
│  └──────────────┘      └──────────────┘      └──────────┘ │
│         │                     │                     │       │
│         │                     │                     │       │
│    ┌────▼────┐          ┌────▼─────┐         ┌────▼────┐ │
│    │ Hawkes  │          │ Matching │         │ Logger  │ │
│    │  Model  │          │  Engine  │         │ History │ │
│    └─────────┘          └──────────┘         └─────────┘ │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

---

## Features

### **1. Exchange Service**
- **Matching Engine**: Price-time priority (FIFO) order matching algorithm
- **Order Book**: Real-time bid/ask depth with price level aggregation
- **Multi-Symbol Support**: Concurrent handling of multiple instruments
- **Price History**: Trade and mid-price tracking with configurable buffer sizes
- **ZMQ REQ-REP**: Synchronous order submission and status queries

### **2. Traffic Generator**
Advanced market data generation using **pluggable price models**:

| Model | Description | Use Case |
|-------|-------------|----------|
| **Linear** | Constant price growth | Baseline testing |
| **GBM** | Geometric Brownian Motion (μ, σ) | Classic stochastic process |
| **Hawkes** | Self-exciting point process with momentum | Realistic microstructure simulation |

**Hawkes Microstructure Model** combines:
- Price evolution via GBM with configurable drift/volatility
- Self-exciting order arrivals (Hawkes process)
- Momentum-based buy/sell direction (logistic function)
- Power-law price offsets (Pareto distribution)
- Log-normal volume generation

### **3. Monitor Service**
- **Real-Time Display**: Live order book, trade feed, and statistics
- **History Recording**: Time-series data export for analysis
- **Configurable Polling**: Adjustable update frequency
- **Formatted Output**: `tabulate` library for clean console tables

### **4. I/O Handler**
- **ZMQ Abstractions**: Type-safe wrappers (Requester, Replier, Publisher, Subscriber)
- **Protobuf Serialization**: Automatic message marshaling
- **Context Management**: RAII-based lifecycle handling

---

## Technology Stack

### **Core Technologies**
- **C++20**: Concepts, ranges, coroutines (planned)
- **CMake 3.10+**: Build system with vcpkg integration
- **Ninja**: Fast parallel builds

### **Libraries**
- **ZeroMQ (cppzmq)**: High-performance asynchronous messaging
- **Protocol Buffers**: Efficient binary serialization
- **tabulate**: Console table formatting

### **Mathematical Tools**
- Custom distributions (Exponential, Bernoulli, Truncated Pareto, Log-Normal)
- Logistic function for probability mapping
- Random number generation with seeding support

---

## Quick Start

### **Prerequisites**
```bash
# Windows (Visual Studio 2022)
- CMake 3.10+
- vcpkg (included in repo)
- C++20-compliant compiler (MSVC 19.29+)
```

### **Build**
```bash
# Clone repository
git clone https://github.com/sajal-c137/MarketSim.git
cd MarketSim

# Configure with CMake preset
cmake --preset x64-debug

# Build all targets
cmake --build out/build/x64-debug --parallel
```

### **Run the System**

Open **4 separate terminals** and run:

#### **Terminal 1: Exchange Server**
```bash
.\out\build\x64-debug\MarketSim\test_exchange_server.exe
```
> Listens on `tcp://*:5555` (orders) and `tcp://*:5557` (status)

#### **Terminal 2: Traffic Generator (Hawkes Model)**
```bash
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe hawkes
```
> Generates realistic order flow with self-exciting dynamics

#### **Terminal 3: Monitor**
```bash
.\out\build\x64-debug\MarketSim\monitor_exchange.exe AAPL
```
> Real-time display of order book and trades

#### **Terminal 4: Manual Order Client (Optional)**
```bash
.\out\build\x64-debug\MarketSim\manual_order_client.exe
```
> Interactive order submission for testing

---

## Project Structure

```
MarketSim/
├── MarketSim/
│   ├── src/
│   │   ├── common/              # Shared utilities
│   │   │   └── math/            # Statistical distributions
│   │   ├── exchange/            # Order matching engine
│   │   │   ├── config/          # Configuration
│   │   │   ├── data/            # Price history, ticks
│   │   │   ├── main/            # ExchangeService orchestrator
│   │   │   └── operations/      # MatchingEngine, OrderBook
│   │   ├── io_handler/          # ZMQ wrappers, serialization
│   │   ├── monitor/             # Logging, history, monitoring
│   │   └── traffic_generator/   # Market data generation
│   │       ├── models/          # Price models (Linear, GBM, Hawkes)
│   │       ├── operations/      # Pure math generators
│   │       └── threads/         # Producer-consumer pattern
│   ├── test/                    # Executables and integration tests
│   └── CMakeLists.txt           # Build configuration
├── proto/
│   └── src/common/
│       └── exchange.proto       # Message definitions
└── README.md
```

---

## Technical Deep Dive

### **Hawkes Microstructure Model**

The Hawkes model simulates realistic order arrival dynamics:

**Intensity Function:**
```
λ(t) = μ + α · Σ exp(-β(t - tᵢ))
```
Where:
- `μ`: Baseline order rate (events/sec)
- `α`: Self-excitation coefficient
- `β`: Decay rate (mean reversion speed)

**Order Direction:**
```
P(BUY) = σ(k · Δp)
```
Where `σ` is the logistic function and `Δp` is price momentum.

**Price Offset Distribution:**
```
f(x) = (α · L^α) / x^(α+1)  for L ≤ x ≤ xₘₐₓ
```
Truncated Pareto with tail index `α` ∈ [1.5, 2.5]

### **Matching Engine Algorithm**

1. **Price-Time Priority**: Orders matched at best price, FIFO within level
2. **Partial Fills**: Large orders split across multiple price levels
3. **Trade Generation**: Creates trade records with unique IDs
4. **Mid-Price Tracking**: (Best Bid + Best Ask) / 2

---

## Example Output

```
═══════════════════════════════════════════════════════════════
                    MARKET EXCHANGE MONITOR
═══════════════════════════════════════════════════════════════

[ORDER #342] BUY AAPL @ $100.05 qty=5.0 (TrafficGenerator)

[PRICE UPDATE] $100.03 | Trades: 168 | Volume: 840.00

╔══════════════════════════════════════════════════════════════╗
║                        ORDER BOOK - AAPL                     ║
╠══════════════════════════════════════════════════════════════╣
║       BID SIDE       │   SPREAD: $0.06   │      ASK SIDE     ║
╠══════════════════════╪═══════════════════╪═════════=═════════╣
║  Price  │  Quantity  │                   │  Price  │ Quantity║
╠─────────┼────────────┤                   ├─────────┼─────────║
║ $100.02 │      8.00  │                   │ $100.08 │    6.00 ║
║ $100.01 │     12.00  │                   │ $100.09 │   10.00 ║
║ $100.00 │      5.00  │                   │ $100.10 │    8.00 ║
╚═════════╧════════════╧═══════════════════╧═════════╧═════════╝
```

---

## Key Concepts Demonstrated

- **Concurrent Programming**: Multi-threaded architecture with thread-safe queues
- **Design Patterns**: Factory, Strategy, Producer-Consumer, RAII
- **Financial Mathematics**: Stochastic processes, statistical distributions
- **Distributed Systems**: Message-passing concurrency, service-oriented architecture
- **Modern C++**: Smart pointers, move semantics, RAII, templated libraries
- **Software Engineering**: Clean architecture, dependency injection, testability

---

## License

This project is licensed under the MIT License

---

## Author

**Sajal Agarwal**
- GitHub: [@sajal-c137](https://github.com/sajal-c137)
- LinkedIn: [@sajalagarwal33](https://www.linkedin.com/in/sajalagarwal33/)

---

<div align="center">

**If you find this project interesting, please consider giving it a star!**

</div>
