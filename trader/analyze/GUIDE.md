# MarketSim Trader Service - Complete Guide

## 📦 Package Created

A Python package for analyzing and visualizing OHLCV candlestick data from MarketSim.

---

## 📁 File Structure

```
trader/
├── __init__.py              # Package initialization
├── ohlcv_reader.py          # Reads OHLCV CSV files from MarketSim
├── candlestick_plotter.py   # Creates candlestick charts (Plotly & Matplotlib)
├── main.py                  # Command-line interface
├── example.py               # Quick example script
├── requirements.txt         # Python dependencies
└── README.md               # Documentation
```

---

## 🚀 Quick Start

### Step 1: Install Dependencies

```bash
pip install -r trader/requirements.txt
```

**Dependencies:**
- `pandas` - Data processing
- `plotly` - Interactive charts
- `matplotlib` - Static charts
- `numpy` - Numerical operations

### Step 2: Generate Data with MarketSim

Run the C++ MarketSim system (3 terminals):

```powershell
# Terminal 1 - Exchange
.\out\build\x64-debug\MarketSim\test_exchange_server.exe

# Terminal 2 - Traffic Generator (HIGH FREQUENCY MODE)
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe hawkes

# Terminal 3 - Monitor (Records OHLCV every 1 second)
.\out\build\x64-debug\MarketSim\monitor_exchange.exe AAPL
```

**Let it run for 30-60 seconds** to collect data. You'll see OHLCV bars every second!

Data is saved to: `MarketSim/market_history/AAPL_ohlcv.csv`

### Step 3: Analyze Data

```bash
# Quick example
python trader/example.py

# Full analysis with CLI
python trader/main.py --symbol AAPL

# Save to HTML
python trader/main.py --output aapl_chart.html

# Show moving averages
python trader/main.py --ma --output aapl_ma.html
```

---

## 📊 Features

### 1. **OHLCV Reader** (`ohlcv_reader.py`)

Reads and processes CSV files from MarketSim:

```python
from trader import OHLCVReader

reader = OHLCVReader(data_directory='../MarketSim/market_history')

# List available files
files = reader.list_available_files('AAPL')

# Read latest file
df = reader.read_latest('AAPL')

# Get summary statistics
summary = reader.get_summary(df)
print(f"Total bars: {summary['total_bars']}")
print(f"Price range: ${summary['price']['min']} - ${summary['price']['max']}")
```

**Methods:**
- `list_available_files(symbol)` - List all OHLCV files
- `read_latest(symbol)` - Read most recent file
- `read_file(filepath)` - Read specific file
- `read_multiple(symbol, limit)` - Read and combine multiple files
- `get_summary(df)` - Get statistics

### 2. **Candlestick Plotter** (`candlestick_plotter.py`)

Creates beautiful charts:

```python
from trader import CandlestickPlotter

plotter = CandlestickPlotter(title="AAPL Analysis")

# Interactive Plotly chart
fig = plotter.plot_interactive(df, symbol='AAPL', show_volume=True)
fig.show()

# Static Matplotlib chart
plotter.plot_static(df, symbol='AAPL', save_png='chart.png')

# With moving averages
fig = plotter.plot_with_moving_averages(df, ma_periods=[10, 20, 50])
fig.show()
```

**Chart Types:**
- `plot_interactive()` - Interactive Plotly chart with zoom/pan
- `plot_static()` - Static Matplotlib PNG
- `plot_with_moving_averages()` - Technical analysis view

---

## 📈 Example Output

### Console Output

```
==============================================================
OHLCV DATA SUMMARY - AAPL
==============================================================
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
==============================================================
```

### Interactive Chart Features

- ✅ **Zoom & Pan** - Explore data interactively
- ✅ **Hover Info** - See exact OHLCV values
- ✅ **Volume Subplot** - Volume bars color-coded by direction
- ✅ **Time Slider** - Focus on specific time ranges
- ✅ **Export** - Save as PNG/HTML

---

## 🎯 Use Cases

### 1. **Real-time Monitoring**

```bash
# Watch live data (re-run every few seconds)
while true; do
  python trader/main.py --output latest.html
  sleep 5
done
```

### 2. **Historical Analysis**

```python
# Analyze multiple sessions
df = reader.read_multiple('AAPL', limit=5)  # Last 5 files
summary = reader.get_summary(df)
print(f"Analyzed {summary['total_bars']} bars across 5 sessions")
```

### 3. **Trading Strategy Backtesting**

```python
from trader import OHLCVReader

reader = OHLCVReader()
df = reader.read_latest('AAPL')

# Simple Moving Average Crossover
df['SMA10'] = df['close'].rolling(10).mean()
df['SMA50'] = df['close'].rolling(50).mean()

# Generate signals
df['signal'] = 0
df.loc[df['SMA10'] > df['SMA50'], 'signal'] = 1   # Buy
df.loc[df['SMA10'] < df['SMA50'], 'signal'] = -1  # Sell

# Analyze performance
signals = df[df['signal'] != 0]
print(f"Generated {len(signals)} trading signals")
```

