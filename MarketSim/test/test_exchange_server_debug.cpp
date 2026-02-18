#include <iostream>
#include <windows.h>

/**
 * Minimal test to verify executable starts
 */
int main() {
    std::cout << "TEST: Program started\n" << std::flush;
    
    try {
        std::cout << "TEST: About to include exchange service\n" << std::flush;
        
        #include "exchange/main/exchange_service.h"
        
        std::cout << "TEST: Creating service\n" << std::flush;
        marketsim::exchange::main::ExchangeService service;
        
        std::cout << "TEST: Running service\n" << std::flush;
        service.run();
        
    } catch (const std::exception& e) {
        std::cerr << "TEST ERROR: " << e.what() << "\n" << std::flush;
        std::cerr << "Press Enter to exit..." << std::flush;
        std::cin.get();
        return 1;
    }
    
    return 0;
}
