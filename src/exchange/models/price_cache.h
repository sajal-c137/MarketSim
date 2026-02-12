#pragma once

#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>

namespace marketsim::exchange::models {

struct alignas(64) SymbolPriceData {
    std::atomic<double> last_price{0.0};
    std::atomic<int64_t> last_trade_time{0};
    std::atomic<double> best_bid{0.0};
    std::atomic<double> best_ask{0.0};
    std::atomic<double> volume_24h{0.0};
    std::atomic<uint64_t> trade_count{0};
    
    void update_trade(double price, double volume, int64_t timestamp) {
        last_price.store(price, std::memory_order_relaxed);
        last_trade_time.store(timestamp, std::memory_order_relaxed);
        volume_24h.fetch_add(volume, std::memory_order_relaxed);
        trade_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    void update_bbo(double bid, double ask) {
        best_bid.store(bid, std::memory_order_relaxed);
        best_ask.store(ask, std::memory_order_relaxed);
    }
    
    double get_spread() const {
        double bid = best_bid.load(std::memory_order_relaxed);
        double ask = best_ask.load(std::memory_order_relaxed);
        return ask - bid;
    }
    
    double get_mid_price() const {
        double bid = best_bid.load(std::memory_order_relaxed);
        double ask = best_ask.load(std::memory_order_relaxed);
        return (bid + ask) / 2.0;
    }
};

class PriceCache {
public:
    PriceCache() = default;
    ~PriceCache() = default;
    
    SymbolPriceData* get_or_create(const std::string& symbol);
    SymbolPriceData* get(const std::string& symbol) const;
    
    void update_trade(const std::string& symbol, double price, double volume, int64_t timestamp);
    void update_bbo(const std::string& symbol, double bid, double ask);
    
    double get_last_price(const std::string& symbol) const;
    double get_best_bid(const std::string& symbol) const;
    double get_best_ask(const std::string& symbol) const;
    double get_spread(const std::string& symbol) const;
    double get_mid_price(const std::string& symbol) const;
    
private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, SymbolPriceData> prices_;
};

}
