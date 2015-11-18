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
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/Netlink_Addr.h"
#endif

#include "stream_common.h"
#include "stream_iallocator.h"

#include "net_common.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

// forward declarations
struct Net_Configuration;
struct Net_ConnectionState;
typedef Stream_Statistic Net_RuntimeStatistic_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Net_Configuration,
                                 Net_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 ////////
                                 Net_UserData> Net_IInetConnectionManager_t;

struct Net_ProtocolConfiguration
{
  inline Net_ProtocolConfiguration ()
   : bufferSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , peerPingInterval (0)
   , pingAutoAnswer (true)
   , printPongMessages (true)
  {};

  int          bufferSize; // pdu size (if fixed)
  unsigned int peerPingInterval; // ms [0: off]
  bool         pingAutoAnswer;
  bool         printPongMessages;
};

struct Net_ModuleHandlerConfiguration
 : public Stream_ModuleHandlerConfiguration
{
  inline Net_ModuleHandlerConfiguration ()
   : active (false)
   , sessionData (NULL)
  {};

  bool                active;
  // *TODO*: remove this (--> session message data)
  Stream_SessionData* sessionData;
};

struct Net_StreamConfiguration
 : Stream_Configuration
{
  inline Net_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration_2 ()
   , moduleHandlerConfiguration_2 ()
   , protocolConfiguration (NULL)
   , userData (NULL)
  {
    bufferSize = NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
  };

  Stream_ModuleConfiguration     moduleConfiguration_2;        // module configuration
  Net_ModuleHandlerConfiguration moduleHandlerConfiguration_2; // module handler configuration
  Net_ProtocolConfiguration*     protocolConfiguration;        // protocol configuration

  Net_UserData*                  userData;                     // user data
};

struct Net_Configuration
{
  inline Net_Configuration ()
   : socketConfiguration ()
   , socketHandlerConfiguration ()
   , streamConfiguration ()
   , protocolConfiguration ()
   , userData ()
  {};

  // **************************** socket data **********************************
  Net_SocketConfiguration        socketConfiguration;
  Net_SocketHandlerConfiguration socketHandlerConfiguration;
  // **************************** stream data **********************************
  Net_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************
  Net_ProtocolConfiguration      protocolConfiguration;
  Net_UserData                   userData;
};

struct Net_ListenerConfiguration
{
  inline Net_ListenerConfiguration ()
   : address (static_cast<unsigned short> (0), INADDR_NONE)
   , addressFamily (ACE_ADDRESS_FAMILY_INET)
   , connectionManager (NULL)
   , messageAllocator (NULL)
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (0)
   , useLoopBackDevice (false)
  {};

  ACE_INET_Addr                   address;
  int                             addressFamily;
  Net_IInetConnectionManager_t*   connectionManager;
  Stream_IAllocator*              messageAllocator;
  Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  unsigned int                    statisticReportingInterval; // (second(s)) [0: off]
  bool                            useLoopBackDevice;
};

#endif
