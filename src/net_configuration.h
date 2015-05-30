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

#include "ace/INET_Addr.h"
#include "ace/Netlink_Addr.h"

#include "stream_common.h"
#include "stream_iallocator.h"

#include "net_defines.h"

struct Net_SocketConfiguration_t
{
  inline Net_SocketConfiguration_t ()
   : bufferSize (NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE)
   , linger (NET_SOCKET_DEFAULT_LINGER)
 #if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    , netlinkAddress (ACE_sap_any_cast (const ACE_Netlink_Addr&))
    , netlinkProtocol (NET_PROTOCOL_DEFAULT_NETLINK)
 #endif
   , peerAddress (ACE_sap_any_cast (const ACE_INET_Addr&))
   , useLoopbackDevice (NET_INTERFACE_DEFAULT_USE_LOOPBACK)
  {};

  int                 bufferSize;
  bool                linger;
  // *TODO*: remove address information (pass as AddressType in open() instead)
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  ACE_Netlink_Addr    netlinkAddress;
  int                 netlinkProtocol;
#endif
  ACE_INET_Addr       peerAddress;
  bool                useLoopbackDevice;
  // *TODO*: add network interface specifier (interface index on linux, (G)UID on windows)
};

struct Net_SocketHandlerConfiguration_t
{
  inline Net_SocketHandlerConfiguration_t ()
   : bufferSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , messageAllocator (NULL)
   , socketConfiguration ()
  {};

  int                       bufferSize; // pdu size (if fixed)
  Stream_IAllocator*        messageAllocator;
  Net_SocketConfiguration_t socketConfiguration;
};

struct Net_ProtocolConfiguration_t
{
  inline Net_ProtocolConfiguration_t ()
   : bufferSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , peerPingInterval (0)
   , pingAutoAnswer (true)
   , printPongMessages (true)
  {};

  int          bufferSize; // pdu size (if fixed)
  unsigned int peerPingInterval; // ms {0 --> OFF}
  bool         pingAutoAnswer;
  bool         printPongMessages;
};

// *NOTE*: I speculate that this is the main reason that C# was ever invented !
struct Net_UserData_t
{
  inline Net_UserData_t ()
   : userData (NULL)
  {};

  void* userData;
};

struct Net_StreamConfiguration_t
{
  inline Net_StreamConfiguration_t ()
   : protocolConfiguration (NULL)
   , sessionID (0)
   , streamConfiguration ()
   , userData (NULL)
  {};

  Net_ProtocolConfiguration_t* protocolConfiguration; // protocol configuration
  unsigned int                 sessionID;             // session ID
  Stream_Configuration_t       streamConfiguration;   // stream configuration
  Net_UserData_t*              userData;              // user data
};

struct Net_Configuration_t
{
  inline Net_Configuration_t ()
   : socketConfiguration ()
   , streamConfiguration ()
   , streamSessionData ()
   , protocolConfiguration ()
  {};

  // **************************** socket data **********************************
  Net_SocketConfiguration_t   socketConfiguration;
  // **************************** stream data **********************************
  Net_StreamConfiguration_t   streamConfiguration;
  Net_UserData_t              streamSessionData;
  // *************************** protocol data *********************************
  Net_ProtocolConfiguration_t protocolConfiguration;
};

#endif
