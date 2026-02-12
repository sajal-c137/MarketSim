#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>

namespace marketsim::exchange::utils {

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() : stopped_(false) {}
    
    ~ThreadSafeQueue() {
        stop();
    }
    
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) {
                return;
            }
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }
    
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    std::optional<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        
        if (stopped_ && queue_.empty()) {
            return std::nullopt;
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    
    template<typename Rep, typename Period>
    std::optional<T> wait_and_pop_for(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (!cv_.wait_for(lock, timeout, [this] { return !queue_.empty() || stopped_; })) {
            return std::nullopt;
        }
        
        if (stopped_ && queue_.empty()) {
            return std::nullopt;
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }
    
    bool is_stopped() const {
        return stopped_.load();
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }
    
private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
    std::atomic<bool> stopped_;
};

}
