# Traffic Generator Price Model Architecture

## ? Implementation Complete

### ?? New Package: `src/traffic_generator/models/price_models/`

A **pluggable architecture** for price generation models using polymorphism.

---

## ?? Files Created (10 files)

### 1. **Base Interface**
- `i_price_model.h` - Interface for all price models

### 2. **Price Models**
- `linear_price_model.h/.cpp` - Linear deterministic model: `P(t) = P? + rate * t`
- `gbm_price_model.h/.cpp` - Geometric Brownian Motion: `dS/S = ?*dt + ?*dW`

### 3. **Factory Pattern**
- `price_model_factory.h/.cpp` - Creates models by name with configuration

### 4. **Updated Thread**
- `price_generation_thread.h/.cpp` - Now accepts any `IPriceModel` (model-agnostic)

### 5. **Unified Test**
- `test_traffic_generator_unified.cpp` - Accepts model name as command-line argument

---

## ?? Architecture Diagram

```
IPriceModel (interface)
    ??? LinearPriceModel
    ??? GBMPriceModel
    ??? [Future: JumpDiffusionModel, OrnsteinUhlenbeckModel, etc.]

PriceModelFactory
    ??? create(model_name, config, dt) ? IPriceModel*

PriceGenerationThread
    ??? accepts IPriceModel* (pluggable)

Test Program
    ??? argv[1] ? "linear" or "gbm" ? Factory ? Thread
```

---

## ?? Usage

### **Command-Line Interface**

```sh
# Run with GBM model (default)
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe gbm

# Run with linear model
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe linear

# Show help
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe --help
```

### **Example Output**

```
=== Traffic Generator with gbm Model ===

Configuration:
  Symbol: AAPL
  Initial Price: $100
  Model: gbm
  Drift: 5%
  Volatility: 3%
  Interval: 100 ms
  Duration: 30 seconds
  Total Steps: 300
  Simulated Time Per Step: 0.00333333 years

Model Description: Geometric Brownian Motion: dS/S = ?*dt + ?*dW

Starting threads...
[PriceGenerator] Starting price generation...
  Model: gbm
  Description: Geometric Brownian Motion: dS/S = ?*dt + ?*dW
  Initial Price: 100
  ...
```

---

## ?? How to Add New Models

### Step 1: Create Model Class

```cpp
// src/traffic_generator/models/price_models/my_new_model.h
#pragma once
#include "i_price_model.h"

class MyNewModel : public IPriceModel {
public:
    MyNewModel(/* params */);
    
    double next_price() override;
    double current_price() const override;
    void reset() override;
    std::string model_name() const override { return "mynew"; }
    std::string description() const override { return "My custom model"; }
    
private:
    // model state
};
```

### Step 2: Implement in .cpp

```cpp
// src/traffic_generator/models/price_models/my_new_model.cpp
#include "my_new_model.h"

MyNewModel::MyNewModel(/* params */) { /* init */ }
double MyNewModel::next_price() { /* math */ }
// ...
```

### Step 3: Register in Factory

```cpp
// price_model_factory.cpp
if (name_lower == "mynew") {
    return std::make_unique<MyNewModel>(/* config params */);
}
```

### Step 4: Update available_models()

```cpp
std::string PriceModelFactory::available_models() {
    return "linear, gbm, mynew";
}
```

### Step 5: Add to CMakeLists.txt

```cmake
"src/traffic_generator/models/price_models/my_new_model.cpp"
```

---

## ?? Current Models

### 1. **Linear Model** (`linear`)
- **Formula:** `P(t) = P? + rate * t`
- **Parameters:** `base_price`, `rate_per_step`
- **Use Case:** Debugging, testing order matching with predictable prices

### 2. **GBM Model** (`gbm`)
- **Formula:** `S(t+?t) = S(t) * exp((? - ?²/2)*?t + ?*??t*Z)`
- **Parameters:** `drift` (?), `volatility` (?), `dt`
- **Use Case:** Realistic market simulation with stochastic price movements

---

## ?? Configuration

All models share `GenerationParameters`:

```cpp
struct GenerationParameters {
    std::string symbol;         // "AAPL"
    double base_price;          // 100.0
    double drift;               // 5.0% (for GBM)
    double volatility;          // 3.0% (for GBM)
    double price_rate;          // 0.1 (for linear)
    double step_interval_ms;    // 100ms
    double duration_seconds;    // 30s
    double order_quantity;      // 1.0
};
```

---

## ?? Testing

### Run All Models

```sh
# Test linear model
.\test_traffic_generator_unified.exe linear

# Test GBM model
.\test_traffic_generator_unified.exe gbm

# Compare outputs
```

### Integration with Exchange

1. **Terminal 1:** Exchange Server
   ```sh
   .\test_exchange_server.exe
   ```

2. **Terminal 2:** Traffic Generator (choose model)
   ```sh
   .\test_traffic_generator_unified.exe gbm
   ```

3. **Terminal 3:** Monitor
   ```sh
   .\monitor_exchange.exe
   ```

---

## ?? Benefits of This Architecture

1. ? **Single test executable** - No need for `test_linear_generator`, `test_gbm_generator`, etc.
2. ? **Easy to extend** - Add new models without changing thread code
3. ? **Polymorphic** - All models implement same interface
4. ? **Factory pattern** - Centralized model creation
5. ? **Command-line driven** - Switch models without recompiling
6. ? **Future-proof** - Ready for jump-diffusion, mean-reversion, etc.

---

## ?? CMakeLists.txt Changes

Added to `traffic_generator_lib`:
```cmake
# Price Models (Pluggable pricing strategies)
"src/traffic_generator/models/price_models/linear_price_model.cpp"
"src/traffic_generator/models/price_models/gbm_price_model.cpp"
"src/traffic_generator/models/price_models/price_model_factory.cpp"
```

Added test executable:
```cmake
# Unified traffic generator test (accepts model name as argument)
add_executable(test_traffic_generator_unified "test/test_traffic_generator_unified.cpp")
target_link_libraries(test_traffic_generator_unified PRIVATE traffic_generator_lib)
```

---

## ?? Next Steps

To build and run:

```sh
# Clean and reconfigure (from project root)
cmake --preset x64-debug

# Build
cmake --build out/build/x64-debug --target test_traffic_generator_unified

# Run with model
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe gbm
```

---

## ?? Future Model Ideas

- **Jump Diffusion** - Adds sudden price jumps to GBM
- **Ornstein-Uhlenbeck** - Mean-reverting process
- **Heston Model** - Stochastic volatility
- **GARCH** - Time-varying volatility clustering
- **Sine Wave** - Periodic oscillation for testing
- **Random Walk** - Pure diffusion without drift

All follow the same pattern:
1. Inherit `IPriceModel`
2. Implement 5 methods
3. Add to factory
4. Run with `./test_traffic_generator_unified <model_name>`

---

**Status:** ? **Ready to build and test!**
