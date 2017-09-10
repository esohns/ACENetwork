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

#ifndef NET_COMMON_TOOLS_H
#define NET_COMMON_TOOLS_H

#include <limits>
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "dbus/dbus.h"
#endif

#include "ace/config-lite.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "net_common.h"

//////////////////////////////////////////

enum Net_LinkLayerType& operator++ (enum Net_LinkLayerType& lhs);
enum Net_LinkLayerType  operator++ (enum Net_LinkLayerType& lhs, int);

//////////////////////////////////////////

class Net_Common_Tools
{
 public:
  // physical layer

  // link layer
  // *NOTE*: today, such general-purpose link-layer APIs really do not make much
  //         sense because the interface to choose always depends on the peer
  //         address(es) and -type (note how even if such information where
  //         available in advance - if the (corresponding IP-) address is not
  //         'local' (here: relative to the netmask and host IP address) to any
  //         connected LAN, there is currently no public API to determine the
  //         'best' route; only internet routers maintain such information).
  // [*TODO*: for a known peer IP address, implement a function that uses ICMP
  //         over each IP interface to determine the 'best' route.]
  //         Notwithstanding, some platforms assign (hard-coded) priorities to
  //         their routing table entries (e.g. MS-Windows desktop PCs: 'netsh
  //         int ipv4 show route'). Also, some platforms (e.g.: MS-Windows
  //         desktop PCs: 'netsh int ipv4 show interfaces') assign (hard-coded)
  //         priorities to each installed NIC as well.
  //         For practical purposes, this currently selects:
  //         - the first 'connected' interface (iff applicable) [with the
  //         - highest priority (e.g. lowest 'metric' (MS-Windows)), if
  //           specified]
  // *NOTE*: returns the 'default' internet network interface, i.e. the device
  //         identifier associated with the 'default' (currently: IP- only)
  //         'route' (or 'gateway').
  // *TODO*: - the Linux version is incomplete (selects Ethernet/PPP only, and
  //         does not check connectedness yet)
  //         - the Win32 is incomplete (returns first 'connected' interface)
  static std::string getDefaultInterface (enum Net_LinkLayerType = NET_LINKLAYER_802_3);
  // *NOTE*: queries the system for installed network interfaces, returning the
  //         'default' one (bitmask-version of the above)
  // *TODO*: only Ethernet (IEEE 802.3) and PPP is currently supported
  static std::string getDefaultInterface (int); // link layer type(s) (bitmask)

  // *NOTE*: make sure the argument points at at least 6 (!) bytes of allocated memory
  static std::string LinkLayerAddressToString (const unsigned char* const, // pointer to physical address data (i.e. START of ethernet header address field !)
                                               enum Net_LinkLayerType = NET_LINKLAYER_802_3);
  static std::string LinkLayerTypeToString (enum Net_LinkLayerType);
  static std::string EthernetProtocolTypeIDToString (unsigned short); // ethernet frame type (network (== big-endian) byte order !)

    // *WARNING*: ensure that the array argument can hold at least 6 bytes !
  static bool interfaceToMACAddress (const std::string&, // interface identifier
                                     unsigned char[]);   // return value: MAC address

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static std::string associatedSSID (HANDLE,   // WLAN API client handle
                                     REFGUID); // interface identifier
  static bool hasSSID (HANDLE,              // WLAN API client handle
                       REFGUID,             // interface identifier
                       const std::string&); // SSID
  static std::string interfaceToString (HANDLE,   // WLAN API client handle
                                        REFGUID); // interface identifier
  static bool getDeviceSettingBool (HANDLE,   // WLAN API client handle
                                    REFGUID, // interface identifier
                                    enum _WLAN_INTF_OPCODE); // code
  static bool setDeviceSettingBool (HANDLE,                 // WLAN API client handle
                                    REFGUID,                // interface identifier
                                    enum _WLAN_INTF_OPCODE, // code
                                    bool);                  // enable ? : disable
  //static bool associateWithWLAN (REFGUID,            // interface identifier
#else
  // *NOTE*: this tries to call the SIOCGIWNAME ioctl
  static bool interfaceIsWireless (const std::string&); // interface identifier
  static std::string associatedSSID (const std::string&); // interface identifier
  static bool hasSSID (const std::string&,  // interface identifier
                       const std::string&); // SSID
  //  static std::string associatedSSID (struct DBusConnection*, // D-Bus connection handle
//                                     const std::string&);    // interface identifier
  //static bool associateWithWLAN (const std::string&, // interface adapter
#endif
                                 //const std::string&, // SSID
                                 //ACE_INET_Addr&,     // return value: peer SAP
                                 //ACE_INET_Addr&,     // return value: local SAP
                                 //bool = true);       // scan for networks ?

  // network layer
  static bool getAddress (std::string&,  // host name
                          std::string&); // dotted-decimal

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: 'gateway' really means the 'next hop' router with regard to some
  //         policy (e.g. routing table entry metric/priority/...).
  //         Consequently, this API is non-functional at this point
  static ACE_INET_Addr getGateway (const std::string&); // interface identifier
#endif

  // *NOTE*: this returns the external (i.e. routable) IP address (for clients
  //         behind a (NATted-) gateway)
  static bool interfaceToExternalIPAddress (const std::string&, // interface identifier
                                            ACE_INET_Addr&);    // return value: external IP address
  // *NOTE*: on Win32 systems, specify the 'AdapterName' (not 'FriendlyName')
  //         --> i.e. use Common_Tools::GUIDToString() on the device identifier
  static bool interfaceToIPAddress (const std::string&, // device identifier
                                    ACE_INET_Addr&,     // return value: (first) IP address
                                    ACE_INET_Addr&);    // return value: (first) gateway IP address
  static bool IPAddressToInterface (const ACE_INET_Addr&, // IP address
                                    std::string&);        // return value: interface identifier

