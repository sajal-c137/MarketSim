#include "hawkes_microstructure_model.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace marketsim::traffic_generator::models::price_models {

HawkesMicrostructureModel::HawkesMicrostructureModel(
    double initial_price,
    double drift,
    double volatility,
    double dt,
    const GenerationParameters& params,
    uint64_t seed)
    : previous_price_(initial_price)
    , hawkes_mu_(params.hawkes_mu)
    , hawkes_alpha_(params.hawkes_alpha)
    , hawkes_beta_(params.hawkes_beta)
    , momentum_k_(params.momentum_k)
    , price_offset_L_(params.price_offset_L)
    , price_offset_alpha_(params.price_offset_alpha)
    , price_offset_max_(params.price_offset_max)
    , volume_mu_(params.volume_mu)
    , volume_sigma_(params.volume_sigma)
    , orders_per_event_(params.orders_per_event)
    , enable_regime_switching_(params.enable_regime_switching)
    , regime_switch_interval_(params.regime_switch_interval_seconds)
    , last_regime_switch_time_(0.0)
    , current_regime_(MarketRegime::SIDEWAYS_NORMAL)
    , config_(params)
    , current_time_(0.0)
    , dt_(dt)
    , rng_(seed == 0 ? common::math::RandomGenerator() : common::math::RandomGenerator(seed))
    , next_order_id_(1)
{
    // Initialize GBM price generator
    gbm_generator_ = std::make_unique<operations::GBMPriceGenerator>(
        initial_price,
        drift,
        volatility,
        dt,
        seed
    );

    // Initialize with a random regime if enabled
    if (enable_regime_switching_) {
        current_regime_ = select_regime();
        apply_regime(current_regime_);
    }
}

double HawkesMicrostructureModel::next_price() {
    // Clear orders from previous step
    current_orders_.clear();

    // Check for regime switch
    check_regime_switch(current_time_);

    // Step 1: Advance price using GBM
    double new_price = gbm_generator_->next_price();

    // Step 2: Check if Hawkes event occurs
    double lambda = compute_hawkes_intensity(current_time_);
    double event_prob = std::min(lambda * dt_, 1.0);

    bool event_occurred = dist_utils_.sample_bernoulli(event_prob, rng_);

    if (event_occurred) {
        event_times_.push_back(current_time_);
        prune_old_events(current_time_);
        generate_order_cloud(new_price, current_time_);
    }

    // Update state
    previous_price_ = new_price;
    current_time_ += dt_;

    return new_price;
}

double HawkesMicrostructureModel::current_price() const {
    return gbm_generator_->current_price();
}

void HawkesMicrostructureModel::reset() {
    gbm_generator_->reset();
    previous_price_ = gbm_generator_->current_price();
    current_time_ = 0.0;
    event_times_.clear();
    current_orders_.clear();
    next_order_id_ = 1;
}

double HawkesMicrostructureModel::current_intensity() const {
    return const_cast<HawkesMicrostructureModel*>(this)->compute_hawkes_intensity(current_time_);
}

double HawkesMicrostructureModel::compute_hawkes_intensity(double t) {
    // Step 2: Hawkes Process Intensity
    // Formula: ?(t) = ? + ? * ? exp(-?(t - t_j))
    // where the sum is over all past events t_j < t
    //
    // Interpretation:
    //   - ?: Baseline rate (constant background activity)
    //   - ?: Excitation coefficient (how much each event boosts future rate)
    //   - ?: Decay rate (how fast the excitement fades)
    //   - Each past event t_j contributes: ? * exp(-?(t - t_j))
    
    double intensity = hawkes_mu_;  // Start with baseline
    
    // Add contribution from each past event
    for (double t_j : event_times_) {
        double time_since_event = t - t_j;
        
        // Each event contributes: ? * exp(-? * ?t)
        intensity += hawkes_alpha_ * std::exp(-hawkes_beta_ * time_since_event);
    }
    
    return intensity;
}

void HawkesMicrostructureModel::prune_old_events(double t) {
    // Remove events that have negligible contribution to intensity
    // If exp(-?(t - t_j)) < threshold, the event is "forgotten"
    // Threshold: exp(-? * ?t) < 0.001 ? ?t > ln(1000) / ? ? 6.9 / ?
    
    double cutoff_time = t - (6.9 / hawkes_beta_);
    
    // Remove all events before cutoff
    while (!event_times_.empty() && event_times_.front() < cutoff_time) {
        event_times_.pop_front();
    }
}

bool HawkesMicrostructureModel::generate_order_direction(double price_change) {
    // Step 3: Order Direction using Logistic Function
    // Formula: P(Buy) = 1 / (1 + exp(-k * ?S))
    // where ?S = S(t) - S(t-1)
    //
    // Interpretation:
    //   - k > 0: Trend-following (buy when price rising)
    //   - k < 0: Mean-reverting (buy when price falling)
    //   - k = 0: Random (50/50)
    //
    // Example with k = 10:
    //   ?S = +0.1 ? P(Buy) = 0.73 (73% chance of buy)
    //   ?S = -0.1 ? P(Buy) = 0.27 (27% chance of buy, 73% sell)
    //   ?S = 0   ? P(Buy) = 0.50 (50/50)
    
    // Compute momentum-weighted probability
    double scaled_momentum = momentum_k_ * price_change;
    double buy_probability = dist_utils_.logistic(scaled_momentum);
    
    // Sample direction using Bernoulli trial
    return dist_utils_.sample_bernoulli(buy_probability, rng_);
}

