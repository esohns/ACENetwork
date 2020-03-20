# ACENetworkConfig.cmake.in
#  ACENetwork_INCLUDE_DIRS - include directories for FooBar
#  ACENetwork_LIBRARIES    - libraries to link against

# Compute paths
get_filename_component (ACENetwork_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set (ACENetwork_INCLUDE_DIRS "${ACENetwork_CMAKE_DIR}/../../../include")

# library dependencies (contains definitions for IMPORTED targets)
if (NOT TARGET ACENetwork AND NOT ACENetwork_BINARY_DIR)
include ("${ACENetwork_CMAKE_DIR}/ACENetworkTargets.cmake")
endif ()

# These are IMPORTED targets created by ACENetworkTargets.cmake
set (ACENetwork_LIBRARIES ACENetwork)
