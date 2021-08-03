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

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <ifdef.h>
#else
#include "net/ethernet.h"

//#if defined (DHCLIENT_SUPPORT)
//extern "C"
//{
//#include "dhcpctl/dhcpctl.h"
//}
//#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "net_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "net_packet_headers.h"
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
//struct nl_msg;
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
#if defined (SSL_SUPPORT)
class ACE_SSL_Context;
#endif // SSL_SUPPORT

//////////////////////////////////////////

enum Net_LinkLayerType& operator++ (enum Net_LinkLayerType&);
enum Net_LinkLayerType  operator++ (enum Net_LinkLayerType&, int);

//////////////////////////////////////////

class Net_Common_Tools
{
 public:
  // physical layer
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: as of Vista, interface GUIDs can be used consistently; prior to
  //         that, the 'FriendlyName' attribute must be used (which may not be
  //         truly unique)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool isInterfaceEnabled (REFGUID);
  static bool toggleInterface (REFGUID);
#else
  static bool isInterfaceEnabled (const std::string&);
  static bool toggleInterface (const std::string&);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  static bool isInterfaceEnabled (const std::string&);
  static bool toggleInterface (const std::string&);
#endif // ACE_WIN32 || ACE_WIN64

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  inline static struct _GUID getDefaultInterface_2 (enum Net_LinkLayerType type_in = NET_LINKLAYER_802_3) { return (Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (Net_Common_Tools::getDefaultInterface (type_in)))); }
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  static std::string getDefaultInterface (enum Net_LinkLayerType = NET_LINKLAYER_802_3);
#else
  static std::string getDefaultInterface (enum Net_LinkLayerType = NET_LINKLAYER_802_3);
#endif // ACE_WIN32 || ACE_WIN64
  // *NOTE*: queries the system for installed network interfaces, returning the
  //         'default' one (bitmask-version of the above)
  // *TODO*: only Ethernet (IEEE 802.3) and PPP is currently supported
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  inline static struct _GUID getDefaultInterface_2 (int types_in) { return (Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (Net_Common_Tools::getDefaultInterface (types_in)))); }
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#endif // ACE_WIN32 || ACE_WIN64
  static std::string getDefaultInterface (int); // link layer type(s) (bitmask)

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static NET_IFINDEX interfaceToIndex_2 (REFGUID); // interface identifier
  static struct _GUID indexToInterface_2 (NET_IFINDEX); // interface index
  static std::string adapterName (const std::string&); // interface identifier ('FriendlyName')
  static NET_IFINDEX interfaceToIndex (const std::string&); // interface identifier ('AdapterName')
  static std::string indexToInterface (NET_IFINDEX); // interface index

#if defined (_DEBUG)
  static void dump (const std::string&); // interface identifier ('FriendlyName')
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64

  // *NOTE*: the argument is assumed to be in network byte order (i.e. bytes
  //         ordered left to right)
  // *NOTE*: make sure the argument points to at least ETH_ALEN bytes of allocated (!) memory
  static std::string LinkLayerAddressToString (const uint8_t* const, // pointer to address data (i.e. ethernet header address field)
                                               enum Net_LinkLayerType = NET_LINKLAYER_802_3);
  static struct ether_addr stringToLinkLayerAddress (const std::string&);
  static std::string LinkLayerTypeToString (enum Net_LinkLayerType);

  // *** ethernet ***
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  inline static bool isAny (const struct ether_addr& address_in) { for (int i = 0; i < ETH_ALEN; ++i) if (address_in.ether_addr_octet[i]) return false; return true; }
  inline static bool isBroadcast (const struct ether_addr& address_in) { for (int i = 0; i < ETH_ALEN; ++i) if (address_in.ether_addr_octet[i] != 0xFF) return false; return true; }
#endif // ACE_WIN32 || ACE_WIN64
  static std::string EthernetProtocolTypeIdToString (unsigned short); // ethernet frame type (in network (== big-endian) byte order)

  // *NOTE*: returns the ethernet 'MAC' address
  // *TODO*: support other link layers
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static struct ether_addr interfaceToLinkLayerAddress_2 (REFGUID); // interface identifier
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static struct _GUID linkLayerAddressToInterface_2 (const struct ether_addr&);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#endif // ACE_WIN32 || ACE_WIN64
  static struct ether_addr interfaceToLinkLayerAddress (const std::string&); // interface identifier
  static std::string linkLayerAddressToInterface (const struct ether_addr&);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static std::string interfaceToConnection (REFGUID);
  inline static std::string interfaceToString (REFGUID interfaceIdentifier_in) { return Net_Common_Tools::indexToInterface (Net_Common_Tools::interfaceToIndex_2 (interfaceIdentifier_in)); }
  static std::string interfaceToString (NET_IFINDEX); // interface identifier
#endif // ACE_WIN32 || ACE_WIN64

  // network layer
