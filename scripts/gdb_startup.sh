#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script initializes GDB
# parameters:   - none
# return value: - 0 success, 1 failure

# signal handling
handle SIGINT pass nostop print
handle SIGPIPE pass nostop print
handle SIGUSR1 pass nostop print
handle SIGUSR2 pass nostop print
handle SIG34 pass nostop print
