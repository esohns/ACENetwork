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
#if defined (DBUS_SUPPORT)
#include "dbus/dbus.h"
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "net_common.h"
#include "net_packet_headers.h"

//#include "net_wlan_exports.h"
#include "net_wlan_common.h"

//class Net_WLAN_Export Net_WLAN_Tools
class Net_WLAN_Tools
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool initialize (HANDLE&); // return value: WLAN API client handle
  static void finalize (HANDLE); // WLAN API client handle

  static std::string associatedSSID (HANDLE,   // WLAN API client handle
                                     REFGUID); // interface identifier
  static struct ether_addr associatedBSSID (HANDLE,   // WLAN API client handle
                                            REFGUID); // interface identifier
  static Net_InterfaceIdentifiers_t getInterfaces (HANDLE); // WLAN API client handle
  static struct ether_addr getAccessPointAddress (HANDLE,              // WLAN API client handle
                                                  REFGUID,             // interface identifier
                                                  const std::string&); // (E)SSID ("": return first)
  static Net_WLAN_Profiles_t getProfiles (HANDLE,   // WLAN API client handle
                                          REFGUID); // interface identifier (GUID_NULL: all)
  static std::string getProfile (HANDLE,              // WLAN API client handle
                                 REFGUID,             // interface identifier
                                 const std::string&); // (E)SSID
  static Net_WLAN_SSIDs_t getSSIDs (HANDLE,   // WLAN API client handle
                                    REFGUID); // interface identifier (GUID_NULL: all)
  static bool hasSSID (HANDLE,              // WLAN API client handle
                       REFGUID,             // interface identifier
                       const std::string&); // (E)SSID

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
  static bool disassociate (HANDLE,   // WLAN API client handle
                            REFGUID); // interface identifier (GUID_NULL: all)
  static void scan (HANDLE,              // WLAN API client handle
                    REFGUID,             // interface identifier
                    const std::string&); // (E)SSID ("": scan all)
#else
  static std::string associatedSSID (const std::string&, // interface identifier
                                     ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  //  static std::string associatedSSID (struct DBusConnection*, // DBus connection handle
//                                     const std::string&);    // interface identifier
  static struct ether_addr associatedBSSID (const std::string&, // interface identifier
                                            ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  static Net_InterfaceIdentifiers_t getInterfaces (int = AF_UNSPEC, // address family {default: any; use AF_MAX for any IP}
                                                   int = 0);        // flag(s) (e.g. IFF_UP; may be ORed)
  static Net_WLAN_SSIDs_t getSSIDs (const std::string&, // interface identifier (empty: all)
                                    ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  static bool hasSSID (const std::string&,  // interface identifier
                       const std::string&); // (E)SSID
  // *TODO*: only tests for SIOCGIWNAME ioctl
  static bool isWireless (const std::string&); // interface identifier

  static bool associate (const std::string&,       // interface identifier
                         const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
                         const std::string&,       // (E)SSID
                         ACE_HANDLE);              // (socket) handle to effectuate the ioctl (if any)
  static bool disassociate (const std::string&, // interface identifier ("": all)
                            ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  // *NOTE*: invokes SIOCSIWSCAN; result data must be polled with SIOCGIWSCAN
  static void scan (const std::string&, // interface identifier
                    const std::string&, // (E)SSID (if any)
                    ACE_HANDLE,         // (socket) handle to effectuate the ioctl (if any)
                    bool = false);      // wait for the (first bit of-) result data ? (times out after 250ms)
#endif

  // --- DBus ---
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DBUS_SUPPORT)
  static bool activateConnection (struct DBusConnection*, // DBus connection handle
                                  const std::string&,     // connection object path
                                  const std::string&,     // device object path
                                  const std::string&);    // access point object path
  // *NOTE*: 'gateway' really means the 'next hop' router with regard to some
  //         policy (e.g. routing table entry metric/priority/...).
  //         Consequently, this API is non-functional at this point
  static ACE_INET_Addr getGateway (const std::string&,      // interface identifier
                                   struct DBusConnection*); // DBus connection handle

  static std::string accessPointDBusPathToSSID (struct DBusConnection*, // DBus connection handle
                                                const std::string&);    // access point object path
  static std::string activeConnectionDBusPathToDeviceDBusPath (struct DBusConnection*, // DBus connection handle
                                                               const std::string&);    // active connection object path
  static std::string activeConnectionDBusPathToIp4ConfigDBusPath (struct DBusConnection*, // DBus connection handle
                                                                  const std::string&);    // connection object path
  static std::string connectionDBusPathToSSID (struct DBusConnection*, // DBus connection handle
                                               const std::string&);    // connection object path
  static std::string deviceDBusPathToActiveAccessPointDBusPath (struct DBusConnection*, // DBus connection handle
                                                                const std::string&);    // device object path
  static std::string deviceDBusPathToIdentifier (struct DBusConnection*, // DBus connection handle
                                                 const std::string&);    // device object path
  static std::string deviceDBusPathToIp4ConfigDBusPath (struct DBusConnection*, // DBus connection handle
                                                        const std::string&);    // device object path
  static std::string deviceToDBusPath (struct DBusConnection*, // DBus connection handle
                                       const std::string&);    // interface identifier
  static std::string Ip4ConfigDBusPathToGateway (struct DBusConnection*, // DBus connection handle
                                                 const std::string&);    // IPv4Config object path
  static std::string SSIDToAccessPointDBusPath (struct DBusConnection*, // DBus connection handle
                                                const std::string&,     // device object path
                                                const std::string&);    // SSID
  static std::string SSIDToDeviceDBusPath (struct DBusConnection*, // DBus connection handle
                                           const std::string&);    // SSID
  static std::string SSIDToConnectionDBusPath (struct DBusConnection*, // DBus connection handle
                                               const std::string&,     // device object path
                                               const std::string&);    // SSID
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_WLAN_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools (const Net_WLAN_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools& operator= (const Net_WLAN_Tools&))
};

#endif
