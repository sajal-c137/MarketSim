# Trader Service

Python package for analyzing OHLCV data from MarketSim.

## Installation

```bash
pip install -r analyze/requirements.txt
```

## Usage

```bash
# From MarketSim root
python -m trader.analyze.main --symbol AAPL

# Or from this directory
cd MarketSim/src/trader/analyze
python main.py --symbol AAPL
```

## Structure

```
trader/
├── analyze/
│   ├── main.py              # CLI interface
│   ├── example.py           # Quick example
│   ├── ohlcv_reader.py      # Read CSV files
│   └── candlestick_plotter.py   # Create charts
```

See [analyze/README.md](analyze/README.md) for details.

