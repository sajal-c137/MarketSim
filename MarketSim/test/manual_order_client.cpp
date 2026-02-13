#include "io_handler/io_context.h"
#include "io_handler/zmq_requester.h"
#include "exchange.pb.h"
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>

using namespace marketsim;

void print_help() {
    std::cout << "\n===========================================\n";
    std::cout << "Manual Order Client\n";
    std::cout << "===========================================\n";
    std::cout << "Format: <ticker> <price> <qty> <buy|sell>\n";
    std::cout << "Example: AAPL 150.50 10 buy\n";
    std::cout << "Commands:\n";
    std::cout << "  help  - Show this help\n";
    std::cout << "  quit  - Exit the client\n";
    std::cout << "===========================================\n\n";
}

bool parse_and_send_order(
    const std::string& input,
    io_handler::ZmqRequester& requester,
    int& order_counter)
{
    std::istringstream iss(input);
    std::string ticker, side_str;
    double price, qty;
    
    // Parse: ticker price qty buy/sell
    if (!(iss >> ticker >> price >> qty >> side_str)) {
        std::cerr << "[ERROR] Invalid format. Use: <ticker> <price> <qty> <buy|sell>\n";
        return false;
    }
    
    // Validate side
    bool is_buy;
    if (side_str == "buy" || side_str == "BUY") {
        is_buy = true;
    } else if (side_str == "sell" || side_str == "SELL") {
        is_buy = false;
    } else {
        std::cerr << "[ERROR] Side must be 'buy' or 'sell', got: " << side_str << "\n";
        return false;
    }
    
    // Create order
    exchange::Order order;
    order.set_order_id("MANUAL-" + std::to_string(++order_counter));
    order.set_symbol(ticker);
    order.set_side(is_buy ? exchange::OrderSide::BUY : exchange::OrderSide::SELL);
    order.set_type(exchange::OrderType::LIMIT);
    order.set_price(price);
    order.set_quantity(qty);
    order.set_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    order.set_client_id("ManualClient");
    
    // Send order and receive ack
    std::cout << "[SENDING] " << ticker << " " 
              << (is_buy ? "BUY" : "SELL") << " " 
              << qty << " @ $" << price << "\n";
    
    exchange::OrderAck ack;
    try {
        if (requester.request(order, ack)) {
            std::cout << "[ACK] Order " << ack.order_id() 
                      << " - Status: ";
            
            switch (ack.status()) {
                case exchange::OrderStatus::ACCEPTED:
                    std::cout << "ACCEPTED";
                    break;
                case exchange::OrderStatus::REJECTED:
                    std::cout << "REJECTED";
                    break;
                case exchange::OrderStatus::FILLED:
                    std::cout << "FILLED";
                    break;
                default:
                    std::cout << "UNKNOWN";
            }
            
            std::cout << " - " << ack.message() << "\n";
            return true;
        } else {
            std::cerr << "[ERROR] Failed to send order\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception: " << e.what() << "\n";
        return false;
    }
}

int main() {
    print_help();
    
    try {
        // Connect to Exchange
        io_handler::IOContext io_context(1);
        io_handler::ZmqRequester requester(
            io_context,
            "ManualClient",
            "tcp://localhost:5555"
        );
        
        std::cout << "[CLIENT] Connecting to Exchange on tcp://localhost:5555...\n";
        requester.connect();
        std::cout << "[CLIENT] Connected!\n\n";
        
        int order_counter = 0;
        std::string input;
        
        while (true) {
            std::cout << "Order> ";
            std::getline(std::cin, input);
            
            // Trim whitespace
            input.erase(0, input.find_first_not_of(" \t\n\r"));
            input.erase(input.find_last_not_of(" \t\n\r") + 1);
            
            if (input.empty()) {
                continue;
            }
            
            // Check for commands
            if (input == "quit" || input == "exit") {
                std::cout << "[CLIENT] Exiting...\n";
                break;
            }
            
            if (input == "help") {
                print_help();
                continue;
            }
            
            // Parse and send order
            parse_and_send_order(input, requester, order_counter);
            std::cout << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
