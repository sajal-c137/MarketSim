# TrafficGenerator Main

Entry point and orchestration for the traffic generator component.

## Responsibilities

- Initialize TrafficGenerator subsystem components
- Manage component lifecycle and graceful shutdown
- Coordinate communication with Exchange via IOHandler
- Manage generation threads and background processes
- Handle configuration loading and validation
- Provide logging and monitoring integration

## Files

- `traffic_generator_main.cpp/h` - Main entry point and orchestrator
- `traffic_generator_config.cpp/h` - Configuration management

## Architecture

The main module orchestrates:
1. **Initialization Phase**: Set up ZeroMQ sockets, load config, start threads
2. **Generation Phase**: Run market data generation loop
3. **Shutdown Phase**: Graceful termination with cleanup

## Dependencies

- IOHandler: For ZeroMQ communication
- Monitor: For logging and metrics
- Operations: Core generation logic
- Threads: Background processing
- Configs: Configuration management
