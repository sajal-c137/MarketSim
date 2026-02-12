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

# TODO: Add tests and install targets if needed.
