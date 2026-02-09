#pragma once

#include <string>
#include <cstdint>
#include "order_model.h"

namespace marketsim::exchange::models {

struct Trade {
    std::string trade_id;
    std::string symbol;
    
    std::string buyer_order_id;
    std::string seller_order_id;
    std::string buyer_client_id;
    std::string seller_client_id;
    
    double price;
    double quantity;
    
    int64_t timestamp;
    OrderSide aggressor_side;
    
    Trade()
        : price(0.0)
        , quantity(0.0)
        , timestamp(0)
        , aggressor_side(OrderSide::BUY)
    {}
    
    Trade(std::string id, std::string sym, 
          std::string buyer_oid, std::string seller_oid,
          std::string buyer_cid, std::string seller_cid,
          double p, double q, int64_t ts, OrderSide aggressor)
        : trade_id(std::move(id))
        , symbol(std::move(sym))
        , buyer_order_id(std::move(buyer_oid))
        , seller_order_id(std::move(seller_oid))
        , buyer_client_id(std::move(buyer_cid))
        , seller_client_id(std::move(seller_cid))
        , price(p)
        , quantity(q)
        , timestamp(ts)
        , aggressor_side(aggressor)
    {}
    
    double notional_value() const { return price * quantity; }
    bool is_buyer_aggressor() const { return aggressor_side == OrderSide::BUY; }
    bool is_seller_aggressor() const { return aggressor_side == OrderSide::SELL; }
};

}
