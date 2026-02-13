# CMakeList.txt : CMake project for MarketSim, include source and define
# project specific logic here.
#

# Generate protobuf files
set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto/src/common")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# Custom command to generate protobuf C++ files
add_custom_command(
    OUTPUT "${PROTO_GEN_DIR}/exchange.pb.h" "${PROTO_GEN_DIR}/exchange.pb.cc"
    COMMAND protobuf::protoc
        --cpp_out=${PROTO_GEN_DIR}
        --proto_path=${PROTO_SRC_DIR}
        ${PROTO_SRC_DIR}/exchange.proto
    DEPENDS "${PROTO_SRC_DIR}/exchange.proto"
    COMMENT "Generating C++ protobuf files..."
)

# Protobuf library (ensures headers are generated first)
add_library(proto_lib STATIC
    "${PROTO_GEN_DIR}/exchange.pb.cc"
)

target_include_directories(proto_lib PUBLIC
    "${PROTO_GEN_DIR}"
)

target_link_libraries(proto_lib PUBLIC
    protobuf::libprotobuf
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET proto_lib PROPERTY CXX_STANDARD 20)
endif()

# Add source to this project's executable.
add_executable (MarketSim "MarketSim.cpp" "MarketSim.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MarketSim PROPERTY CXX_STANDARD 20)
endif()

# Link vcpkg libraries
target_link_libraries(MarketSim 
    PRIVATE 
    cppzmq cppzmq-static
    protobuf::libprotobuf
)

# Monitor library (core logging, no I/O dependencies to avoid circular deps)
add_library(monitor_lib STATIC
    "src/monitor/status_monitor.cpp"
    "src/monitor/exchange_logger.cpp"
)

# Monitor service library (has I/O dependencies)
add_library(monitor_service_lib STATIC
    "src/monitor/exchange_monitor.cpp"
)

target_include_directories(monitor_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROTO_GEN_DIR}"
)

target_link_libraries(monitor_lib PUBLIC
    proto_lib
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET monitor_lib PROPERTY CXX_STANDARD 20)
endif()

# IOHandler library
add_library(io_handler_lib STATIC
    "src/io_handler/io_context.cpp"
    "src/io_handler/message_serializer.cpp"
    "src/io_handler/zmq_publisher.cpp"
    "src/io_handler/zmq_subscriber.cpp"
    "src/io_handler/zmq_requester.cpp"
    "src/io_handler/zmq_replier.cpp"
)

# Exchange library (Matching Engine + Service)
add_library(exchange_lib STATIC
    "src/exchange/operations/order_book.cpp"
    "src/exchange/operations/matching_engine.cpp"
    "src/exchange/main/exchange_service.cpp"
)

target_include_directories(io_handler_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROTO_GEN_DIR}"
)

target_link_libraries(io_handler_lib PUBLIC
    monitor_lib
    proto_lib
    cppzmq cppzmq-static
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET io_handler_lib PROPERTY CXX_STANDARD 20)
endif()

# Exchange library includes
target_include_directories(exchange_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROTO_GEN_DIR}"
)

target_link_libraries(exchange_lib PUBLIC
    monitor_lib
    proto_lib
    io_handler_lib
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET exchange_lib PROPERTY CXX_STANDARD 20)
endif()

# Monitor service library (depends on io_handler_lib and monitor_lib)
target_include_directories(monitor_service_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROTO_GEN_DIR}"
)

target_link_libraries(monitor_service_lib PUBLIC
    monitor_lib
    io_handler_lib
    exchange_lib
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET monitor_service_lib PROPERTY CXX_STANDARD 20)
endif()

# Test executables
add_executable(test_status_monitor "test/test_status_monitor.cpp")
target_link_libraries(test_status_monitor PRIVATE monitor_lib)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_status_monitor PROPERTY CXX_STANDARD 20)
endif()

add_executable(test_logging "test/test_logging.cpp")
target_link_libraries(test_logging PRIVATE monitor_lib)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_logging PROPERTY CXX_STANDARD 20)
endif()

add_executable(test_io_handler "test/test_io_handler.cpp")
target_link_libraries(test_io_handler PRIVATE io_handler_lib monitor_lib)
target_include_directories(test_io_handler PRIVATE "${PROTO_GEN_DIR}")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_io_handler PROPERTY CXX_STANDARD 20)
endif()

add_executable(test_matching_engine "test/test_matching_engine.cpp")
target_link_libraries(test_matching_engine PRIVATE exchange_lib monitor_lib)
target_include_directories(test_matching_engine PRIVATE "${PROTO_GEN_DIR}")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_matching_engine PROPERTY CXX_STANDARD 20)
endif()

# Traffic Generator library
add_library(traffic_generator_lib STATIC
    # Operations (Pure math)
    "src/traffic_generator/operations/price_movement_calculator.cpp"
    "src/traffic_generator/operations/order_flow_generator.cpp"
    
    # Utils
    "src/traffic_generator/utils/time_utils.cpp"
    
    # Threads
    "src/traffic_generator/threads/generation_thread.cpp"
    
    # Main
    "src/traffic_generator/main/traffic_generator_main.cpp"
)

target_include_directories(traffic_generator_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROTO_GEN_DIR}"
)

target_link_libraries(traffic_generator_lib PUBLIC
    monitor_lib
    io_handler_lib
    proto_lib
    cppzmq
    cppzmq-static
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET traffic_generator_lib PROPERTY CXX_STANDARD 20)
endif()

# Test executable for traffic generator
add_executable(test_traffic_generator "test/test_traffic_generator.cpp")
target_link_libraries(test_traffic_generator PRIVATE traffic_generator_lib)
target_include_directories(test_traffic_generator PRIVATE "${PROTO_GEN_DIR}")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_traffic_generator PROPERTY CXX_STANDARD 20)
endif()

# Test executable for exchange server
add_executable(test_exchange_server "test/test_exchange_server.cpp")
target_link_libraries(test_exchange_server PRIVATE io_handler_lib monitor_lib exchange_lib)
target_include_directories(test_exchange_server PRIVATE "${PROTO_GEN_DIR}")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_exchange_server PROPERTY CXX_STANDARD 20)
endif()

# Monitor service (queries Exchange and displays logs)
add_executable(monitor_exchange "test/monitor_exchange.cpp")
target_link_libraries(monitor_exchange PRIVATE monitor_service_lib)
target_include_directories(monitor_exchange PRIVATE "${PROTO_GEN_DIR}")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET monitor_exchange PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
