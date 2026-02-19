"""
Quick example: Analyze OHLCV data from MarketSim

This demonstrates the basic usage of the trader.analyze package.
"""

import sys
import os
# Add MarketSim/src to path when running from this directory
current_dir = os.path.dirname(os.path.abspath(__file__))
src_dir = os.path.abspath(os.path.join(current_dir, '../..'))
if src_dir not in sys.path:
    sys.path.insert(0, src_dir)

from trader.analyze import OHLCVReader, CandlestickPlotter


def main():
    print("="*70)
    print("MarketSim OHLCV Analysis - Quick Example")
    print("="*70)
    
    # Initialize reader
    print("\n1. Reading OHLCV data...")
    reader = OHLCVReader(data_directory='market_history')

    try:
        # Read latest data
        df = reader.read_latest('AAPL')
        print(f"   ✓ Loaded {len(df)} candlestick bars")

        # Show first few rows
        print("\n2. Sample data (first 5 bars):")
        print(df[['open', 'high', 'low', 'close', 'volume']].head())

        # Get summary
        print("\n3. Summary statistics:")
        summary = reader.get_summary(df)
        print(f"   Total bars: {summary['total_bars']}")
        print(f"   Duration: {summary['date_range']['duration']}")
        print(f"   Price range: ${summary['price']['min']:.2f} - ${summary['price']['max']:.2f}")
        print(f"   Mean volume: {summary['volume']['mean']:.2f}")

        # Create plot
        print("\n4. Creating interactive candlestick chart...")
        plotter = CandlestickPlotter(title="AAPL - MarketSim Data")

        fig = plotter.plot_interactive(
            df,
            symbol='AAPL',
            show_volume=True,
            save_html='aapl_chart.html'
        )

        print("\n" + "="*70)
        print("✓ Analysis complete!")
        print("✓ Chart saved to: aapl_chart.html")
        print("✓ Opening chart in browser...")
        print("="*70)

        # Show in browser
        fig.show()

    except FileNotFoundError as e:
        print(f"\n✗ Error: {e}")
        print("\nMake sure to run MarketSim first to generate data:")
        print("  1. Start Exchange, Traffic Generator, and Monitor")
        print("  2. Let it run for 30-60 seconds")
        print("  3. Run this script again")
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()


if __name__ == '__main__':
    main()
