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
command -v bison >/dev/null 2>&1 || { echo "bison is not installed, aborting" >&2; exit 1; }
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
HAS_GRAPHVIZ=0
command -v dot -V >/dev/null 2>&1
if [ $? -eq 0 ]; then
 HAS_GRAPHVIZ=1
else
 echo "graphviz is not installed, continuing" >&2
fi
command -v readlink >/dev/null 2>&1 || { echo "readlink is not installed, aborting" >&2; exit 1; }
command -v sed >/dev/null 2>&1 || { echo "sed is not installed, aborting" >&2; exit 1; }

PROJECT_ROOT=$(readlink -e $(dirname $0)/../../../../..)
SCRIPTS_DIRECTORY=${PROJECT_ROOT}/src/protocol/p2p/bittorrent/scripts

SOURCE_FILES="${SCRIPTS_DIRECTORY}/metainfo_parser.yy ${SCRIPTS_DIRECTORY}/parser.y"
for FILE in ${SOURCE_FILES}
do
 if [ ! -f ${FILE} ]; then
  echo "ERROR: file ${FILE} not found, aborting"
  exit 1
 fi
 REPORT_FILE=$(echo ${FILE} | sed -e 's/.[y]*y$/_report.txt/')
 bison --graph --report=all --report-file=${REPORT_FILE} --xml --warnings=all ${FILE}
#bison --feature=caret --graph --report=all --report-file=parser_report.txt --xml --warnings=all ${FILE}
 [ $? -ne 0 ] && echo "ERROR: \"${FILE}\" failed (status was: $?), aborting" && exit 1
 DOT_FILE=$(echo ${FILE} | sed -e 's/.[y]*y$/.dot/')
 DOT_FILE="$(dirname ${DOT_FILE})/bittorrent_$(basename ${DOT_FILE})"
 [ ! -f ${DOT_FILE} ] && echo "ERROR: file ${DOT_FILE} not found, aborting" && exit 1
 IMAGE_FILE=$(echo ${FILE} | sed -e 's/.[y]*y$/.png/')
 if [ ${HAS_GRAPHVIZ} -ne 0 ]; then
  dot -Tpng ${DOT_FILE} >${IMAGE_FILE}
  if [ $? -ne 0 ]; then
   echo "ERROR: \"${DOT_FILE}\" failed (status was: $?), aborting"
   exit 1
  fi
 fi
# *TODO*: xsltproc /usr/local/share/bison/xslt/xml2xhtml.xsl gr.xml >gr.html
done

# --> these files are static (*CHECK*) and included by default
#mv -f parser.h/.cpp ./..
# *NOTE*: a specific method needs to be added to the parser class
# --> copy a pre-patched version (back) into the project directory instead
# *TODO*: needs to be updated after every change
#TARGET_DIRECTORY=${PROJECT_ROOT}
#TARGET_FILE=${TARGET_DIRECTORY}/http_parser.h
#[ ! -f ${SOURCE_FILE} ] && echo "ERROR: file ${SOURCE_FILE} not found, aborting" && exit 1
#cp -f ${SOURCE_FILE} ${TARGET_FILE}
#[ $? -ne 0 ] && echo "ERROR: failed to cp \"${SOURCE_FILE}\", aborting" && exit 1
#echo "copied \"$SOURCE_FILE\"..."
# clean up
#SOURCE_FILE=http_parser.h
#rm -f ${SOURCE_FILE}
#[ $? -ne 0 ] && echo "ERROR: failed to rm \"${SOURCE_FILE}\", aborting" && exit 1

FILES="bittorrent_metainfo_parser.h bittorrent_metainfo_parser.cpp
bittorrent_parser.h bittorrent_parser.cpp
location.hh position.hh stack.hh"
# move the files into the project directory
for FILE in ${FILES}
do
 mv -f ${FILE} ${SCRIPTS_DIRECTORY}/..
 if [ $? -ne 0 ]; then
  echo "ERROR: failed to mv \"${FILE}\", aborting"
  exit 1
 fi
 echo "moved \"${FILE}\"..."
done

