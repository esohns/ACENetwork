include (${CMAKE_CURRENT_SOURCE_DIR}/scripts/VersionConfig.cmake)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../Common/scripts;${CMAKE_CURRENT_SOURCE_DIR}/scripts")
include (Configuration)
include (Macros)
include (OS)
include (compiler_init)

cmake_minimum_required (VERSION 2.8...3.12)

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
#  set_property (GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
 endif (CCACHE_PROGRAM)
 set (ENV{CCACHE_DISABLE} "true")
endif (UNIX)

include (PrePreToolChain)

# *NOTE*: sets CMAKE_TOOLCHAIN_FILE, so include BEFORE project()
include (vcpkg)

# project
cmake_policy (SET CMP0048 NEW) # allow project VERSION
#cmake_policy (SET CMP0056 OLD) # honor per-config flags in try_compile()
#cmake_policy (SET CMP0066 OLD) # honor per-config flags in try_compile()
project (${PROJECT_NAME}
         VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}
         LANGUAGES CXX C)

include (PreToolChain)
include (SetToolChain)
include (ToolChain)
# *NOTE*: or use the cmake.exe '-T' option instead
include (SetTargetPlatform)
include (TargetPlatform)

# directories
# override the installation directories ?
#set (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION OFF)
if (UNIX)
 include (GNUInstallDirs)
