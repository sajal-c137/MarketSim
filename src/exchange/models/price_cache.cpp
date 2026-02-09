#include "price_cache.h"

namespace marketsim::exchange::models {

SymbolPriceData* PriceCache::get_or_create(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    return &prices_[symbol];
}

SymbolPriceData* PriceCache::get(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = prices_.find(symbol);
    return it != prices_.end() ? const_cast<SymbolPriceData*>(&it->second) : nullptr;
}

void PriceCache::update_trade(const std::string& symbol, double price, double volume, int64_t timestamp) {
    auto* data = get_or_create(symbol);
    if (data) {
        data->update_trade(price, volume, timestamp);
    }
}

void PriceCache::update_bbo(const std::string& symbol, double bid, double ask) {
    auto* data = get_or_create(symbol);
    if (data) {
        data->update_bbo(bid, ask);
    }
}

double PriceCache::get_last_price(const std::string& symbol) const {
    auto* data = get(symbol);
    return data ? data->last_price.load(std::memory_order_relaxed) : 0.0;
}

double PriceCache::get_best_bid(const std::string& symbol) const {
    auto* data = get(symbol);
    return data ? data->best_bid.load(std::memory_order_relaxed) : 0.0;
}

double PriceCache::get_best_ask(const std::string& symbol) const {
    auto* data = get(symbol);
    return data ? data->best_ask.load(std::memory_order_relaxed) : 0.0;
}

double PriceCache::get_spread(const std::string& symbol) const {
    auto* data = get(symbol);
    return data ? data->get_spread() : 0.0;
}

double PriceCache::get_mid_price(const std::string& symbol) const {
    auto* data = get(symbol);
    return data ? data->get_mid_price() : 0.0;
}

}
