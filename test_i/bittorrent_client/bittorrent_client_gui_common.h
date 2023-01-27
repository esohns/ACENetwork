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
#include <string>

#include "ace/INET_Addr.h"
#include "ace/os_include/os_pthread.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

//#include "net_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_i_wxwidgets_common.h"
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

#include "bittorrent_stream_common.h"

//#include "bittorrent_client_common.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

// forward declaration(s)
template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
class BitTorrent_Client_GUI_Session_T;
struct BitTorrent_Client_UI_SessionCBData;
typedef BitTorrent_Client_GUI_Session_T<BitTorrent_Client_ISession_t,
                                        BitTorrent_Client_IPeerConnection_t,
                                        struct BitTorrent_Client_UI_SessionCBData> BitTorrent_Client_GUI_Session_t;
typedef std::map<std::string,
                 BitTorrent_Client_GUI_Session_t*> BitTorrent_Client_GUI_Sessions_t;
typedef BitTorrent_Client_GUI_Sessions_t::iterator BitTorrent_Client_GUI_SessionsIterator_t;
typedef BitTorrent_Client_GUI_Sessions_t::const_iterator BitTorrent_Client_GUI_SessionsConstIterator_t;

struct BitTorrent_Client_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#endif
{
  BitTorrent_Client_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
   , transferred (0)
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
   , transferred (0)
#else
   : transferred (0)
#endif // GTK_USE || WXWIDGETS_USE
  {}

  unsigned int transferred;
};

struct BitTorrent_Client_Configuration;
struct BitTorrent_Client_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#endif // GTK_USE || WXWIDGETS_USE
{
  BitTorrent_Client_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
   , configuration (NULL)
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE || WXWIDGETS_USE
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
   , controller (NULL)
   , progressData ()
   , sessions ()
//   , subscribers ()
//   , trackerSubscribers ()
   , UIFileDirectory ()
  {}

  struct BitTorrent_Client_Configuration*  configuration;
#if defined (GTK_USE)
  guint                                    contextId;
#endif // GTK_USE
  BitTorrent_Client_IControl_t*            controller;
  struct BitTorrent_Client_UI_ProgressData progressData;
  BitTorrent_Client_GUI_Sessions_t         sessions;
//  BitTorrent_Client_IPeerSubscribers_t     subscribers;
//  BitTorrent_Client_ITrackerSubscribers_t  trackerSubscribers;
  std::string                              UIFileDirectory;
};

struct BitTorrent_Client_UI_SessionThreadData
 : BitTorrent_Client_SessionThreadData
{
  BitTorrent_Client_UI_SessionThreadData ()
   : BitTorrent_Client_SessionThreadData ()
   , CBData (NULL)
#if defined (GTK_USE)
   , eventSourceId (0)
#endif // GTK_USE
  {}

  struct BitTorrent_Client_UI_CBData* CBData;
#if defined (GTK_USE)
  guint                               eventSourceId;
#endif // GTK_USE
};

struct BitTorrent_Client_UI_SessionCBData
{
  BitTorrent_Client_UI_SessionCBData ()
   : controller (NULL)
   , CBData (NULL)
#if defined (GTK_USE)
   , eventSourceId (0)
#endif // GTK_USE
   , handler (NULL)
   , label ()
   , session (NULL)
  {}

  BitTorrent_Client_IControl_t*           controller;
  struct BitTorrent_Client_UI_CBData*     CBData;
#if defined (GTK_USE)
  guint                                   eventSourceId;
#endif // GTK_USE
  BitTorrent_Client_GUI_Session_t*        handler;
  // *TODO*: remove this
  std::string                             label;
  BitTorrent_Client_ISession_t*           session;
};

struct BitTorrent_Client_UI_SessionProgressData
{
  BitTorrent_Client_UI_SessionProgressData ()
   : label ()
   , message ()
   , pieceIndex (-1)
  {}

  std::string label;

  std::string message; // log-
  int         pieceIndex;
  bool        cancelled;
};

#endif
