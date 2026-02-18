# Monitor

Real-time exchange monitoring and data recording.

## Features

- Live order book display
- Trade feed logging
- OHLCV candlestick generation (1s bars)
- Price history recording

## Structure

```
monitor/
├── exchange_monitor.cpp    # Main monitoring loop
├── exchange_logger.cpp     # Console output formatting
├── history_recorder.cpp    # CSV file writing
└── monitor_config.h        # Configuration
```

## Configuration

```cpp
polling_interval_ms = 100      // Fast polling
ohlcv_interval_seconds = 1     // 1-second bars
enable_history_recording = true
```

## Output

- Console: Live orderbook and trades
- CSV: `market_history/AAPL_ohlcv.csv`


- **Performance**: Latency, throughput, CPU/memory usage
- **Business**: Order counts, trade volumes, P&L
- **System**: Connection status, error rates, queue depths
- **Runtime**: Thread states, socket statistics, message counts

## Status Monitor

The Status Monitor provides real-time visibility into system threads and network sockets.

### Thread Monitoring

Tracks thread lifecycle and activity:
- Thread registration with unique names
- State tracking (RUNNING, IDLE, BLOCKED, TERMINATED)
- Task completion counters
- Idle time detection
- Automatic health checks

### Socket Monitoring

Monitors ZeroMQ socket health:
- Socket type tracking (REQ/REP, PUB/SUB, PUSH/PULL, etc.)
- Connection state (CONNECTED, DISCONNECTED, ERROR, etc.)
- Message and byte counters (sent/received)
- Error tracking with descriptions
- Endpoint information

### Usage

#### Basic Usage

```cpp
#include "monitor/status_monitor.h"

using namespace marketsim::monitor;

// Start periodic monitoring (prints every 5 seconds by default)
StatusMonitor::instance().start_periodic_monitoring();

// Register a thread
StatusMonitor::instance().register_thread(
    std::this_thread::get_id(), 
    "OrderProcessor"
);

// Register a socket
StatusMonitor::instance().register_socket(
    "order_socket", 
    SocketType::REQ, 
    "tcp://localhost:5555"
);

// Update states
StatusMonitor::instance().update_thread_state(
    std::this_thread::get_id(), 
    ThreadState::RUNNING
);

StatusMonitor::instance().update_socket_state(
    "order_socket", 
    SocketState::CONNECTED
);

// Record activity
StatusMonitor::instance().record_socket_send("order_socket", 256);
StatusMonitor::instance().increment_thread_tasks(std::this_thread::get_id());
```

#### RAII Helpers

For automatic lifecycle management:

```cpp
#include "monitor/monitor_helpers.h"

void worker_thread(const std::string& name) {
    // Auto-registers on construction, unregisters on destruction
    MonitoredThread monitor(name);
    
    while (running) {
        monitor.update_state(ThreadState::RUNNING);
        // Do work...
        monitor.increment_tasks();
    }
}

void network_handler() {
    MonitoredSocket socket("my_socket", SocketType::PUB, "tcp://*:5555");
    
    socket.update_state(SocketState::CONNECTED);
    
    // Send data
    socket.record_send(1024);
    
    // Handle errors
    if (error_occurred) {
        socket.record_error("Connection timeout");
    }
}
```

#### Custom Callbacks

Set a custom handler instead of console printing:

```cpp
StatusMonitor::instance().set_status_callback(
    [](const std::vector<ThreadInfo>& threads, 
       const std::vector<SocketInfo>& sockets) {
        // Log to file, send to dashboard, etc.
        for (const auto& thread : threads) {
            std::cout << thread.name << ": " << to_string(thread.state) << std::endl;
        }
    }
);
```

### Status Output

The monitor prints comprehensive status reports:

```
================================================================================
  SYSTEM STATUS MONITOR
================================================================================
Timestamp: Wed Feb 12 18:45:32 2026

--- THREAD STATUS ---
Thread Name              State          Tasks          Idle Time (s)       
--------------------------------------------------------------------------------
OrderProcessor           RUNNING        1234           0                   
TradeExecutor           IDLE           856            12                  
DataAggregator          RUNNING        2045           0                   

--- SOCKET STATUS ---
Socket Name         Type      State          Sent      Recv      Errors    
--------------------------------------------------------------------------------
order_socket        REQ       CONNECTED      1500      1498      0         
market_data         SUB       CONNECTED      0         45632     0         
trade_pub           PUB       CONNECTED      3421      0         1         
    Last error: Timeout on message receive

--- SUMMARY ---
Active Threads: 2 / 3
Active Sockets: 3 / 3
Total Messages Sent: 4921
Total Messages Received: 47130
================================================================================
```

### API Reference

**Thread Monitoring:**
- `register_thread(thread_id, name)` - Register a thread
- `update_thread_state(thread_id, state)` - Update thread state
- `update_thread_activity(thread_id)` - Mark activity timestamp
- `increment_thread_tasks(thread_id)` - Increment task counter
- `unregister_thread(thread_id)` - Mark thread as terminated

**Socket Monitoring:**
- `register_socket(name, type, endpoint)` - Register a socket
- `update_socket_state(name, state)` - Update socket state
- `record_socket_send(name, bytes)` - Record sent message
- `record_socket_receive(name, bytes)` - Record received message
- `record_socket_error(name, error)` - Record error
- `unregister_socket(name)` - Mark socket as disconnected

**Control:**
- `start_periodic_monitoring(interval)` - Start background monitoring
- `stop_periodic_monitoring()` - Stop background monitoring
- `set_status_callback(callback)` - Set custom status handler
- `print_status()` - Manually print status report

**Statistics:**
- `active_thread_count()` - Count of active threads
- `active_socket_count()` - Count of active sockets
- `total_messages_sent()` - Total messages across all sockets
- `total_messages_received()` - Total received messages
- `get_thread_status()` - Get all thread info
- `get_socket_status()` - Get all socket info
