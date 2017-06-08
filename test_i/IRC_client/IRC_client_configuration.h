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

#include "irc_configuration.h"
#include "irc_defines.h"

#include "irc_client_common.h"
#include "irc_client_defines.h"
#include "irc_client_network.h"
#include "irc_client_stream_common.h"

// forward declarations
struct IRC_Client_ConnectionConfiguration;
struct IRC_Client_CursesState;
struct IRC_Client_SessionState;
class IRC_Record;
struct IRC_ModuleHandlerConfiguration;
//class IRC_Client_Stream;
struct IRC_Stream_SessionData;
struct IRC_Client_UserData;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct IRC_Client_ConnectionConfiguration,
//                          struct IRC_Client_ConnectionState,
//                          IRC_Client_RuntimeStatistic_t,
//                          IRC_Client_Stream> IRC_Client_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct IRC_Client_ConnectionConfiguration,
                                 struct IRC_Client_SessionState,
                                 IRC_RuntimeStatistic_t,
                                 struct IRC_Client_UserData> IRC_Client_IConnection_Manager_t;

//struct IRC_Client_ConnectorConfiguration
//{
//  inline IRC_Client_ConnectorConfiguration ()
//   : /*configuration (NULL)
//   ,*/ connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//   //, statisticCollectionInterval (0)
//  {};
//
//  //struct IRC_Client_Configuration*              configuration;
//  IRC_Client_IConnection_Manager_t*      connectionManager;
//  struct IRC_Client_SocketHandlerConfiguration* socketHandlerConfiguration;
//  unsigned int                           statisticCollectionInterval; // statistics collecting interval (second(s)) [0: off]
//};

struct IRC_Client_ConnectionConfiguration;
struct IRC_Client_InputHandlerConfiguration
{
  inline IRC_Client_InputHandlerConfiguration ()
   : controller (NULL)
   , connectionConfiguration (NULL)
  {};

  IRC_IControl*                              controller;

  struct IRC_Client_ConnectionConfiguration* connectionConfiguration;
};

struct IRC_Client_Configuration
{
  inline IRC_Client_Configuration ()
   : parserConfiguration ()
   ///////////////////////////////////////
   , connectionConfigurations ()
   ///////////////////////////////////////
   , allocatorConfiguration ()
   , streamConfiguration ()
   ///////////////////////////////////////
   , protocolConfiguration ()
   ///////////////////////////////////////
   , cursesState (NULL)
   , encoding (IRC_PRT_DEFAULT_ENCODING)
   , groupID (COMMON_EVENT_THREAD_GROUP_ID)
   , logToFile (IRC_CLIENT_SESSION_DEFAULT_LOG)
   , useReactor (NET_EVENT_USE_REACTOR)
   ///////////////////////////////////////
   , userData ()
  {};

  // ****************************** parser *************************************
  struct Common_ParserConfiguration     parserConfiguration;
  // ****************************** socket *************************************
  IRC_Client_ConnectionConfigurations_t connectionConfigurations;
  // ****************************** stream *************************************
  struct IRC_AllocatorConfiguration     allocatorConfiguration;
  struct IRC_Client_StreamConfiguration streamConfiguration;
  // ***************************** protocol ************************************
  struct IRC_ProtocolConfiguration      protocolConfiguration;
  // ***************************************************************************
  // *TODO*: move this somewhere else
  struct IRC_Client_CursesState*        cursesState;
  // *NOTE*: see also https://en.wikipedia.org/wiki/Internet_Relay_Chat#Character_encoding
  // *TODO*: implement support for 7-bit ASCII (as it is the most compatible
  //         encoding)
  enum IRC_CharacterEncoding            encoding;
  int                                   groupID;
  bool                                  logToFile;
  bool                                  useReactor;

  struct IRC_Client_UserData            userData;
};

#endif
