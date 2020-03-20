#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACENetwork/clang/../test_u/file_server
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACENetwork/modules/ATCD/ACE/lib:/mnt/win_d/projects/ACENetwork/../gtk/gtk/Win32/debug:/mnt/win_d/projects/ACENetwork/../Common/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/../Common/cmake/src/ui/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/modules/dev/Debug:/usr/local/lib::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -f /mnt/win_d/temp/H264_720p.ts -g -l -p 5555 -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACENetwork/clang/test_u/file_server/file_server 
	else
		"/mnt/win_d/projects/ACENetwork/clang/test_u/file_server/file_server" -f /mnt/win_d/temp/H264_720p.ts -g -l -p 5555 -t 
	fi
else
	"/mnt/win_d/projects/ACENetwork/clang/test_u/file_server/file_server" -f /mnt/win_d/temp/H264_720p.ts -g -l -p 5555 -t 
fi
