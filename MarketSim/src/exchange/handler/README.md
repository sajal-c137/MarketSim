# Exchange Handlers

Request and response handlers for client communication.

## Contents

- **Order Handler**: Processes incoming order messages, validates them
- **Market Data Handler**: Handles client requests for market data (price, order book)
- **Subscription Handler**: Manages client subscriptions to market data feeds

## Responsibilities

- Parse incoming protobuf messages
- Validate requests
- Route to appropriate operations
- Format responses
