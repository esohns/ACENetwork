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

// WLAN monitor
#define NET_WLAN_MONITOR_SSID_SCAN_RETRIES                                3
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define NET_WLANMONITOR_WIN32_SSID_SCAN_TIMEOUT                           4 // seconds
#else
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
#endif
#define NET_WLAN_MONITOR_THREAD_NAME                                      "WLAN monitor"
#define NET_WLAN_MONITOR_THREAD_GROUP_ID                                  21

// *NOTE*: 'background' refers to an associated state
#define NET_WLAN_MONITOR_DEFAULT_BACKGROUNDSCANS                          false
#define NET_WLAN_MONITOR_DEFAULT_STREAMINGMODE                            true

#endif
