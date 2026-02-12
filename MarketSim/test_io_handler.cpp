#include "io_handler/io_context.h"
#include "io_handler/zmq_publisher.h"
#include "io_handler/zmq_subscriber.h"
#include "io_handler/zmq_requester.h"
#include "io_handler/zmq_replier.h"
#include "monitor/status_monitor.h"
#include "exchange.pb.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace marketsim::io_handler;
using namespace marketsim::monitor;
using namespace marketsim::exchange;

/**
 * Simulates the Exchange component with its own IOContext and thread pool
 */
class ExchangeSimulator {
public:
    ExchangeSimulator() 
        : context_(1)
        , publisher_(context_, "Exchange_MarketData_Pub", "tcp://*:5555")
        , order_server_(context_, "Exchange_Order_Server", "tcp://*:5556")
        , running_(false)
    {}
    
    void start() {
        running_ = true;
        publisher_.bind();
        order_server_.bind();
        
        std::cout << "[Exchange] Started on ports 5555 (market data) and 5556 (orders)\n";
        
        market_data_thread_ = std::thread(&ExchangeSimulator::publish_market_data, this);
        order_processing_thread_ = std::thread(&ExchangeSimulator::process_orders, this);
    }
    
    void stop() {
        running_ = false;
        if (market_data_thread_.joinable()) market_data_thread_.join();
        if (order_processing_thread_.joinable()) order_processing_thread_.join();
        publisher_.close();
        order_server_.close();
    }
    
private:
    void publish_market_data() {
        MonitoredThread monitor("Exchange_MarketData_Thread");
        
        // Give subscriber time to connect
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        int count = 0;
        while (running_ && count < 5) {
            monitor.update_state(ThreadState::RUNNING);
            
            Quote quote;
            quote.set_symbol("AAPL");
            quote.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
            quote.set_bid_price(150.0 + count);
            quote.set_bid_quantity(100);
            quote.set_ask_price(150.5 + count);
            quote.set_ask_quantity(200);
            quote.set_spread(0.5);
            
            MarketDataMessage msg;
            *msg.mutable_quote() = quote;
            
            if (publisher_.publish_with_topic("AAPL", msg)) {
                std::cout << "[Exchange] Published quote " << (count + 1) << "/5 for AAPL: bid=" << quote.bid_price() << "\n";
                monitor.increment_tasks();
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            count++;
        }
        
        std::cout << "[Exchange] Market data publishing complete\n";
        monitor.update_state(ThreadState::TERMINATED);
    }
    
    void process_orders() {
        MonitoredThread monitor("Exchange_Order_Thread");
        
        int count = 0;
        while (running_ && count < 3) {
            monitor.update_state(ThreadState::RUNNING);
            
            OrderMessage order_msg;
            if (order_server_.receive_request(order_msg, 1000)) {
                const Order& order = order_msg.new_order();
                std::cout << "[Exchange] Received order: " << order.order_id() 
                          << " for " << order.symbol() << "\n";
                
                OrderAck ack;
                ack.set_order_id(order.order_id());
                ack.set_status(OrderStatus::ACCEPTED);
                ack.set_message("Order accepted");
                ack.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
                
                ExchangeResponse response;
                *response.mutable_order_ack() = ack;
                
                if (order_server_.send_response(response)) {
                    std::cout << "[Exchange] Sent ack for order: " << order.order_id() << "\n";
                    monitor.increment_tasks();
                    count++;
                }
            }
        }
        
        monitor.update_state(ThreadState::TERMINATED);
    }
    
    IOContext context_;
    ZmqPublisher publisher_;
    ZmqReplier order_server_;
    std::thread market_data_thread_;
    std::thread order_processing_thread_;
    std::atomic<bool> running_;
};

/**
 * Simulates a Trader component with its own IOContext and thread pool
 */
class TraderSimulator {
public:
    TraderSimulator()
        : context_(1)
        , subscriber_(context_, "Trader_MarketData_Sub", "tcp://localhost:5555")
        , order_client_(context_, "Trader_Order_Client", "tcp://localhost:5556")
        , running_(false)
    {}
    
