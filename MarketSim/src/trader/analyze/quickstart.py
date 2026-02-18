"""
MarketSim Quick Start
Run OHLCV analysis on MarketSim data

Usage:
    python quickstart.py [--help]
    python quickstart.py --symbol AAPL
    python quickstart.py --example
"""

import sys
import os

# Add current directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

if '--example' in sys.argv:
    # Run example
    from trader.run_example import main
    main()
else:
    # Run full analysis
    from trader.run_analysis import main
    main()
