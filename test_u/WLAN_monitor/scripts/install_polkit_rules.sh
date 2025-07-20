#!/bin/sh
# author:      Erik Sohns <erik.sohns@posteo.net>
# this script installs required polkit rules
# return value: - 0 success, 1 failure

# sanity checks
command -v exec >/dev/null 2>&1 || { echo "exec is not installed, aborting" >&2; exit 1; }
#command -v gksudo >/dev/null 2>&1 || { echo "gksudo is not installed, aborting" >&2; exit 1; }
command -v sudo >/dev/null 2>&1 || { echo "sudo is not installed, aborting" >&2; exit 1; }

# *NOTE*: regular users may not have the CAP_SETFCAP capability needed to modify
#         (executable) file capabilities --> run as root
# *TODO*: verify this programmatically
HAS_GKSUDO=0
if [ -x gksudo ]; then
 HAS_GKSUDO=1
fi

if [ "${USER}" != "root" ]; then
 SUDO=sudo
 CMDLINE_ARGS="$@"
 if [ ${HAS_GKSUDO} -eq 1 ]; then
  SUDO=gksudo
  CMDLINE_ARGS="--disable-grab $0 $@"
 fi
# echo "invoking sudo $0 \"${CMDLINE_ARGS}\"..."
 exec ${SUDO} $0 ${CMDLINE_ARGS}
fi
#echo "starting..."

# sanity checks
#command -v chgrp >/dev/null 2>&1 || { echo "chgrp is not installed, aborting" >&2; exit 1; }
#command -v chmod >/dev/null 2>&1 || { echo "chmod is not installed, aborting" >&2; exit 1; }
#command -v chown >/dev/null 2>&1 || { echo "chown is not installed, aborting" >&2; exit 1; }
command -v cp >/dev/null 2>&1 || { echo "cp is not installed, aborting" >&2; exit 1; }
command -v cut >/dev/null 2>&1 || { echo "cut is not installed, aborting" >&2; exit 1; }
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
#command -v echo >/dev/null 2>&1 || { echo "echo is not supported, aborting" >&2; exit 1; }
command -v lsb_release >/dev/null 2>&1 || { echo "lsb_release is not installed, aborting" >&2; exit 1; }
command -v readlink >/dev/null 2>&1 || { echo "readlink is not installed, aborting" >&2; exit 1; }
#command -v test >/dev/null 2>&1 || { echo "test is not installed, aborting" >&2; exit 1; }

DEFAULT_PROJECT_DIR="$(dirname $(readlink -f $0))/../../../.."
PROJECT_DIR=${DEFAULT_PROJECT_DIR}
[ ! -d ${PROJECT_DIR} ] && echo "ERROR: invalid project directory (was: \"${PROJECT_DIR}\"), aborting" && exit 1

DISTRIBUTION="$(lsb_release -i | cut -f 2)"
#[ ${DISTRIBUTION} != "ArchLinux" -a ${DISTRIBUTION} != "Ubuntu" ] && echo "invalid/unknown linux distribution (was: \"${DISTRIBUTION}\"), aborting" && exit 1
case "${DISTRIBUTION}" in
# ArchLinux)
#  POLKIT_RULES="${PROJECT_DIR}/ACENetwork/test_u/WLAN_monitor/etc/10-wlan_management.rules" # archlinux
#  ETC_POLKIT_RULES_DIR="/etc/polkit-1/rules.d" # archlinux
#  ;;
 Ubuntu)
  LA_RULES_FILE="${PROJECT_DIR}/ACENetwork/test_u/WLAN_monitor/etc/10-wlan_management.pkla"
  RULES_FILE="${PROJECT_DIR}/ACENetwork/test_u/WLAN_monitor/etc/10-wlan_management.rules"
  ETC_RULES_DIRECTORY="/etc/polkit-1/localauthority/50-local.d" # ubuntu (artful)
  ;;
 *)
  echo "invalid/unknown linux distribution (was: \"${DISTRIBUTION}\"), aborting"
  exit 1
  ;;
esac
USR_SHARE_RULES_DIRECTORY=/usr/share/polkit-1/rules.d

# sanity check(s)
[ ! -r ${LA_RULES_FILE} ] && echo "ERROR: invalid polkit localauthority rules file (was: \"${LA_RULES_FILE}\"), aborting" && exit 1
[ ! -r ${RULES_FILE} ] && echo "ERROR: invalid polkit rules file (was: \"${RULES_FILE}\"), aborting" && exit 1
[ ! -d "${ETC_RULES_DIRECTORY}" ] && echo "ERROR: invalid /etc polkit rules directory (was: \"${ETC_RULES_DIRECTORY}\"), aborting" && exit 1
[ ! -d "${USR_SHARE_RULES_DIRECTORY}" ] && echo "ERROR: invalid /usr/share polkit rules directory (was: \"${USR_SHARE_RULES_DIRECTORY}\"), aborting" && exit 1

cp -f ${LA_RULES_FILE} ${ETC_RULES_DIRECTORY}
[ $? -ne 0 ] && echo "ERROR: failed to copy \"$(basename ${LA_RULES_FILE})\" to \"${ETC_RULES_DIRECTORY}\": $?, aborting" && exit 1
echo "copied \"$(basename ${LA_RULES_FILE})\" to \"${ETC_RULES_DIRECTORY}\"..."
cp -f ${RULES_FILE} ${USR_SHARE_RULES_DIRECTORY}
[ $? -ne 0 ] && echo "ERROR: failed to copy \"$(basename ${RULES_FILE})\" to \"${USR_SHARE_RULES_DIRECTORY}\": $?, aborting" && exit 1
echo "copied \"$(basename ${RULES_FILE})\" to \"${USR_SHARE_RULES_DIRECTORY}\"..."

#chown --quiet root:root ${BIN}
#[ $? -ne 0 ] && echo "ERROR: failed to chown ${BIN}: \"$?\", aborting" && exit 1
#chgrp --quiet root ${BIN}
#[ $? -ne 0 ] && echo "ERROR: failed to chgrp ${BIN}: \"$?\", aborting" && exit 1
#chmod --quiet +s ${BIN_TMP}
#chmod --quiet ug+s ${BIN}
#[ $? -ne 0 ] && echo "ERROR: failed to chmod u+s ${BIN}: \"$?\", aborting" && exit 1
#echo "modified \"$BINS\": suid sgid root"
