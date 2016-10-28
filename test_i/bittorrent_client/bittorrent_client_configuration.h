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

#ifndef BITTORRENT_CLIENT_CONFIGURATION_H
#define BITTORRENT_CLIENT_CONFIGURATION_H

#include <string>

#include <ace/INET_Addr.h>
#include <ace/Time_Value.h>

#include "common_defines.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_defines.h"
#include "net_iconnectionmanager.h"

//#include "bittorrent_configuration.h"
#include "bittorrent_defines.h"

#include "bittorrent_client_common.h"
#include "bittorrent_client_defines.h"
#include "bittorrent_client_stream_common.h"

// forward declarations
struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_CursesState;
struct BitTorrent_Client_SessionState;
struct BitTorrent_Record;
struct BitTorrent_ModuleHandlerConfiguration;
class BitTorrent_SessionMessage;
//class BitTorrent_Client_Stream;
struct BitTorrent_SessionData;
struct BitTorrent_Client_UserData;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          BitTorrent_Client_Configuration,
//                          BitTorrent_Client_ConnectionState,
//                          BitTorrent_Client_RuntimeStatistic_t,
//                          BitTorrent_Client_Stream> BitTorrent_Client_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 BitTorrent_Client_Configuration,
                                 BitTorrent_Client_SessionState,
                                 BitTorrent_RuntimeStatistic_t,
                                 BitTorrent_Client_UserData> BitTorrent_Client_IConnection_Manager_t;

//struct BitTorrent_Client_ConnectorConfiguration
//{
//  inline BitTorrent_Client_ConnectorConfiguration ()
//   : /*configuration (NULL)
//   ,*/ connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//   //, statisticCollectionInterval (0)
//  {};
//
//  //BitTorrent_Client_Configuration*              configuration;
//  BitTorrent_Client_IConnection_Manager_t*      connectionManager;
//  BitTorrent_Client_SocketHandlerConfiguration* socketHandlerConfiguration;
//  unsigned int                           statisticCollectionInterval; // statistics collecting interval (second(s)) [0: off]
//};

struct BitTorrent_Client_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline BitTorrent_Client_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  BitTorrent_Client_UserData* userData;
};

struct BitTorrent_Client_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline BitTorrent_Client_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
   , subscriber (NULL)
   , userData (NULL)
  {};

  /* handler */
  BitTorrent_Client_ISessionNotify_t* subscriber; // (initial) subscriber
  BitTorrent_Client_UserData*         userData;
};

struct BitTorrent_Client_StreamConfiguration
 : Stream_Configuration
{
  inline BitTorrent_Client_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
   , protocolConfiguration (NULL)
   , userData (NULL)
  {
    bufferSize = IRC_BUFFER_SIZE;
  };

  Stream_ModuleConfiguration*            moduleConfiguration;        // stream module configuration
  BitTorrent_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration; // module handler configuration
  IRC_ProtocolConfiguration*             protocolConfiguration;      // protocol configuration

  BitTorrent_Client_UserData*                   userData;
};

struct BitTorrent_Client_InputHandlerConfiguration
{
  inline BitTorrent_Client_InputHandlerConfiguration ()
   : controller (NULL)
   , streamConfiguration (NULL)
  {};

  IRC_IControl*         controller;
  Stream_Configuration* streamConfiguration;
};

struct BitTorrent_Client_Configuration
{
  inline BitTorrent_Client_Configuration ()
   : socketConfiguration ()
   , socketHandlerConfiguration ()
   ///////////////////////////////////////
   , streamConfiguration ()
   , userData ()
   ///////////////////////////////////////
   , protocolConfiguration ()
   ///////////////////////////////////////
   , cursesState (NULL)
   , encoding (IRC_PRT_DEFAULT_ENCODING)
   , groupID (COMMON_EVENT_THREAD_GROUP_ID)
   , logToFile (BitTorrent_Client_SESSION_DEFAULT_LOG)
   , useReactor (BitTorrent_Client_DEFAULT_USE_REACTOR)
  {};

  // ****************************** socket *************************************
  Net_SocketConfiguration               socketConfiguration;
  BitTorrent_Client_SocketHandlerConfiguration socketHandlerConfiguration;
  // ****************************** stream *************************************
  Stream_ModuleConfiguration            moduleConfiguration;
  BitTorrent_Client_ModuleHandlerConfiguration moduleHandlerConfiguration;
  BitTorrent_Client_StreamConfiguration        streamConfiguration;
  BitTorrent_Client_UserData*                  userData;
  // ***************************** protocol ************************************
  IRC_ProtocolConfiguration             protocolConfiguration;
  // ***************************************************************************
  // *TODO*: move this somewhere else
  BitTorrent_Client_CursesState*               cursesState;
  // *NOTE*: see also https://en.wikipedia.org/wiki/Internet_Relay_Chat#Character_encoding
  // *TODO*: implement support for 7-bit ASCII (as it is the most compatible
  //         encoding)
  IRC_CharacterEncoding                 encoding;
  int                                   groupID;
  bool                                  logToFile;
  bool                                  useReactor;
};

#endif
