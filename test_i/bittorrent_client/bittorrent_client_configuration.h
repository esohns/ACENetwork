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

#include "test_i_common.h"
#include "test_i_defines.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_i_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

struct BitTorrent_Client_PeerUserData
{
  BitTorrent_Client_PeerUserData ()
  {}
};
struct BitTorrent_Client_TrackerUserData
{
  BitTorrent_Client_TrackerUserData ()
  {}
};

struct BitTorrent_Client_CursesState;
struct BitTorrent_Client_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  BitTorrent_Client_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   ///////////////////////////////////////
   , peerConnectionConfigurations ()
   , trackerConnectionConfigurations ()
   ///////////////////////////////////////
   , parserConfiguration ()
   , peerStreamConfiguration ()
   , trackerStreamConfiguration ()
   ///////////////////////////////////////
//   , protocolConfiguration ()
   , sessionConfiguration ()
   ///////////////////////////////////////
#if defined (GUI_SUPPORT)
#if defined (CURSES_USE)
   , cursesState (NULL)
#endif // CURSES_USE
#endif // GUI_SUPPORT
   , groupId (COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1)
   , logToFile (TEST_I_DEFAULT_SESSION_LOG)
   , peerUserData ()
   , trackerUserData ()
  {}

  // ****************************** signal *************************************
  struct BitTorrent_Client_SignalHandlerConfiguration signalHandlerConfiguration;
  // ****************************** socket *************************************
  // *TODO*: workaround to accomodate different message allocators for each connection type.
  //         --> there must be a better way to implement this
  BitTorrent_Client_PeerConnectionConfigurations_t    peerConnectionConfigurations;
  BitTorrent_Client_TrackerConnectionConfigurations_t trackerConnectionConfigurations;
  // ****************************** stream *************************************
  struct Common_ParserConfiguration                   parserConfiguration;
  BitTorrent_Client_PeerStreamConfiguration_t         peerStreamConfiguration;
  BitTorrent_Client_TrackerStreamConfiguration_t      trackerStreamConfiguration;
  // ***************************** protocol ************************************
//  struct BitTorrent_ProtocolConfiguration             protocolConfiguration;
  struct BitTorrent_Client_SessionConfiguration       sessionConfiguration;
  // ***************************************************************************
#if defined (GUI_SUPPORT)
#if defined (CURSES_USE)
  // *TODO*: move this somewhere else
  struct BitTorrent_Client_CursesState*               cursesState;
#endif // CURSES_USE
#endif // GUI_SUPPORT
  int                                                 groupId;
  bool                                                logToFile;

  struct BitTorrent_Client_PeerUserData               peerUserData;
  struct BitTorrent_Client_TrackerUserData            trackerUserData;
};

#endif
