# Traffic Generator

Generates realistic order flow using mathematical models.

## Models

- **Linear**: Constant price growth
- **GBM**: Geometric Brownian Motion
- **Hawkes**: Self-exciting point process with momentum

## Structure

```
traffic_generator/
├── models/
│   └── price_models/    # Linear, GBM, Hawkes
├── operations/          # Price generators
├── threads/            # Producer-consumer pattern
└── main/               # Orchestration
```

## Hawkes Model

Combines:
- GBM for price evolution
- Self-exciting order arrivals
- Momentum-based direction
- Pareto price offsets
- Log-normal volume

## Usage

```bash
test_traffic_generator_unified.exe hawkes
test_traffic_generator_unified.exe gbm
test_traffic_generator_unified.exe linear
```

