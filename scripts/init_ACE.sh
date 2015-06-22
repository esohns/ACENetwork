#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script copies config.h of the ACE framework to the source directory.
# --> allows reuse of the same ACE source tree on different platforms.
# *NOTE*: it is neither portable nor particularly stable !
# parameters:   - N/A
# return value: - 0 success, 1 failure

DEFAULT_PROJECT_DIR="$(dirname $(readlink -f $0))/.."
PROJECT_DIR=${DEFAULT_PROJECT_DIR}
# sanity check(s)
[ ! -d ${PROJECT_DIR} ] && echo "ERROR: invalid project directory (was: \"${PROJECT_DIR}\"), aborting" && exit 1

SOURCE_DIRECTORY=${PROJECT_DIR}/3rd_party/ACE_wrappers/ace
[ ! -d ${SOURCE_DIRECTORY} ] && echo "ERROR: invalid source directory (was: \"${SOURCE_DIRECTORY}\"), aborting" && exit 1
ACE_ROOT=/mnt/win_d/projects/ATCD/ACE
[ ! -d ${ACE_ROOT} ] && echo "ERROR: invalid ACE directory (was: \"${ACE_ROOT}\"), aborting" && exit 1
TARGET_DIRECTORY=${ACE_ROOT}/ace
[ ! -d ${TARGET_DIRECTORY} ] && echo "ERROR: invalid target directory (was: \"${TARGET_DIRECTORY}\"), aborting" && exit 1

ORIGINAL_CONFIG_FILE=${SOURCE_DIRECTORY}/config-linux.h
[ ! -f ${ORIGINAL_CONFIG_FILE} ] && echo "ERROR: file not readable (was: \"${ORIGINAL_CONFIG_FILE}\"), aborting" && exit 1
CONFIG_FILE=config.h
cp -f ${ORIGINAL_CONFIG_FILE} ${SOURCE_DIRECTORY}/${CONFIG_FILE} >/dev/null 2>&1
[ $? -ne 0 ] && echo "ERROR: failed to cp \"${ORIGINAL_CONFIG_FILE}\", aborting" && exit 1
mv -f ${SOURCE_DIRECTORY}/${CONFIG_FILE} ${TARGET_DIRECTORY} >/dev/null 2>&1
[ $? -ne 0 ] && echo "ERROR: failed to mv \"${SOURCE_DIRECTORY}/${CONFIG_FILE}\", aborting" && exit 1

echo "processing ${CONFIG_FILE}...DONE"

