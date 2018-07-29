include (${CMAKE_CURRENT_SOURCE_DIR}/../libCommon/scripts/PreToolChain.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../libCommon/scripts/SetToolChain.cmake)
include (${CMAKE_CURRENT_SOURCE_DIR}/../libCommon/scripts/ToolChain.cmake)

cmake_minimum_required (VERSION 2.8)

cmake_policy (SET CMP0069 NEW) # IPO is enforced when enabled
include (CheckIPOSupported)
check_ipo_supported (RESULT IPO_SUPPORTED
                     OUTPUT IPO_REASON)

# global options
if (IPO_SUPPORTED)
 set (CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif (IPO_SUPPORTED)
#set (CMAKE_VERBOSE_MAKEFILE ON)
set_property (GLOBAL PROPERTY USE_FOLDERS ON)
if (UNIX)
 find_program (CCACHE_PROGRAM ccache)
 if (CCACHE_PROGRAM)
  set_property (GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
 endif (CCACHE_PROGRAM)
endif (UNIX)

# version
set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_MICRO 1)
set (VERSION_DEVEL devel)
set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
if (VERSION_DEVEL)
 unset (VERSION)
 set (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}-${VERSION_DEVEL})
endif ()

# build type
#variable_watch (CMAKE_BUILD_TYPE)
# *NOTE*: iff the user specifies -DCMAKE_BUILD_TYPE on the command line, take
#         their definition and dump it in the cache along with proper
#         documentation, otherwise set CMAKE_BUILD_TYPE to Debug prior to
#         calling PROJECT()
set (BUILD_TYPE_CACHE_STRING "choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
if (DEFINED CMAKE_BUILD_TYPE)
 set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING ${BUILD_TYPE_CACHE_STRING})
else ()
 set (CMAKE_BUILD_TYPE Debug CACHE STRING ${BUILD_TYPE_CACHE_STRING})
endif ()

# project
cmake_policy (SET CMP0048 NEW) # allow project VERSION
#cmake_policy (SET CMP0056 OLD) # honor per-config flags in try_compile()
#cmake_policy (SET CMP0066 OLD) # honor per-config flags in try_compile()
project (ACENetwork
         VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
         LANGUAGES CXX C)

# directories
# override the installation directories ?
set (INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set (INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set (INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
if (WIN32 AND NOT CYGWIN)
 set (DEF_INSTALL_CMAKE_DIR CMake)
else ()
 set (DEF_INSTALL_CMAKE_DIR lib/CMake/FooBar)
endif ()
set (INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# make relative paths absolute (needed later on)
foreach (path LIB BIN INCLUDE CMAKE)
 set (var INSTALL_${path}_DIR)
 if (NOT IS_ABSOLUTE "${${var}}")
  set (${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
 endif ()
endforeach ()

# project options
set (DEBUG_DEBUGGER TRUE
     CACHE BOOL "in debugger")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-g)
  add_compile_options (-O0)
  add_compile_options (-fno-inline)
 endif ()
 add_definitions (-D_DEBUG)
 if (WIN32)
  add_definitions (-D_ITERATOR_DEBUG_LEVEL=2)
 endif ()
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS _DEBUG)
 if (DEBUG_DEBUGGER AND
     NOT DEFINED IN_CPACK)
  add_definitions (-DDEBUG_DEBUGGER)
 endif ()
elseif (CMAKE_BUILD_TYPE STREQUAL Release)
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-O3)
 endif ()
 add_definitions (-DACE_NDEBUG)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACE_NDEBUG)
 if (WIN32)
  add_definitions (-D_ITERATOR_DEBUG_LEVEL=0)
 endif ()
endif ()

option (TRACING "enable tracing" OFF)
if (TRACING)
 add_definitions (-DACENETWORK_NTRACE=0)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACENETWORK_NTRACE=0)
else ()
 add_definitions (-DACENETWORK_NTRACE=1)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACENETWORK_NTRACE=1)
endif ()

option (VALGRIND_SUPPORT "enable valgrind support" OFF)
include (CheckIncludeFiles)
include (FindPkgConfig)
if (VALGRIND_SUPPORT)
 check_include_files (valgrind/valgrind.h HAVE_VALGRIND_H)
