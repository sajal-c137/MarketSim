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

# Monitor library
add_library(monitor_lib STATIC
    "src/monitor/status_monitor.cpp"
)

target_include_directories(monitor_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
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
    "${PROTO_GEN_DIR}/exchange.pb.cc"
)

target_include_directories(io_handler_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${PROTO_GEN_DIR}"
)

target_link_libraries(io_handler_lib PUBLIC
    monitor_lib
    cppzmq cppzmq-static
    protobuf::libprotobuf
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET io_handler_lib PROPERTY CXX_STANDARD 20)
endif()

# Test executables
add_executable(test_status_monitor "test_status_monitor.cpp")
target_link_libraries(test_status_monitor PRIVATE monitor_lib)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_status_monitor PROPERTY CXX_STANDARD 20)
endif()

add_executable(test_logging "test_logging.cpp")
target_link_libraries(test_logging PRIVATE monitor_lib)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_logging PROPERTY CXX_STANDARD 20)
endif()

add_executable(test_io_handler "test_io_handler.cpp")
target_link_libraries(test_io_handler PRIVATE io_handler_lib monitor_lib)
target_include_directories(test_io_handler PRIVATE "${PROTO_GEN_DIR}")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_io_handler PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