# set (INSTALL_SHARE_DIR "share" CACHE PATH "Installation directory for shared files")
endif (UNIX)
set (INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set (INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set (INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
if (WIN32 AND NOT CYGWIN)
 set (DEF_INSTALL_CMAKE_DIR CMake)
else ()
 set (DEF_INSTALL_CMAKE_DIR lib/CMake/${PROJECT_NAME})
endif ()
set (INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

# make relative paths absolute (needed later on)
foreach (path BIN CMAKE SHARE INCLUDE LIB)
 set (var INSTALL_${path}_DIR)
# if (NOT IS_ABSOLUTE "${${var}}")
 string (TOLOWER "${path}" RELATIVE_INSTALL_PATH)

 if (UNIX)
  set (${var} "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_PATH}")
 elseif (WIN32)
  set (${var} ${RELATIVE_INSTALL_PATH})
 endif ()
# message (STATUS "set \"${var}\" to \"${${var}}\"")

#endif ()
endforeach ()

# project options
add_compile_options ($<$<CONFIG:Debug>:-D_DEBUG>)
add_compile_options ($<$<CONFIG:Release>:-DACE_NDEBUG>)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-g)
  add_compile_options (-O0)
  add_compile_options (-fno-inline)
 endif (CMAKE_COMPILER_IS_GNUCXX)
elseif (CMAKE_BUILD_TYPE STREQUAL Release)
 if (CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options (-O3)
 endif (CMAKE_COMPILER_IS_GNUCXX)
endif ()
if (MSVC)
 remove_definitions (/DWINVER)
 remove_definitions (/D_WIN32_WINNT)
endif (MSVC)

option (TRACING "enable tracing" OFF)
if (TRACING)
 add_definitions (-DACENETWORK_NTRACE=0)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACENETWORK_NTRACE=0)
else ()
 add_definitions (-DACENETWORK_NTRACE=1)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS ACENETWORK_NTRACE=1)
endif ()

include (valgrind)

option (BUILD_MODULES "build (git) sub-modules" OFF)

if (CMAKE_COMPILER_IS_GNUCXX)
# add_compile_options (-ansi)
# add_compile_options (-std=c++11)
 add_compile_options (-Wall)
 set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -g")
 add_definitions (-D_GNU_SOURCE)
#  set_property (SOURCE PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)
elseif (MSVC)
 add_compile_options (/MP)
 add_definitions (-D_CRT_SECURE_NO_WARNINGS)
 add_definitions (-D_WINSOCK_DEPRECATED_NO_WARNINGS)

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

set (LLVM_COMPILER_ID_STRING "Clang")
string (TOLOWER "${LLVM_COMPILER_ID_STRING}" LLVM_COMPILER_ID_STRING_LOWER)
string (TOLOWER "${CMAKE_CXX_COMPILER_ID}" CXX_COMPILER_ID_STRING_LOWER)
if (CXX_COMPILER_ID_STRING_LOWER STREQUAL LLVM_COMPILER_ID_STRING_LOWER)
 add_compile_options (-fPIC)
# add_definitions (-D_GLIBCXX_USE_CXX11_ABI=0)
endif ()

################################################################################
#if (DEFINED ENV{LIB_ROOT})
# file (TO_CMAKE_PATH "$ENV{LIB_ROOT}/cotire/CMake" COTIRE_PATH)
#else ()
#  set (COTIRE_PATH "modules/cotire/CMake")
#endif (DEFINED ENV{LIB_ROOT})
#list (APPEND CMAKE_MODULE_PATH "${COTIRE_PATH}")
#cmake_policy (SET CMP0011 NEW)
#include (cotire)
#set_directory_properties (PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
#list (APPEND CMAKE_MODULE_PATH "$ENV{LIB_ROOT}/cmake-precompiled-header")
#include (PrecompiledHeader)

################################################################################

# configuration files
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export (PACKAGE ${PROJECT_NAME})

# Create the ACENetworkConfig.cmake and ACENetworkConfigVersion.cmake files
# *NOTE*: see: https://cmake.org/Wiki/CMake/Tutorials/How_to_create_a_ProjectConfig.cmake_file
#file (RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set (CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
if (UNIX)
 set (NEWLINE_STYLE UNIX)
elseif (WIN32)
 set (NEWLINE_STYLE WIN32)
else ()
 set (NEWLINE_STYLE UNIX)
endif ()
configure_file (${PROJECT_NAME}Config.cmake.in
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                @ONLY
                NEWLINE_STYLE ${NEWLINE_STYLE})
# ... for the install tree
set (CONF_INCLUDE_DIRS "\${ACENetwork_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file (${PROJECT_NAME}Config.cmake.in
                "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
                @ONLY
                NEWLINE_STYLE ${NEWLINE_STYLE})
# ... for both
configure_file (${PROJECT_NAME}ConfigVersion.cmake.in
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
                @ONLY
                NEWLINE_STYLE ${NEWLINE_STYLE})

set (PACKAGE ${PROJECT_NAME})
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.h
                @ONLY
                NEWLINE_STYLE ${NEWLINE_STYLE})
#add_custom_target (${PROJECT_NAME}_config)
#set_target_properties (${PROJECT_NAME}_config PROPERTIES
#                       PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.h")
add_definitions (-DHAVE_CONFIG_H)
#set_property (SOURCE PROPERTY COMPILE_DEFINITIONS HAVE_CONFIG_H)

include (libraries_init)

################################################################################

add_subdirectory (src)

################################################################################

# Install the ACENetworkConfig.cmake and ACENetworkConfigVersion.cmake
install (FILES
         "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
         "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
#         "${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake"
#         "${PROJECT_NAME}ConfigVersion.cmake"
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
# Install the export set for use with the install-tree
install (EXPORT ${PROJECT_NAME}Targets
         DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.h"
         DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}"
         COMPONENT dev)

################################################################################
if (DEFINED ENV{LIB_ROOT})
 file (TO_CMAKE_PATH "$ENV{LIB_ROOT}/cmake-modules" CMAKE_MODULES_PATH)
else ()
  set (CMAKE_MODULES_PATH "modules/cmake-modules")
endif (DEFINED ENV{LIB_ROOT})
list (APPEND CMAKE_MODULE_PATH "${CMAKE_MODULES_PATH}")
#include (UseBackportedModules)
cmake_policy (SET CMP0026 OLD)
cmake_policy (SET CMP0053 OLD)
include (CreateLaunchers)

# enable (unit/integration) tests
#include (CTest)
#enable_testing ()

################################################################################

add_subdirectory (test_u)
add_subdirectory (test_i)

################################################################################

if (UNIX)
 include (GNUInstallDirs)
elseif (WIN32)
 if (MSVC)
#  set (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.16299.0") # set Windows SDK version
#  message (STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
#  add_definitions (-CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
 endif (MSVC)
endif ()

#set (NSIS_PROJECT_FILE "${PROJECT_NAME}.nsi")
#add_custom_command (TARGET ${PROJECT_NAME}
#                    POST_BUILD
#                    COMMAND make_installer.bat ${NSIS_PROJECT_FILE}
#                    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/scripts"
#                    COMMENT "making installer..."
#                    VERBATIM)

# build a CPack driven installer package
# *NOTE*: "... If CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS is NOT set, then
#         this file warns about required files that do not exist. You can set
#         this variable to ON before including this file to avoid the warning.
#         For example, the Visual Studio Express editions do not include the
#         redistributable files, so if you include this file on a machine with
#         only VS Express installed, you’ll get the warning. ..."
include (InstallRequiredSystemLibraries)
#set (CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
#set (CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
#set (CPACK_PACKAGE_VERSION_PATCH ${VERSION_MICRO})
#set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
#set (CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README)

# build a CPack driven installer package
include (CPack)
