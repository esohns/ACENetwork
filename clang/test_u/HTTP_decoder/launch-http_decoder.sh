#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACENetwork/clang/../test_u/HTTP_decoder
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACENetwork/modules/ATCD/ACE/lib:/mnt/win_d/projects/ACENetwork/../pdcurses/win32:/mnt/win_d/projects/ACENetwork/../Common/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/modules/dev/Debug:/usr/local/lib::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -l -r -t -u https://www.tagesschau.de/index.html " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACENetwork/clang/test_u/HTTP_decoder/http_decoder 
	else
		"/mnt/win_d/projects/ACENetwork/clang/test_u/HTTP_decoder/http_decoder" -l -r -t -u https://www.tagesschau.de/index.html 
	fi
else
	"/mnt/win_d/projects/ACENetwork/clang/test_u/HTTP_decoder/http_decoder" -l -r -t -u https://www.tagesschau.de/index.html 
fi
