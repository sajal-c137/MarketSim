#!/usr/bin/env python3
"""
MarketSim OHLCV Analysis
Run from project root
python analyze.py --symbol AAPL --output chart.html
"""
import sys
import os

# Add MarketSim/src to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'MarketSim', 'src'))

# Import and run
from trader.analyze.main import main

if __name__ == '__main__':
    main()
