# Install script for directory: /mnt/win_d/projects/ACENetwork/test_i/bittorrent_client

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/bittorrent_client" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/bittorrent_client")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/bittorrent_client"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/bittorrent_client")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/mnt/win_d/projects/ACENetwork/clang/test_i/bittorrent_client/bittorrent_client")
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/bittorrent_client" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/bittorrent_client")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/local/bin/bittorrent_client"
         OLD_RPATH "/media/erik/USB_BLACK/lib/ACE_TAO/ACE/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/bittorrent_client")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/bittorrent_client_gui" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/bittorrent_client_gui")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/bittorrent_client_gui"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/bin/bittorrent_client_gui")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/mnt/win_d/projects/ACENetwork/clang/test_i/bittorrent_client/bittorrent_client_gui")
  if(EXISTS "$ENV{DESTDIR}/usr/local/bin/bittorrent_client_gui" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/bittorrent_client_gui")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/local/bin/bittorrent_client_gui"
         OLD_RPATH "/media/erik/USB_BLACK/lib/ACE_TAO/ACE/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/bittorrent_client_gui")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xconfigx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/etc/libACENetwork/test_i" TYPE FILE FILES
    "/mnt/win_d/projects/ACENetwork/test_i/bittorrent_client/bittorrent_client.ini"
    "/mnt/win_d/projects/ACENetwork/test_i/bittorrent_client/bittorrent_client.rc"
    "/mnt/win_d/projects/ACENetwork/test_i/bittorrent_client/bittorrent_client_main.glade"
    "/mnt/win_d/projects/ACENetwork/test_i/bittorrent_client/bittorrent_client_session.glade"
    )
endif()

