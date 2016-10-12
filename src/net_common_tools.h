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

#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>

#include "net_exports.h"

//class Net_Export Net_Common_Tools
class Net_Common_Tools
{
 public:
  // --- general tools ---
  // *NOTE*: if (the first argument == 0), the trailing ":0" will be cropped
  //         from the return value
  static std::string IPAddress2String (unsigned short, // port (network byte order !)
                                       unsigned int);  // IP address (network byte order !)
  static std::string IPProtocol2String (unsigned char); // protocol
  static std::string MACAddress2String (const unsigned char* const); // pointer to message data (START of ethernet header address field !)
  static std::string EthernetProtocolTypeID2String (unsigned short); // ethernet frame type (network (== big-endian) byte order !)

//   static const bool selectNetworkInterface(const std::string&, // default interface identifier
//                                            std::string&);      // return value: interface identifier
  static bool interface2IPAddress (const std::string&, // interface identifier
                                   ACE_INET_Addr&);    // return value: (first) IP address
  // *WARNING*: ensure that the array argument can hold at least 6 bytes !
  static bool interface2MACAddress (const std::string&, // interface identifier
                                    unsigned char[]);   // return value: MAC address
  static bool getAddress (std::string&,  // host name
                          std::string&); // dotted-decimal
  static bool getHostname (std::string&); // return value: hostname

  // --- socket options ---
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

#if defined (ACE_LINUX)
  static bool enableErrorQueue (ACE_HANDLE); // socket handle
#endif
  static int getProtocol (ACE_HANDLE); // socket handle

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: applies to TCP sockets (see also: SO_MAX_MSG_SIZE)
  static bool setLoopBackFastPath (ACE_HANDLE); // socket handle
#endif

//  static Net_IInetConnectionManager_t* getConnectionManager ();

  static std::string URL2HostName (const std::string&, // URL
                                   bool = true,        // return protocol (if any) ?
                                   bool = true);       // return port (if any) ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Common_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools (const Net_Common_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools& operator= (const Net_Common_Tools&))
};

#endif
