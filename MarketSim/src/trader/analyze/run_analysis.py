"""
Quick Start - Run OHLCV Analysis
This script handles all path setup automatically.

Usage:
    python run_analysis.py [--symbol AAPL] [--output chart.html] [--ma]
"""

import sys
import os

# Ensure the trader package is in the path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

# Now import and run the main analysis
from trader.analyze.main import main

if __name__ == '__main__':
    main()
