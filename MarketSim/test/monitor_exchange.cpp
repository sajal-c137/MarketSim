#include "monitor/exchange_monitor.h"
#include <iostream>

using namespace marketsim;

/**
 * Monitor Service Entry Point
 * 
 * Just instantiates and runs the ExchangeMonitor service.
 * All logic is in src/monitor/exchange_monitor.cpp
 */
int main() {
    try {
        // Create monitor service
        monitor::ExchangeMonitor monitor("tcp://localhost:5557");
        
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
