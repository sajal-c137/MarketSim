#pragma once

#include <zmq.hpp>
#include <memory>
#include <string>
#include <mutex>

namespace marketsim::io_handler {

/**
 * @brief Manages ZeroMQ context lifecycle
 * 
 * Each component (Generator, Exchange, Trader) should have its own IOContext.
 * This ensures complete isolation between components.
 */
class IOContext {
public:
    /**
     * @brief Construct a new IOContext
     * @param io_threads Number of I/O threads for this context (default: 1)
     */
    explicit IOContext(int io_threads = 1);
    
    /**
     * @brief Get the underlying ZMQ context
     * @return Reference to zmq::context_t
     */
    zmq::context_t& get_context();
    
    /**
     * @brief Shutdown the context gracefully
     */
    void shutdown();
    
    /**
     * @brief Check if context is still active
     */
    bool is_active() const;
    
private:
    std::unique_ptr<zmq::context_t> context_;
    mutable std::mutex mutex_;
    bool active_;
};

}
