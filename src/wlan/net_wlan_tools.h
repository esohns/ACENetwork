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

#ifndef NET_WLAN_TOOLS_H
#define NET_WLAN_TOOLS_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <wlanapi.h>
#else
#include "dbus/dbus.h"
#endif

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "net_packet_headers.h"

//#include "net_wlan_exports.h"

//class Net_WLAN_Export Net_WLAN_Tools
class Net_WLAN_Tools
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static std::string associatedSSID (HANDLE,   // WLAN API client handle
                                     REFGUID); // interface identifier
  static struct ether_addr associatedBSSID (HANDLE,   // WLAN API client handle
                                            REFGUID); // interface identifier

  static bool hasSSID (HANDLE,              // WLAN API client handle
                       REFGUID,             // interface identifier
                       const std::string&); // SSID

  static bool getDeviceSettingBool (HANDLE,   // WLAN API client handle
                                    REFGUID, // interface identifier
                                    enum _WLAN_INTF_OPCODE); // code
  static bool setDeviceSettingBool (HANDLE,                 // WLAN API client handle
                                    REFGUID,                // interface identifier
                                    enum _WLAN_INTF_OPCODE, // code
                                    bool);                  // enable ? : disable

  static bool associate (HANDLE,              // WLAN API client handle
                         REFGUID,             // interface identifier
                         const std::string&); // (E)SSID
  static void scan (HANDLE,              // WLAN API client handle
                    REFGUID,             // interface identifier
                    const std::string&); // ESSID (if any)
#else
  static std::string associatedSSID (const std::string&, // interface identifier
                                     ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  //  static std::string associatedSSID (struct DBusConnection*, // D-Bus connection handle
//                                     const std::string&);    // interface identifier
  static struct ether_addr associatedBSSID (const std::string&, // interface identifier
                                            ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)

  // *NOTE*: 'gateway' really means the 'next hop' router with regard to some
  //         policy (e.g. routing table entry metric/priority/...).
  //         Consequently, this API is non-functional at this point
  static ACE_INET_Addr getGateway (const std::string&,      // interface identifier
                                   struct DBusConnection*); // D-Bus connection handle

  static bool hasSSID (const std::string&,  // interface identifier
                       const std::string&); // SSID
  // *TODO*: merely tests SIOCGIWNAME
  static bool interfaceIsWLAN (const std::string&); // interface identifier

  static bool associate (const std::string&,       // interface identifier
                         const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
                         const std::string&,       // (E)SSID
                         ACE_HANDLE);              // (socket) handle to effectuate the ioctl (if any)
  static void disassociate (const std::string&, // interface identifier
                            ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  // *NOTE*: invokes SIOCSIWSCAN; result data must be polled with SIOCGIWSCAN
  static void scan (const std::string&, // interface identifier
                    const std::string&, // ESSID (if any)
                    ACE_HANDLE,         // (socket) handle to effectuate the ioctl (if any)
                    bool = false);      // wait for the (first bit of-) result data ? (times out after 250ms)
#endif

  // --- D-Bus ---
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  static bool activateConnection (struct DBusConnection*, // D-Bus connection handle
                                  const std::string&,     // connection object path
                                  const std::string&,     // device object path
                                  const std::string&);    // access point object path
  static std::string accessPointDBusPathToSSID (struct DBusConnection*, // D-Bus connection handle
                                                const std::string&);    // access point object path
  static std::string activeConnectionDBusPathToDeviceDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                               const std::string&);    // active connection object path
  static std::string activeConnectionDBusPathToIp4ConfigDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                                  const std::string&);    // connection object path
  static std::string connectionDBusPathToSSID (struct DBusConnection*, // D-Bus connection handle
                                               const std::string&);    // connection object path
  static std::string deviceDBusPathToActiveAccessPointDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                                const std::string&);    // device object path
  static std::string deviceDBusPathToIdentifier (struct DBusConnection*, // D-Bus connection handle
                                                 const std::string&);    // device object path
  static std::string deviceDBusPathToIp4ConfigDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                        const std::string&);    // device object path
  static std::string deviceToDBusPath (struct DBusConnection*, // D-Bus connection handle
                                       const std::string&);    // device identifier
  static std::string Ip4ConfigDBusPathToGateway (struct DBusConnection*, // D-Bus connection handle
                                                 const std::string&);    // IPv4Config object path
  static std::string SSIDToAccessPointDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                const std::string&,     // device object path
                                                const std::string&);    // SSID
  static std::string SSIDToDeviceDBusPath (struct DBusConnection*, // D-Bus connection handle
                                           const std::string&);    // SSID
  static std::string SSIDToConnectionDBusPath (struct DBusConnection*, // D-Bus connection handle
                                               const std::string&,     // device object path
                                               const std::string&);    // SSID
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_WLAN_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools (const Net_WLAN_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools& operator= (const Net_WLAN_Tools&))
};

#endif
