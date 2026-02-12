#pragma once

#include <string>
#include <cstdint>

namespace marketsim::exchange::models {

enum class OrderSide : uint8_t {
    BUY = 0,
    SELL = 1
};

enum class OrderType : uint8_t {
    LIMIT = 0,
    MARKET = 1
};

enum class OrderStatus : uint8_t {
    PENDING = 0,
    ACCEPTED = 1,
    PARTIALLY_FILLED = 2,
    FILLED = 3,
    CANCELLED = 4,
    REJECTED = 5
};

struct Order {
    std::string order_id;
    std::string symbol;
    std::string client_id;
    
    OrderSide side;
    OrderType type;
    OrderStatus status;
    
    double price;
    double quantity;
    double filled_quantity;
    
    int64_t timestamp;
    int64_t accepted_timestamp;
    
    Order()
        : side(OrderSide::BUY)
        , type(OrderType::LIMIT)
        , status(OrderStatus::PENDING)
        , price(0.0)
        , quantity(0.0)
        , filled_quantity(0.0)
        , timestamp(0)
        , accepted_timestamp(0)
    {}
    
    Order(std::string id, std::string sym, std::string client, 
          OrderSide s, OrderType t, double p, double q, int64_t ts)
        : order_id(std::move(id))
        , symbol(std::move(sym))
        , client_id(std::move(client))
        , side(s)
        , type(t)
        , status(OrderStatus::PENDING)
        , price(p)
        , quantity(q)
        , filled_quantity(0.0)
        , timestamp(ts)
        , accepted_timestamp(0)
    {}
    
    bool is_buy() const { return side == OrderSide::BUY; }
    bool is_sell() const { return side == OrderSide::SELL; }
    bool is_limit() const { return type == OrderType::LIMIT; }
    bool is_market() const { return type == OrderType::MARKET; }
    bool is_active() const { 
        return status == OrderStatus::PENDING || 
               status == OrderStatus::ACCEPTED || 
               status == OrderStatus::PARTIALLY_FILLED; 
    }
    bool is_filled() const { return status == OrderStatus::FILLED; }
    bool is_cancelled() const { return status == OrderStatus::CANCELLED; }
    bool is_rejected() const { return status == OrderStatus::REJECTED; }
    
    bool can_be_cancelled() const {
        return status == OrderStatus::PENDING || 
               status == OrderStatus::ACCEPTED || 
               status == OrderStatus::PARTIALLY_FILLED;
    }
    
    bool has_fills() const {
        return filled_quantity > 0.0;
    }
    
    double remaining_quantity() const { return quantity - filled_quantity; }
    double fill_percentage() const { 
        return quantity > 0.0 ? (filled_quantity / quantity) * 100.0 : 0.0; 
    }
};

inline const char* to_string(OrderSide side) {
    switch (side) {
        case OrderSide::BUY: return "BUY";
        case OrderSide::SELL: return "SELL";
        default: return "UNKNOWN";
    }
}

inline const char* to_string(OrderType type) {
    switch (type) {
        case OrderType::LIMIT: return "LIMIT";
        case OrderType::MARKET: return "MARKET";
        default: return "UNKNOWN";
    }
}

inline const char* to_string(OrderStatus status) {
    switch (status) {
        case OrderStatus::PENDING: return "PENDING";
        case OrderStatus::ACCEPTED: return "ACCEPTED";
        case OrderStatus::PARTIALLY_FILLED: return "PARTIALLY_FILLED";
        case OrderStatus::FILLED: return "FILLED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        case OrderStatus::REJECTED: return "REJECTED";
        default: return "UNKNOWN";
    }
}

}
