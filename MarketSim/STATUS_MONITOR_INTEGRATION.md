# Status Monitor Integration Guide

This guide shows how to integrate the Status Monitor into existing MarketSim components.

## Quick Start

1. **Include the headers:**
```cpp
#include "monitor/status_monitor.h"
#include "monitor/monitor_helpers.h"
```

2. **Start monitoring in your main:**
```cpp
int main() {
    // Start periodic monitoring (prints every 5 seconds)
    marketsim::monitor::StatusMonitor::instance().start_periodic_monitoring();
    
    // Your application code...
    
    // Stop monitoring before exit
    marketsim::monitor::StatusMonitor::instance().stop_periodic_monitoring();
}
```

## Integration Examples

### Example 1: Monitoring Exchange Threads

Update your thread classes to use monitoring:

```cpp
// In order_ingest_thread.h
#include "monitor/monitor_helpers.h"

class OrderIngestThread {
public:
    void start() {
        running_ = true;
        thread_ = std::thread(&OrderIngestThread::run, this);
    }
    
private:
    void run() {
        // Register this thread with the monitor
        marketsim::monitor::MonitoredThread monitor("OrderIngestThread");
        
        while (running_) {
            monitor.update_state(marketsim::monitor::ThreadState::RUNNING);
            
            // Process orders...
            auto order = order_queue_.wait_and_pop();
            if (order) {
                process_order(*order);
                monitor.increment_tasks();
            } else {
                monitor.update_state(marketsim::monitor::ThreadState::IDLE);
            }
        }
        
        monitor.update_state(marketsim::monitor::ThreadState::TERMINATED);
    }
    
    std::thread thread_;
    std::atomic<bool> running_;
};
```

### Example 2: Monitoring ZeroMQ Sockets

Add monitoring to your IOHandler:

```cpp
// In your socket wrapper class
#include "monitor/monitor_helpers.h"

class MarketDataPublisher {
public:
    MarketDataPublisher(zmq::context_t& ctx, const std::string& endpoint) 
        : socket_(ctx, zmq::socket_type::pub)
        , monitor_("market_data_pub", 
                   marketsim::monitor::SocketType::PUB, 
                   endpoint)
    {
        socket_.bind(endpoint);
        monitor_.update_state(marketsim::monitor::SocketState::LISTENING);
    }
    
    void publish(const std::string& data) {
        try {
            socket_.send(zmq::buffer(data), zmq::send_flags::none);
            monitor_.record_send(data.size());
        } catch (const zmq::error_t& e) {
            monitor_.record_error(e.what());
            monitor_.update_state(marketsim::monitor::SocketState::ERROR);
        }
    }
    
private:
    zmq::socket_t socket_;
    marketsim::monitor::MonitoredSocket monitor_;
};
```

### Example 3: OrderBookReader Integration

Here's how to add monitoring to the existing OrderBookReader:

```cpp
// In orderbook_reader.h
#include "monitor/monitor_helpers.h"

class OrderBookReader {
public:
    explicit OrderBookReader(ThreadSafeQueue<Order*>& order_queue)
        : order_queue_(order_queue)
        , running_(false)
        , orders_processed_(0)
    {}
    
    void start() {
        running_ = true;
        thread_ = std::thread(&OrderBookReader::run, this);
        LOG_INFO("OrderBookReader started");
    }
    
private:
    void run() {
        // Add monitoring
        marketsim::monitor::MonitoredThread monitor("OrderBookReader");
        
        while (running_) {
            monitor.update_state(marketsim::monitor::ThreadState::RUNNING);
            
            auto order = order_queue_.wait_and_pop_for(std::chrono::milliseconds(100));
            
            if (!order) {
                monitor.update_state(marketsim::monitor::ThreadState::IDLE);
                continue;
            }
            
            process_order(order.value());
            orders_processed_++;
            monitor.increment_tasks();  // Track processed orders
        }
        
        monitor.update_state(marketsim::monitor::ThreadState::TERMINATED);
    }
    
    // ... rest of the class
};
```

### Example 4: ProxyOrderClient Integration

Add socket monitoring to the client:

