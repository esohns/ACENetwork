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

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_defines.h"
#include "net_iconnectionmanager.h"

//#include "net_configuration.h"

#include "irc_configuration.h"

#include "IRC_client_common.h"
#include "IRC_client_defines.h"
#include "IRC_client_stream_common.h"

// forward declarations
struct IRC_Client_Configuration;
struct IRC_Client_CursesState;
struct IRC_Client_SessionState;
class IRC_Record;
struct IRC_ModuleHandlerConfiguration;
class IRC_SessionMessage;
//class IRC_Client_Stream;
struct IRC_Stream_SessionData;
struct IRC_Client_UserData;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          IRC_Client_Configuration,
//                          IRC_Client_ConnectionState,
//                          IRC_Client_RuntimeStatistic_t,
//                          IRC_Client_Stream> IRC_Client_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 IRC_Client_Configuration,
                                 IRC_Client_SessionState,
                                 IRC_RuntimeStatistic_t,
                                 ////////
                                 IRC_Client_UserData> IRC_Client_IConnection_Manager_t;

//struct IRC_Client_ConnectorConfiguration
//{
//  inline IRC_Client_ConnectorConfiguration ()
//   : /*configuration (NULL)
//   ,*/ connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//   //, statisticCollectionInterval (0)
//  {};
//
//  //IRC_Client_Configuration*              configuration;
//  IRC_Client_IConnection_Manager_t*      connectionManager;
//  IRC_Client_SocketHandlerConfiguration* socketHandlerConfiguration;
//  unsigned int                           statisticCollectionInterval; // statistics collecting interval (second(s)) [0: off]
//};

struct IRC_Client_SocketHandlerConfiguration
 : IRC_SocketHandlerConfiguration
{
  inline IRC_Client_SocketHandlerConfiguration ()
   : IRC_SocketHandlerConfiguration ()
   //////////////////////////////////////
   , userData (NULL)
  {};

  IRC_Client_UserData* userData;
};

struct IRC_Client_ModuleHandlerConfiguration
 : IRC_ModuleHandlerConfiguration
{
  inline IRC_Client_ModuleHandlerConfiguration ()
   : IRC_ModuleHandlerConfiguration ()
   //////////////////////////////////////
   , subscriber (NULL)
   , userData (NULL)
  {};

  /* handler */
  IRC_Client_IStreamNotify_t* subscriber; // (initial) subscriber
  IRC_Client_UserData*        userData;
};

struct IRC_Client_StreamConfiguration
 : Stream_Configuration
{
  inline IRC_Client_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration_2 ()
   , moduleHandlerConfiguration_2 ()
   , protocolConfiguration (NULL)
   , userData (NULL)
  {
    bufferSize = IRC_BUFFER_SIZE;
  };

  Stream_ModuleConfiguration            moduleConfiguration_2;        // stream module configuration
  IRC_Client_ModuleHandlerConfiguration moduleHandlerConfiguration_2; // module handler configuration
  IRC_ProtocolConfiguration*            protocolConfiguration;        // protocol configuration

  IRC_Client_UserData*                  userData;
};

struct IRC_Client_InputHandlerConfiguration
{
  inline IRC_Client_InputHandlerConfiguration ()
   : controller (NULL)
   , streamConfiguration (NULL)
  {};

  IRC_Client_IControl_t* controller;
  Stream_Configuration*  streamConfiguration;
};

struct IRC_Client_Configuration
{
  inline IRC_Client_Configuration ()
   : socketConfiguration ()
   , socketHandlerConfiguration ()
   //////////////////////////////////////
   , streamConfiguration ()
   , userData ()
   //////////////////////////////////////
   , protocolConfiguration ()
   //////////////////////////////////////
   , cursesState (NULL)
   , encoding (IRC_DEF_ENCODING)
   , groupID (COMMON_EVENT_THREAD_GROUP_ID)
   , logToFile (IRC_SESSION_DEF_LOG)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // ****************************** socket *************************************
  Net_SocketConfiguration               socketConfiguration;
  IRC_Client_SocketHandlerConfiguration socketHandlerConfiguration;
  // ****************************** stream *************************************
  IRC_Client_StreamConfiguration        streamConfiguration;
  IRC_Client_UserData*                  userData;
  // ***************************** protocol ************************************
  IRC_ProtocolConfiguration             protocolConfiguration;
  // ***************************************************************************
  // *TODO*: move this somewhere else
  IRC_Client_CursesState*               cursesState;
  // *NOTE*: see also https://en.wikipedia.org/wiki/Internet_Relay_Chat#Character_encoding
  // *TODO*: implement support for 7-bit ASCII (as it is the most compatible
  //         encoding)
  IRC_CharacterEncoding                 encoding;
  int                                   groupID;
  bool                                  logToFile;
  bool                                  useReactor;
};

#endif
