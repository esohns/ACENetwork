#!/bin/sh
# //%%%FILE%%%////////////////////////////////////////////////////////////////////
# // File Name: exports.sh
# //
# // History:
# //   Date   |Name | Description of modification
# // ---------|-----|-------------------------------------------------------------
# // 20/02/06 | soh | Creation.
# //%%%FILE%%%////////////////////////////////////////////////////////////////////

# sanity check(s)
command -v basename >/dev/null 2>&1 || { echo "basename is not installed, aborting" >&2; exit 1; }
if [ -z ${ACE_ROOT+x} ]; then echo "ERROR: \"ACE_ROOT\" not set, aborting" && exit 1
command -v perl >/dev/null 2>&1 || { echo "Perl is not installed, aborting" >&2; exit 1; }

# generate exports file
#PERL_SCRIPT=/usr/lib/ace/bin/generate_export_file.pl
PERL_SCRIPT=${ACE_ROOT}/bin/generate_export_file.pl
if [ ! -f ${PERL_SCRIPT} ]; then echo "ERROR: script file \"${PERL_SCRIPT}\" not found, aborting" && exit 1
BASE_DIRECTORY=$(basename $0)
TARGET_DIRECTORY=${BASE_DIRECTORY}/../src
if [ ! -d ${TARGET_DIRECTORY} ]; then echo "ERROR: target directory \"${TARGET_DIRECTORY}\" does not exist, aborting" && exit 1

perl ${PERL_SCRIPT} -n libACENetwork > ${TARGET_DIRECTORY}/net_exports.h
if [ $? -ne 0 ]; then
 echo "ERROR: \"${PERL_SCRIPT}\" failed (status was: $?), aborting"
 exit 1
fi

#generate parser

