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
# *NOTE*: readlink is not available on MinGW
HAS_READLINK=0
command -v readlink >/dev/null 2>&1
if [ $? -eq 0 ]; then
 HAS_READLINK=1
else
 echo "readlink is not installed, continuing" >&2
fi
command -v sed >/dev/null 2>&1 || { echo "sed is not installed, aborting" >&2; exit 1; }

if [ ${HAS_READLINK} -ne 0 ]; then
 PROJECT_ROOT=$(readlink -e $(dirname $0)/../../../../..)
else
 PROJECT_ROOT=$(cd ${0%/*}/../../../../.. && echo $PWD)
fi
SCRIPTS_DIRECTORY=${PROJECT_ROOT}/src/protocol/p2p/bittorrent/scripts

SOURCE_FILES="${SCRIPTS_DIRECTORY}/bencoding_parser.yy ${SCRIPTS_DIRECTORY}/parser.yy"
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
 #[ ! -f ${DOT_FILE} ] && echo "ERROR: file ${DOT_FILE} not found, aborting" && exit 1
 #IMAGE_FILE=$(echo ${FILE} | sed -e 's/.[y]*y$/.png/')
 #if [ ${HAS_GRAPHVIZ} -ne 0 ]; then
 # dot -Tpng ${DOT_FILE} >${IMAGE_FILE}
 # if [ $? -ne 0 ]; then
 #  echo "ERROR: \"${DOT_FILE}\" failed (status was: $?), aborting"
 #  exit 1
 # fi
 #fi
# *TODO*: xsltproc /usr/local/share/bison/xslt/xml2xhtml.xsl gr.xml >gr.html
done

# move generated file(s) into the project directory
# --> these files are static (*CHECK*) and included by default
#mv -f HTTP_parser.h/.cpp ./..
# *NOTE*: a specific method needs to be added to the parser class
# --> copy a pre-patched version (back) into the project directory instead
# *TODO*: needs to be updated after every change
TARGET_DIRECTORY=${SCRIPTS_DIRECTORY}/..
SOURCE_FILE=${SCRIPTS_DIRECTORY}/bittorrent_parser_patched.h
TARGET_FILE=${TARGET_DIRECTORY}/bittorrent_parser.h
[ ! -f ${SOURCE_FILE} ] && echo "ERROR: file ${SOURCE_FILE} not found, aborting" && exit 1
cp -f ${SOURCE_FILE} ${TARGET_FILE}
[ $? -ne 0 ] && echo "ERROR: failed to cp \"${SOURCE_FILE}\", aborting" && exit 1
echo "copied \"$SOURCE_FILE\"..."
# clean up
SOURCE_FILE=bittorrent_parser.h
#rm -f ${SOURCE_FILE}
#[ $? -ne 0 ] && echo "ERROR: failed to rm \"${SOURCE_FILE}\", aborting" && exit 1

FILES="bittorrent_bencoding_parser.h bittorrent_bencoding_parser.cpp
bittorrent_parser.cpp"
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

# move generated file(s) into the project directory
# --> these files are static (*CHECK*) and included by default
TARGET_DIRECTORY=${PROJECT_ROOT}/3rd_party/bison
FILES="location.hh position.hh stack.hh"
# move the files into the 3rd_party include directory
for FILE in $FILES
do
 mv -f $FILE ${TARGET_DIRECTORY}
 if [ $? -ne 0 ]; then
  echo "ERROR: failed to mv \"$FILE\", aborting"
  exit 1
 fi
 echo "moved \"$FILE\"..."
done
