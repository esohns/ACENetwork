#!/bin/sh
#//%%%FILE%%%////////////////////////////////////////////////////////////////////
#// File Name: antlr.sh
#//
#// History:
#//   Date   |Name | Description of modification
#// ---------|-----|-------------------------------------------------------------
#// 20/02/06 | soh | Creation.
#//%%%FILE%%%////////////////////////////////////////////////////////////////////

# sanity check(s)
command -v dirname >/dev/null 2>&1 || { echo "dirname is not installed, aborting" >&2; exit 1; }
command -v java >/dev/null 2>&1 || { echo "java is not installed, aborting" >&2; exit 1; }
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

if [ ${HAS_READLINK} -ne 0 ]; then
 PROJECT_ROOT=$(readlink -e $(dirname $0)/../../../..)
else
 PROJECT_ROOT=$(cd ${0%/*}/../../../.. && echo $PWD)
fi
SCRIPTS_DIRECTORY=${PROJECT_ROOT}/src/protocol/http/scripts

SCANNER_SOURCE_FILE=${SCRIPTS_DIRECTORY}/http_antlr_scanner.g4
[ ! -f ${SCANNER_SOURCE_FILE} ] && echo "ERROR: file ${SCANNER_SOURCE_FILE} not found, aborting" && exit 1
PARSER_SOURCE_FILE=${SCRIPTS_DIRECTORY}/http_antlr_parser.g4
[ ! -f ${PARSER_SOURCE_FILE} ] && echo "ERROR: file ${PARSER_SOURCE_FILE} not found, aborting" && exit 1
LOCATION=${LIB_ROOT}/antlr/tool/antlr-4.13.2-complete.jar
[ ! -f ${LOCATION} ] && echo "ERROR: file ${LOCATION} not found, aborting" && exit 1
#java -jar ${LOCATION} -Dlanguage=Cpp -listener -visitor -o generated/ -package antlrcpptest ${SCANNER_SOURCE_FILE} ${PARSER_SOURCE_FILE}
java -jar ${LOCATION} -Dlanguage=Cpp -o generated/ ${SCANNER_SOURCE_FILE} ${PARSER_SOURCE_FILE}
[ $? -ne 0 ] && echo "ERROR: \"java ${LOCATION}\" failed (status was: $?), aborting" && exit 1

#DOT_FILE=${SCRIPTS_DIRECTORY}/http_parser.dot
#[ ! -f ${DOT_FILE} ] && echo "ERROR: file ${DOT_FILE} not found, aborting" && exit 1
#IMAGE_FILE=http_parser.png
#if [ ${HAS_GRAPHVIZ} -ne 0 ]; then
# dot -Tpng ${DOT_FILE} >${IMAGE_FILE}
# [ $? -ne 0 ] && echo "ERROR: \"${DOT_FILE}\" failed (status was: $?), aborting" && exit 1
#fi
# *TODO*: xsltproc /usr/local/share/bison/xslt/xml2xhtml.xsl gr.xml >gr.html

# move generated file(s) into the project directory
# --> these files are static (*CHECK*) and included by default
EXCLUDE_PATTERNS='
*.tokens
*.interp
'
GENERATED_DIRECTORY="generated"
FILES="http_antlr_parser.h http_antlr_parser.cpp \
http_antlr_parserBaseListener.h http_antlr_parserBaseListener.cpp \
http_antlr_parserListener.h http_antlr_parserListener.cpp \
http_antlr_scanner.h http_antlr_scanner.cpp"
for i in $FILES; do
 eval "FILE=./${GENERATED_DIRECTORY}/$i"
# echo -e "$FILE"
 if [ -f $FILE ] ; then
  mv -f $FILE ${SCRIPTS_DIRECTORY}/..
  if [ $? -ne 0 ]; then
   echo "ERROR: failed to mv \"$l\", aborting"
   exit 1
  fi
  echo "moved \"$FILE\"..."
 fi
done
