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

#include "test_i_defines.h"

#include "bittorrent_client_session_common.h"
#include "bittorrent_client_stream_common.h"

struct BitTorrent_Client_PeerConnectionConfiguration;
struct BitTorrent_Client_PeerModuleHandlerConfiguration;
struct BitTorrent_Client_PeerUserData
{
  inline BitTorrent_Client_PeerUserData ()
   : configuration (NULL)
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  struct BitTorrent_Client_PeerConnectionConfiguration*    configuration;

  // *TODO*: remove these ASAP
  struct Stream_ModuleConfiguration*                       moduleConfiguration;
  struct BitTorrent_Client_PeerModuleHandlerConfiguration* moduleHandlerConfiguration;
};
struct BitTorrent_Client_TrackerConnectionConfiguration;
struct BitTorrent_Client_TrackerModuleHandlerConfiguration;
struct BitTorrent_Client_TrackerUserData
{
  inline BitTorrent_Client_TrackerUserData ()
   : configuration (NULL)
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  struct BitTorrent_Client_TrackerConnectionConfiguration*    configuration;

  // *TODO*: remove these ASAP
  struct Stream_ModuleConfiguration*                          moduleConfiguration;
  struct BitTorrent_Client_TrackerModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct BitTorrent_Client_CursesState;
struct BitTorrent_Client_Configuration
{
  inline BitTorrent_Client_Configuration ()
   : signalHandlerConfiguration ()
   ///////////////////////////////////////
   , socketConfiguration ()
   , peerSocketHandlerConfiguration ()
   , trackerSocketHandlerConfiguration ()
   , peerConnectionConfiguration ()
   , trackerConnectionConfiguration ()
   ///////////////////////////////////////
//   , allocatorConfiguration ()
   , moduleConfiguration ()
   , peerModuleHandlerConfiguration ()
   , peerStreamConfiguration ()
   , trackerModuleHandlerConfiguration ()
   , trackerStreamConfiguration ()
   , peerUserData ()
   , trackerUserData ()
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
  struct BitTorrent_Client_SignalHandlerConfiguration        signalHandlerConfiguration;
  // ****************************** socket *************************************
  struct Net_SocketConfiguration                             socketConfiguration;
  // *TODO*: workaround to accomodate different message allocators for each connection type.
  //         --> there must be a better way to implement this
  struct BitTorrent_Client_PeerSocketHandlerConfiguration    peerSocketHandlerConfiguration;
  struct BitTorrent_Client_TrackerSocketHandlerConfiguration trackerSocketHandlerConfiguration;
  struct BitTorrent_Client_PeerConnectionConfiguration       peerConnectionConfiguration;
  struct BitTorrent_Client_TrackerConnectionConfiguration    trackerConnectionConfiguration;
  // ****************************** stream *************************************
//  struct BitTorrent_AllocatorConfiguration            allocatorConfiguration;
  struct Stream_ModuleConfiguration                          moduleConfiguration;
  struct BitTorrent_Client_PeerModuleHandlerConfiguration    peerModuleHandlerConfiguration;
  struct BitTorrent_Client_PeerStreamConfiguration           peerStreamConfiguration;
  struct BitTorrent_Client_TrackerModuleHandlerConfiguration trackerModuleHandlerConfiguration;
  struct BitTorrent_Client_TrackerStreamConfiguration        trackerStreamConfiguration;
  struct BitTorrent_Client_PeerUserData                      peerUserData;
  struct BitTorrent_Client_TrackerUserData                   trackerUserData;
  // ***************************** protocol ************************************
//  struct BitTorrent_ProtocolConfiguration             protocolConfiguration;
  struct BitTorrent_Client_SessionConfiguration              sessionConfiguration;
  // ***************************************************************************
  // *TODO*: move this somewhere else
  struct BitTorrent_Client_CursesState*                      cursesState;
  int                                                        groupID;
  bool                                                       logToFile;
  bool                                                       useReactor;
};

#endif
