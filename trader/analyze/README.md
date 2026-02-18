# MarketSim Trader - Analyze Module

Python package for analyzing and visualizing OHLCV (candlestick) data from MarketSim.

## Installation

**Option 1: Quick Install (Recommended)**
```bash
# Install dependencies only
pip install -r trader/analyze/requirements.txt
```

**Option 2: Full Install (Development)**
```bash
# Install package in development mode
pip install -e .
```

## Quick Start

### 1. Generate Data with MarketSim

First, run the MarketSim system to generate OHLCV data:

```powershell
# Terminal 1 - Exchange
.\out\build\x64-debug\MarketSim\test_exchange_server.exe

# Terminal 2 - Traffic Generator
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe hawkes

# Terminal 3 - Monitor (generates OHLCV files)
.\out\build\x64-debug\MarketSim\monitor_exchange.exe AAPL
```

Let it run for 30-60 seconds to collect data. OHLCV data is saved to `MarketSim/market_history/AAPL_ohlcv.csv`.

### 2. Analyze and Visualize

**From project root:**
```bash
# Run with Python module syntax
python -m trader.analyze.main --symbol AAPL

# Or use the convenience scripts in trader/analyze/
cd trader/analyze
python run_analysis.py --symbol AAPL
python run_example.py
```

**After installing (pip install -e .):**
```bash
marketsim-analyze --symbol AAPL
```

**All options:**
```bash
python -m trader.analyze.main --list-files
python -m trader.analyze.main --symbol AAPL --output chart.html
python -m trader.analyze.main --symbol AAPL --ma --output analysis.html
python -m trader.analyze.main --symbol AAPL --static --output chart.png
```

## Usage Examples

### Python API

```python
from trader.analyze import OHLCVReader, CandlestickPlotter

# Read data
reader = OHLCVReader(data_directory='../MarketSim/market_history')
df = reader.read_latest('AAPL')

# Get summary
summary = reader.get_summary(df)
print(f"Total bars: {summary['total_bars']}")
print(f"Price range: ${summary['price']['min']:.2f} - ${summary['price']['max']:.2f}")

# Create interactive chart
plotter = CandlestickPlotter(title="AAPL Analysis")
fig = plotter.plot_interactive(df, symbol='AAPL')
fig.show()

# Create chart with moving averages
fig = plotter.plot_with_moving_averages(df, symbol='AAPL', ma_periods=[10, 20, 50])
fig.show()
```

## File Structure

```
trader/
├── __init__.py              # Package initialization
└── analyze/
    ├── __init__.py          # Analyze subpackage
    ├── ohlcv_reader.py      # CSV reading logic
    ├── candlestick_plotter.py   # Charting logic
    ├── main.py              # CLI entry point
    ├── example.py           # Quick example
    ├── requirements.txt     # Dependencies
    ├── README.md           # This file
    └── GUIDE.md            # Complete guide
```

## Data Format

OHLCV CSV files from MarketSim have this format:

```csv
timestamp,timestamp_ms,interval_seconds,open,high,low,close,volume
2025-02-17 14:23:00.000,1739797380000,1,100.50,100.75,100.45,100.60,87.00
```

## Command Line Options

```
python trader/main.py [options]

Options:
  --symbol SYMBOL        Trading symbol (default: AAPL)
  --data-dir PATH        Path to market_history directory
  --output FILE          Save chart to file (HTML or PNG)
  --static               Use matplotlib instead of plotly
  --ma                   Show moving averages (10, 20, 50 period)
  --list-files           List available OHLCV files
  -h, --help            Show help message
```

## Example Output

```
OHLCV DATA SUMMARY - AAPL
============================================================
Total bars: 145
Date range: 2025-02-17 14:23:00 to 2025-02-17 14:25:25
Duration: 0 days 00:02:25

Price Statistics:
  Min:  $99.82
  Max:  $100.95
  Mean: $100.38
  Std:  $0.27

Volume Statistics:
  Total: 12450
  Mean:  85.86
  Max:   142

Interval: 1s bars
============================================================
```

## Integration with Trading Strategies

This package can be extended to implement trading strategies:

```python
# Example: Simple Moving Average Crossover Strategy
def sma_crossover_strategy(df, fast=10, slow=50):
    df['SMA_fast'] = df['close'].rolling(fast).mean()
    df['SMA_slow'] = df['close'].rolling(slow).mean()
    
    # Generate signals
    df['signal'] = 0
    df.loc[df['SMA_fast'] > df['SMA_slow'], 'signal'] = 1  # Buy
    df.loc[df['SMA_fast'] < df['SMA_slow'], 'signal'] = -1  # Sell
    
    return df

# Use it
df = reader.read_latest('AAPL')
df = sma_crossover_strategy(df)
print(df[['close', 'SMA_fast', 'SMA_slow', 'signal']].tail())
```

## Requirements

- Python 3.8+
- pandas
- plotly (for interactive charts)
- matplotlib (for static charts)
- numpy

## License

Part of the MarketSim project.