    void start() {
        running_ = true;
        subscriber_.connect();
        subscriber_.subscribe("AAPL");
        order_client_.connect();
        
        std::cout << "[Trader] Connected to Exchange\n";
        
        market_data_thread_ = std::thread(&TraderSimulator::consume_market_data, this);
        trading_thread_ = std::thread(&TraderSimulator::send_orders, this);
    }
    
    void stop() {
        running_ = false;
        if (market_data_thread_.joinable()) market_data_thread_.join();
        if (trading_thread_.joinable()) trading_thread_.join();
        subscriber_.close();
        order_client_.close();
    }
    
private:
    void consume_market_data() {
        MonitoredThread monitor("Trader_MarketData_Thread");
        
        int count = 0;
        while (running_ && count < 5) {
            monitor.update_state(ThreadState::RUNNING);
            
            std::string topic;
            MarketDataMessage msg;
            
            if (subscriber_.receive_with_topic(topic, msg)) {
                if (msg.has_quote()) {
                    const Quote& quote = msg.quote();
                    std::cout << "[Trader] Received quote: " << quote.symbol() 
                              << " bid=" << quote.bid_price() 
                              << " ask=" << quote.ask_price() << "\n";
                    monitor.increment_tasks();
                    count++;
                }
            }
        }
        
        monitor.update_state(ThreadState::TERMINATED);
    }
    
    void send_orders() {
        MonitoredThread monitor("Trader_Trading_Thread");
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        for (int i = 0; i < 3 && running_; ++i) {
            monitor.update_state(ThreadState::RUNNING);
            
            Order order;
            order.set_order_id("ORD_" + std::to_string(i + 1));
            order.set_symbol("AAPL");
            order.set_side(OrderSide::BUY);
            order.set_type(OrderType::LIMIT);
            order.set_price(150.0 + i);
            order.set_quantity(100);
            order.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
            order.set_client_id("TRADER_001");
            
            OrderMessage order_msg;
            *order_msg.mutable_new_order() = order;
            
            ExchangeResponse response;
            if (order_client_.request_with_timeout(order_msg, response, 2000)) {
                if (response.has_order_ack()) {
                    const OrderAck& ack = response.order_ack();
                    std::cout << "[Trader] Received ack: " << ack.order_id() 
                              << " status=" << OrderStatus_Name(ack.status()) << "\n";
                    monitor.increment_tasks();
                }
            } else {
                std::cout << "[Trader] Timeout waiting for ack\n";
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }
        
        monitor.update_state(ThreadState::TERMINATED);
    }
    
    IOContext context_;
    ZmqSubscriber subscriber_;
    ZmqRequester order_client_;
    std::thread market_data_thread_;
    std::thread trading_thread_;
    std::atomic<bool> running_;
};

int main() {
    std::cout << "=== IOHandler Integration Test ===\n";
    std::cout << "Demonstrates separate IOContext instances per component\n\n";
    
    StatusMonitor::instance().start_periodic_monitoring(std::chrono::seconds(3));
    
    ExchangeSimulator exchange;
    TraderSimulator trader;
    
    std::cout << "\n--- Starting Components ---\n";
    
    exchange.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    trader.start();
    
    std::cout << "\n--- Components Running ---\n";
    std::cout << "Exchange and Trader have separate IOContext instances\n";
    std::cout << "Each manages its own thread pool and sockets\n\n";
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    std::cout << "\n--- Stopping Components ---\n";
    
    trader.stop();
    exchange.stop();
    
    std::cout << "\n--- Final Status ---\n";
    StatusMonitor::instance().print_status();
    
    StatusMonitor::instance().stop_periodic_monitoring();
    
    std::cout << "\n=== Test Complete ===\n";
    std::cout << "? Exchange had its own IOContext and thread pool\n";
    std::cout << "? Trader had its own IOContext and thread pool\n";
    std::cout << "? No shared objects between sender and receiver\n";
    std::cout << "? Each component managed its own sockets independently\n";
    
    return 0;
}
