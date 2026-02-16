#pragma once

#include "i_price_model.h"
#include "../../operations/gbm_price_generator.h"
#include "common/math/distributions.h"
#include "../generation_parameters.h"
#include "common/math/random.h"
#include <memory>
#include <vector>
#include <deque>

namespace marketsim::traffic_generator::models::price_models {

/**
 * @brief Hawkes Microstructure Model - Advanced market simulation
 * 
 * Combines multiple stochastic processes to simulate realistic market dynamics:
 * 
 * Step 1: Price evolution using Geometric Brownian Motion
 * Step 2: Order arrivals using self-exciting Hawkes process
 * Step 3: Order direction via logistic function of price momentum
 * Step 4: Price placement using truncated power law (Pareto)
 * Step 5: Volume generation using log-normal distribution
 * Step 6: Order cloud generation at each event
 * 
 * References:
 * - Hawkes (1971): "Spectra of some self-exciting and mutually exciting point processes"
 * - Cont, Stoikov, Talreja (2010): "A Stochastic Model for Order Book Dynamics"
 */
class HawkesMicrostructureModel : public IPriceModel {
public:
    /**
     * @brief Represents a generated order with all attributes
     */
    struct Order {
        double time;       // Event time (seconds)
        bool is_buy;       // true = BUY, false = SELL
        double price;      // Limit order price
        double volume;     // Order size
        uint64_t order_id; // Unique identifier
    };
    
    /**
     * @brief Construct Hawkes microstructure model
     * @param initial_price Starting mid-price (S?)
     * @param drift Annual drift for GBM (?)
     * @param volatility Annual volatility for GBM (?)
     * @param dt Simulated time step (fraction of year)
     * @param params Hawkes-specific parameters
     * @param seed Random seed (0 = random)
     */
    HawkesMicrostructureModel(
        double initial_price,
        double drift,
        double volatility,
        double dt,
        const GenerationParameters& params,
        uint64_t seed = 0
    );
    
    // IPriceModel interface
    double next_price() override;
    double current_price() const override;
    void reset() override;
    std::string model_name() const override { return "hawkes"; }
    std::string description() const override {
        return "Hawkes Microstructure: Self-exciting orders with momentum-based direction";
    }
    
    /**
     * @brief Get orders generated at current step
     * 
     * After calling next_price(), this returns all orders generated
     * during that time step via the Hawkes process.
     * 
     * @return Vector of orders (may be empty if no event occurred)
     */
    const std::vector<Order>& current_orders() const { return current_orders_; }
    
    /**
     * @brief Get current Hawkes intensity
     * @return ?(t) = ? + ? * ? exp(-?(t - t_j))
     */
    double current_intensity() const;
    
private:
    // Step 1: Price evolution (GBM)
    std::unique_ptr<operations::GBMPriceGenerator> gbm_generator_;
    double previous_price_;
    
    // Step 2: Hawkes process parameters
    double hawkes_mu_;     // Baseline rate
    double hawkes_alpha_;  // Excitation
    double hawkes_beta_;   // Decay
    std::deque<double> event_times_;  // History of arrival times
    
    // Step 3: Order direction (momentum)
    double momentum_k_;    // Sensitivity parameter
    
    // Step 4: Price offset (power law)
    double price_offset_L_;
    double price_offset_alpha_;
    double price_offset_max_;
    
    // Step 5: Volume (log-normal)
    double volume_mu_;
    double volume_sigma_;
    
    // Step 6: Order generation
    int orders_per_event_;
    
    // Time tracking
    double current_time_;
    double dt_;
    
    // Random number generation
    common::math::RandomGenerator rng_;
    common::math::DistributionUtils dist_utils_;
    
    // State
    std::vector<Order> current_orders_;
    uint64_t next_order_id_;
    
    // Internal methods
    
    /**
     * @brief Update Hawkes intensity based on event history
     * @param t Current time
     * @return ?(t)
     */
    double compute_hawkes_intensity(double t);
    
    /**
     * @brief Clean old events from history (optimization)
     * @param t Current time
     */
    void prune_old_events(double t);
    
    /**
     * @brief Generate order direction based on momentum
     * @param price_change ?S = S(t) - S(t-1)
     * @return true if BUY, false if SELL
     */
    bool generate_order_direction(double price_change);
    
    /**
     * @brief Generate price offset from mid-price using power law
     * @return ?p (price offset)
     */
    double generate_price_offset();
    
    /**
     * @brief Generate order volume using log-normal
     * @return V (volume)
     */
    double generate_volume();
    
    /**
     * @brief Generate a cloud of N orders at current event time
     * @param mid_price Current mid-price S(t)
     * @param event_time Time of Hawkes event
     */
    void generate_order_cloud(double mid_price, double event_time);
};

} // namespace marketsim::traffic_generator::models::price_models
