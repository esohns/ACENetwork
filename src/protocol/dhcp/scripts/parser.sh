#!/bin/sh
#//%%%FILE%%%////////////////////////////////////////////////////////////////////
#// File Name: parser.sh
#//
#// History:
#//   Date   |Name | Description of modification
#// ---------|-----|-------------------------------------------------------------
#// 20/02/06 | soh | Creation.
#//%%%FILE%%%////////////////////////////////////////////////////////////////////

# sanity check(s)
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v bison >/dev/null 2>&1 || { echo "bison is not installed, aborting" >&2; exit 1; }
command -v dot >/dev/null 2>&1 || { echo "graphviz is not installed, aborting" >&2; exit 1; }

PROJECT_ROOT=$(dirname $0)/../../../..
SCRIPTS_DIRECTORY=${PROJECT_ROOT}/src/protocol/dhcp/scripts

SOURCE_FILE=${SCRIPTS_DIRECTORY}/parser.y
[ ! -f ${SOURCE_FILE} ] && echo "ERROR: file ${SOURCE_FILE} not found, aborting" && exit 1
bison --feature=caret --graph --report=all --report-file=parser_report.txt --xml --warnings=all ${SOURCE_FILE}
[ $? -ne 0 ] && echo "ERROR: \"${SOURCE_FILE}\" failed (status was: $?), aborting" && exit 1
DOT_FILE=${SCRIPTS_DIRECTORY}/dhcp_parser.dot
[ ! -f ${DOT_FILE} ] && echo "ERROR: file ${DOT_FILE} not found, aborting" && exit 1
IMAGE_FILE=dhcp_parser.png
dot -Tpng ${DOT_FILE} >${IMAGE_FILE}
[ $? -ne 0 ] && echo "ERROR: \"${DOT_FILE}\" failed (status was: $?), aborting" && exit 1
# *TODO*: xsltproc /usr/local/share/bison/xslt/xml2xhtml.xsl gr.xml >gr.html

# move generated file(s) into the project directory
#mv -f position.hh ./..
#mv -f stack.hh ./..
#mv -f location.hh ./..
# --> these files are static (*CHECK*) and included by default
#mv -f dhcp_parser.h/.cpp ./..
# *NOTE*: a specific method needs to be added to the parser class
# --> copy a pre-patched version (back) into the project directory instead
# *TODO*: needs to be updated after every change
#TARGET_DIRECTORY=${PROJECT_ROOT}
#TARGET_FILE=${TARGET_DIRECTORY}/dhcp_parser.h
#[ ! -f ${SOURCE_FILE} ] && echo "ERROR: file ${SOURCE_FILE} not found, aborting" && exit 1
#cp -f ${SOURCE_FILE} ${TARGET_FILE}
#[ $? -ne 0 ] && echo "ERROR: failed to cp \"${SOURCE_FILE}\", aborting" && exit 1
#echo "copied \"$SOURCE_FILE\"..."
# clean up
#SOURCE_FILE=dhcp_parser.h
#rm -f ${SOURCE_FILE}
#[ $? -ne 0 ] && echo "ERROR: failed to rm \"${SOURCE_FILE}\", aborting" && exit 1

FILES="dhcp_parser.h dhcp_parser.cpp"
#location.hh
#position.hh
#stack.hh"
# move the files into the project directory
for FILE in $FILES
do
 mv -f $FILE ${SCRIPTS_DIRECTORY}/..
 if [ $? -ne 0 ]; then
  echo "ERROR: failed to mv \"$FILE\", aborting"
  exit 1
 fi
 echo "moved \"$FILE\"..."
done
