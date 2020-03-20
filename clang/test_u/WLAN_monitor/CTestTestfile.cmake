# CMake generated Testfile for 
# Source directory: /mnt/win_d/projects/ACENetwork/test_u/WLAN_monitor
# Build directory: /mnt/win_d/projects/ACENetwork/clang/test_u/WLAN_monitor
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(wlan_monitor_test "/mnt/win_d/projects/ACENetwork/clang/wlan_monitor" "-a" "-l" "-t")
set_tests_properties(wlan_monitor_test PROPERTIES  ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;/mnt/win_d/projects/ACENetwork\\..\\Common\\cmake\\src\\Debug;/mnt/win_d/projects/ACENetwork\\..\\Common\\cmake\\src\\ui\\Debug;/mnt/win_d/projects/ACENetwork\\cmake\\src\\Debug;/mnt/win_d/projects/ACENetwork\\cmake\\src\\modules\\dev\\Debug;/mnt/win_d/projects/ACENetwork\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%" WORKING_DIRECTORY "/mnt/win_d/projects/ACENetwork")
