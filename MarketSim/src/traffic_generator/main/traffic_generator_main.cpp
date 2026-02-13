#include "traffic_generator_main.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace marketsim::traffic_generator::main {

TrafficGeneratorMain::TrafficGeneratorMain(const std::string& endpoint)
    : endpoint_(endpoint)
    , io_context_(1) // 1 I/O thread
{
    std::cout << "[TrafficGeneratorMain] Initialized with endpoint: " << endpoint_ << "\n";
}

TrafficGeneratorMain::~TrafficGeneratorMain() {
    stop();
    io_context_.shutdown();
}

void TrafficGeneratorMain::initialize(const models::GenerationParameters& params) {
    std::cout << "[TrafficGeneratorMain] Initializing generation thread...\n";
    
    generation_thread_ = std::make_unique<threads::GenerationThread>(
        params,
        io_context_,
        endpoint_
    );
}

void TrafficGeneratorMain::start() {
    if (!generation_thread_) {
        std::cerr << "[TrafficGeneratorMain] Error: Call initialize() first\n";
        return;
    }
    
    std::cout << "[TrafficGeneratorMain] Starting generation...\n";
    generation_thread_->start();
}

void TrafficGeneratorMain::stop() {
    if (generation_thread_) {
        std::cout << "[TrafficGeneratorMain] Stopping generation...\n";
        generation_thread_->stop();
    }
}

void TrafficGeneratorMain::wait_for_completion() {
    if (!generation_thread_) {
        return;
    }
    
    // Poll until generation is complete
    while (generation_thread_->is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "[TrafficGeneratorMain] Generation completed\n";
    
    // Explicitly stop and clean up
    stop();
    io_context_.shutdown();
    
    std::cout << "[TrafficGeneratorMain] Shutdown complete\n";
}

bool TrafficGeneratorMain::is_running() const {
    return generation_thread_ && generation_thread_->is_running();
}

} // namespace marketsim::traffic_generator::main
