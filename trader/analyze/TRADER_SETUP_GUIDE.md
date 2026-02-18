# MarketSim Trader Package - Complete Setup Guide

## ✅ What's Been Created

### Core Files
- ✅ `trader/analyze/` - OHLCV analysis module
- ✅ `run_analysis.py` - Easy-to-use analysis script
- ✅ `run_example.py` - Quick example script  
- ✅ `setup.py` - Package installation
- ✅ `test_setup.py` - Setup verification
- ✅ `TROUBLESHOOTING.md` - Complete troubleshooting guide

---

## 🚀 How to Use (3 Simple Steps)

### Step 1: Install Dependencies

```bash
pip install -r trader/analyze/requirements.txt
```

This installs:
- pandas (data processing)
- numpy (numerical operations)
- plotly (interactive charts)
- matplotlib (static charts)
- kaleido (image export)

### Step 2: Generate Data with MarketSim

Run these 3 commands in **separate terminals**:

```powershell
# Terminal 1 - Exchange
.\out\build\x64-debug\MarketSim\test_exchange_server.exe

# Terminal 2 - Traffic Generator (HIGH FREQUENCY)
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe hawkes

# Terminal 3 - Monitor (Records OHLCV every 1 second!)
.\out\build\x64-debug\MarketSim\monitor_exchange.exe AAPL
```

**Let it run for 30-60 seconds** to generate data.

You'll see:
- Terminal 1: Exchange accepting orders
- Terminal 2: Traffic generator sending ~100 orders/second
- Terminal 3: Monitor showing OHLCV bars every second

Data saved to: `MarketSim/market_history/AAPL_ohlcv.csv`

### Step 3: Analyze Data

```bash
# Quick example (opens interactive chart)
python run_example.py

# Full analysis with options
python run_analysis.py --symbol AAPL --output chart.html

# With moving averages
python run_analysis.py --symbol AAPL --ma --output analysis.html

# List available files
python run_analysis.py --list-files
```

---

## 🎯 Common Commands

### Generate and Analyze Data
```bash
# 1. Generate data (run for 60 seconds)
# ... (3 terminals as shown above)

# 2. Analyze
python run_analysis.py --symbol AAPL

# 3. Save to HTML
python run_analysis.py --output aapl_chart.html

# 4. Technical analysis
python run_analysis.py --ma --output ta_chart.html
```

### From Python
```python
from trader.analyze import OHLCVReader, CandlestickPlotter

# Read data
reader = OHLCVReader()
df = reader.read_latest('AAPL')

# Display summary
summary = reader.get_summary(df)
print(f"Total bars: {summary['total_bars']}")
print(f"Price range: ${summary['price']['min']:.2f} - ${summary['price']['max']:.2f}")

# Create chart
plotter = CandlestickPlotter()
fig = plotter.plot_interactive(df, symbol='AAPL')
fig.show()
```

---

## ❌ Troubleshooting

### Issue: "No module named trader"

**Fix:** Use the provided scripts that handle paths automatically:
```bash
python run_analysis.py
python run_example.py
```

### Issue: "No OHLCV files found"

**Fix:** Generate data first by running the 3 MarketSim components.

### Issue: Dependencies not installed

**Fix:**
```bash
pip install -r trader/analyze/requirements.txt
```

### Verify Setup
```bash
python test_setup.py
```

**See:** [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for all solutions.

---

## 📊 What You Get

### Interactive Charts
- ✅ **Candlestick visualization** (green/red bars)
- ✅ **Volume analysis** (color-coded bars)
- ✅ **Zoom & Pan** (explore data interactively)
- ✅ **Hover tooltips** (OHLCV values)
- ✅ **Time range selector**
- ✅ **Export** (HTML, PNG)

### Technical Analysis
- ✅ **Moving Averages** (10, 20, 50 period)
- ✅ **Volume trends**
- ✅ **Price statistics**
- ✅ **Data quality metrics**

### Summary Statistics
```
OHLCV DATA SUMMARY - AAPL
============================================================
Total bars: 145
Duration: 0 days 00:02:25
Price range: $99.82 - $100.95
Mean volume: 85.86
Interval: 1s bars
============================================================
```

---

## 📁 File Structure

```
MarketSim/                          ← Project root (run commands here)
├── run_analysis.py                 ← Main analysis script
├── run_example.py                  ← Quick example
├── test_setup.py                   ← Verify setup
├── setup.py                        ← Package installer
├── TROUBLESHOOTING.md             ← Complete guide
│
├── trader/                         ← Trader package
│   ├── __init__.py
│   ├── README.md
│   └── analyze/                    ← Analysis module
│       ├── __init__.py
│       ├── main.py                 ← CLI interface
│       ├── example.py              ← Example code
│       ├── ohlcv_reader.py         ← CSV reader
│       ├── candlestick_plotter.py  ← Chart creator
│       ├── requirements.txt        ← Dependencies
│       ├── README.md
│       └── GUIDE.md
│
└── MarketSim/                      ← C++ system
    └── market_history/             ← Generated data
        └── AAPL_ohlcv.csv          ← OHLCV data file
```

---

## 🎓 Next Steps

### 1. Collect More Data
Run MarketSim for 5-10 minutes to get rich dataset:
```bash
# Let all 3 terminals run for longer
# More data = better analysis
```

### 2. Explore Different Visualizations
```bash
# Interactive with volume
python run_analysis.py --output chart.html

# Technical analysis
python run_analysis.py --ma --output ta.html

# Static PNG
python run_analysis.py --static --output chart.png
```

### 3. Build Trading Strategies
```python
from trader.analyze import OHLCVReader

reader = OHLCVReader()
df = reader.read_latest('AAPL')

# Simple Moving Average Crossover
df['SMA10'] = df['close'].rolling(10).mean()
df['SMA50'] = df['close'].rolling(50).mean()

# Generate signals
df['signal'] = 0
df.loc[df['SMA10'] > df['SMA50'], 'signal'] = 1   # Buy
df.loc[df['SMA10'] < df['SMA50'], 'signal'] = -1  # Sell

print(df[['close', 'SMA10', 'SMA50', 'signal']].tail())
```

### 4. Extend the Package
- Add more technical indicators
- Implement backtesting
- Add risk management
- Build automated trading strategies

---

## 🔗 Quick Links

- [Analyze Module README](trader/analyze/README.md) - Module docs
- [Analyze Module Guide](trader/analyze/GUIDE.md) - Complete tutorial
- [Troubleshooting](TROUBLESHOOTING.md) - Fix common issues
- [Package README](trader/README.md) - Package overview

---

## ✅ Verification Checklist

Run these to verify everything works:

```bash
# 1. Test setup
python test_setup.py

# 2. Install dependencies
pip install -r trader/analyze/requirements.txt

# 3. Generate data (60 seconds)
# ... (3 terminals)

# 4. Run analysis
python run_analysis.py --output test.html

# 5. Check output
ls test.html  # Should exist
```

---

**You're all set! Run `python test_setup.py` to verify.** 🎉
