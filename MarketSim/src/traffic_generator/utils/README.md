# TrafficGenerator Utils

Utility functions and helpers for the TrafficGenerator component.

## Responsibilities

- Random number generation (distributions, seeding)
- Time and timestamp handling
- Numerical calculations and helpers
- String formatting and parsing
- Logging utilities
- Statistical calculations

## Key Utilities

### RandomNumberGenerator
Provides reproducible random number generation:
- Normal distributions (price movements, volatility)
- Uniform distributions (order sizes, spreads)
- Exponential distributions (event timing)
- Seeding for reproducibility/testing

### TimeUtils
Time and timestamp utilities:
- Current timestamp in milliseconds
- Time interval calculations
- Bar alignment (minute, hour boundaries)
- Deadline/timeout tracking

### PriceUtils
Price and quantity calculations:
- Tick rounding (price to nearest tick)
- Percentage changes
- Basis point calculations
- Quantity/size rounding

### StatisticalCalculator
Running statistics:
- Mean and standard deviation
- Volatility estimation (rolling window)
- Trend detection
- Momentum calculations

## Files

- `random_number_generator.cpp/h` - Random number utilities
- `time_utils.cpp/h` - Time and timestamp handling
- `price_utils.cpp/h` - Price calculation helpers
- `statistical_calculator.cpp/h` - Statistical computations
- `logging_utils.cpp/h` - Logging helpers

## Dependencies

- Standard C++20 library (random, chrono, cmath)