### 4. **Data Quality Checks**

```python
summary = reader.get_summary(df)

# Check data completeness
expected_bars = (summary['date_range']['duration'].seconds / 
                summary['interval_seconds'])
actual_bars = summary['total_bars']
print(f"Data completeness: {actual_bars/expected_bars*100:.1f}%")

# Check for anomalies
price_change = df['close'].pct_change()
large_moves = price_change[abs(price_change) > 0.05]  # >5% moves
print(f"Found {len(large_moves)} large price moves")
```

---

## 🔧 Command Line Interface

```bash
python trader/main.py [options]
```

**Options:**

| Option | Description | Default |
|--------|-------------|---------|
| `--symbol SYMBOL` | Trading symbol | AAPL |
| `--data-dir PATH` | Data directory | ../MarketSim/market_history |
| `--output FILE` | Save chart (HTML/PNG) | None (show in browser) |
| `--static` | Use Matplotlib | Plotly interactive |
| `--ma` | Show moving averages | Off |
| `--list-files` | List available files | Off |

**Examples:**

```bash
# Basic usage
python trader/main.py

# Save interactive chart
python trader/main.py --output aapl.html

# Technical analysis with MAs
python trader/main.py --ma --output analysis.html

# Static PNG chart
python trader/main.py --static --output chart.png

# Different symbol (if you recorded it)
python trader/main.py --symbol BTC --output btc.html
```

---

## 📝 Data Format

MarketSim generates CSV files with this structure:

```csv
timestamp,timestamp_ms,interval_seconds,open,high,low,close,volume
2025-02-17 14:23:00.000,1739797380000,1,100.50,100.75,100.45,100.60,87.00
2025-02-17 14:23:01.000,1739797381000,1,100.60,100.82,100.55,100.65,93.00
```

**Columns:**
- `timestamp` - Human-readable time
- `timestamp_ms` - Unix milliseconds
- `interval_seconds` - Bar interval (1s with current config)
- `open, high, low, close` - OHLC prices
- `volume` - Number of trades in this bar

---

## 🚦 Current MarketSim Configuration

**HIGH FREQUENCY MODE** (10x faster than default):

- **Monitor polling:** 100ms (10x faster)
- **OHLCV interval:** 1 second bars
- **Order generation:** Every 10ms
- **Hawkes rate:** ~100 orders/second
- **Expected data:** ~60 bars/minute, ~50-100 ticks/bar

---

## 🔄 Integration Workflow

```
┌─────────────────┐
│   MarketSim     │
│   (C++ System)  │
│                 │
│ Exchange        │──┐
│ Traffic Gen     │  │ High-frequency
│ Monitor         │  │ trading simulation
└─────────────────┘  │
         │           │
         │ Generates │
         ▼           │
┌─────────────────┐  │
│  OHLCV CSV      │◀─┘
│  market_history/│
│  AAPL_ohlcv.csv │
└─────────────────┘
         │
         │ Read
         ▼
┌─────────────────┐
│ Trader Package  │
│   (Python)      │
│                 │
│ • Read CSV      │
│ • Analyze       │
│ • Visualize     │
│ • Backtest      │
└─────────────────┘
         │
         │ Output
         ▼
┌─────────────────┐
│ • Console Stats │
│ • HTML Charts   │
│ • PNG Images    │
│ • Trading       │
│   Signals       │
└─────────────────┘
```

---

## ✅ Testing Checklist

1. ✅ **Generate data:**
   ```bash
   # Run MarketSim for 60 seconds
   # Check: market_history/AAPL_ohlcv.csv exists
   ```

2. ✅ **Install dependencies:**
   ```bash
   pip install -r trader/requirements.txt
   ```

3. ✅ **Run example:**
   ```bash
   python trader/example.py
   # Should open interactive chart in browser
   ```

4. ✅ **Run full CLI:**
   ```bash
   python trader/main.py --ma --output test.html
   # Check: test.html created with moving averages
   ```

---

## 🎓 Next Steps

1. **Collect Data:** Run MarketSim for 5-10 minutes to get rich dataset

2. **Explore API:** Try different visualization options

3. **Build Strategies:** Implement trading algorithms using the data

4. **Extend Package:** Add more technical indicators, risk metrics, etc.

5. **Real-time Trading:** Connect to live market data and execute trades

---

## 📚 Resources

- **Plotly Docs:** https://plotly.com/python/candlestick-charts/
- **Pandas Docs:** https://pandas.pydata.org/docs/
- **Technical Analysis:** Add `ta-lib` for advanced indicators

---

**The trader package is ready to use!** 🎉
