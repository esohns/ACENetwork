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

#include "stream_defines.h"

#include "net_defines.h"
//#include "net_itransportlayer.h"

// forward declarations
class Stream_IAllocator;
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

struct Net_UserData
{
  inline Net_UserData ()
   : /*configuration (NULL)
   ,*/ userData (NULL)
  {};

  //Net_Configuration* configuration;
  void* userData;
};

struct Net_SocketConfiguration
{
  inline Net_SocketConfiguration ()
   : bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
   , linger (NET_SOCKET_DEFAULT_LINGER)
 #if defined (ACE_WIN32) || defined (ACE_WIN64)
 #else
   , netlinkAddress (ACE_sap_any_cast (const ACE_Netlink_Addr&))
   , netlinkProtocol (NET_PROTOCOL_DEFAULT_NETLINK)
 #endif
   , peerAddress (static_cast<u_short> (0), 
                  static_cast<ACE_UINT32> (INADDR_ANY))
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
   , writeOnly (false)
  {
    int result = -1;
    if (useLoopBackDevice)
    {
      result = peerAddress.set (static_cast<unsigned short> (0),
                                INADDR_LOOPBACK,
                                1,
                                0);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
    } // end IF
  };

  int              bufferSize; // socket buffer size (I/O)
  bool             linger;
  // *TODO*: remove address information (pass as AddressType in open() instead)
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  ACE_Netlink_Addr netlinkAddress;
  int              netlinkProtocol;
#endif
  ACE_INET_Addr    peerAddress;
  bool             useLoopBackDevice;
  bool             writeOnly; // UDP ?
  // *TODO*: add network interface specifier (interface index on linux, (G)UID
  //         on windows)
};

struct Net_SocketHandlerConfiguration
{
  inline Net_SocketHandlerConfiguration ()
   : messageAllocator (NULL)
   , PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , socketConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING)
   //////////////////////////////////////
   , userData (NULL)
  {};

  Stream_IAllocator*       messageAllocator;
  // *NOTE*: applies to the corresponding protocol datagram, if it has fixed
  //         size; otherwise, this is the size of the individual (amorphuous)
  //         stream buffers
  unsigned int             PDUSize; // package data unit size
  Net_SocketConfiguration* socketConfiguration;
  unsigned int             statisticReportingInterval; // seconds [0: off]

  Net_UserData*            userData;
};

typedef Net_ITransportLayer_T<Net_SocketConfiguration> Net_ITransportLayer_t;

#endif
