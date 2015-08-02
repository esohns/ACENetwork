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

PROJECT_ROOT=$(dirname $0)/..
SCRIPTS_DIRECTORY=${PROJECT_ROOT}/scripts

SOURCE_FILE=${SCRIPTS_DIRECTORY}/IRCparser.y
[ ! -f ${SOURCE_FILE} ] && echo "ERROR: file ${SOURCE_FILE} not found, aborting" && exit 1
#bison --verbose --graph=parser_graph.txt --xml=parser_graph.xml ${SOURCE_FILE} --report=all --report-file=parser_report.txt --warnings=all --warnings=no-precedence
bison --verbose --graph=parser_graph.txt --xml=parser_graph.xml ${SOURCE_FILE} --report=all --report-file=parser_report.txt --warnings=all
[ $? -ne 0 ] && echo "ERROR: \"${SOURCE_FILE}\" failed (status was: $?), aborting" && exit 1

# move generated file(s) into the project directory
#mv -f position.hh ./..
#mv -f stack.hh ./..
#mv -f location.hh ./..
# --> these files are static (*CHECK*) and included by default
#mv -f IRC_client_IRCparser.h/.cpp ./..
# *NOTE*: a specific method needs to be added to the parser class
# --> copy a pre-patched version (back) into the project directory instead
# *TODO*: needs to be updated after every change
TARGET_DIRECTORY=${PROJECT_ROOT}
TARGET_FILE=${TARGET_DIRECTORY}/IRC_client_IRCparser.h
SOURCE_FILE=${SCRIPTS_DIRECTORY}/IRC_client_IRCparser_patched.h
[ ! -f ${SOURCE_FILE} ] && echo "ERROR: file ${SOURCE_FILE} not found, aborting" && exit 1
cp -f ${SOURCE_FILE} ${TARGET_FILE}
[ $? -ne 0 ] && echo "ERROR: failed to cp \"${SOURCE_FILE}\", aborting" && exit 1
echo "copied \"$SOURCE_FILE\"..."
# clean up
SOURCE_FILE=IRC_client_IRCparser.h
rm -f ${SOURCE_FILE}
[ $? -ne 0 ] && echo "ERROR: failed to rm \"${SOURCE_FILE}\", aborting" && exit 1

FILES="IRC_client_IRCparser.cpp"
#location.hh
#position.hh
#stack.hh"
# move the files into the project directory
for FILE in $FILES
do
 mv -f $FILE ${PROJECT_ROOT}
 if [ $? -ne 0 ]; then
  echo "ERROR: failed to mv \"$FILE\", aborting"
  exit 1
 fi
 echo "moved \"$FILE\"..."
done

