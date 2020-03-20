#!/bin/sh
bindir=$(pwd)
cd /mnt/win_d/projects/ACENetwork/clang/../test_u/wlan_monitor
export LD_LIBRARY_PATH=/mnt/win_d/projects/ACENetwork/modules/ATCD/ACE/lib::$LD_LIBRARY_PATH

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r -g./etc/wlan_monitor.glade -a -l -t " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /mnt/win_d/projects/ACENetwork/clang/test_u/WLAN_monitor/wlan_monitor 
	else
		"/mnt/win_d/projects/ACENetwork/clang/test_u/WLAN_monitor/wlan_monitor" -g./etc/wlan_monitor.glade -a -l -t 
	fi
else
	"/mnt/win_d/projects/ACENetwork/clang/test_u/WLAN_monitor/wlan_monitor" -g./etc/wlan_monitor.glade -a -l -t 
fi
