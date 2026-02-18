"""
Quick Example - Analyze OHLCV Data
This script handles all path setup automatically.

Usage:
    python run_example.py
"""

import sys
import os

# Ensure the trader package is in the path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

# Now import and run the example
from trader.analyze.example import main

if __name__ == '__main__':
    main()
