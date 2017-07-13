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

#ifndef NET_CONFIGURATION_H
#define NET_CONFIGURATION_H

#include <map>
#include <string>

#include "ace/Basic_Types.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/Time_Value.h"

#include "net_common.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

// forward declarations
struct Net_UserData;
class Stream_IAllocator;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct Net_ConnectionConfiguration,
                                 struct Net_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 struct Net_UserData> Net_IInetConnectionManager_t;

struct Net_SocketConfigurationBase
{
  inline Net_SocketConfigurationBase ()
   : bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
   , networkInterface (ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET))
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {};
  inline virtual ~Net_SocketConfigurationBase () {};

  int         bufferSize; // socket buffer size (I/O)
  std::string networkInterface; // NIC identifier
  bool        useLoopBackDevice;
};

#if defined (ACE_HAS_NETLINK)
struct Net_NetlinkSocketConfiguration
 : Net_SocketConfigurationBase
{
  inline Net_NetlinkSocketConfiguration ()
   : Net_SocketConfigurationBase ()
   , address ()
   , protocol (NET_PROTOCOL_DEFAULT_NETLINK)
  {};
  inline virtual ~Net_NetlinkSocketConfiguration () {};

  Net_Netlink_Addr address;
  int              protocol;
};
#endif

struct Net_TCPSocketConfiguration
 : Net_SocketConfigurationBase
{
  inline Net_TCPSocketConfiguration ()
   : Net_SocketConfigurationBase ()
   , address (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , linger (NET_SOCKET_DEFAULT_LINGER)
  {
    int result = -1;
    if (useLoopBackDevice)
    {
      result = address.set (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                            static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                            1,
                            0);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
    } // end IF
  };
  inline virtual ~Net_TCPSocketConfiguration () {};

  ACE_INET_Addr address;
  bool          linger;
};

struct Net_UDPSocketConfiguration
 : Net_SocketConfigurationBase
{
  inline Net_UDPSocketConfiguration ()
   : Net_SocketConfigurationBase ()
   , address (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , connect (NET_SOCKET_DEFAULT_UDP_CONNECT)
   // *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
   //                SO_LINGER
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , linger (NET_SOCKET_DEFAULT_LINGER)
#endif
   , listenAddress ()
   , sourcePort (0)
   , writeOnly (false)
  {
    int result = -1;
    if (useLoopBackDevice)
    {
      result = address.set (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
                            static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                            1,
                            0);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
    } // end IF
  };
  inline virtual ~Net_UDPSocketConfiguration () {};

  ACE_INET_Addr address;
  // *IMPORTANT NOTE*: set this for asynchronous event dispatch; the socket
  //                   needs to be associated with the peer address, as the data
  //                   dispatch happens out of context
  bool          connect;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  bool          linger;
#endif
  ACE_INET_Addr listenAddress;
  ACE_UINT16    sourcePort;
  bool          writeOnly;
};

//typedef std::deque<ACE_INET_Addr> Net_InetAddressStack_t;
//typedef Net_InetAddressStack_t::iterator Net_InetAddressStackIterator_t;

struct Net_ConnectionConfiguration;
struct Net_SocketHandlerConfiguration
{
  inline Net_SocketHandlerConfiguration ()
   : connectionConfiguration (NULL)
   , socketConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL,
                                 0)
   , useThreadPerConnection (false)
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Net_ConnectionConfiguration* connectionConfiguration;
  struct Net_SocketConfigurationBase* socketConfiguration;
  ACE_Time_Value                      statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                                useThreadPerConnection;

  struct Net_UserData*                userData;
};

struct Net_ListenerConfiguration
{
  inline Net_ListenerConfiguration ()
   : addressFamily (ACE_ADDRESS_FAMILY_INET)
   , socketHandlerConfiguration ()
  {};

  int                                   addressFamily;
  struct Net_SocketHandlerConfiguration socketHandlerConfiguration;
};

struct Stream_Configuration;
struct Net_ConnectionConfiguration
{
  inline Net_ConnectionConfiguration ()
   : connectionManager (NULL)
   , messageAllocator (NULL)
   , PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , socketHandlerConfiguration ()
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  Net_IInetConnectionManager_t*         connectionManager;
  Stream_IAllocator*                    messageAllocator;
  // *NOTE*: applies to the corresponding protocol, if it has fixed size
  //         datagrams; otherwise, this is the size of the individual (opaque)
  //         stream buffers
  unsigned int                          PDUSize; // package data unit size
  struct Net_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Stream_Configuration*          streamConfiguration;

  struct Net_UserData*                  userData;
};
typedef std::map<std::string,
                 struct Net_ConnectionConfiguration> Net_ConnectionConfigurations_t;
typedef Net_ConnectionConfigurations_t::iterator Net_ConnectionConfigurationIterator_t;

struct Common_ParserConfiguration;
struct Net_SessionConfiguration
{
  inline Net_SessionConfiguration ()
   : parserConfiguration (NULL)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  struct Common_ParserConfiguration* parserConfiguration;
  bool                               useReactor;
};

#endif
