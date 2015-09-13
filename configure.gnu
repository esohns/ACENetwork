#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script 'configure's the automake project in the current directory
# *NOTE*: this is neither portable nor particularly stable !
# parameters:   - none
# return value: - 0 success, 1 failure

PWD=$(dirname $0)
CONFIGURE=configure
# sanity check(s)
[ ! -x ${PWD}/${CONFIGURE} ] && echo "ERROR: executable configure script not found (was: \"${PWD}/${CONFIGURE}\"), aborting" && exit 1

${PWD}/${CONFIGURE} $@ --disable-module-support

