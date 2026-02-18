# MarketSim Trader Service

Python trading analysis package for MarketSim.

## ?? Clean Structure

All files organized in `trader/analyze/` - **no clutter in root!**

```
trader/
??? __init__.py
??? README.md (this file)
??? analyze/                        # Everything here!
    ??? __init__.py
    ??? main.py                     # CLI entry point
    ??? example.py                  # Quick example
    ??? ohlcv_reader.py             # Read CSV files
    ??? candlestick_plotter.py      # Create charts
    ??? requirements.txt            # Dependencies
    ??? run_analysis.py             # Easy launcher
    ??? run_example.py              # Example launcher
    ??? test_setup.py               # Setup verifier
    ??? quickstart.py               # Quick start
    ??? README.md                   # Module docs
    ??? GUIDE.md                    # Complete guide
    ??? TROUBLESHOOTING.md          # Fix issues
    ??? TRADER_SETUP_GUIDE.md       # Setup guide
```

## ?? Quick Start

```bash
# 1. Install dependencies
pip install -r trader/analyze/requirements.txt

# 2. Generate data (run MarketSim for 60 seconds)
# Exchange + Traffic Generator + Monitor

# 3. Analyze
python -m trader.analyze.main --symbol AAPL
```

## ?? Documentation

- **[trader/analyze/README.md](analyze/README.md)** - Module documentation
- **[trader/analyze/GUIDE.md](analyze/GUIDE.md)** - Complete tutorial  
- **[trader/analyze/TROUBLESHOOTING.md](analyze/TROUBLESHOOTING.md)** - Fix common issues
- **[trader/analyze/TRADER_SETUP_GUIDE.md](analyze/TRADER_SETUP_GUIDE.md)** - Full setup guide

## ?? Usage

### From Command Line
```bash
# From project root
python -m trader.analyze.main --symbol AAPL --output chart.html

# After pip install -e .
marketsim-analyze --symbol AAPL
```

### From Python
```python
from trader.analyze import OHLCVReader, CandlestickPlotter

# Read data
reader = OHLCVReader()
df = reader.read_latest('AAPL')

# Create interactive chart
plotter = CandlestickPlotter()
fig = plotter.plot_interactive(df, symbol='AAPL')
fig.show()
```

---

Part of the **[MarketSim](https://github.com/sajal-c137/MarketSim)** project.
