#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script copies config.h of the ACE framework to the source directory.
# --> allows reuse of the same ACE source tree on different platforms.
# *NOTE*: it is neither portable nor particularly stable !
# parameters:   - (UNIX) platform [linux|solaris]
# return value: - 0 success, 1 failure

# sanity checks
command -v cmake >/dev/null 2>&1 || { echo "cmake is not installed, aborting" >&2; exit 1; }
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v readlink >/dev/null 2>&1 || { echo "readlink is not installed, aborting" >&2; exit 1; }

#DEFAULT_PLATFORM="linux"
#PLATFORM=${DEFAULT_PLATFORM}
#if [ $# -lt 1 ]; then
# echo "INFO: using default platform: \"${PLATFORM}\""
#else
# # parse any arguments
# if [ $# -ge 1 ]; then
#  PLATFORM="$1"
# fi
#fi
#echo "DEBUG: platform: \"${PLATFORM}\""

DEFAULT_PROJECTS_DIRECTORY="$(dirname $(readlink -f $0))/../.."
PROJECTS_DIRECTORY=${DEFAULT_PROJECTS_DIRECTORY}
# sanity check(s)
[ ! -d ${PROJECTS_DIRECTORY} ] && echo "ERROR: invalid projects directory (was: \"${PROJECTS_DIRECTORY}\"), aborting" && exit 1
#echo "DEBUG: project directory: \"${PROJECT_DIRECTORY}\""
echo "set projects directory: \"${PROJECTS_DIRECTORY}\""

CMAKE_PARAMETERS=$'-G \"Unix Makefiles\" -Wdev'
PROJECTS="libCommon
libACEStream
libACENetwork"
for PROJECT in $PROJECTS
do
 PROJECT_PATH="${PROJECTS_DIRECTORY}/${PROJECT}"
 [ ! -d ${PROJECT_PATH} ] && echo "ERROR: invalid project directory (was: \"${PROJECT_PATH}\"), aborting" && exit 1
 BUILD_PATH="${PROJECT_PATH}/cmake"
 [ ! -d ${BUILD_PATH} ] && echo "ERROR: invalid build directory (was: \"${BUILD_PATH}\"), aborting" && exit 1
 cd $BUILD_PATH
 [ $? -ne 0 ] && echo "ERROR: failed to cd to \"${BUILD_PATH}\": $?, aborting" && exit 1

 CMAKE_CACHE_FILE="$BUILD_PATH/CMakeCache.txt"
 if [ -r $CMAKE_CACHE_FILE ]; then
  rm -f $CMAKE_CACHE_FILE
  [ $? -ne 0 ] && echo "ERROR: failed to rm \"$CMAKE_CACHE_FILE\": $?, aborting" && exit 1 
  echo "processing ${PROJECT}...deleted cache"
 fi
 
 SOURCE_PATH=$PROJECT_PATH
 cmake $CMAKE_PARAMETERS $SOURCE_PATH
 cmake $CMAKE_PARAMETERS $SOURCE_PATH
 [ $? -ne 0 ] && echo "ERROR: failed to cmake (source path was: \"${SOURCE_PATH}\"): $?, aborting" && exit 1

 echo "processing ${PROJECT}...DONE"
done

