/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef NET_WLAN_DEFINES_H
#define NET_WLAN_DEFINES_H

#include "ace/config-lite.h"

// monitor
#define NET_WLAN_MONITOR_AP_ASSOCIATION_RETRIES                           3
// *NOTE*: 'connection' really refers to the level-3 (i.e. IP) address handshake
//         procedure (e.g. by DHCP) taking place after successful AP association
#define NET_WLAN_MONITOR_AP_CONNECTION_RETRIES                            3
// *NOTE*: the monitor will continually scan until the configured SSID (if any)
//         is detected. It will then periodically rescan at this interval iff it
//         is not configured to 'auto-associate'
#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *TODO*: see timeout below
#define NET_WLAN_MONITOR_WIN32_SCAN_INTERVAL                              5 // s
#else
#define NET_WLAN_MONITOR_UNIX_SCAN_INTERVAL                               500 // ms
#endif
#define NET_WLAN_MONITOR_SCAN_SSID_RETRIES                                3

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: this is a 'Windows-Logo' requirement
#define NET_WLAN_MONITOR_WIN32_SCAN_SSID_TIMEOUT                          4 // seconds

#define NET_WLAN_MONITOR_WIN32_DEFAULT_AUTOCONF                           true
// *NOTE*: 'background' refers to an associated state
#define NET_WLAN_MONITOR_WIN32_DEFAULT_BACKGROUNDSCANS                    false
#define NET_WLAN_MONITOR_WIN32_DEFAULT_MEDIASTREAMINGMODE                 false

// XML WLAN profile
#define NET_WLAN_PROFILE_HEX_ELEMENT_STRING                               "hex"
#define NET_WLAN_PROFILE_SSIDCONFIG_ELEMENT_STRING                        "SSIDConfig"
#define NET_WLAN_PROFILE_SSID_ELEMENT_STRING                              "SSID"
#else
#define NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL         50 // ms
#define NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT                      3 // s
#define NET_WLAN_MONITOR_SCAN_DEFAULT_RESULT_POLL_INTERVAL                50 // ms

#if defined (DHCLIENT_SUPPORT)
#define NET_WLAN_MONITOR_DHCLIENT_LOCALHOST_IP_STRING                     "127.0.0.1"
#define NET_WLAN_MONITOR_DHCLIENT_OMAPI_PORT                              7911
#endif // DHCLIENT_SUPPORT

#if defined (DBUS_SUPPORT)
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE                      "org.freedesktop.NetworkManager"

#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_INTERFACE                    "org.freedesktop.NetworkManager"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_ACCESSPOINT_INTERFACE        "org.freedesktop.NetworkManager.AccessPoint"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE   "org.freedesktop.NetworkManager.Connection.Active"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICE_INTERFACE             "org.freedesktop.NetworkManager.Device"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE     "org.freedesktop.NetworkManager.Device.Wireless"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_IP4CONFIG_INTERFACE          "org.freedesktop.NetworkManager.IP4Config"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_PROPERTIES_INTERFACE         "org.freedesktop.DBus.Properties"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SETTINGS_INTERFACE           "org.freedesktop.NetworkManager.Settings"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SETTINGSCONNECTION_INTERFACE "org.freedesktop.NetworkManager.Settings.Connection"

#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_OBJECT_PATH                  "/org/freedesktop/NetworkManager"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_ACCESSPOINT_OBJECT_PATH      "/org/freedesktop/NetworkManager/AccessPoint"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_DEVICE_OBJECT_PATH           "/org/freedesktop/NetworkManager/Device"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SETTINGS_OBJECT_PATH         "/org/freedesktop/NetworkManager/Settings"
#define NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_WIRELESS_OBJECT_PATH         "/org/freedesktop/NetworkManager/Device/Wireless"
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#define NET_WLAN_MONITOR_THREAD_NAME                                      "WLAN monitor"
#define NET_WLAN_MONITOR_THREAD_GROUP_ID                                  21

#endif