//  static ACE_INET_Addr getLocalAddress (std::string&); // interface
  static bool getAddress (std::string&,  // host name
                          std::string&); // dotted-decimal
  // *IMPORTANT NOTE*: return value is in host byte order
  static ACE_UINT32 getAddress (std::string&);  // host name

  // *NOTE*: 'gateway' really means the 'next hop' router with regard to some
  //         policy (e.g. routing table entry metric/priority/...).
  //         Consequently, this API is non-functional at this point
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static ACE_INET_Addr getGateway (REFGUID); // interface identifier
#else
  static ACE_INET_Addr getGateway (const std::string&); // interface identifier ('FriendlyName')
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  static ACE_INET_Addr getGateway (const std::string&); // interface identifier
#endif // ACE_WIN32 || ACE_WIN64

  // *NOTE*: this returns the external (i.e. routable) IP address (for clients
  //         behind a (NATted-) gateway)
  // *TODO*: this implementation is broken; it does not consider the specific
  //         interface, but returns the external IP of the interface that
  //         happens to route the DNS resolution query (see below)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  static bool interfaceToExternalIPAddress (REFGUID,            // interface identifier
#else
  static bool interfaceToExternalIPAddress (const std::string&, // interface identifier ('FriendlyName')
#endif // _WIN32_WINNT_VISTA
#else
  static bool interfaceToExternalIPAddress (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                                            ACE_INET_Addr&);    // return value: external IP address
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline static bool interfaceToIPAddress_2 (REFGUID interfaceIdentifier_in, ACE_INET_Addr& IPAddress_out, ACE_INET_Addr& gatewayAddress_out) { return Net_Common_Tools::interfaceToIPAddress (Net_Common_Tools::indexToInterface (Net_Common_Tools::interfaceToIndex_2 (interfaceIdentifier_in)), IPAddress_out, gatewayAddress_out); }
#endif // ACE_WIN32 || ACE_WIN64
  static bool interfaceToIPAddress (const std::string&,     // device identifier
                                    ACE_INET_Addr&,         // return value: (first) IP address
                                    ACE_INET_Addr&);        // return value: (first) gateway IP address
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  inline static struct _GUID IPAddressToInterface_2 (const ACE_INET_Addr& IPAddress_in) { return (Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (Net_Common_Tools::IPAddressToInterface (IPAddress_in)))); }
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#endif // ACE_WIN32 || ACE_WIN64
  static std::string IPAddressToInterface (const ACE_INET_Addr&); // IP address

  inline static std::string IPAddressToString (const ACE_INET_Addr& address_in, bool addressOnly_in = false) { return Net_Common_Tools::IPAddressToString ((addressOnly_in ? 0 : ACE_HTONS (address_in.get_port_number ())), ACE_HTONL (address_in.get_ip_address ())); }
#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
  static std::string NetlinkAddressToString (const Net_Netlink_Addr&);

//  static std::string dump (struct nl_msg*);
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT
  // *NOTE*: if the first argument is '0', the trailing ":0" will be cropped
  //         from the return value
  static std::string IPAddressToString (unsigned short, // port (network byte order !)
                                        ACE_UINT32);    // IP address (network byte order !)
  static std::string IPProtocolToString (unsigned char); // protocol

  static bool matchIPAddress (std::string&); // dotted-decimal
  // *NOTE*: (see also: ace/INET_Addr.h:237)
  static ACE_INET_Addr stringToIPAddress (std::string&,        // host name (DNS name or dotted-decimal)
                                          unsigned short = 0); // port# if none provided (i.e. no trailing ":x")
  static bool isLocal (const ACE_INET_Addr&);

  // transport layer
  static std::string TransportLayerTypeToString (enum Net_TransportLayerType);

  // *** UDP ***

  // *NOTE*: uses sendto(); does not send message block continuations ATM
  static bool sendDatagram (const ACE_INET_Addr&, // local SAP (may be 'any')
                            const ACE_INET_Addr&, // remote SAP
                            ACE_Message_Block*);  // data

  // session layer (and above)
  static bool getHostname (std::string&); // return value: hostname

  static std::string URLToHostName (const std::string&, // URL
                                    bool = true,        // return protocol (if any) ?
                                    bool = true);       // return port (if any) ?

  // --- socket API ---

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_SDK_VERSION(0x0602) // _WIN32_WINNT_WIN8
  // *NOTE*: applies to TCP sockets only (see also: SO_MAX_MSG_SIZE)
  static bool setLoopBackFastPath (ACE_HANDLE); // socket handle
#endif // _WIN32_WINNT_WIN8
#endif // ACE_WIN32 || ACE_WIN64

  // MTU
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: configures packet fragmentation to support sending datagrams
  //         larger than the Path MTU (see: man ip(7))
  static bool setPathMTUDiscovery (ACE_HANDLE, // socket handle
                                   int);       // option
  static bool getPathMTU (const ACE_INET_Addr&, // destination address
                          unsigned int&);       // return value: (initial) path MTU
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_LINUX)
  static bool enableErrorQueue (ACE_HANDLE); // socket handle
#endif // ACE_LINUX
  static ACE_INET_Addr getBoundAddress (ACE_HANDLE);
  static int getProtocol (ACE_HANDLE); // socket handle

//  // stream
//  static std::string generateUniqueName (const std::string&); // prefix

#if defined (SSL_SUPPORT)
  // --- SSL ---
  // *IMPORTANT NOTE*: certificates must be in PEM format
  static bool setCertificates (const std::string&,       // certificate file (FQ-)path
                               const std::string&,       // private key file (FQ-)path
                               ACE_SSL_Context* = NULL); // context handle {NULL: global}
#endif // SSL_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools (const Net_Common_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools& operator= (const Net_Common_Tools&))
};

#endif
