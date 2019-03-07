#!/bin/bash
# author:      Erik Sohns <eriksohns@123mail.org>
# this script gathers the dependent libraries in one place for in-source-tree
# debugging
# *NOTE*: it is neither portable nor particularly stable !
# arguments:    - -b: build ("debug" || "debug_tracing" || "release" || ...) {"debug"}
#               - -c: use CMake build {false: use autotools build}
# return value: - 0 success, 1 failure

# sanity checks
command -v basename>/dev/null 2>&1 || { echo "basename is not installed, aborting" >&2; exit 1; }
command -v cut>/dev/null 2>&1 || { echo "cut is not installed, aborting" >&2; exit 1; }
command -v dirname>/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v echo>/dev/null 2>&1 || { echo "echo is not supported, aborting" >&2; exit 1; }
command -v getopts>/dev/null 2>&1 || { echo "getopts is not supported, aborting" >&2; exit 1; }
command -v mkdir>/dev/null 2>&1 || { echo "mkdir is not installed, aborting" >&2; exit 1; }
command -v readlink>/dev/null 2>&1 || { echo "readlink is not installed, aborting" >&2; exit 1; }

DEFAULT_PROJECT_DIR="$(dirname $(readlink -f $0))/.."
PROJECT_DIR=${DEFAULT_PROJECT_DIR}
# sanity check(s)
[ ! -d ${PROJECT_DIR} ] && echo "ERROR: invalid project dir (was: \"${PROJECT_DIR}\"), aborting" && exit 1

# parse arguments
DEFAULT_BUILD="debug"
BUILD=${DEFAULT_BUILD}
DEFAULT_USE_CMAKE_BUILD=0
USE_CMAKE_BUILD=${DEFAULT_USE_CMAKE_BUILD}
while getopts "b:c" option; do
 case $option in
  b)
   BUILD=$OPTARG
   echo "using build: $BUILD"
   ;;
  c)
   USE_CMAKE_BUILD=1
   echo "using CMake build"
   ;;
  *)
   ;;
 esac
done

# sanity check(s)
if [ ${USE_CMAKE_BUILD} -eq 0 ]; then
 [ ${BUILD} != "debug" -a ${BUILD} != "debug_tracing" -a ${BUILD} != "release" ] && echo "WARNING: invalid/unknown build (was: \"${BUILD}\"), continuing"
 BUILD_DIR="${PROJECT_DIR}/build/${BUILD}"
else
 [ ${BUILD} != "Debug" -a ${BUILD} != "Release" -a ${BUILD} != "RelWithoutDeb" ] && echo "WARNING: invalid/unknown build (was: \"${BUILD}\"), continuing"
 BUILD_DIR="${PROJECT_DIR}/cmake"
fi
[ ! -d "${BUILD_DIR}" ] && echo "ERROR: invalid build dir (was: \"${BUILD_DIR}\"), aborting" && exit 1

LIB_DIR="lib"
TARGET_DIR="${BUILD_DIR}/${LIB_DIR}"
if [ ! -d "${TARGET_DIR}" ]; then
 mkdir ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to mkdir \"${TARGET_DIR}\", aborting" && exit 1
 echo "INFO: created directory \"${TARGET_DIR}\", continuing"
fi

echo "copying 3rd-party libraries"
LIB_DIR="lib"
MODULES_DIR="${PROJECT_DIR}/modules"
SUB_DIRS="ACE/build/linux"
#declare -a LIBS=("libACE.so")
LIBS="libACE.so"
i=0
for DIR in $SUB_DIRS
do
# LIB="${MODULES_DIR}/${DIR}/${LIB_DIR}/${LIBS[$i]}"
 LIB="${MODULES_DIR}/${DIR}/${LIB_DIR}/${LIBS}"
 [ ! -r "${LIB}" ] && echo "ERROR: invalid library file (was: \"${LIB}\"), aborting" && exit 1

 VERSION="$(echo $(basename $(readlink -f ${LIB})) | cut --bytes=11- -)"
 echo "found ACE version: \"${VERSION}\"..."
 [ ! -r "${LIB}.${VERSION}" ] && echo "ERROR: invalid library file (was: \"${LIB}.${VERSION}\"), aborting" && exit 1

 cp -f "${LIB}.${VERSION}" ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to copy \"${LIB}.${VERSION}\" to \"${TARGET_DIR}\": $?, aborting" && exit 1
 echo "copied \"$LIB.${VERSION}\"..."

 i=$i+1
done

echo "copying external module libraries"
LIB_DIR=".libs"
VERSION="0.0.1"
if [ ${USE_CMAKE_BUILD} -eq 0 ]; then
 SUB_DIRS="modules/libCommon/src
modules/libCommon/src/ui
modules/libACEStream/src"
else
 SUB_DIRS="../../libCommon/cmake/src
../../libCommon/cmake/src/ui
../../libACEStream/cmake/src"
fi
#declare -a LIBS=("libCommon.so"
LIBS="libCommon.so libCommonUI.so libACEStream.so"
set -- $LIBS
#i=0
for DIR in $SUB_DIRS
do
# LIB="${BUILD_DIR}/${DIR}/${LIB_DIR}/${LIBS[$i]}"
 if [ ${USE_CMAKE_BUILD} -eq 0 ]; then
  LIB="${BUILD_DIR}/${DIR}/${LIB_DIR}/$1"
 else
  LIB="${BUILD_DIR}/${DIR}/$1"
 fi

 [ ! -r "${LIB}.${VERSION}" ] && echo "ERROR: invalid library file (was: \"${LIB}.${VERSION}\"), aborting" && exit 1
 cp -f "${LIB}.${VERSION}" ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to copy \"${LIB}.${VERSION}\" to \"${TARGET_DIR}\": $?, aborting" && exit 1
 echo "copied \"$LIB.${VERSION}\"..."
# i=$i+1
 shift
done

echo "copying framework libraries"
LIB_DIR=".libs"
SUB_DIRS="src
src/client_server
src/client_server"
#declare -a LIBS=("libACENetwork.so"
LIBS="libACENetwork.so libACENetwork_Client.so libACENetwork_Server.so"
set -- $LIBS
#i=0
for DIR in $SUB_DIRS
do
# LIB="${BUILD_DIR}/${DIR}/${LIB_DIR}/${LIBS[$i]}"
 if [ ${USE_CMAKE_BUILD} -eq 0 ]; then
  LIB="${BUILD_DIR}/${DIR}/${LIB_DIR}/$1"
 else
  LIB="${BUILD_DIR}/${DIR}/$1"
 fi

 [ ! -r "${LIB}.${VERSION}" ] && echo "ERROR: invalid library file (was: \"${LIB}.${VERSION}\"), aborting" && exit 1
 cp -f "${LIB}.${VERSION}" ${TARGET_DIR}
 [ $? -ne 0 ] && echo "ERROR: failed to copy \"${LIB}.${VERSION}\" to \"${TARGET_DIR}\": $?, aborting" && exit 1
 echo "copied \"$LIB.${VERSION}\"..."
# i=$i+1
 shift
done
