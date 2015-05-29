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

#include <string>

#include "ace/Global_Macros.h"

#include "net_connection_manager_common.h"
#include "net_exports.h"

class Net_Export Net_Common_Tools
{
 public:
  // --- general tools ---
  // *NOTE*: if (the first argument == 0), the trailing ":0" will be cropped from the return value !
  static std::string IPAddress2String (unsigned short, // port (network byte order !)
                                       unsigned int);  // IP address (network byte order !)
  static std::string IPProtocol2String (unsigned char); // protocol
  static std::string MACAddress2String (const char* const); // pointer to message data (START of ethernet header address field !)
  static std::string EthernetProtocolTypeID2String (unsigned short); // ethernet frame type (network byte order !)

//   static const bool selectNetworkInterface(const std::string&, // default interface identifier
//                                            std::string&);      // return value: interface identifier
  static bool getIPAddress (const std::string&, // interface identifier
                            std::string&);      // return value: IP address (dotted-decimal)
  static bool getHostname (std::string&); // return value: hostname
  static bool setSocketBuffer (ACE_HANDLE, // socket handle
                               int,        // option (SO_RCVBUF || SO_SNDBUF)
                               int);       // size (bytes)
  // *NOTE*: this should toggle Nagle's algorithm
  static bool setNoDelay (ACE_HANDLE, // socket handle
                          bool);      // TCP_NODELAY ?
  static bool setKeepAlive (ACE_HANDLE, // socket handle
                            bool);      // SO_KEEPALIVE ?
  static bool setLinger (ACE_HANDLE,  // socket handle
                         bool,        // on ? : off
                         int = -1);  // seconds {0  --> send RST on close,
                                     //          -1 --> reuse current value}

  static Net_IInetConnectionManager_t* getConnectionManager ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools ());
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Common_Tools ());
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools (const Net_Common_Tools&));
  ACE_UNIMPLEMENTED_FUNC (Net_Common_Tools& operator= (const Net_Common_Tools&));
};

#endif
