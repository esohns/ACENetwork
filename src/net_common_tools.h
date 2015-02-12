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

#ifndef Net_COMMON_TOOLS_H
#define Net_COMMON_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Signal.h"

#include "net_exports.h"
#include "net_remote_comm.h"

// forward declaration(s)
struct dirent;

class Net_Export Net_Common_Tools
{
 public:
  // --- general tools ---
  // *NOTE*: if (the first argument == 0), the trailing ":0" will be cropped from the return value !
  static std::string IPAddress2String(const unsigned short&, // port (network byte order !)
                                      const unsigned int&); // IP address (network byte order !)
  static std::string IPProtocol2String(const unsigned char&); // protocol
  static std::string MACAddress2String(const char* const); // pointer to message data (START of ethernet header address field !)
  static std::string EthernetProtocolTypeID2String(const unsigned short&); // ethernet frame type (network byte order !)

//   static const bool selectNetworkInterface(const std::string&, // default interface identifier
//                                            std::string&);      // return value: interface identifier
  static bool retrieveLocalIPAddress(const std::string&, // interface identifier
                                     std::string&);      // return value: IP address (dotted-decimal)
  static bool retrieveLocalHostname(std::string&); // return value: hostname
  static bool setSocketBuffer(const ACE_HANDLE&, // socket handle
                              const int&,        // option (SO_RCVBUF || SO_SNDBUF)
                              const int&);       // size (bytes)
  // *NOTE*: this should toggle Nagle's algorithm
  static bool setNoDelay(const ACE_HANDLE&, // socket handle
                         const bool&);      // TCP_NODELAY ?
  static bool setKeepAlive(const ACE_HANDLE&,    // socket handle
                           const bool&);         // SO_KEEPALIVE ?
  static bool setLinger(const ACE_HANDLE&,    // socket handle
                        const unsigned int&); // seconds {0 --> off}

 private:
  ACE_UNIMPLEMENTED_FUNC(Net_Common_Tools());
  ACE_UNIMPLEMENTED_FUNC(virtual ~Net_Common_Tools());
  ACE_UNIMPLEMENTED_FUNC(Net_Common_Tools(const Net_Common_Tools&));
  ACE_UNIMPLEMENTED_FUNC(Net_Common_Tools& operator=(const Net_Common_Tools&));
};

#endif
