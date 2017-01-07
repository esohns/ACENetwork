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

#ifndef BITTORRENT_CLIENT_GUI_COMMON_H
#define BITTORRENT_CLIENT_GUI_COMMON_H

#include <map>
#include <set>
#include <string>

#include <ace/INET_Addr.h>
#include <ace/os_include/os_pthread.h>
#include <ace/Synch_Traits.h>
#include <ace/Singleton.h>

#include <gtk/gtk.h>

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "net_common.h"
#include "net_iconnection.h"

#include "test_i_gtk_common.h"

#include "bittorrent_stream_common.h"

#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

// forward declaration(s)
struct BitTorrent_Client_PeerConnectionConfiguration;
struct BitTorrent_Client_PeerConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct BitTorrent_Client_PeerConnectionConfiguration,
                          struct BitTorrent_Client_PeerConnectionState,
                          BitTorrent_RuntimeStatistic_t> BitTorrent_Client_IPeerConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct BitTorrent_Client_TrackerConnectionConfiguration,
//                          struct BitTorrent_Client_ConnectionState,
//                          BitTorrent_RuntimeStatistic_t> BitTorrent_Client_ITrackerConnection_t;
template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
class BitTorrent_Client_GUI_Session_T;
struct BitTorrent_Client_GTK_SessionCBData;
typedef BitTorrent_Client_GUI_Session_T<BitTorrent_Client_ISession_t,
                                        BitTorrent_Client_IPeerConnection_t,
                                        struct BitTorrent_Client_GTK_SessionCBData> BitTorrent_Client_GUI_Session_t;

typedef std::map<std::string, BitTorrent_Client_GUI_Session_t*> BitTorrent_Client_GUI_Sessions_t;
typedef BitTorrent_Client_GUI_Sessions_t::iterator BitTorrent_Client_GUI_SessionsIterator_t;
typedef BitTorrent_Client_GUI_Sessions_t::const_iterator BitTorrent_Client_GUI_SessionsConstIterator_t;

typedef std::map<ACE_thread_t, guint> BitTorrent_Client_GUI_PendingActions_t;
typedef BitTorrent_Client_GUI_PendingActions_t::iterator BitTorrent_Client_GUI_PendingActionsIterator_t;
typedef std::set<ACE_thread_t> BitTorrent_Client_GUI_CompletedActions_t;
typedef BitTorrent_Client_GUI_CompletedActions_t::iterator BitTorrent_Client_GUI_CompletedActionsIterator_t;

struct BitTorrent_Client_GTK_ProgressData
 : Test_I_GTK_ProgressData
{
  inline BitTorrent_Client_GTK_ProgressData ()
   : Test_I_GTK_ProgressData ()
   , completedActions ()
   , pendingActions ()
   , transferred (0)
  {};

  BitTorrent_Client_GUI_CompletedActions_t completedActions;
  BitTorrent_Client_GUI_PendingActions_t   pendingActions;
  unsigned int                             transferred;
};

struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_GTK_CBData
 : Test_I_GTK_CBData
{
  inline BitTorrent_Client_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , configuration (NULL)
   , contextId (0)
   , controller (NULL)
   , progressData ()
   , sessions ()
   , subscribers ()
   , trackerSubscribers ()
   , UIFileDirectory ()
  {};

  struct BitTorrent_Client_Configuration*   configuration;
  guint                                     contextId;
  BitTorrent_Client_IControl_t*             controller;
  struct BitTorrent_Client_GTK_ProgressData progressData;
  BitTorrent_Client_GUI_Sessions_t          sessions;
  BitTorrent_Client_IPeerSubscribers_t      subscribers;
  BitTorrent_Client_ITrackerSubscribers_t   trackerSubscribers;
  std::string                               UIFileDirectory;
};

struct BitTorrent_Client_SessionThreadData
 : BitTorrent_Client_ThreadData
{
  inline BitTorrent_Client_SessionThreadData ()
   : BitTorrent_Client_ThreadData ()
   , configuration (NULL)
   , CBData (NULL)
  {};

  struct BitTorrent_Client_Configuration* configuration;
  struct BitTorrent_Client_GTK_CBData*    CBData;
};

struct BitTorrent_Client_GTK_SessionCBData
{
  inline BitTorrent_Client_GTK_SessionCBData ()
   : configuration (NULL)
   , controller (NULL)
   , eventSourceId (0)
   , GTKState (NULL)
   , handler (NULL)
   , label ()
   , session (NULL)
  {};

  struct BitTorrent_Client_Configuration* configuration;
  BitTorrent_Client_IControl_t*           controller;
  guint                                   eventSourceId;
  struct Common_UI_GTKState*              GTKState;
  BitTorrent_Client_GUI_Session_t*        handler;
  // *TODO*: remove this
  std::string                             label;
  BitTorrent_Client_ISession_t*           session;
};

typedef Common_UI_GtkBuilderDefinition_T<struct BitTorrent_Client_GTK_CBData> BitTorrent_Client_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct BitTorrent_Client_GTK_CBData> BitTorrent_Client_GTK_Manager_t;
typedef ACE_Singleton<BitTorrent_Client_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> BITTORRENT_CLIENT_UI_GTK_MANAGER_SINGLETON;

#endif
