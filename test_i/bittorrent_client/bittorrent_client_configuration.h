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

#include "common_defines.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"

#include "bittorrent_client_defines.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

#include "test_i_defines.h"

struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_ModuleHandlerConfiguration;
struct BitTorrent_Client_UserData
{
  inline BitTorrent_Client_UserData ()
   : configuration (NULL)
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  struct BitTorrent_Client_Configuration*              configuration;

  // *TODO*: remove these ASAP
  struct Stream_ModuleConfiguration*                   moduleConfiguration;
  struct BitTorrent_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct BitTorrent_Client_CursesState;
struct BitTorrent_Client_Configuration
{
  inline BitTorrent_Client_Configuration ()
   : signalHandlerConfiguration ()
   ///////////////////////////////////////
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , trackerSocketHandlerConfiguration ()
   ///////////////////////////////////////
//   , allocatorConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
   ///////////////////////////////////////
//   , protocolConfiguration ()
   , sessionConfiguration ()
   ///////////////////////////////////////
   , cursesState (NULL)
   , groupID (COMMON_EVENT_THREAD_GROUP_ID)
   , logToFile (TEST_I_DEFAULT_SESSION_LOG)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // ****************************** signal *************************************
  struct BitTorrent_Client_SignalHandlerConfiguration signalHandlerConfiguration;
  // ****************************** socket *************************************
  struct Net_SocketConfiguration                      socketConfiguration;
  // *TODO*: this is a workaround to accomodate different message allocators for
  //         each connection type. There must be a better way to implement this
  struct BitTorrent_Client_SocketHandlerConfiguration socketHandlerConfiguration;
  struct BitTorrent_Client_SocketHandlerConfiguration trackerSocketHandlerConfiguration;
  // ****************************** stream *************************************
//  struct BitTorrent_AllocatorConfiguration            allocatorConfiguration;
  struct Stream_ModuleConfiguration                   moduleConfiguration;
  struct BitTorrent_Client_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct BitTorrent_Client_StreamConfiguration        streamConfiguration;
  struct BitTorrent_Client_UserData*                  userData;
  // ***************************** protocol ************************************
//  struct BitTorrent_ProtocolConfiguration             protocolConfiguration;
  struct BitTorrent_Client_SessionConfiguration       sessionConfiguration;
  // ***************************************************************************
  // *TODO*: move this somewhere else
  struct BitTorrent_Client_CursesState*               cursesState;
  int                                                 groupID;
  bool                                                logToFile;
  bool                                                useReactor;
};

#endif
