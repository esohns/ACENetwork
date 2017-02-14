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

#include <string>

#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>

#include "net_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_netlinksockethandler.h"
#endif

// forward declarations
struct Net_UserData;
class Stream_IAllocator;

struct Net_SocketConfiguration
{
  inline Net_SocketConfiguration ()
   : address (static_cast<u_short> (0),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
   , connect (NET_SOCKET_DEFAULT_UDP_CONNECT)
   , linger (NET_SOCKET_DEFAULT_LINGER)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , netlinkAddress ()
   , netlinkProtocol (NET_PROTOCOL_DEFAULT_NETLINK)
#endif
   , networkInterface (ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET))
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
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

  ACE_INET_Addr    address;
  int              bufferSize; // socket buffer size (I/O)
  bool             connect; // UDP
  bool             linger;
  // *TODO*: remove address information (pass as AddressType in open() instead)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Net_Netlink_Addr netlinkAddress;
  int              netlinkProtocol;
#endif
  std::string      networkInterface; // NIC identifier
  bool             useLoopBackDevice;
  bool             writeOnly; // UDP
};

struct Net_SocketHandlerConfiguration;
struct Net_ListenerConfiguration
{
  inline Net_ListenerConfiguration ()
   : address (static_cast<u_short> (NET_ADDRESS_DEFAULT_PORT),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , addressFamily (ACE_ADDRESS_FAMILY_INET)
//   , connectionManager (NULL)
   , messageAllocator (NULL)
   , socketHandlerConfiguration (NULL)
   , useLoopBackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {};

  ACE_INET_Addr                          address;
  int                                    addressFamily;
//  Net_IInetConnectionManager_t*   connectionManager;
  Stream_IAllocator*                     messageAllocator;
  struct Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  bool                                   useLoopBackDevice;
};

struct Net_ConnectionConfiguration
{
  inline Net_ConnectionConfiguration ()
   : messageAllocator (NULL)
   , PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , socketHandlerConfiguration (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  Stream_IAllocator*                     messageAllocator;
  // *NOTE*: applies to the corresponding protocol, if it has fixed size
  //         datagrams; otherwise, this is the size of the individual (opaque)
  //         stream buffers
  unsigned int                           PDUSize; // package data unit size
  struct Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct Stream_Configuration*           streamConfiguration;

  struct Net_UserData*                   userData;
};

struct Net_SocketHandlerConfiguration
{
  inline Net_SocketHandlerConfiguration ()
   : listenerConfiguration (NULL)
   , messageAllocator (NULL)
   , PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , socketConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL,
                                 0)
   , useThreadPerConnection (false)
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Net_ListenerConfiguration* listenerConfiguration;
  Stream_IAllocator*                messageAllocator;
  unsigned int                      PDUSize;
  struct Net_SocketConfiguration*   socketConfiguration;
  ACE_Time_Value                    statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                              useThreadPerConnection;

  struct Net_UserData*              userData;
};

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
