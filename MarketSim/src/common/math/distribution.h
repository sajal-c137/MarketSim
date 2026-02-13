#pragma once

#include <cmath>
#include <numbers>

namespace marketsim::common::math {

/**
 * @brief Statistical distribution functions (PDF/CDF)
 * 
 * Implements standard normal distribution functions using C++20
 */
class Distribution {
public:
    // Standard normal PDF: ?(x) = (1/?(2?)) * exp(-x²/2)
    static double standard_normal_pdf(double x) {
        constexpr double inv_sqrt_2pi = 0.3989422804014327;  // 1/?(2?)
        return inv_sqrt_2pi * std::exp(-0.5 * x * x);
    }
    
    // Normal PDF with mean and stddev
    static double normal_pdf(double x, double mean, double stddev) {
        double z = (x - mean) / stddev;
        return standard_normal_pdf(z) / stddev;
    }
    
    // Standard normal CDF: ?(x) - approximation using error function
    static double standard_normal_cdf(double x) {
        return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
    }
    
    // Normal CDF with mean and stddev
    static double normal_cdf(double x, double mean, double stddev) {
        double z = (x - mean) / stddev;
        return standard_normal_cdf(z);
    }
    
    // Inverse CDF (quantile function) - approximation
    // Uses Beasley-Springer-Moro algorithm
    static double standard_normal_inv_cdf(double p) {
        if (p <= 0.0 || p >= 1.0) {
            return 0.0;  // Handle edge cases
        }
        
        // Coefficients for rational approximation
        constexpr double a[] = {
            -3.969683028665376e+01,
             2.209460984245205e+02,
            -2.759285104469687e+02,
             1.383577518672690e+02,
            -3.066479806614716e+01,
             2.506628277459239e+00
        };
        
        constexpr double b[] = {
            -5.447609879822406e+01,
             1.615858368580409e+02,
            -1.556989798598866e+02,
             6.680131188771972e+01,
            -1.328068155288572e+01
        };
        
        constexpr double c[] = {
            -7.784894002430430e-03,
            -3.223964580411365e-01,
            -2.400758277161838e+00,
            -2.549732539343734e+00,
             4.374664141464968e+00,
             2.938163982698783e+00
        };
        
        constexpr double d[] = {
             7.784695709041462e-03,
             3.224671290700398e-01,
             2.445134137142996e+00,
             3.754408661907416e+00
        };
        
        double q = p - 0.5;
        double r, x;
        
        if (std::abs(q) <= 0.425) {
            // Central region
            r = 0.180625 - q * q;
            x = q * (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) /
                    (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
        } else {
            // Tail region
            if (q < 0.0) {
                r = p;
            } else {
                r = 1.0 - p;
            }
            
            if (r <= 0.0) {
                return 0.0;
            }
            
            r = std::sqrt(-std::log(r));
            
            if (r <= 5.0) {
                r = r - 1.6;
                x = (((((c[0] * r + c[1]) * r + c[2]) * r + c[3]) * r + c[4]) * r + c[5]) /
                    ((((d[0] * r + d[1]) * r + d[2]) * r + d[3]) * r + 1.0);
            } else {
                r = r - 5.0;
                x = (((((c[0] * r + c[1]) * r + c[2]) * r + c[3]) * r + c[4]) * r + c[5]) /
                    ((((d[0] * r + d[1]) * r + d[2]) * r + d[3]) * r + 1.0);
            }
            
            if (q < 0.0) {
                x = -x;
            }
        }
        
        return x;
    }
    
    // Normal inverse CDF with mean and stddev
    static double normal_inv_cdf(double p, double mean, double stddev) {
        return mean + stddev * standard_normal_inv_cdf(p);
    }
};

} // namespace marketsim::common::math
