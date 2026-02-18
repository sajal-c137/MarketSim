"""
OHLCV Reader - Reads MarketSim OHLCV CSV files

File format expected:
timestamp,timestamp_ms,interval_seconds,open,high,low,close,volume
2025-02-17 14:23:00.000,1739797380000,1,100.50,100.75,100.45,100.60,87.00
"""

import pandas as pd
from pathlib import Path
from typing import Optional, List
import glob


class OHLCVReader:
    """Reads OHLCV candlestick data from MarketSim CSV files"""

    def __init__(self, data_directory: str = "../../market_history"):
        """
        Initialize OHLCV reader

        Args:
            data_directory: Path to market_history directory (relative to MarketSim/src/trader/analyze/)
        """
        self.data_directory = Path(data_directory)
        
    def list_available_files(self, symbol: Optional[str] = None) -> List[str]:
        """
        List all available OHLCV CSV files
        
        Args:
            symbol: Optional symbol filter (e.g., 'AAPL')
            
        Returns:
            List of OHLCV file paths
        """
        if symbol:
            pattern = f"{symbol}_ohlcv*.csv"
        else:
            pattern = "*_ohlcv*.csv"
            
        files = glob.glob(str(self.data_directory / pattern))
        return sorted(files)
    
    def read_latest(self, symbol: str = "AAPL") -> pd.DataFrame:
        """
        Read the most recent OHLCV file for a symbol
        
        Args:
            symbol: Trading symbol (e.g., 'AAPL')
            
        Returns:
            DataFrame with OHLCV data
        """
        files = self.list_available_files(symbol)
        if not files:
            raise FileNotFoundError(f"No OHLCV files found for {symbol}")
        
        latest_file = files[-1]  # Most recent file
        return self.read_file(latest_file)
    
    def read_file(self, filepath: str) -> pd.DataFrame:
        """
        Read OHLCV data from a specific file
        
        Args:
            filepath: Path to OHLCV CSV file
            
        Returns:
            DataFrame with columns: timestamp, open, high, low, close, volume
        """
        df = pd.read_csv(filepath)
        
        # Convert timestamp to datetime
        df['timestamp'] = pd.to_datetime(df['timestamp'])
        
        # Set timestamp as index
        df.set_index('timestamp', inplace=True)
        
        # Ensure numeric types
        df['open'] = pd.to_numeric(df['open'])
        df['high'] = pd.to_numeric(df['high'])
        df['low'] = pd.to_numeric(df['low'])
        df['close'] = pd.to_numeric(df['close'])
        df['volume'] = pd.to_numeric(df['volume'])
        
        return df
    
    def read_multiple(self, symbol: str = "AAPL", limit: Optional[int] = None) -> pd.DataFrame:
        """
        Read multiple OHLCV files and concatenate them
        
        Args:
            symbol: Trading symbol
            limit: Maximum number of files to read (most recent)
            
        Returns:
            Combined DataFrame
        """
        files = self.list_available_files(symbol)
        
        if limit:
            files = files[-limit:]  # Take most recent N files
        
        if not files:
            raise FileNotFoundError(f"No OHLCV files found for {symbol}")
        
        dfs = []
        for file in files:
            try:
                df = self.read_file(file)
                dfs.append(df)
            except Exception as e:
                print(f"Warning: Failed to read {file}: {e}")
                continue
        
        if not dfs:
            raise ValueError("No valid data files found")
        
        # Concatenate all dataframes
        combined = pd.concat(dfs)
        
        # Remove duplicates based on timestamp
        combined = combined[~combined.index.duplicated(keep='last')]
        
        # Sort by timestamp
        combined.sort_index(inplace=True)
        
        return combined
    
    def get_summary(self, df: pd.DataFrame) -> dict:
        """
        Get summary statistics for OHLCV data
        
        Args:
            df: OHLCV DataFrame
            
        Returns:
            Dictionary with summary statistics
        """
        return {
            'total_bars': len(df),
            'date_range': {
                'start': df.index.min(),
                'end': df.index.max(),
                'duration': df.index.max() - df.index.min()
            },
            'price': {
                'min': df['low'].min(),
                'max': df['high'].max(),
                'mean': df['close'].mean(),
                'std': df['close'].std()
            },
            'volume': {
                'total': df['volume'].sum(),
                'mean': df['volume'].mean(),
                'max': df['volume'].max()
            },
            'interval_seconds': df['interval_seconds'].iloc[0] if 'interval_seconds' in df.columns else None
        }
