#pragma once

#include <string>
#include <cstdint>

namespace marketsim::exchange::models {

struct MarketStats {
    std::string symbol;
    int64_t timestamp;
    
    double last_price;
    double open_price;
    double high_price;
    double low_price;
    
    double total_volume;
    uint64_t trade_count;
    
    double volume_24h;
    double high_24h;
    double low_24h;
    double price_change_24h;
    double price_change_pct_24h;
    
    double bid_price;
    double bid_quantity;
    double ask_price;
    double ask_quantity;
    double spread;
    
    MarketStats()
        : timestamp(0)
        , last_price(0.0)
        , open_price(0.0)
        , high_price(0.0)
        , low_price(0.0)
        , total_volume(0.0)
        , trade_count(0)
        , volume_24h(0.0)
        , high_24h(0.0)
        , low_24h(0.0)
        , price_change_24h(0.0)
        , price_change_pct_24h(0.0)
        , bid_price(0.0)
        , bid_quantity(0.0)
        , ask_price(0.0)
        , ask_quantity(0.0)
        , spread(0.0)
    {}
    
    explicit MarketStats(std::string sym)
        : symbol(std::move(sym))
        , timestamp(0)
        , last_price(0.0)
        , open_price(0.0)
        , high_price(0.0)
        , low_price(0.0)
        , total_volume(0.0)
        , trade_count(0)
        , volume_24h(0.0)
        , high_24h(0.0)
        , low_24h(0.0)
        , price_change_24h(0.0)
        , price_change_pct_24h(0.0)
        , bid_price(0.0)
        , bid_quantity(0.0)
        , ask_price(0.0)
        , ask_quantity(0.0)
        , spread(0.0)
    {}
    
    void update_trade(double price, double volume, int64_t ts) {
        last_price = price;
        timestamp = ts;
        total_volume += volume;
        trade_count++;
        
        if (open_price == 0.0) {
            open_price = price;
        }
        
        if (high_price == 0.0 || price > high_price) {
            high_price = price;
        }
        
        if (low_price == 0.0 || price < low_price) {
            low_price = price;
        }
    }
    
    void update_bbo(double bid_p, double bid_q, double ask_p, double ask_q) {
        bid_price = bid_p;
        bid_quantity = bid_q;
        ask_price = ask_p;
        ask_quantity = ask_q;
        spread = ask_price - bid_price;
    }
    
    double mid_price() const {
        return (bid_price + ask_price) / 2.0;
    }
    
    double spread_bps() const {
        double mid = mid_price();
        return mid > 0.0 ? (spread / mid) * 10000.0 : 0.0;
    }
    
    double average_trade_size() const {
        return trade_count > 0 ? total_volume / trade_count : 0.0;
    }
};

struct OHLCV {
    std::string symbol;
    int64_t timestamp;
    int32_t interval_seconds;
    
    double open;
    double high;
    double low;
    double close;
    double volume;
    uint32_t trade_count;
    
    OHLCV()
        : timestamp(0)
        , interval_seconds(60)
        , open(0.0)
        , high(0.0)
        , low(0.0)
        , close(0.0)
        , volume(0.0)
        , trade_count(0)
    {}
    
    OHLCV(std::string sym, int64_t ts, int32_t interval)
        : symbol(std::move(sym))
        , timestamp(ts)
        , interval_seconds(interval)
        , open(0.0)
        , high(0.0)
        , low(0.0)
        , close(0.0)
        , volume(0.0)
        , trade_count(0)
    {}
    
    void update(double price, double qty) {
        if (open == 0.0) {
            open = price;
        }
        
        if (high == 0.0 || price > high) {
            high = price;
        }
        
        if (low == 0.0 || price < low) {
            low = price;
        }
        
        close = price;
        volume += qty;
        trade_count++;
    }
    
    double price_change() const {
        return close - open;
    }
    
    double price_change_pct() const {
        return open > 0.0 ? ((close - open) / open) * 100.0 : 0.0;
    }
    
    double average_price() const {
        return (open + high + low + close) / 4.0;
    }
};

}
