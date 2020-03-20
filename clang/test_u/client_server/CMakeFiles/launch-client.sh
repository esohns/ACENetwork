#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACENetwork/clang/../test_u/client_server
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACENetwork/modules/ACE/lib:/mnt/win_d/projects/ACENetwork/../Common/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/../Common/cmake/src/ui/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/modules/dev/Debug:/usr/local/lib::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACENetwork/clang/test_u/client_server/client 
	else
		"/mnt/win_d/projects/ACENetwork/clang/test_u/client_server/client" -l -t 
	fi
else
	"/mnt/win_d/projects/ACENetwork/clang/test_u/client_server/client" -l -t 
fi
