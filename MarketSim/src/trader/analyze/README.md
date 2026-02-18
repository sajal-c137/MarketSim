# Trader Analyze Module

Reads OHLCV CSV files and creates candlestick charts.

## Installation

```bash
pip install -r requirements.txt
```

## Usage

```bash
# From MarketSim root
python -m trader.analyze.main --symbol AAPL --output chart.html

# From this directory
python main.py --symbol AAPL
python example.py
```

## Options

```
--symbol SYMBOL      Trading symbol (default: AAPL)
--data-dir PATH      Data directory (default: ../../market_history)
--output FILE        Save to HTML or PNG
--ma                 Show moving averages
--static             Use matplotlib instead of plotly
--list-files         List available files
```

## Python API

```python
from trader.analyze import OHLCVReader, CandlestickPlotter

reader = OHLCVReader()
df = reader.read_latest('AAPL')

plotter = CandlestickPlotter()
fig = plotter.plot_interactive(df, symbol='AAPL')
fig.show()
```

## Data Format

```csv
timestamp,timestamp_ms,interval_seconds,open,high,low,close,volume
2025-02-17 14:23:00.000,1739797380000,1,100.50,100.75,100.45,100.60,87.00
```

Source: `MarketSim/market_history/AAPL_ohlcv.csv`


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