```cpp
// In proxy_order_client.h
#include "monitor/monitor_helpers.h"

class ProxyOrderClient {
public:
    ProxyOrderClient(ThreadSafeQueue<Order*>& order_queue)
        : order_queue_(order_queue)
        , running_(false)
    {}
    
    void start() {
        running_ = true;
        thread_ = std::thread(&ProxyOrderClient::run, this);
        
        // Register a mock socket for demonstration
        marketsim::monitor::StatusMonitor::instance().register_socket(
            "ProxyOrderClient",
            marketsim::monitor::SocketType::PUSH,
            "inproc://orders"
        );
        marketsim::monitor::StatusMonitor::instance().update_socket_state(
            "ProxyOrderClient",
            marketsim::monitor::SocketState::CONNECTED
        );
    }
    
private:
    void run() {
        marketsim::monitor::MonitoredThread monitor("ProxyOrderClient");
        
        while (running_) {
            monitor.update_state(marketsim::monitor::ThreadState::RUNNING);
            
            auto order = create_random_order();
            order_queue_.push(order);
            
            // Track "sending" through the queue as socket activity
            marketsim::monitor::StatusMonitor::instance().record_socket_send(
                "ProxyOrderClient", 
                sizeof(Order)
            );
            
            monitor.increment_tasks();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        marketsim::monitor::StatusMonitor::instance().update_socket_state(
            "ProxyOrderClient",
            marketsim::monitor::SocketState::DISCONNECTED
        );
    }
    
    // ... rest of the class
};
```

## Custom Status Reporting

Instead of console output, you can export to a file or dashboard:

```cpp
#include "monitor/status_monitor.h"
#include <fstream>

void setup_file_logging() {
    marketsim::monitor::StatusMonitor::instance().set_status_callback(
        [](const std::vector<marketsim::monitor::ThreadInfo>& threads,
           const std::vector<marketsim::monitor::SocketInfo>& sockets) {
            
            std::ofstream log("status_report.txt", std::ios::app);
            
            auto now = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(now);
            
            log << "=== Status Report: " << std::ctime(&time);
            log << "Active Threads: " << threads.size() << "\n";
            log << "Active Sockets: " << sockets.size() << "\n";
            
            for (const auto& thread : threads) {
                log << "  " << thread.name << ": " 
                    << to_string(thread.state) << "\n";
            }
            
            log << std::endl;
        }
    );
}
```

## Performance Considerations

The Status Monitor is designed for minimal overhead:

1. **Lock-free for most operations**: Updates use simple mutex locks
2. **Separate monitoring thread**: Status collection doesn't block workers
3. **Configurable intervals**: Adjust monitoring frequency based on needs
4. **RAII helpers**: Zero-cost abstractions for automatic registration

### Recommended Settings

- **Development**: 1-3 second intervals for detailed tracking
- **Production**: 10-30 second intervals for lightweight monitoring
- **High-frequency trading**: Consider disabling or 60+ second intervals

```cpp
// Production setting
StatusMonitor::instance().start_periodic_monitoring(std::chrono::seconds(30));
```

## Best Practices

1. **Use meaningful names**: Name threads/sockets descriptively
   ```cpp
   MonitoredThread monitor("OrderProcessor-AAPL");
   ```

2. **Update states accurately**: Reflect actual thread states
   ```cpp
   monitor.update_state(ThreadState::BLOCKED);  // Waiting on lock
   // ... acquire lock ...
   monitor.update_state(ThreadState::RUNNING);   // Processing
   ```

3. **Track errors**: Always record socket errors
   ```cpp
   catch (const std::exception& e) {
       socket_monitor.record_error(e.what());
   }
   ```

4. **Use RAII helpers**: Automatic cleanup prevents resource leaks
   ```cpp
   void worker() {
       MonitoredThread mt("Worker");  // Auto-registers
       // ... work ...
   }  // Auto-unregisters
   ```

5. **Periodic health checks**: Let the monitor detect stale connections
   - Threads idle > 30s marked as IDLE
   - Sockets inactive > 60s flagged for review

## Testing

Run the test program to verify monitoring:

```bash
./test_status_monitor
```

Expected output shows periodic status updates with thread and socket metrics.

## Troubleshooting

**Q: Thread not showing up in status?**
- Ensure `register_thread` is called from within the thread
- Check that thread hasn't already terminated

**Q: Socket counts seem wrong?**
- Verify `record_send`/`record_receive` are called after successful operations
- Check for exceptions that skip recording

**Q: High monitoring overhead?**
- Increase monitoring interval
- Reduce callback complexity
- Consider disabling in production

## Future Enhancements

Potential additions:
- Metrics export (Prometheus, StatsD)
- Alert thresholds with callbacks
- Historical trend analysis
- Web dashboard integration
- Thread CPU usage tracking (platform-specific)
- Memory usage per component
