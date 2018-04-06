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
#if defined (WLANAPI_SUPPORT)
#include <wlanapi.h>
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
#include <linux/nl80211.h>

#include <netlink/handlers.h>
#endif // NL80211_SUPPORT

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

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
struct sockaddr_nl;
struct nlmsgerr;
struct nl_msg;

// error handling
int
network_wlan_nl80211_error_cb (struct sockaddr_nl*,
                               struct nlmsgerr*,
                               void*);

// protocol handling
int
network_wlan_nl80211_interface_cb (struct nl_msg*,
                                   void*);
int
network_wlan_nl80211_bssid_cb (struct nl_msg*,
                               void*);
int
network_wlan_nl80211_ssid_cb (struct nl_msg*,
                              void*);
int
network_wlan_nl80211_ssids_cb (struct nl_msg*,
                               void*);
int
network_wlan_nl80211_feature_cb (struct nl_msg*,
                                 void*);
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

//class Net_WLAN_Export Net_WLAN_Tools
class Net_WLAN_Tools
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
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
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
#if defined (WEXT_SUPPORT)
  static Net_InterfaceIdentifiers_t getInterfaces (int = AF_UNSPEC, // address family {default: any; use AF_MAX for any IP}
                                                   int = 0);        // flag(s) (e.g. IFF_UP; may be ORed)

  static struct ether_addr associatedBSSID (const std::string&, // interface identifier
                                            ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  static std::string associatedSSID (const std::string&, // interface identifier
                                     ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  static Net_WLAN_SSIDs_t getSSIDs (const std::string&, // interface identifier (empty: all)
                                    ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  // *NOTE*: this uses iwlib
  static bool hasSSID (const std::string&,  // interface identifier
                       const std::string&); // (E)SSID
  // *NOTE*: the WEXT_SUPPORT implementation merely tests SIOCGIWNAME ioctl
  static bool isInterface (const std::string&); // interface identifier

  static bool associate (const std::string&,       // interface identifier
                         const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
                         const std::string&,       // (E)SSID
                         ACE_HANDLE);              // (socket) handle to effectuate the ioctl (if any)
  static bool disassociate (const std::string&, // interface identifier ("": all)
                            ACE_HANDLE);        // (socket) handle to effectuate the ioctl (if any)
  // *NOTE*: invokes SIOCSIWSCAN; result data must be polled with SIOCGIWSCAN
  static void scan (const std::string&, // interface identifier
                    const std::string&, // (E)SSID {"": all}
                    ACE_HANDLE,         // (socket) handle to effectuate the ioctl (if any)
                    bool = false);      // wait for the (first bit of-) result data ? (times out after 250ms)
#endif // WEXT_SUPPORT

#if defined (NL80211_SUPPORT)
  // *IMPORTANT NOTE*: passing a valid socket handle introduces race conditions,
  //                   as the callback handlers cannot be exchanged at runtime

  static Net_InterfaceIdentifiers_t getInterfaces (struct nl_sock*, // nl80211 (socket) handle (if any)
                                                   int,             // "nl80211" driver family identifier
                                                   int = AF_UNSPEC, // address family {default: any; pass AF_MAX for any IP}
                                                   int = 0);        // flag(s) (e.g. IFF_UP; may be ORed)

  static struct ether_addr associatedBSSID (const std::string&, // interface identifier
                                            struct nl_sock*,    // nl80211 (socket) handle (if any)
                                            int);               // "nl80211" driver family identifier
  static std::string associatedSSID (const std::string&, // interface identifier
                                     struct nl_sock*,    // nl80211 (socket) handle (if any)
                                     int);               // "nl80211" driver family identifier
  static Net_WLAN_SSIDs_t getSSIDs (const std::string&, // interface identifier (empty: all)
                                    struct nl_sock*,    // nl80211 (socket) handle (if any)
                                    int);               // "nl80211" driver family identifier

  static bool getFeatures (const std::string&,                    // interface identifier
                           struct nl_sock*,                       // nl80211 (socket) handle (if any)
                           int,                                   // "nl80211" driver family identifier
                           ACE_UINT32&,                           // return value: feature set
                           Net_WLAN_nl80211_ExtendedFeatures_t&); // return value: extended feature set
  static bool isInterface (const std::string&, // interface identifier
                           struct nl_sock*,    // nl80211 (socket) handle (if any)
                           int);               // "nl80211" driver family identifier

  // *NOTE*: sends NL80211_CMD_CONNECT (and dispatches the result(s)); data
  //         is forwarded to the appropriate callback(s)
  static bool associate (const std::string&,       // interface identifier
                         const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
                         const std::string&,       // (E)SSID
                         struct nl_sock*,          // nl80211 (socket) handle (if any)
                         int);                     // "nl80211" driver family identifier
  // *NOTE*: sends NL80211_CMD_DISCONNECT (and dispatches the result(s)); data
  //         is forwarded to the appropriate callback(s)
  static bool disassociate (const std::string&, // interface identifier ("": all)
                            struct nl_sock*,    // nl80211 (socket) handle (if any)
                            int);               // "nl80211" driver family identifier
  // *NOTE*: sends NL80211_CMD_TRIGGER_SCAN (and dispatches the result(s)); data
  //         is forwarded to the appropriate callback(s)
  static void scan (const std::string&, // interface identifier
                    const std::string&, // (E)SSID {"": all}
                    struct nl_sock*,    // nl80211 (socket) handle (if any)
                    int,                // "nl80211" driver family identifier
                    bool = false);      // wait for-/dispatch result data ?
#endif // NL80211_SUPPORT

#if defined (DBUS_SUPPORT)
  // NetworkManager
  static Net_InterfaceIdentifiers_t getInterfaces (struct DBusConnection*, // DBus connection handle
                                                   int = AF_UNSPEC,        // address family {default: any; pass AF_MAX for any IP}
                                                   int = 0);               // flag(s) (e.g. IFF_UP; may be ORed)

  static std::string associatedSSID (struct DBusConnection*, // DBus connection handle
                                     const std::string&);    // interface identifier

  static bool activateConnection (struct DBusConnection*, // DBus connection handle
                                  const std::string&,     // connection object path
                                  const std::string&,     // device object path
                                  const std::string&);    // access point object path
  // *NOTE*: 'gateway' really means the 'next hop' router with regard to some
  //         policy (e.g. routing table entry metric/priority/...).
  //         Consequently, this API is non-functional at this point
  static ACE_INET_Addr getGateway (const std::string&,      // interface identifier
                                   struct DBusConnection*); // DBus connection handle

  static std::string accessPointDBusObjectPathToSSID (struct DBusConnection*, // DBus connection handle
                                                      const std::string&);    // access point object path
  static std::string activeConnectionDBusObjectPathToDeviceDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                                                           const std::string&);    // active connection object path
  static std::string activeConnectionDBusObjectPathToIp4ConfigDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                                                              const std::string&);    // connection object path
  static std::string connectionDBusObjectPathToSSID (struct DBusConnection*, // DBus connection handle
                                                     const std::string&);    // connection object path
  static std::string deviceDBusObjectPathToActiveAccessPointDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                                                            const std::string&);    // device object path
  static std::string deviceDBusObjectPathToIdentifier (struct DBusConnection*, // DBus connection handle
                                                       const std::string&);    // device object path
  static std::string deviceDBusObjectPathToIp4ConfigDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                                                    const std::string&);    // device object path
  static std::string deviceToDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                             const std::string&);    // interface identifier
  static std::string Ip4ConfigDBusObjectPathToGateway (struct DBusConnection*, // DBus connection handle
                                                       const std::string&);    // IPv4Config object path
  static std::string SSIDToAccessPointDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                                      const std::string&,     // device object path
                                                      const std::string&);    // SSID
  static std::string SSIDToDeviceDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                                 const std::string&);    // SSID
  static std::string SSIDToConnectionDBusObjectPath (struct DBusConnection*, // DBus connection handle
                                                     const std::string&,     // device object path
                                                     const std::string&);    // SSID

  // wpa_supplicant
  static bool WPASupplicantManageInterface (struct DBusConnection*,
                                            const std::string&,
                                            bool); // toggle
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64 || ACE_LINUX

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_WLAN_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools (const Net_WLAN_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Tools& operator= (const Net_WLAN_Tools&))

  // helper methods
#if defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
  // *NOTE*: fire-and-forget API for the last argument
  static bool nL80211Command (struct nl_sock*,     // socket handle
                              nl_recvmsg_msg_cb_t, // message handler callback
                              void*,               // message handler callback user data
                              const std::string&,  // interface identifier (sets NL80211_ATTR_IFINDEX, if any)
                              struct nl_msg*&);    // message handle
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
  static bool isWPASupplicantManagingInterface (struct DBusConnection*,
                                                const std::string&);

  static std::string dBusObjectPathToIdentifier (struct DBusConnection*,
                                                 const std::string&);
  static std::string identifierToDBusObjectPath (struct DBusConnection*,
                                                 const std::string&);
#endif // DBUS_SUPPORT
#endif // ACE_LINUX
};

#endif
