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

// protocol
// (see also: http://standards.ieee.org/findstds/standard/802.11-2016.html)

// information elements
// (see also: "Table 7-26—Element IDs")
#define NET_WLAN_IEEE80211_INFORMATION_ELEMENT_ID_SSID                    0

//////////////////////////////////////////

// monitor
#define NET_WLAN_MONITOR_AP_ASSOCIATION_RETRIES                           3
// *NOTE*: 'connection' really refers to the level-3 (i.e. IP) address handshake
//         procedure (e.g. by DHCP) taking place after successful AP association
#define NET_WLAN_MONITOR_AP_CONNECTION_RETRIES                            3
// *NOTE*: the monitor will continually scan until the configured SSID (if any)
//         is detected. It will then periodically rescan at this interval iff it
//         is not configured to 'auto-associate'
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_WLAN_MONITOR_DEFAULT_API                                      NET_WLAN_MONITOR_API_WLANAPI
// *TODO*: see timeout below
#define NET_WLAN_MONITOR_WIN32_SCAN_INTERVAL                              NET_WLAN_MONITOR_WIN32_SCAN_SSID_TIMEOUT // s
#else
#define NET_WLAN_MONITOR_DEFAULT_API                                      NET_WLAN_MONITOR_API_NL80211
#define NET_WLAN_MONITOR_UNIX_SCAN_INTERVAL                               3 // s
#endif // ACE_WIN32 || ACE_WIN64
#define NET_WLAN_MONITOR_SCAN_SSID_RETRIES                                3

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: this is a 'Windows-Logo' requirement (see also: https://docs.microsoft.com/en-us/windows/desktop/api/wlanapi/nf-wlanapi-wlanscan)
#define NET_WLAN_MONITOR_WIN32_SCAN_SSID_TIMEOUT                          4 // seconds

#define NET_WLAN_MONITOR_WIN32_DEFAULT_AUTOCONF                           true
// *NOTE*: 'background' refers to an associated state
#define NET_WLAN_MONITOR_WIN32_DEFAULT_BACKGROUNDSCANS                    false
#define NET_WLAN_MONITOR_WIN32_DEFAULT_MEDIASTREAMINGMODE                 false

// XML WLAN profile
#define NET_WLAN_PROFILE_TEMPLATE_FILENAME                                "wlanapi_profile_template.xml"
#define NET_WLAN_PROFILE_SSIDCONFIG_SSID_XPATH                            L"//SSIDConfig/SSID[1]/name"

#define NET_WLAN_PROFILE_HEX_ELEMENT_STRING                               "hex"
#define NET_WLAN_PROFILE_SSIDCONFIG_ELEMENT_STRING                        "SSIDConfig"
#define NET_WLAN_PROFILE_SSID_ELEMENT_STRING                              "SSID"
#else
#define NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL         50 // ms
#define NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT                      3 // s
// *TODO*: try to unify this setting (see above)
#define NET_WLAN_MONITOR_UNIX_DEFAULT_BACKGROUNDSCANS                     false
#define NET_WLAN_MONITOR_SCAN_DEFAULT_RESULT_POLL_INTERVAL                50 // ms

#if defined (NL80211_SUPPORT)
// *TODO*: use official definitions from the standard as soon as they are
//         available programatically (see also: Std 802.11-2007 7.3.1.7)
#define NET_WLAN_MONITOR_NL80211_REASON_CODE_LEAVING                      3

#define NET_WLAN_MONITOR_NL80211_CONTROL_NAME_STRING                      "nlctrl"

// features (device)
#define NET_WLAN_MONITOR_NL80211_DEFAULT_FLUSHCACHEBEFORESCANS            false
#define NET_WLAN_MONITOR_NL80211_DEFAULT_LOWPRIORITYSCANS                 true
#define NET_WLAN_MONITOR_NL80211_DEFAULT_RANDOMIZEMACADDRESSFORSCANS      false

// features (protocol)
#define NET_WLAN_MONITOR_NL80211_DEFAULT_SPLITWIPHYDUMPS                  true
#endif // NL80211_SUPPORT

#if defined (DBUS_SUPPORT)
// NetworkManager
#define NET_WLAN_DBUS_NETWORKMANAGER_SERVICE                      "org.freedesktop.NetworkManager"

#define NET_WLAN_DBUS_NETWORKMANAGER_INTERFACE                    "org.freedesktop.NetworkManager"
#define NET_WLAN_DBUS_NETWORKMANAGER_ACCESSPOINT_INTERFACE        "org.freedesktop.NetworkManager.AccessPoint"
#define NET_WLAN_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE   "org.freedesktop.NetworkManager.Connection.Active"
#define NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_INTERFACE             "org.freedesktop.NetworkManager.Device"
#define NET_WLAN_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE     "org.freedesktop.NetworkManager.Device.Wireless"
#define NET_WLAN_DBUS_NETWORKMANAGER_IP4CONFIG_INTERFACE          "org.freedesktop.NetworkManager.IP4Config"
#define NET_WLAN_DBUS_NETWORKMANAGER_SETTINGS_INTERFACE           "org.freedesktop.NetworkManager.Settings"
#define NET_WLAN_DBUS_NETWORKMANAGER_SETTINGSCONNECTION_INTERFACE "org.freedesktop.NetworkManager.Settings.Connection"

#define NET_WLAN_DBUS_NETWORKMANAGER_OBJECT_PATH                  "/org/freedesktop/NetworkManager"
#define NET_WLAN_DBUS_NETWORKMANAGER_ACCESSPOINT_OBJECT_PATH      "/org/freedesktop/NetworkManager/AccessPoint"
#define NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_OBJECT_PATH           "/org/freedesktop/NetworkManager/Device"
#define NET_WLAN_DBUS_NETWORKMANAGER_SETTINGS_OBJECT_PATH         "/org/freedesktop/NetworkManager/Settings"
#define NET_WLAN_DBUS_NETWORKMANAGER_WIRELESS_OBJECT_PATH         "/org/freedesktop/NetworkManager/Device/Wireless"

// wpa_supplicant
#define NET_WLAN_DBUS_WPASUPPLICANT_SERVICE                       "fi.w1.wpa_supplicant1"

#define NET_WLAN_DBUS_WPASUPPLICANT_INTERFACE                     "fi.w1.wpa_supplicant1"
#define NET_WLAN_DBUS_WPASUPPLICANT_INTERFACE_INTERFACE           "fi.w1.wpa_supplicant1.Interface"

#define NET_WLAN_DBUS_WPASUPPLICANT_OBJECT_PATH                  "/fi/w1/wpa_supplicant1"
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#define NET_WLAN_MONITOR_THREAD_NAME                                      "WLAN monitor"
#define NET_WLAN_MONITOR_THREAD_GROUP_ID                                  21

// state-machine
#define NET_WLAN_STATEMACHINE_MONITOR_NAME                                "WLANStateMachineMonitor"

#endif