  inline static std::string IPAddressToString (const ACE_INET_Addr& address_in,
                                               bool addressOnly_in = false) { return Net_Common_Tools::IPAddressToString ((addressOnly_in ? 0 : ACE_HTONS (address_in.get_port_number ())), ACE_HTONL (address_in.get_ip_address ())); };
#if defined (ACE_HAS_NETLINK)
  static std::string NetlinkAddressToString (const Net_Netlink_Addr&);
#endif
  // *NOTE*: if (the first argument is '0'), the trailing ":0" will be cropped
  //         from the return value
  static std::string IPAddressToString (unsigned short, // port (network byte order !)
                                        ACE_UINT32);    // IP address (network byte order !)
  static std::string IPProtocolToString (unsigned char); // protocol

  static bool matchIPAddress (std::string&); // dotted-decimal
  // *NOTE*: (see also: ace/INET_Addr.h:237)
  static ACE_INET_Addr stringToIPAddress (std::string&); // host name (DNS name or dotted-decimal)
  static bool isLocal (const ACE_INET_Addr&);

  // transport layer
  static std::string TransportLayerTypeToString (enum Net_TransportLayerType);

  // session layer (and above)
  static bool getHostname (std::string&); // return value: hostname

  static std::string URLToHostName (const std::string&, // URL
                                    bool = true,        // return protocol (if any) ?
                                    bool = true);       // return port (if any) ?

  // --- socket API ---

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: applies to TCP sockets (see also: SO_MAX_MSG_SIZE)
  static bool setLoopBackFastPath (ACE_HANDLE); // socket handle
#endif

  // MTU
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: configures packet fragmentation to support sending datagrams
  //         larger than the Path MTU (see: man ip(7))
  static bool setPathMTUDiscovery (ACE_HANDLE, // socket handle
                                   int);       // option
  static bool getPathMTU (const ACE_INET_Addr&, // destination address
                          unsigned int&);       // return value: (initial) path MTU
#endif
  // *NOTE*: applies to (connect()ed) UDP sockets (see also: SO_MAX_MSG_SIZE)
  static unsigned int getMTU (ACE_HANDLE); // socket handle

  // buffers
  static bool setSocketBuffer (ACE_HANDLE, // socket handle
                               int,        // option (SO_RCVBUF || SO_SNDBUF)
                               int);       // size (bytes)

  // options
  // *NOTE*: toggle Nagle's algorithm
  static bool getNoDelay (ACE_HANDLE); // socket handle
  static bool setNoDelay (ACE_HANDLE, // socket handle
                          bool);      // TCP_NODELAY ?
  static bool setKeepAlive (ACE_HANDLE, // socket handle
                            bool);      // SO_KEEPALIVE ?
  static bool setLinger (ACE_HANDLE,                                                    // socket handle
                         bool,                                                          // on ? : off
                         unsigned short = std::numeric_limits<unsigned short>::max ()); // seconds {0     --> send RST on close,
                                                                                        //          65535 --> reuse default/current value}
  static bool setReuseAddress (ACE_HANDLE    // socket handle
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              );
#else
                               , bool = false); // (try to) set SO_REUSEPORT as well ?
#endif

#if defined (ACE_LINUX)
  static bool enableErrorQueue (ACE_HANDLE); // socket handle
#endif
  static int getProtocol (ACE_HANDLE); // socket handle

  // --- D-Bus ---
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  static bool activateConnection (struct DBusConnection*, // D-Bus connection handle
                                  const std::string&,     // connection object path
                                  const std::string&,     // device object path
                                  const std::string&);    // access point object path
  // *IMPORTANT NOTE* fire-and-forget the second argument
  static struct DBusMessage* dBusMessageExchange (struct DBusConnection*, // D-Bus connection handle
                                                  struct DBusMessage*&,   // D-Bus message handle
                                                  int = DBUS_TIMEOUT_INFINITE); // timeout (ms) {default: block}

  static std::string accessPointDBusPathToSSID (struct DBusConnection*, // D-Bus connection handle
                                                const std::string&);    // access point object path
  static std::string activeConnectionDBusPathToDeviceDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                               const std::string&);    // active connection object path
  static std::string connectionDBusPathToSSID (struct DBusConnection*, // D-Bus connection handle
                                               const std::string&);    // connection object path
  static std::string deviceDBusPathToAccessPointDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                          const std::string&);    // device object path
  static std::string deviceDBusPathToIdentifier (struct DBusConnection*, // D-Bus connection handle
                                                 const std::string&);    // device object path
  static std::string deviceToDBusPath (struct DBusConnection*, // D-Bus connection handle
                                       const std::string&);    // device identifier
  static std::string SSIDToAccessPointDBusPath (struct DBusConnection*, // D-Bus connection handle
                                                const std::string&);    // SSID
  static std::string SSIDToDeviceDBusPath (struct DBusConnection*, // D-Bus connection handle
                                           const std::string&);    // SSID
  static std::string SSIDToConnectionDBusPath (struct DBusConnection*, // D-Bus connection handle
                                               const std::string&);    // SSID
#endif

  // --- miscellaneous ---

//  static Net_IInetConnectionManager_t* getConnectionManager ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools (const Net_Common_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools& operator= (const Net_Common_Tools&))
};

#endif
