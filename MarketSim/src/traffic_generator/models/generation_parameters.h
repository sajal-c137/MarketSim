#pragma once

#include <string>
#include <cstdint>
#include <map>

namespace marketsim::traffic_generator::models {

/**
 * @brief Market regime types
 */
enum class MarketRegime {
    BULL_NORMAL,        // Normal bull market
    BEAR_NORMAL,        // Normal bear market  
    SIDEWAYS_NORMAL,    // Normal sideways/range-bound
    BULL_EXTREME,       // Extreme bull (high volatility)
    BEAR_EXTREME        // Extreme bear (high volatility)
};

/**
 * @brief Parameters for a specific market regime
 */
struct RegimeParameters {
    double drift;
    double volatility;
    double hawkes_mu;
    double hawkes_alpha;
    double hawkes_beta;
    double momentum_k;
    double price_offset_L;
    double price_offset_alpha;
    double price_offset_max;

    RegimeParameters() = default;

    RegimeParameters(double d, double v, double mu, double alpha, double beta,
                    double k, double L, double p_alpha, double p_max)
        : drift(d), volatility(v), hawkes_mu(mu), hawkes_alpha(alpha),
          hawkes_beta(beta), momentum_k(k), price_offset_L(L),
          price_offset_alpha(p_alpha), price_offset_max(p_max) {}
};

/**
 * @brief Configuration parameters for traffic generation
 */
struct GenerationParameters {
    std::string symbol;
    double base_price;
    double price_rate;
    double order_quantity;
    double step_interval_ms;
    double duration_seconds;

    // Base parameters (used when regime switching is disabled)
    double drift;
    double volatility;
    double hawkes_mu;
    double hawkes_alpha;
    double hawkes_beta;
    double momentum_k;
    double price_offset_L;
    double price_offset_alpha;
    double price_offset_max;
    double volume_mu;
    double volume_sigma;
    int orders_per_event;

    // Regime switching configuration
    bool enable_regime_switching;
    double regime_switch_interval_seconds;  // How often to check for regime switch (10s)

    // Regime parameters
    std::map<MarketRegime, RegimeParameters> regime_configs;
    std::map<MarketRegime, double> regime_probabilities;  // Cumulative probabilities

    GenerationParameters()
        : symbol("AAPL")
        , base_price(100.0)
        , price_rate(10.0)
        , order_quantity(1.0)
        , step_interval_ms(10.0)
        , duration_seconds(300.0)
        , drift(5.0)
        , volatility(3.0)
        , hawkes_mu(10.0)
        , hawkes_alpha(2.0)
        , hawkes_beta(5.0)
        , momentum_k(2.0)
        , price_offset_L(0.10)
        , price_offset_alpha(3.5)
        , price_offset_max(5.0)
        , volume_mu(0.0)
        , volume_sigma(0.5)
        , orders_per_event(5)
        , enable_regime_switching(true)
        , regime_switch_interval_seconds(10.0)
    {
        // Initialize regime configurations

        // Normal Bull: Moderate uptrend, normal volatility
        regime_configs[MarketRegime::BULL_NORMAL] = RegimeParameters(
            8.0,    // drift: +8% uptrend
            3.0,    // volatility: normal
            10.0,   // hawkes_mu: normal order rate
            2.0,    // hawkes_alpha: moderate clustering
            5.0,    // hawkes_beta: normal decay
            3.0,    // momentum_k: moderate trend-following
            0.10,   // price_offset_L
            3.0,    // price_offset_alpha: normal spread
            5.0     // price_offset_max
        );

        // Normal Bear: Moderate downtrend, normal volatility  
        regime_configs[MarketRegime::BEAR_NORMAL] = RegimeParameters(
            -8.0,   // drift: -8% downtrend
            3.0,    // volatility: normal
            10.0,   // hawkes_mu: normal order rate
            2.0,    // hawkes_alpha: moderate clustering
            5.0,    // hawkes_beta: normal decay
            -3.0,   // momentum_k: contrarian (buying dips)
            0.10,   // price_offset_L
            3.0,    // price_offset_alpha: normal spread
            5.0     // price_offset_max
        );

        // Sideways: No trend, low volatility
        regime_configs[MarketRegime::SIDEWAYS_NORMAL] = RegimeParameters(
            0.0,    // drift: no trend
            2.0,    // volatility: low
            8.0,    // hawkes_mu: lower order rate
            1.5,    // hawkes_alpha: less clustering
            4.0,    // hawkes_beta: faster decay
            0.5,    // momentum_k: minimal momentum
            0.08,   // price_offset_L: tighter spread
            2.5,    // price_offset_alpha: tighter orders
            3.0     // price_offset_max: smaller range
        );

        // Extreme Bull: Strong uptrend, HIGH volatility
        regime_configs[MarketRegime::BULL_EXTREME] = RegimeParameters(
            20.0,   // drift: +20% strong uptrend!
            15.0,   // volatility: VERY HIGH
            25.0,   // hawkes_mu: HIGH order rate
            4.0,    // hawkes_alpha: STRONG clustering
            8.0,    // hawkes_beta: faster decay but high rate
            8.0,    // momentum_k: STRONG trend-following
            0.20,   // price_offset_L: wider spread
            4.0,    // price_offset_alpha: heavy tail
            10.0    // price_offset_max: wide range
        );

        // Extreme Bear: Strong downtrend, HIGH volatility
        regime_configs[MarketRegime::BEAR_EXTREME] = RegimeParameters(
            -20.0,  // drift: -20% strong downtrend!
            15.0,   // volatility: VERY HIGH
            25.0,   // hawkes_mu: HIGH order rate (panic)
            4.0,    // hawkes_alpha: STRONG clustering
            8.0,    // hawkes_beta: faster decay but high rate
            -8.0,   // momentum_k: STRONG contrarian
            0.20,   // price_offset_L: wider spread
            4.0,    // price_offset_alpha: heavy tail
            10.0    // price_offset_max: wide range
        );

        // Set regime probabilities (cumulative)
        // 3 normal regimes = 75% total (25% each)
        // 2 extreme regimes = 25% total (12.5% each)
        regime_probabilities[MarketRegime::BULL_NORMAL] = 0.25;      // 0.00 - 0.25
        regime_probabilities[MarketRegime::BEAR_NORMAL] = 0.50;      // 0.25 - 0.50  
        regime_probabilities[MarketRegime::SIDEWAYS_NORMAL] = 0.75;  // 0.50 - 0.75
        regime_probabilities[MarketRegime::BULL_EXTREME] = 0.875;    // 0.75 - 0.875
        regime_probabilities[MarketRegime::BEAR_EXTREME] = 1.0;      // 0.875 - 1.0
    }
};

/**
 * @brief Current state of traffic generation
 */
struct GenerationState {
    double elapsed_seconds;
    int64_t orders_sent;
    int64_t start_timestamp_ms;
    bool is_running;

    // Regime switching state
    MarketRegime current_regime;
    double last_regime_switch_time;

    GenerationState()
        : elapsed_seconds(0.0)
        , orders_sent(0)
        , start_timestamp_ms(0)
        , is_running(false)
        , current_regime(MarketRegime::SIDEWAYS_NORMAL)
        , last_regime_switch_time(0.0)
    {}
};

} // namespace marketsim::traffic_generator::models
