#include "exchange/main/exchange_service.h"
#include <iostream>

using namespace marketsim;

/**
 * Exchange Server Entry Point
 * 
 * Just instantiates and runs the ExchangeService.
 * All logic is in src/exchange/main/exchange_service.cpp
 */
int main() {
    try {
        exchange::main::ExchangeService service;
        service.run();
    } catch (const std::exception& e) {
        std::cerr << "[EXCHANGE] Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
