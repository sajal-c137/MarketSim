#include "distributions.h"
#include <cmath>
#include <algorithm>

namespace marketsim::common::math {

double DistributionUtils::sample_exponential(double lambda, RandomGenerator& rng) {
    // Formula: X = -ln(U) / ?
    // where U ~ Uniform(0,1)
    
    // Generate uniform random number in (0, 1)
    double u = rng.uniform_01();
    
    // Avoid log(0) by clamping to small positive value
    u = std::max(u, 1e-10);
    
    // Apply inverse CDF: F^(-1)(u) = -ln(u) / ?
    return -std::log(u) / lambda;
}

double DistributionUtils::logistic(double x) {
    // Formula: ?(x) = 1 / (1 + exp(-x))
    // 
    // This is the standard logistic (sigmoid) function
    // Maps (-?, +?) ? (0, 1)
    
    // For numerical stability when x is large negative:
    // ?(x) = exp(x) / (1 + exp(x)) if x < 0
    if (x < 0) {
        double exp_x = std::exp(x);
        return exp_x / (1.0 + exp_x);
    } else {
        return 1.0 / (1.0 + std::exp(-x));
    }
}

bool DistributionUtils::sample_bernoulli(double p, RandomGenerator& rng) {
    // Formula: X ~ Bernoulli(p)
    // Returns 1 with probability p, 0 with probability 1-p
    //
    // Implementation: Generate U ~ Uniform(0,1)
    //                 Return (U < p)
    
    return rng.uniform_01() < p;
}

double DistributionUtils::sample_truncated_power_law(
    double L, 
    double alpha, 
    double x_max, 
    RandomGenerator& rng)
{
    // Truncated Pareto (Power Law) Distribution
    // PDF: f(x) = (? * L^?) / (x^(?+1))  for L ? x ? x_max
    //
    // CDF: F(x) = 1 - (L/x)^?  for x ? L
    //
    // For truncated version at x_max:
    // Inverse CDF: X = L * (1 - U * (1 - (L/x_max)^?))^(-1/?)
    // where U ~ Uniform(0,1)
    
    // Generate uniform random number
    double u = rng.uniform_01();
    
    // Calculate the truncation factor
    double truncation_factor = 1.0 - std::pow(L / x_max, alpha);
    
    // Apply inverse CDF formula
    double base = 1.0 - u * truncation_factor;
    double x = L * std::pow(base, -1.0 / alpha);
    
    // Ensure result is within bounds [L, x_max]
    x = std::max(L, std::min(x, x_max));
    
    return x;
}

double DistributionUtils::sample_lognormal(
    double mu, 
    double sigma, 
    RandomGenerator& rng)
{
    // Log-Normal Distribution
    // If Y ~ Normal(?, ?²), then X = exp(Y) ~ LogNormal(?, ?)
    //
    // Properties:
    //   - X > 0 always (positive values)
    //   - E[X] = exp(? + ?²/2)
    //   - Median[X] = exp(?)
    //   - Mode[X] = exp(? - ?²)
    
    // Sample from standard normal N(0,1)
    double z = rng.standard_normal();
    
    // Scale to N(?, ?²)
    double y = mu + sigma * z;
    
    // Transform to log-normal: X = exp(Y)
    return std::exp(y);
}

} // namespace marketsim::common::math
