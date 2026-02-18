"""
Example usage of the Trader OHLCV analysis package

Run this script after collecting data from MarketSim:
1. Run Exchange, Traffic Generator, and Monitor
2. Let it collect data for a while (creates CSV files)
3. Run this script to analyze and visualize the data
"""

import sys
import os
# Add MarketSim/src to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..')))

from trader.analyze import OHLCVReader, CandlestickPlotter
import argparse


def main():
    """Main entry point for OHLCV analysis"""
    
    parser = argparse.ArgumentParser(description='Analyze and plot MarketSim OHLCV data')
    parser.add_argument('--symbol', type=str, default='AAPL', help='Trading symbol')
    parser.add_argument('--data-dir', type=str, default='../../market_history',
                       help='Path to market_history directory (relative to MarketSim/src/trader/analyze/)')
    parser.add_argument('--output', type=str, help='Output HTML file path')
    parser.add_argument('--static', action='store_true', help='Use static matplotlib plot')
    parser.add_argument('--ma', action='store_true', help='Show moving averages')
    parser.add_argument('--list-files', action='store_true', help='List available files')
    
    args = parser.parse_args()
    
    # Initialize reader
    reader = OHLCVReader(data_directory=args.data_dir)
    
    # List files if requested
    if args.list_files:
        print(f"\nAvailable OHLCV files for {args.symbol}:")
        files = reader.list_available_files(args.symbol)
        for i, file in enumerate(files, 1):
            print(f"  {i}. {file}")
        print()
        return
    
    try:
        # Read data
        print(f"Reading OHLCV data for {args.symbol}...")
        df = reader.read_latest(args.symbol)
        
        # Display summary
        summary = reader.get_summary(df)
        print("\n" + "="*60)
        print(f"OHLCV DATA SUMMARY - {args.symbol}")
        print("="*60)
        print(f"Total bars: {summary['total_bars']}")
        print(f"Date range: {summary['date_range']['start']} to {summary['date_range']['end']}")
        print(f"Duration: {summary['date_range']['duration']}")
        print(f"\nPrice Statistics:")
        print(f"  Min:  ${summary['price']['min']:.2f}")
        print(f"  Max:  ${summary['price']['max']:.2f}")
        print(f"  Mean: ${summary['price']['mean']:.2f}")
        print(f"  Std:  ${summary['price']['std']:.2f}")
        print(f"\nVolume Statistics:")
        print(f"  Total: {summary['volume']['total']:.0f}")
        print(f"  Mean:  {summary['volume']['mean']:.2f}")
        print(f"  Max:   {summary['volume']['max']:.0f}")
        print(f"\nInterval: {summary['interval_seconds']}s bars")
        print("="*60 + "\n")
        
        # Create plot
        plotter = CandlestickPlotter(title=f"{args.symbol} Market Data")
        
        if args.static:
            print("Creating static candlestick chart...")
            plotter.plot_static(df, symbol=args.symbol, save_png=args.output)
        elif args.ma:
            print("Creating interactive chart with moving averages...")
            fig = plotter.plot_with_moving_averages(
                df, 
                symbol=args.symbol,
                ma_periods=[10, 20, 50],
                save_html=args.output
            )
            if not args.output:
                fig.show()
        else:
            print("Creating interactive candlestick chart...")
            fig = plotter.plot_interactive(
                df,
                symbol=args.symbol,
                show_volume=True,
                save_html=args.output
            )
            if not args.output:
                fig.show()
        
        print("\n✓ Analysis complete!")
        
    except FileNotFoundError as e:
        print(f"\n✗ Error: {e}")
        print(f"Make sure MarketSim has generated OHLCV data in: {args.data_dir}")
        print("Run the MarketSim system first to generate data.")
    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()


if __name__ == '__main__':
    main()
