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

#ifndef IRC_CLIENT_CONFIGURATION_H
#define IRC_CLIENT_CONFIGURATION_H

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "IRC_client_common.h"
#include "IRC_client_defines.h"

// forward declarations
class IRC_Client_IIRCControl;
class IRC_Client_Stream;
struct IRC_Client_Configuration;
struct IRC_Client_CursesState;
struct IRC_Client_SessionData;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Net_SocketConfiguration,
                                 IRC_Client_Configuration,
                                 IRC_Client_SessionData,
                                 Stream_Statistic,
                                 IRC_Client_Stream> IRC_Client_IConnection_Manager_t;

typedef Stream_Statistic IRC_Client_RuntimeStatistic_t;
struct IRC_Client_SessionData
{
  inline IRC_Client_SessionData ()
   : currentStatistics ()
   , lastCollectionTimestamp (ACE_Time_Value::zero)
  {};

  IRC_Client_RuntimeStatistic_t currentStatistics;
  ACE_Time_Value                lastCollectionTimestamp;
};

struct IRC_Client_SocketHandlerConfiguration
{
  inline IRC_Client_SocketHandlerConfiguration ()
   : bufferSize (IRC_CLIENT_BUFFER_SIZE)
   , connectionManager (NULL)
   , messageAllocator (NULL)
   , socketConfiguration ()
   , statisticCollectionInterval (0)
  {};

  int                               bufferSize; // pdu size (if fixed)
  IRC_Client_IConnection_Manager_t* connectionManager;
  Stream_IAllocator*                messageAllocator;
  Net_SocketConfiguration           socketConfiguration;
  unsigned int                      statisticCollectionInterval; // seconds [0: OFF]
};

struct IRC_Client_ProtocolConfiguration
{
  inline IRC_Client_ProtocolConfiguration ()
   : automaticPong (IRC_CLIENT_STREAM_DEF_AUTOPONG)
   , loginOptions ()
   , printPingDot (IRC_CLIENT_DEF_PRINT_PINGDOT)
  {};

  bool                       automaticPong;
  IRC_Client_IRCLoginOptions loginOptions;
  bool                       printPingDot;
};

struct IRC_Client_StreamModuleConfiguration
{
  inline IRC_Client_StreamModuleConfiguration ()
   : connection (NULL)
   , moduleConfiguration ()
  {};

  IRC_Client_IConnection_t*  connection;
  Stream_ModuleConfiguration moduleConfiguration;
};

struct IRC_Client_StreamConfiguration
{
  inline IRC_Client_StreamConfiguration ()
   : crunchMessageBuffers (IRC_CLIENT_DEF_CRUNCH_MESSAGES)
   , debugScanner (IRC_CLIENT_DEF_LEX_TRACE)
   , debugParser (IRC_CLIENT_DEF_YACC_TRACE)
   , sessionID (0)
   , streamConfiguration ()
   , streamModuleConfiguration ()
   , sessionData (NULL)
   , protocolConfiguration (NULL)
  {};

  bool                                 crunchMessageBuffers;      // crunch message buffers ?
  bool                                 debugScanner;              // debug yacc ?
  bool                                 debugParser;               // debug lex ?
  unsigned int                         sessionID;                 // session ID
  Stream_Configuration                 streamConfiguration;       // stream configuration
  IRC_Client_StreamModuleConfiguration streamModuleConfiguration; // stream module configuration
  IRC_Client_SessionData*              sessionData;               // session data
  IRC_Client_ProtocolConfiguration*    protocolConfiguration;     // protocol configuration
};

struct IRC_Client_InputHandlerConfiguration
{
  inline IRC_Client_InputHandlerConfiguration ()
   : IRCSessionState (NULL)
   , streamConfiguration (NULL)
  {};

  IRC_Client_SessionState* IRCSessionState;
  Stream_Configuration*    streamConfiguration;
};

struct IRC_Client_Configuration
{
  inline IRC_Client_Configuration ()
   : socketConfiguration ()
   //////////////////////////////////////
   , streamConfiguration ()
   //, streamSessionData ()
   //////////////////////////////////////
   , protocolConfiguration ()
   //////////////////////////////////////
   , cursesState (NULL)
   , groupID (COMMON_EVENT_DISPATCH_THREAD_GROUP_ID)
   , logToFile (IRC_CLIENT_SESSION_DEF_LOG)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // **************************** socket data **********************************
  Net_SocketConfiguration          socketConfiguration;
  // **************************** stream data **********************************
  IRC_Client_StreamConfiguration   streamConfiguration;
  //IRC_Client_SessionData*          streamSessionData;
  // *************************** protocol data *********************************
  IRC_Client_ProtocolConfiguration protocolConfiguration;
  // ***************************************************************************
  IRC_Client_CursesState*          cursesState;
  int                              groupID;
  bool                             logToFile;
  bool                             useReactor;
};

#endif
