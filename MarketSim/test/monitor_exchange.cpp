#include "monitor/exchange_monitor.h"
#include "monitor/monitor_config.h"
#include <iostream>

using namespace marketsim;

/**
 * Monitor Service Entry Point
 * 
 * Usage: monitor_exchange [ticker]
 * Example: monitor_exchange AAPL
 * 
 * All logic is in src/monitor/exchange_monitor.cpp
 */
int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        monitor::MonitorConfig config;
        
        if (argc > 1) {
            config.ticker = argv[1];
            std::cout << "[MONITOR] Monitoring ticker: " << config.ticker << "\n";
        } else {
            std::cout << "[MONITOR] No ticker specified, using default: " << config.ticker << "\n";
            std::cout << "[MONITOR] Usage: monitor_exchange [ticker]\n";
            std::cout << "[MONITOR] Example: monitor_exchange TSLA\n\n";
        }
        
        // Create monitor service
        monitor::ExchangeMonitor monitor(config);
        
        // Start monitoring
        monitor.start();
        
        // Wait for completion (runs until Ctrl+C)
        monitor.wait();
        
    } catch (const std::exception& e) {
        std::cerr << "[MONITOR] FATAL: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
