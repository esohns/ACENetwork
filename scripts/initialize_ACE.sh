#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script copies config.h of the ACE framework to the source directory.
# --> allows reuse of the same ACE source tree on different platforms.
# *NOTE*: it is neither portable nor particularly stable !
# parameters:   - (UNIX) platform [linux|solaris]
# return value: - 0 success, 1 failure

DEFAULT_PLATFORM="linux"
PLATFORM=${DEFAULT_PLATFORM}
if [ $# -lt 1 ]; then
 echo "INFO: using default platform: \"${PLATFORM}\""
else
 # parse any arguments
 if [ $# -ge 1 ]; then
  PLATFORM="$1"
 fi
fi
#echo "DEBUG: platform: \"${PLATFORM}\""

DEFAULT_PROJECT_DIRECTORY="$(dirname $(readlink -f $0))/.."
PROJECT_DIRECTORY=${DEFAULT_PROJECT_DIRECTORY}
# sanity check(s)
[ ! -d ${PROJECT_DIRECTORY} ] && echo "ERROR: invalid project directory (was: \"${PROJECT_DIRECTORY}\"), aborting" && exit 1
#echo "DEBUG: project directory: \"${PROJECT_DIRECTORY}\""

SOURCE_DIRECTORY=${PROJECT_DIRECTORY}/3rd_party/ACE_wrappers/ace
[ ! -d ${SOURCE_DIRECTORY} ] && echo "ERROR: invalid source directory (was: \"${SOURCE_DIRECTORY}\"), aborting" && exit 1
ACE_ROOT=/mnt/win_d/projects/ATCD/ACE
[ ! -d ${ACE_ROOT} ] && echo "ERROR: invalid ACE directory (was: \"${ACE_ROOT}\"), aborting" && exit 1
TARGET_DIRECTORY=${ACE_ROOT}/ace
[ ! -d ${TARGET_DIRECTORY} ] && echo "ERROR: invalid target directory (was: \"${TARGET_DIRECTORY}\"), aborting" && exit 1

ORIGINAL_CONFIGURATION_FILE=${SOURCE_DIRECTORY}/config-${PLATFORM}.h
[ ! -f ${ORIGINAL_CONFIGURATION_FILE} ] && echo "ERROR: file not readable (was: \"${ORIGINAL_CONFIGURATION_FILE}\"), aborting" && exit 1
CONFIGURATION_FILE=config.h
cp -f ${ORIGINAL_CONFIGURATION_FILE} ${SOURCE_DIRECTORY}/${CONFIGURATION_FILE} >/dev/null 2>&1
[ $? -ne 0 ] && echo "ERROR: failed to cp \"${ORIGINAL_CONFIGURATION_FILE}\", aborting" && exit 1
mv -f ${SOURCE_DIRECTORY}/${CONFIGURATION_FILE} ${TARGET_DIRECTORY} >/dev/null 2>&1
[ $? -ne 0 ] && echo "ERROR: failed to mv \"${SOURCE_DIRECTORY}/${CONFIGURATION_FILE}\", aborting" && exit 1
echo "processing ${CONFIGURATION_FILE}...DONE"

SOURCE_DIRECTORY=${PROJECT_DIRECTORY}/3rd_party/ACE_wrappers/include/makeinclude
TARGET_DIRECTORY=${ACE_ROOT}/include/makeinclude
[ ! -d ${TARGET_DIRECTORY} ] && echo "ERROR: invalid target directory (was: \"${TARGET_DIRECTORY}\"), aborting" && exit 1

ORIGINAL_MACROS_FILE=${SOURCE_DIRECTORY}/platform_macros_${PLATFORM}.GNU
[ ! -f ${ORIGINAL_CONFIGURATION_FILE} ] && echo "ERROR: file not readable (was: \"${ORIGINAL_CONFIGURATION_FILE}\"), aborting" && exit 1
MACROS_FILE=platform_macros.GNU
cp -f ${ORIGINAL_MACROS_FILE} ${SOURCE_DIRECTORY}/${MACROS_FILE} >/dev/null 2>&1
[ $? -ne 0 ] && echo "ERROR: failed to cp \"${ORIGINAL_MACROS_FILE}\", aborting" && exit 1
mv -f ${SOURCE_DIRECTORY}/${MACROS_FILE} ${TARGET_DIRECTORY} >/dev/null 2>&1
[ $? -ne 0 ] && echo "ERROR: failed to mv \"${SOURCE_DIRECTORY}/${MACROS_FILE}\", aborting" && exit 1
echo "processing ${MACROS_FILE}...DONE"
