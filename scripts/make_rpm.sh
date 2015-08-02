#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script creates a distribution RPM
# *TODO*: it is neither portable nor particularly stable !
# parameters:   - $1 ["debug"] || "release"
#               - $2 ["tarball"] || "binary" || "source" || "all"
#               - $3 [""] (target platform: default: (build) host platform)
# return value: - 0 success, 1 failure

# sanity checks
command -v basename >/dev/null 2>&1 || { echo "basename is not installed, aborting" >&2; exit 1; }
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v readlink >/dev/null 2>&1 || { echo "readlink is not installed, aborting" >&2; exit 1; }
command -v make >/dev/null 2>&1 || { echo "make is not installed, aborting" >&2; exit 1; }
command -v which >/dev/null 2>&1 || { echo "which is not installed, aborting" >&2; exit 1; }
command -v uname >/dev/null 2>&1 || { echo "uname is not installed, aborting" >&2; exit 1; }
command -v rpmbuild >/dev/null 2>&1 || { echo "rpmbuild is not installed, aborting" >&2; exit 1; }
RPMBUILD_EXEC=$(which rpmbuild)
[ ! -x ${RPMBUILD_EXEC} ] && echo "ERROR: invalid \"rpmbuild\" executable \"${RPMBUILD_EXEC}\" (not executable), aborting" && exit 1

DEFAULT_PROJECT_DIR="$(dirname $(readlink -f $0))/.."
PROJECT_DIR=${DEFAULT_PROJECT_DIR}
[ ! -d ${PROJECT_DIR} ] && echo "ERROR: invalid project dir (was: \"${PROJECT_DIR}\"), aborting" && exit 1
TARGET_DIR=${PROJECT_DIR}/releases
[ ! -d ${TARGET_DIR} ] && echo "ERROR: invalid target dir (was: \"${TARGET_DIR}\"), aborting" && exit 1

DEFAULT_BUILD="release"
BUILD=${DEFAULT_BUILD}
[ $# -lt 1 ] && echo "using default build: \"${BUILD}\""

DEFAULT_TYPE="tarball"
TYPE=${DEFAULT_TYPE}
[ $# -lt 2 ] && echo "using default type: \"${TYPE}\""

DEFAULT_PLATFORM=""
PLATFORM=${DEFAULT_PLATFORM}
[ $# -lt 3 ] && echo "using default (=host) platform"

# parse any arguments
[ $# -ge 1 ] && BUILD=$1
[ "${BUILD}" != "debug" -a "${BUILD}" != "release" ] && echo "ERROR: invalid build (was: \"${BUILD}\"), aborting" && exit 1
[ $# -ge 2 ] && TYPE=$2
[ "${TYPE}" != "tarball" -a "${TYPE}" != "binary" -a "${TYPE}" != "source" -a "${TYPE}" != "all" ] && echo "ERROR: invalid rpm type (was: \"${TYPE}\"), aborting" && exit 1
[ $# -ge 3 ] && PLATFORM=$3
[ "${PLATFORM}" != "" -a "${PLATFORM}" != "x86_64" -a "${PLATFORM}" != "i686" -a "${PLATFORM}" != "i386" ] && echo "ERROR: invalid platform (was: \"${PLATFORM}\"), aborting" && exit 1

# set build mode arguments for "rpmbuild"
PARAMETERS="-ta"
[ "${TYPE}" = "binary" ] && PARAMETERS="-bb"
[ "${TYPE}" = "source" ] && PARAMETERS="-bs"
[ "${TYPE}" = "all" ] && PARAMETERS="-ba"

# set target platform arguments for "rpmbuild"
TARGET=""
if [ "${PLATFORM}" != "" ]; then
  TARGET="--target ${PLATFORM}"
else
  PLATFORM=$(uname -m)
fi

# remember current dir...
pushd . >/dev/null 2>&1

# make distribution tarball
BUILD_DIR=${PROJECT_DIR}/build/${BUILD}
# sanity check(s)
[ ! -d ${BUILD_DIR} ] && echo "ERROR: invalid build dir (was: \"${BUILD_DIR}\"), aborting" && exit 1
cd ${BUILD_DIR}
echo "INFO: making distribution tarball..."
RETVAL=$(make dist >/dev/null 2>&1)
[ $? -ne 0 ] && echo "ERROR: failed to generate distribution tarball, aborting" && exit 1
for TARBALL in *.tar.gz; do
  echo "INFO: created \"${TARBALL}\"..."
#*NOTE*: rename the tarball to lower-case - necessary for rpmbuild :(, see below
#  LOWER_TARBALL=$(echo ${TARBALL} | tr [:upper:] [:lower:])
#  mv ${TARBALL} ${LOWER_TARBALL}
#  echo "INFO: created \"${LOWER_TARBALL}\"..."
#  TARBALL=${LOWER_TARBALL}
  if [ "${BUILD}" = "release" ]; then
    cp -f ${TARBALL} ${TARGET_DIR}
    [ $? -ne 0 ] && echo "ERROR: failed to copy distribution tarball, aborting" && exit 1
    echo "INFO: copied \"${TARBALL}\" to ${TARGET_DIR}..."
  fi
  if [ "${TYPE}" != "tarball" ]; then
    cp -f ${TARBALL} ${HOME}/rpmbuild/SOURCES
    [ $? -ne 0 ] && echo "ERROR: failed to copy distribution tarball, aborting" && exit 1
    echo "INFO: copied \"${TARBALL}\" to ${HOME}/rpmbuild/SOURCES..."
  fi
done
echo "INFO: making distribution tarball...DONE"

# make rpm
SPECFILE=${BUILD_DIR}/scripts/libACENetwork.spec
[ ! -f ${SPECFILE} ] && echo "ERROR: invalid .spec file \"${SPECFILE}\" (not a file), aborting" && exit 1
if [ "${TYPE}" != "tarball" ]; then
  CMDLINE="${RPMBUILD_EXEC} ${PARAMETERS} ${TARGET} ${SPECFILE}"
else
  CMDLINE="${RPMBUILD_EXEC} ${PARAMETERS} ${TARBALL}"
fi
echo "INFO: making rpm (${TYPE})..."
RETVAL=$(${CMDLINE})
#RETVAL=$(${CMDLINE} >/dev/null 2>&1)
[ $? -ne 0 ] && echo "ERROR: failed to make rpm, aborting" && exit 1
for RPM in ${HOME}/rpmbuild/RPMS/${PLATFORM}/*.rpm; do
  echo "INFO: created \"`basename ${RPM}`\"..."
  cp -f ${RPM} ${TARGET_DIR}
  [ $? -ne 0 ] && echo "ERROR: failed to cp rpm, aborting" && exit 1
  echo "INFO: copied \"${RPM}\" to ${TARGET_DIR}..."
done
echo "INFO: making rpm (${TYPE})...DONE"

# clean up
echo "INFO: cleaning up ..."
if [ "${TYPE}" != "tarball" ]; then
  CMDLINE="${RPMBUILD_EXEC} --rmsource --rmspec ${SPECFILE}"
  RETVAL=$(${CMDLINE} >/dev/null 2>&1)
  [ $? -ne 0 ] && echo "ERROR: failed to clean up source, aborting" && exit 1
else
  rm -f ${TARBALL}
  [ $? -ne 0 ] && echo "ERROR: failed to clean up tarball, aborting" && exit 1
fi
echo "INFO: cleaning up ...DONE"

# ...go back
popd >/dev/null 2>&1

