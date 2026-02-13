# Math Utilities - Built-in C++20 Only

All math utilities using **only** C++20 standard library - no external dependencies!

## Components

### 1. Random Number Generation (`random.h`)

```cpp
#include "common/math/random.h"

using namespace marketsim::common::math;

// Create RNG
RandomGenerator rng;  // Random seed
// or
RandomGenerator rng(42);  // Fixed seed for testing

// Generate random numbers
double z = rng.standard_normal();     // N(0, 1)
double x = rng.normal(100, 15);       // N(100, 15)
double u = rng.uniform(0, 1);         // Uniform [0, 1)
double price = rng.uniform(90, 110);  // Uniform [90, 110)
```

### 2. Distributions (`distribution.h`)

```cpp
#include "common/math/distribution.h"

using namespace marketsim::common::math;

// Normal distribution functions
double pdf = Distribution::normal_pdf(105, 100, 15);  // PDF at x=105
double cdf = Distribution::normal_cdf(105, 100, 15);  // P(X ? 105)
double quantile = Distribution::normal_inv_cdf(0.95, 100, 15);  // 95th percentile

// Standard normal (mean=0, stddev=1)
double phi = Distribution::standard_normal_pdf(1.96);
double Phi = Distribution::standard_normal_cdf(1.96);  // ? 0.975
```

### 3. Brownian Motion (`brownian_motion.h`)

```cpp
#include "common/math/brownian_motion.h"

RandomGenerator rng;

// Standard Brownian Motion
BrownianMotion bm(rng);
double dW = bm.standard_increment(0.01);  // dt = 0.01
auto path = bm.generate_path(100, 0.01);  // 100 steps, dt=0.01

// Geometric Brownian Motion (for stock prices)
double drift = 0.05;      // 5% annual return
double volatility = 0.20;  // 20% annual volatility

GeometricBrownianMotion gbm(rng, drift, volatility);

// Single step
double new_price = gbm.step(100.0, 1.0/252);  // 1 trading day

// Full path
auto prices = gbm.generate_path(100.0, 252, 1.0/252);  // 1 year daily

// Terminal price only (efficient)
double final_price = gbm.terminal_price(100.0, 1.0);  // 1 year
```

### 4. Monte Carlo Simulation (`monte_carlo.h`)

```cpp
#include "common/math/monte_carlo.h"

RandomGenerator rng;
MonteCarlo mc(rng);

// Example: European Call Option Pricing
double S0 = 100.0;      // Current price
double K = 105.0;       // Strike price
double T = 1.0;         // 1 year
double r = 0.05;        // Risk-free rate
double sigma = 0.20;    // Volatility

GeometricBrownianMotion gbm(rng, r, sigma);

auto payoff = [&]() {
    double ST = gbm.terminal_price(S0, T);
    return std::exp(-r * T) * std::max(ST - K, 0.0);
};

// Simple MC
double price = mc.simulate(100000, payoff);

// MC with confidence interval
auto result = mc.simulate_with_confidence(100000, payoff);
std::cout << "Price: " << result.mean << " ± " << result.std_error << "\n";
std::cout << "95% CI: [" << result.confidence_lower << ", " 
          << result.confidence_upper << "]\n";

// Antithetic variance reduction
auto payoff_z = [&](double z) {
    double ST = S0 * std::exp((r - 0.5*sigma*sigma)*T + sigma*std::sqrt(T)*z);
    return std::exp(-r * T) * std::max(ST - K, 0.0);
};
double price_av = mc.simulate_antithetic(50000, payoff_z);
```

### 5. Online Statistics (`monte_carlo.h`)

```cpp
#include "common/math/monte_carlo.h"

Statistics stats;

// Add data points
for (int i = 0; i < 1000; ++i) {
    double value = rng.normal(100, 15);
    stats.add(value);
}

// Get statistics
std::cout << "Count: " << stats.count() << "\n";
std::cout << "Mean: " << stats.mean() << "\n";
std::cout << "StdDev: " << stats.stddev() << "\n";
std::cout << "Min: " << stats.min() << "\n";
std::cout << "Max: " << stats.max() << "\n";
```

## Complete Example: Price Path Simulation

```cpp
#include "common/math/random.h"
#include "common/math/brownian_motion.h"
#include "common/math/monte_carlo.h"

using namespace marketsim::common::math;

void simulate_price_paths() {
    RandomGenerator rng;
    
    // Market parameters
    double initial_price = 100.0;
    double annual_return = 0.10;   // 10%
    double annual_volatility = 0.25;  // 25%
    
    GeometricBrownianMotion gbm(rng, annual_return, annual_volatility);
    
    // Simulate 1 year, daily steps
    int trading_days = 252;
    double dt = 1.0 / trading_days;
    
    // Generate 10 price paths
    for (int path = 0; path < 10; ++path) {
        auto prices = gbm.generate_path(initial_price, trading_days, dt);
        
        std::cout << "Path " << path << " final price: " 
                  << prices.back() << "\n";
    }
    
    // Monte Carlo: What's the probability price > 120?
    Statistics stats;
    int n_sims = 100000;
    
    for (int i = 0; i < n_sims; ++i) {
        double final_price = gbm.terminal_price(initial_price, 1.0);
        stats.add(final_price);
    }
    
    std::cout << "Expected final price: " << stats.mean() << "\n";
    std::cout << "Price volatility: " << stats.stddev() << "\n";
}
```

## Key Features

? **Zero external dependencies** - Uses only C++20 standard library  
? **Fast RNG** - Mersenne Twister 64-bit  
? **Accurate distributions** - Properly implemented PDF/CDF/inverse CDF  
? **Efficient** - Optimized for Monte Carlo simulations  
? **Header-only** - No compilation needed  
? **Modern C++20** - Uses `<numbers>`, `std::erf`, etc.

## Performance Tips

1. **Reuse RNG objects** - Don't create new ones in loops
2. **Use terminal_price()** for MC - Avoid generating full paths
3. **Antithetic variates** - Reduces variance by ~50%
4. **Online statistics** - Single-pass computation

## Next Steps

Add to your code:
- Option pricing (Black-Scholes, Monte Carlo)
- Value at Risk (VaR) calculations
- Portfolio optimization
- Risk metrics (Sharpe ratio, max drawdown)