double HawkesMicrostructureModel::generate_price_offset() {
    // Step 4: Price Offset using Truncated Power Law (Pareto)
    // Formula: f(?p) = (? * L^?) / (?p^(?+1))  for L ? ?p ? ?p_max
    //
    // Properties:
    //   - Heavy-tailed: Most orders near mid-price (small ?p)
    //   - Some orders deep in book (large ?p)
    //   - Lower ? ? more deep orders
    //
    // Example with ? = 2.0, L = 0.01, max = 1.0:
    //   ~60% of orders within 0.01-0.10 from mid
    //   ~30% of orders within 0.10-0.50 from mid
    //   ~10% of orders within 0.50-1.00 from mid
    
    return dist_utils_.sample_truncated_power_law(
        price_offset_L_,
        price_offset_alpha_,
        price_offset_max_,
        rng_
    );
}

double HawkesMicrostructureModel::generate_volume() {
    // Step 5: Volume using Log-Normal Distribution
    // Formula: V ~ LogNormal(?_v, ?_v)
    // i.e., log(V) ~ Normal(?_v, ?_v)
    //
    // Properties:
    //   - Always positive: V > 0
    //   - Right-skewed: Most orders small, some very large
    //   - Median volume: exp(?_v)
    //
    // Example with ?_v = 0.0, ?_v = 0.5:
    //   Median: exp(0) = 1.0
    //   Mean: exp(0 + 0.5²/2) ? 1.13
    //   95% of volumes between 0.4 and 2.5
    
    return dist_utils_.sample_lognormal(volume_mu_, volume_sigma_, rng_);
}

void HawkesMicrostructureModel::generate_order_cloud(double mid_price, double event_time) {
    // Step 6: Generate Order Cloud
    // At each Hawkes event, generate N orders (a "cloud")
    // This simulates a burst of market activity
    
    // Calculate price momentum for order direction
    double price_change = mid_price - previous_price_;
    
    // Generate N orders
    for (int i = 0; i < orders_per_event_; ++i) {
        Order order;
        
        // Set event time
        order.time = event_time;

        // Step 3: Determine order direction (BUY or SELL)
        order.is_buy = generate_order_direction(price_change);

        // Step 4: Generate price offset from mid-price
        double offset = generate_price_offset();

        // Set order price:
        //   - BUY orders: Below mid-price (bid side)
        //   - SELL orders: Above mid-price (ask side)
        if (order.is_buy) {
            order.price = mid_price - offset;
        } else {
            order.price = mid_price + offset;
        }

        // Step 5: Generate order volume
        order.volume = generate_volume();

        // Assign unique ID
        order.order_id = next_order_id_++;

        // Add to current orders
        current_orders_.push_back(order);
    }
}

void HawkesMicrostructureModel::check_regime_switch(double elapsed_time) {
    if (!enable_regime_switching_) {
        return;
    }

    // Check if it's time to evaluate regime switch
    if (elapsed_time - last_regime_switch_time_ >= regime_switch_interval_) {
        // Draw random uniform [0,1]
        double rand_val = rng_.uniform(0.0, 1.0);

        // Select new regime based on probabilities
        MarketRegime new_regime = MarketRegime::SIDEWAYS_NORMAL;
        for (const auto& [regime, cum_prob] : config_.regime_probabilities) {
            if (rand_val <= cum_prob) {
                new_regime = regime;
                break;
            }
        }

        // Apply new regime if different
        if (new_regime != current_regime_) {
            std::cout << "[REGIME SWITCH] t=" << elapsed_time << "s: ";
            switch (current_regime_) {
                case MarketRegime::BULL_NORMAL: std::cout << "BULL_NORMAL"; break;
                case MarketRegime::BEAR_NORMAL: std::cout << "BEAR_NORMAL"; break;
                case MarketRegime::SIDEWAYS_NORMAL: std::cout << "SIDEWAYS"; break;
                case MarketRegime::BULL_EXTREME: std::cout << "BULL_EXTREME"; break;
                case MarketRegime::BEAR_EXTREME: std::cout << "BEAR_EXTREME"; break;
            }
            std::cout << " -> ";
            switch (new_regime) {
                case MarketRegime::BULL_NORMAL: std::cout << "BULL_NORMAL"; break;
                case MarketRegime::BEAR_NORMAL: std::cout << "BEAR_NORMAL"; break;
                case MarketRegime::SIDEWAYS_NORMAL: std::cout << "SIDEWAYS"; break;
                case MarketRegime::BULL_EXTREME: std::cout << "BULL_EXTREME"; break;
                case MarketRegime::BEAR_EXTREME: std::cout << "BEAR_EXTREME"; break;
            }
            std::cout << "\n";

            current_regime_ = new_regime;
            apply_regime(new_regime);
        }

        last_regime_switch_time_ = elapsed_time;
    }
}

MarketRegime HawkesMicrostructureModel::select_regime() {
    double rand_val = rng_.uniform(0.0, 1.0);

    for (const auto& [regime, cum_prob] : config_.regime_probabilities) {
        if (rand_val <= cum_prob) {
            return regime;
        }
    }

    return MarketRegime::SIDEWAYS_NORMAL;
}

void HawkesMicrostructureModel::apply_regime(MarketRegime regime) {
    auto it = config_.regime_configs.find(regime);
    if (it == config_.regime_configs.end()) {
        return;
    }

    const RegimeParameters& params = it->second;

    // Update Hawkes parameters
    hawkes_mu_ = params.hawkes_mu;
    hawkes_alpha_ = params.hawkes_alpha;
    hawkes_beta_ = params.hawkes_beta;
    momentum_k_ = params.momentum_k;
    price_offset_L_ = params.price_offset_L;
    price_offset_alpha_ = params.price_offset_alpha;
    price_offset_max_ = params.price_offset_max;

    // Update GBM parameters
    gbm_generator_->set_drift(params.drift);
    gbm_generator_->set_volatility(params.volatility);
}

} // namespace marketsim::traffic_generator::models::price_models
