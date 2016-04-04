#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script set proper file capabilities
# debugging
# *NOTE*: it is neither portable nor particularly stable !
# parameters:   - $1 [BUILD] {"debug" || "debug_tracing" || "release" || ...}
# return value: - 0 success, 1 failure

# sanity checks
command -v gksudo >/dev/null 2>&1 || { echo "gksudo is not installed, aborting" >&2; exit 1; }

[ "root" != "$USER" ] && exec gksudo $0 "$@"

echo "starting..."

# sanity checks
command -v chgrp >/dev/null 2>&1 || { echo "chgrp is not installed, aborting" >&2; exit 1; }
command -v chmod >/dev/null 2>&1 || { echo "chmod is not installed, aborting" >&2; exit 1; }
command -v chown >/dev/null 2>&1 || { echo "chown is not installed, aborting" >&2; exit 1; }
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v echo >/dev/null 2>&1 || { echo "echo is not supported, aborting" >&2; exit 1; }
command -v readlink >/dev/null 2>&1 || { echo "readlink is not installed, aborting" >&2; exit 1; }
command -v /sbin/setcap >/dev/null 2>&1 || { echo "setcap is not installed, aborting" >&2; exit 1; }

DEFAULT_PROJECT_DIR="$(dirname $(readlink -f $0))/.."
PROJECT_DIR=${DEFAULT_PROJECT_DIR}
# sanity check(s)
[ ! -d ${PROJECT_DIR} ] && echo "ERROR: invalid project dir (was: \"${PROJECT_DIR}\"), aborting" && exit 1

DEFAULT_BUILD="Debug"
BUILD=${DEFAULT_BUILD}
if [ $# -lt 1 ]; then
 echo "INFO: using default build: \"${BUILD}\""
else
 # parse any arguments
 if [ $# -ge 1 ]; then
  BUILD="$1"
 fi
fi
DEFAULT_TMP_DIR="${TEMP}"
TMP_DIR="$DEFAULT_TMP_DIR"
if [ -z "${TMP_DIR}" ]; then
 TMP_DIR="/tmp"
fi
[ ! -d ${TMP_DIR} ] && echo "ERROR: invalid tmp dir (was: \"${TMP_DIR}\"), aborting" && exit 1
echo "using tmp directory \"$TMP_DIR\"..."

# sanity check(s)
[ ${BUILD} != "Debug" -a ${BUILD} != "debug_tracing" -a ${BUILD} != "release" ] && echo "WARNING: invalid/unknown build (was: \"${BUILD}\"), continuing"
BUILD_DIR="${PROJECT_DIR}/cmake"
[ ! -d "${BUILD_DIR}" ] && echo "ERROR: invalid build dir (was: \"${BUILD_DIR}\"), aborting" && exit 1

TEST_U_DIR="test_u"
SUB_DIRS="DHCP_client"
#declare -a LIBS=("libACE.so")
BINS="DHCP_client"
i=0
for DIR in $SUB_DIRS
do
# LIB="${MODULES_DIR}/${DIR}/${LIB_DIR}/${LIBS[$i]}"
 BIN="${BUILD_DIR}/${TEST_U_DIR}/${DIR}/${BINS}"
 [ ! -r "${BIN}" ] && echo "ERROR: invalid binary file (was: \"${BIN}\"), aborting" && exit 1

# cp -f ${BIN} ${TMP_DIR}
# [ $? -ne 0 ] && echo "ERROR: failed to cp ${BIN}: \"$?\", aborting" && exit 1
# echo "copied \"$BIN\"..."

# BIN_TMP="${TMP_DIR}/${BINS}"
# [ ! -r "${BIN_TMP}" ] && echo "ERROR: invalid binary file (was: \"${BIN_TMP}\"), aborting" && exit 1

# chown --quiet root ${BIN_TMP}
 chown --quiet root ${BIN}
 [ $? -ne 0 ] && echo "ERROR: failed to chown ${BIN}: \"$?\", aborting" && exit 1
 chgrp --quiet root ${BIN}
 [ $? -ne 0 ] && echo "ERROR: failed to chgrp ${BIN}: \"$?\", aborting" && exit 1
# chmod --quiet +s ${BIN_TMP}
 chmod --quiet +s ${BIN}
 [ $? -ne 0 ] && echo "ERROR: failed to chmod +s ${BIN}: \"$?\", aborting" && exit 1

# /sbin/setcap 'cap_net_bind_service=eip' ${BIN_TMP}
 /sbin/setcap 'cap_net_bind_service=eip' ${BIN}
 [ $? -ne 0 ] && echo "ERROR: failed to /sbin/setcap ${BIN_TMP}: \"$?\", aborting" && exit 1

 echo "modified \"$BINS\"..."

 i=$i+1
done
