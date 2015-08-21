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

//#include "net_common.h"
//#include "net_connection_manager_common.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"
#include "net_stream_common.h"

// forward declarations
struct Net_Configuration;
struct Net_ConnectionState;
class Net_Stream;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Net_Configuration,
                                 Net_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 Net_Stream,
                                 ////////
                                 Net_StreamUserData> Net_IInetConnectionManager_t;

struct Net_SocketHandlerConfiguration
{
  inline Net_SocketHandlerConfiguration ()
   : bufferSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , messageAllocator (NULL)
   , socketConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING)
   //////////////////////////////////////
   , userData (NULL)
  {};

  int                      bufferSize; // pdu size (if fixed)
  Stream_IAllocator*       messageAllocator;
  Net_SocketConfiguration* socketConfiguration;
  unsigned int             statisticReportingInterval; // seconds [0: off]

  Net_StreamUserData*      userData;
};

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
   , sessionID (0)
   , userData (NULL)
  {
    bufferSize = NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
  };

  Stream_ModuleConfiguration     moduleConfiguration_2;        // module configuration
  Net_ModuleHandlerConfiguration moduleHandlerConfiguration_2; // module handler configuration
  Net_ProtocolConfiguration*     protocolConfiguration;        // protocol configuration
  unsigned int                   sessionID;                    // session ID

  Net_StreamUserData*            userData;                     // user data
};

struct Net_Configuration
{
  inline Net_Configuration ()
   : socketConfiguration ()
   , socketHandlerConfiguration ()
   , streamConfiguration ()
   , streamUserData ()
   , protocolConfiguration ()
  {};

  // **************************** socket data **********************************
  Net_SocketConfiguration        socketConfiguration;
  Net_SocketHandlerConfiguration socketHandlerConfiguration;
  // **************************** stream data **********************************
  Net_StreamConfiguration        streamConfiguration;
  Net_StreamUserData             streamUserData;
  // *************************** protocol data *********************************
  Net_ProtocolConfiguration      protocolConfiguration;
};

struct Net_ListenerConfiguration
{
  inline Net_ListenerConfiguration ()
   : addressFamily (ACE_ADDRESS_FAMILY_INET)
   , connectionManager (NULL)
   , messageAllocator (NULL)
   , portNumber (0)
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (0)
   , useLoopbackDevice (false)
  {};

  int                             addressFamily;
  Net_IInetConnectionManager_t*   connectionManager;
  Stream_IAllocator*              messageAllocator;
  unsigned short                  portNumber;
  Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  unsigned int                    statisticReportingInterval; // (second(s)) [0: off]
  bool                            useLoopbackDevice;
};

#endif
