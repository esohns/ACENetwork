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

#ifndef NET_COMMON_H
#define NET_COMMON_H

#include "ace/INET_Addr.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/Netlink_Addr.h"
#endif
#include "ace/Svc_Handler.h"

#include "net_defines.h"
//#include "net_itransportlayer.h"

// forward declarations
template <typename ConfigurationType>
class Net_ITransportLayer_T;

enum Net_ClientServerRole
{
  NET_ROLE_INVALID = -1,
  NET_ROLE_CLIENT = 0,
  NET_ROLE_SERVER,
  ///////////////////////////////////////
  NET_ROLE_MAX
};

enum Net_TransportLayerType
{
  NET_TRANSPORTLAYER_INVALID = -1,
  NET_TRANSPORTLAYER_IP_BROADCAST = 0,
  NET_TRANSPORTLAYER_IP_MULTICAST,
  NET_TRANSPORTLAYER_NETLINK,
  NET_TRANSPORTLAYER_TCP,
  NET_TRANSPORTLAYER_UDP,
  ///////////////////////////////////////
  NET_TRANSPORTLAYER_MAX
};

// *NOTE*: this extends ACE_Svc_Handler_Close (see Svc_Handler.h)
enum Net_Connection_CloseReason
{
  NET_CONNECTION_CLOSE_REASON_INVALID = CLOSE_DURING_NEW_CONNECTION,
  ///////////////////////////////////////
  NET_CONNECTION_CLOSE_REASON_INITIALIZATION,
  NET_CONNECTION_CLOSE_REASON_USER_ABORT,
  ///////////////////////////////////////
  NET_CONNECTION_CLOSE_REASON_MAX
};

enum Net_Connection_Status
{
  NET_CONNECTION_STATUS_INVALID = -1,
  ///////////////////////////////////////
  NET_CONNECTION_STATUS_OK = 0,
  ///////////////////////////////////////
  NET_CONNECTION_STATUS_INITIALIZATION_FAILED,
  ///////////////////////////////////////
  NET_CONNECTION_STATUS_MAX
};

struct Net_SocketConfiguration
{
  inline Net_SocketConfiguration ()
   : bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
   , linger (NET_SOCKET_DEFAULT_LINGER)
 #if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    , netlinkAddress (ACE_sap_any_cast (const ACE_Netlink_Addr&))
    , netlinkProtocol (NET_PROTOCOL_DEFAULT_NETLINK)
 #endif
   , peerAddress (ACE_sap_any_cast (const ACE_INET_Addr&))
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {};

  int              bufferSize; // socket buffer size (I/O)
  bool             linger;
  // *TODO*: remove address information (pass as AddressType in open() instead)
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  ACE_Netlink_Addr netlinkAddress;
  int              netlinkProtocol;
#endif
  ACE_INET_Addr    peerAddress;
  bool             useLoopBackDevice;
  // *TODO*: add network interface specifier (interface index on linux, (G)UID
  //         on windows)
};

typedef Net_ITransportLayer_T<Net_SocketConfiguration> Net_ITransportLayer_t;

#endif
