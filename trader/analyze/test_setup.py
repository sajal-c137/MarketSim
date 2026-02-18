"""
Test Script - Verify Trader Package Setup

Run this to check if everything is configured correctly.
"""

import sys
import os

# Move to project root
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
os.chdir(project_root)

print("="*70)
print("MarketSim Trader Package - Setup Test")
print("="*70)

# Test 1: Python version
print("\n[1/6] Checking Python version...")
py_version = sys.version_info
print(f"   Python {py_version.major}.{py_version.minor}.{py_version.micro}")
if py_version >= (3, 8):
    print("   ✓ Python version OK")
else:
    print("   ✗ Need Python 3.8+")
    sys.exit(1)

# Test 2: Current directory
print("\n[2/6] Checking current directory...")
cwd = os.getcwd()
print(f"   {cwd}")
if os.path.exists('trader') and os.path.exists('MarketSim'):
    print("   ✓ In correct directory (MarketSim root)")
else:
    print("   ✗ Please run from MarketSim root directory")
    print("   Run: cd C:\\Users\\YourName\\source\\repos\\MarketSim")
    sys.exit(1)

# Test 3: Trader package structure
print("\n[3/6] Checking trader package structure...")
required_files = [
    'trader/__init__.py',
    'trader/analyze/__init__.py',
    'trader/analyze/main.py',
    'trader/analyze/ohlcv_reader.py',
    'trader/analyze/candlestick_plotter.py',
]
all_exist = True
for file in required_files:
    exists = os.path.exists(file)
    symbol = "✓" if exists else "✗"
    print(f"   {symbol} {file}")
    if not exists:
        all_exist = False

if all_exist:
    print("   ✓ All files present")
else:
    print("   ✗ Some files missing")
    sys.exit(1)

# Test 4: Import trader package
print("\n[4/6] Testing trader package import...")
try:
    sys.path.insert(0, cwd)
    from trader.analyze import OHLCVReader, CandlestickPlotter
    print("   ✓ Successfully imported trader.analyze")
except ImportError as e:
    print(f"   ✗ Import failed: {e}")
    sys.exit(1)

# Test 5: Check dependencies
print("\n[5/6] Checking dependencies...")
required_packages = ['pandas', 'numpy', 'plotly', 'matplotlib']
missing = []
for pkg in required_packages:
    try:
        __import__(pkg)
        print(f"   ✓ {pkg}")
    except ImportError:
        print(f"   ✗ {pkg} (not installed)")
        missing.append(pkg)

if missing:
    print(f"\n   To install missing packages:")
    print(f"   pip install {' '.join(missing)}")
    print("\n   Or install all:")
    print("   pip install -r trader/analyze/requirements.txt")
    sys.exit(1)

# Test 6: Check for data
print("\n[6/6] Checking for OHLCV data...")
data_dir = 'MarketSim/market_history'
if os.path.exists(data_dir):
    files = [f for f in os.listdir(data_dir) if f.endswith('_ohlcv.csv')]
    if files:
        print(f"   ✓ Found {len(files)} OHLCV file(s):")
        for f in files[:3]:  # Show first 3
            print(f"      - {f}")
    else:
        print("   ⚠ No OHLCV files found")
        print("   Run MarketSim to generate data first")
        print("   (Exchange + Traffic Generator + Monitor)")
else:
    print(f"   ⚠ Directory not found: {data_dir}")
    print("   Run MarketSim Monitor to create this directory")

# Summary
print("\n" + "="*70)
print("Setup Test Complete!")
print("="*70)

if all_exist and not missing:
    print("\n✓ Everything looks good!")
    print("\nNext steps:")
    print("  1. Generate data: Run Exchange + Traffic Generator + Monitor")
    print("  2. Analyze: python run_analysis.py --symbol AAPL")
    print("  3. Example: python run_example.py")
else:
    print("\n⚠ Some issues found. See messages above.")

print("\n" + "="*70)
