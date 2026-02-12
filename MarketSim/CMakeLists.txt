# CMakeList.txt : CMake project for MarketSim, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (MarketSim "MarketSim.cpp" "MarketSim.h"         )

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

# Test executables
add_executable(test_status_monitor "test_status_monitor.cpp")
target_link_libraries(test_status_monitor PRIVATE monitor_lib)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET test_status_monitor PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
