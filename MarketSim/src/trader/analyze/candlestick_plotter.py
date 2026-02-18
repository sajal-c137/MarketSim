"""
Candlestick Plotter - Creates candlestick charts from OHLCV data
Uses plotly for interactive charts and matplotlib for static charts
"""

import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from typing import Optional, Tuple


class CandlestickPlotter:
    """Creates candlestick charts from OHLCV data"""
    
    def __init__(self, title: str = "Market Data - Candlestick Chart"):
        """
        Initialize plotter
        
        Args:
            title: Chart title
        """
        self.title = title
    
    def plot_interactive(
        self, 
        df: pd.DataFrame, 
        symbol: str = "AAPL",
        show_volume: bool = True,
        save_html: Optional[str] = None
    ) -> go.Figure:
        """
        Create interactive candlestick chart using Plotly
        
        Args:
            df: OHLCV DataFrame with datetime index
            symbol: Trading symbol
            show_volume: Whether to show volume subplot
            save_html: Optional path to save HTML file
            
        Returns:
            Plotly Figure object
        """
        # Create subplots
        if show_volume:
            fig = make_subplots(
                rows=2, cols=1,
                shared_xaxes=True,
                vertical_spacing=0.03,
                subplot_titles=(f'{symbol} - Candlestick Chart', 'Volume'),
                row_heights=[0.7, 0.3]
            )
        else:
            fig = go.Figure()
        
        # Add candlestick chart
        candlestick = go.Candlestick(
            x=df.index,
            open=df['open'],
            high=df['high'],
            low=df['low'],
            close=df['close'],
            name='OHLC',
            increasing_line_color='green',
            decreasing_line_color='red'
        )
        
        if show_volume:
            fig.add_trace(candlestick, row=1, col=1)
            
            # Add volume bars
            colors = ['green' if close >= open else 'red' 
                     for close, open in zip(df['close'], df['open'])]
            
            volume_bars = go.Bar(
                x=df.index,
                y=df['volume'],
                name='Volume',
                marker_color=colors,
                showlegend=False
            )
            fig.add_trace(volume_bars, row=2, col=1)
            
            # Update axes
            fig.update_xaxes(title_text="Time", row=2, col=1)
            fig.update_yaxes(title_text="Price", row=1, col=1)
            fig.update_yaxes(title_text="Volume", row=2, col=1)
        else:
            fig.add_trace(candlestick)
            fig.update_xaxes(title_text="Time")
            fig.update_yaxes(title_text="Price")
        
        # Update layout
        fig.update_layout(
            title=self.title,
            xaxis_rangeslider_visible=False,
            height=800 if show_volume else 600,
            hovermode='x unified'
        )
        
        # Save to HTML if requested
        if save_html:
            fig.write_html(save_html)
            print(f"Chart saved to {save_html}")
        
        return fig
    
    def plot_static(
        self, 
        df: pd.DataFrame, 
        symbol: str = "AAPL",
        figsize: Tuple[int, int] = (14, 8),
        save_png: Optional[str] = None
    ):
        """
        Create static candlestick chart using Matplotlib
        
        Args:
            df: OHLCV DataFrame with datetime index
            symbol: Trading symbol
            figsize: Figure size (width, height)
            save_png: Optional path to save PNG file
        """
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=figsize, 
                                        gridspec_kw={'height_ratios': [3, 1]},
                                        sharex=True)
        
        # Plot candlesticks
        for idx, row in df.iterrows():
            color = 'green' if row['close'] >= row['open'] else 'red'
            
            # Body
            body_height = abs(row['close'] - row['open'])
            body_bottom = min(row['open'], row['close'])
            
            rect = Rectangle(
                (idx, body_bottom),
                width=pd.Timedelta(seconds=row.get('interval_seconds', 1)),
                height=body_height,
                facecolor=color,
                edgecolor='black',
                linewidth=0.5
            )
            ax1.add_patch(rect)
            
            # Wick (high-low line)
            ax1.plot([idx, idx], [row['low'], row['high']], 
                    color='black', linewidth=0.5)
        
        # Set price chart properties
        ax1.set_ylabel('Price ($)')
        ax1.set_title(f'{symbol} - Candlestick Chart')
        ax1.grid(True, alpha=0.3)
        ax1.autoscale()
        
        # Plot volume
        colors = ['green' if close >= open else 'red' 
                 for close, open in zip(df['close'], df['open'])]
        ax2.bar(df.index, df['volume'], color=colors, alpha=0.6)
        ax2.set_ylabel('Volume')
        ax2.set_xlabel('Time')
        ax2.grid(True, alpha=0.3)
        
        plt.tight_layout()
        
        # Save if requested
        if save_png:
            plt.savefig(save_png, dpi=150, bbox_inches='tight')
            print(f"Chart saved to {save_png}")
        
        plt.show()
    
    def plot_with_moving_averages(
        self,
        df: pd.DataFrame,
        symbol: str = "AAPL",
        ma_periods: list = [10, 20, 50],
        save_html: Optional[str] = None
    ) -> go.Figure:
        """
        Create candlestick chart with moving averages
        
        Args:
            df: OHLCV DataFrame
            symbol: Trading symbol
            ma_periods: List of moving average periods
            save_html: Optional path to save HTML
            
        Returns:
            Plotly Figure
        """
        fig = make_subplots(
            rows=2, cols=1,
            shared_xaxes=True,
            vertical_spacing=0.03,
            subplot_titles=(f'{symbol} with Moving Averages', 'Volume'),
            row_heights=[0.7, 0.3]
        )
        
        # Add candlestick
        fig.add_trace(
            go.Candlestick(
                x=df.index,
                open=df['open'],
                high=df['high'],
                low=df['low'],
                close=df['close'],
                name='OHLC',
                increasing_line_color='green',
                decreasing_line_color='red'
            ),
            row=1, col=1
        )
        
        # Add moving averages
        colors = ['blue', 'orange', 'purple']
        for period, color in zip(ma_periods, colors):
            ma = df['close'].rolling(window=period).mean()
            fig.add_trace(
                go.Scatter(
                    x=df.index,
                    y=ma,
                    name=f'MA{period}',
                    line=dict(color=color, width=1)
                ),
                row=1, col=1
            )
        
        # Add volume
        colors_vol = ['green' if close >= open else 'red' 
                     for close, open in zip(df['close'], df['open'])]
        fig.add_trace(
            go.Bar(
                x=df.index,
                y=df['volume'],
                name='Volume',
                marker_color=colors_vol,
                showlegend=False
            ),
            row=2, col=1
        )
        
        # Update layout
        fig.update_layout(
            title=f'{symbol} - Technical Analysis',
            xaxis_rangeslider_visible=False,
            height=900,
            hovermode='x unified'
        )
        
        fig.update_xaxes(title_text="Time", row=2, col=1)
        fig.update_yaxes(title_text="Price", row=1, col=1)
        fig.update_yaxes(title_text="Volume", row=2, col=1)
        
        if save_html:
            fig.write_html(save_html)
            print(f"Chart saved to {save_html}")
        
        return fig
