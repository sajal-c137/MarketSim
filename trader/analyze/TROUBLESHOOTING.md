# Troubleshooting Guide - Trader Package

## Common Issues and Solutions

### ❌ Issue: "No module named trader"

**Cause:** Python can't find the trader package.

**Solutions:**

#### Solution 1: Use the Quick-Start Scripts (Easiest)
```bash
# From the MarketSim root directory
python run_example.py
python run_analysis.py --symbol AAPL
```

These scripts automatically handle path setup.

#### Solution 2: Install the Package
```bash
# From the MarketSim root directory
pip install -e .
```

Then you can use:
```bash
marketsim-analyze --symbol AAPL
# OR
python -m trader.analyze.main --symbol AAPL
```

#### Solution 3: Set PYTHONPATH
```bash
# Windows PowerShell
$env:PYTHONPATH = "C:\Users\YourName\source\repos\MarketSim"
python trader/analyze/main.py --symbol AAPL

# Windows Command Prompt
set PYTHONPATH=C:\Users\YourName\source\repos\MarketSim
python trader/analyze/main.py --symbol AAPL

# Linux/Mac
export PYTHONPATH=/path/to/MarketSim
python trader/analyze/main.py --symbol AAPL
```

#### Solution 4: Run from Correct Directory
```bash
# Make sure you're in the MarketSim root directory
cd C:\Users\YourName\source\repos\MarketSim
python run_analysis.py
```

---

### ❌ Issue: "No such file or directory: market_history"

**Cause:** MarketSim hasn't generated data yet.

**Solution:**

1. Run the MarketSim system:
```powershell
# Terminal 1
.\out\build\x64-debug\MarketSim\test_exchange_server.exe

# Terminal 2
.\out\build\x64-debug\MarketSim\test_traffic_generator_unified.exe hawkes

# Terminal 3
.\out\build\x64-debug\MarketSim\monitor_exchange.exe AAPL
```

2. Let it run for 30-60 seconds

3. Verify the file exists:
```powershell
ls MarketSim\market_history\
```

You should see `AAPL_ohlcv.csv`

---

### ❌ Issue: "FileNotFoundError: No OHLCV files found"

**Cause:** Looking in wrong directory or files don't exist.

**Solutions:**

1. **Check the data directory:**
```python
import os
print(os.path.exists('MarketSim/market_history'))  # Should be True
print(os.listdir('MarketSim/market_history'))      # Should show CSV files
```

2. **Specify custom path:**
```bash
python run_analysis.py --data-dir "./MarketSim/market_history"
```

3. **List available files:**
```bash
python run_analysis.py --list-files
```

---

### ❌ Issue: "ModuleNotFoundError: pandas"

**Cause:** Dependencies not installed.

**Solution:**
```bash
pip install -r trader/analyze/requirements.txt
```

If that fails, install individually:
```bash
pip install pandas numpy plotly matplotlib kaleido
```

---

### ❌ Issue: Charts don't display

**Cause:** Plotly needs a browser or specific renderer.

**Solutions:**

1. **Save to HTML instead:**
```bash
python run_analysis.py --output chart.html
```
Then open `chart.html` in your browser.

2. **Use static PNG:**
```bash
python run_analysis.py --static --output chart.png
```

3. **Set renderer for Plotly:**
```python
import plotly.io as pio
pio.renderers.default = "browser"  # or "png", "svg", "notebook"
```

---

### ❌ Issue: "ImportError: DLL load failed"

**Cause:** Missing C++ runtime or incompatible numpy/pandas build.

**Solution:**
```bash
# Reinstall with specific versions
pip uninstall pandas numpy
pip install pandas==2.0.3 numpy==1.24.3
```

---

### ❌ Issue: Permission denied when writing files

**Cause:** Directory is read-only or file is open.

**Solutions:**

1. Run as administrator (Windows)
2. Check if CSV file is open in Excel/another program
3. Change output directory:
```bash
python run_analysis.py --output ~/Desktop/chart.html
```

---

## Verification Checklist

Run these commands to verify your setup:

```bash
# 1. Check Python version (needs 3.8+)
python --version

# 2. Check if in correct directory
pwd  # or cd on Windows

# 3. Check if trader package exists
ls trader/

# 4. Check if data exists
ls MarketSim/market_history/

# 5. Test import
python -c "import sys; sys.path.insert(0, '.'); from trader.analyze import OHLCVReader; print('✓ Import works')"

# 6. Run quick test
python run_example.py
```

---

## Quick Reference

### Working Directory Structure

```
MarketSim/                          ← You should be HERE
├── run_analysis.py                 ← Use this script
├── run_example.py                  ← Or this one
├── setup.py                        ← For pip install
├── trader/
│   └── analyze/
│       ├── main.py
│       └── requirements.txt
└── MarketSim/
    └── market_history/
        └── AAPL_ohlcv.csv          ← Data file
```

### Recommended Workflow

```bash
# 1. Navigate to project root
cd C:\Users\YourName\source\repos\MarketSim

# 2. Install dependencies (once)
pip install -r trader/analyze/requirements.txt

# 3. Generate data (run 3 terminals, wait 60 seconds)
# ... (Exchange, Traffic Generator, Monitor)

# 4. Analyze
python run_analysis.py --symbol AAPL --output chart.html

# 5. Open chart.html in browser
```

---

## Still Having Issues?

1. **Check the full path:**
```python
import os
print("Current directory:", os.getcwd())
print("Files here:", os.listdir('.'))
```

2. **Use absolute paths:**
```bash
python run_analysis.py --data-dir "C:\Users\YourName\source\repos\MarketSim\MarketSim\market_history"
```

3. **Try the simplest possible test:**
```bash
cd C:\Users\YourName\source\repos\MarketSim
python run_example.py
```

This should work if:
- ✓ You're in the MarketSim root directory
- ✓ Dependencies are installed
- ✓ Data files exist in MarketSim/market_history/

---

## Contact

If none of these solutions work, please provide:
1. Full error message
2. Output of `python --version`
3. Output of `pwd` (or `cd` on Windows)
4. Output of `ls trader/` and `ls MarketSim/market_history/`
