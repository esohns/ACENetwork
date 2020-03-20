#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACENetwork/clang/../test_i/url_stream_load
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACENetwork/modules/ATCD/ACE/lib:/mnt/win_d/projects/ACENetwork/../Common/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/Debug:/mnt/win_d/projects/ACENetwork/cmake/src/modules/dev/Debug:/usr/local/lib::$LD_LIBRARY_PATH
export foo=bar

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -l -t -u https://www.youtube.com/watch?v=fvq5uhqGCWM&list=RDfvq5uhqGCWM " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACENetwork/clang/test_i/url_stream_load/url_stream_load 
	else
		"/mnt/win_d/projects/ACENetwork/clang/test_i/url_stream_load/url_stream_load" -l -t -u https://www.youtube.com/watch?v=fvq5uhqGCWM&list=RDfvq5uhqGCWM 
	fi
else
	"/mnt/win_d/projects/ACENetwork/clang/test_i/url_stream_load/url_stream_load" -l -t -u https://www.youtube.com/watch?v=fvq5uhqGCWM&list=RDfvq5uhqGCWM 
fi
