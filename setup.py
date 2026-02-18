"""
MarketSim Trader Package Setup

To install:
    pip install -e .

To use:
    python -m trader.analyze.main --symbol AAPL
    # OR
    marketsim-analyze --symbol AAPL
"""

from setuptools import setup, find_packages
import os

# Read README from trader/analyze
readme_path = os.path.join('trader', 'analyze', 'README.md')
if os.path.exists(readme_path):
    with open(readme_path, 'r', encoding='utf-8') as f:
        long_description = f.read()
else:
    long_description = "Trading system for MarketSim - OHLCV analysis and visualization"

setup(
    name="marketsim-trader",
    version="1.0.0",
    description="Trading system for MarketSim - OHLCV analysis and visualization",
    long_description=long_description,
    long_description_content_type="text/markdown",
    author="MarketSim",
    url="https://github.com/sajal-c137/MarketSim",
    packages=find_packages(),
    install_requires=[
        'pandas>=2.0.0',
        'numpy>=1.24.0',
        'plotly>=5.14.0',
        'matplotlib>=3.7.0',
        'kaleido>=0.2.1',
    ],
    python_requires='>=3.8',
    entry_points={
        'console_scripts': [
            'marketsim-analyze=trader.analyze.main:main',
        ],
    },
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Financial and Insurance Industry',
        'Topic :: Office/Business :: Financial :: Investment',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
    ],
)
