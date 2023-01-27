#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script initializes the project
# *NOTE*: it is neither portable nor particularly stable !
# parameters:   - N/A
# return value: - 0 success, 1 failure

# sanity checks
command -v basename >/dev/null 2>&1 || { echo "basename is not installed, aborting" >&2; exit 1; }
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v readlink >/dev/null 2>&1 || { echo "readlink is not installed, aborting" >&2; exit 1; }

# ACE
# INIT_ACE="$(dirname $(readlink -f $0))/../../Common/scripts/initialize_ACE.sh"
# [ ! -x ${INIT_ACE} ] && echo "ERROR: invalid ACE initialization script (was: \"${INIT_ACE}\"), aborting" && exit 1
# ${INIT_ACE} linux
# [ $? -ne 0 ] && echo "ERROR: failed to execute \"${INIT_ACE}\": $?, aborting" && exit 1 
# echo "processed \"$(basename ${INIT_ACE})\"..."