#  include (CheckLibraryExists)
#  check_library_exists (libvalgrind "" "" HAVE_LIB_VALGRIND)
 pkg_check_modules (PKGVALGRIND REQUIRED valgrind)
 add_definitions (-DLIBACENETWORK_ENABLE_VALGRIND_SUPPORT)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS LIBACENETWORK_ENABLE_VALGRIND_SUPPORT)
endif ()

option (BUILD_MODULES "build (git) sub-modules" OFF)

if (CMAKE_COMPILER_IS_GNUCXX)
 add_compile_options (-ansi)
 add_compile_options (-std=c++11)
 add_compile_options (-Wall)
 set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -g")
 add_definitions (-D_GNU_SOURCE)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)
elseif (MSVC)
 add_compile_options (/MP)
# add_definitions (-D_CRT_SECURE_NO_WARNINGS)
# add_definitions (-D_WINSOCK_DEPRECATED_NO_WARNINGS)

 set (MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:NO")
 foreach (FLAG_TYPE EXE MODULE SHARED STATIC)
  string (REGEX REPLACE "/INCREMENTAL* " "/INCREMENTAL:NO " CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG "${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}")
#  message (STATUS "flag: ${FLAG_TYPE}: ${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}")
  string (REPLACE "/EDITANDCONTINUE"     ""                 CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG "${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}")
  set (CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG "${CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG}"
       CACHE STRING "Overriding default debug ${FLAG_TYPE} linker flags." FORCE)
  mark_as_advanced (CMAKE_${FLAG_TYPE}_LINKER_FLAGS_DEBUG)
 endforeach ()
endif ()

################################################################################

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cotire/CMake")
#list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (cotire)
set_directory_properties (PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)

################################################################################

add_subdirectory (src)

################################################################################

# configuration files
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export (PACKAGE ${PROJECT_NAME})

# Create the ACENetworkConfig.cmake and ACENetworkConfigVersion.cmake files
# *NOTE*: see: https://cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file
file (RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set (CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file (${PROJECT_NAME}Config.cmake.in
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)
# ... for the install tree
set (CONF_INCLUDE_DIRS "\${ACENetwork_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file (${PROJECT_NAME}Config.cmake.in
                "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake" @ONLY)
# ... for both
configure_file (${PROJECT_NAME}ConfigVersion.cmake.in
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

set (PACKAGE ${PROJECT_NAME})
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_config.h
                @ONLY)
#add_custom_target (${PROJECT_NAME}_config)
#set_target_properties (${PROJECT_NAME}_config PROPERTIES
#                       PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.h")
add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

################################################################################

# Install the libACENetworkConfig.cmake and libACENetworkConfigVersion.cmake
install (FILES
         "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
         "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
# Install the export set for use with the install-tree
install (EXPORT ${PROJECT_NAME}Targets
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_config.h"
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT dev)

################################################################################

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-modules")
include (UseBackportedModules)
cmake_policy (SET CMP0026 OLD)
cmake_policy (SET CMP0053 OLD)
include (CreateLaunchers)

# enable (unit/integration) tests
#include (CTest)
enable_testing ()

################################################################################

add_subdirectory (test_u)
add_subdirectory (test_i)

################################################################################

if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 include (GNUInstallDirs)

 set (CMAKE_THREAD_PREFER_PTHREAD ON)
elseif (WIN32)
 if (MSVC)
#  message (STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
#  set (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.16299.0") # set Windows SDK version
  message (STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
#  add_definitions (-CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
 endif ()
 include (InstallRequiredSystemLibraries)
endif ()

#set (NSIS_PROJECT_FILE "lib${PROJECT_NAME}.nsi")
#add_custom_command (TARGET ${PROJECT_NAME}
#                    POST_BUILD
#                    COMMAND make_installer.bat ${NSIS_PROJECT_FILE}
#                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/scripts"
#                    COMMENT "making installer..."
#                    VERBATIM)

# build a CPack driven installer package
#include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${VERSION_MICRO})
set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set (CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README)

# build a CPack driven installer package
include (CPack)
