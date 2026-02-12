#include "io_context.h"

namespace marketsim::io_handler {

IOContext::IOContext(int io_threads) 
    : context_(std::make_unique<zmq::context_t>(io_threads))
    , active_(true)
{}

zmq::context_t& IOContext::get_context() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!active_) {
        throw std::runtime_error("IOContext has been shutdown");
    }
    return *context_;
}

void IOContext::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (active_) {
        context_->shutdown();
        context_->close();
        active_ = false;
    }
}

bool IOContext::is_active() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
}

}
