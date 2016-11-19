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

#include <ace/os_include/os_pthread.h>

#include <gtk/gtk.h>

#include "test_i_gtk_common.h"

// forward declaration(s)
struct Common_UI_GTKState;
struct BitTorrent_Client_GUI_Session;

typedef std::map<std::string, struct BitTorrent_Client_GUI_Session*> BitTorrent_Client_GUI_Sessions_t;
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
   , contextID (0)
   , progressData ()
   , sessions ()
   , UIFileDirectory ()
  {};

  struct BitTorrent_Client_Configuration*   configuration;
  guint                                     contextID;
  struct BitTorrent_Client_GTK_ProgressData progressData;
  BitTorrent_Client_GUI_Sessions_t          sessions;
  std::string                               UIFileDirectory;
};

struct BitTorrent_Client_SessionThreadData
{
  inline BitTorrent_Client_SessionThreadData ()
   : configuration (NULL)
   , CBData (NULL)
  {};

  struct BitTorrent_Client_Configuration* configuration;
  struct BitTorrent_Client_GTK_CBData*    CBData;
};

class BitTorrent_IControl;
struct Common_UI_GTKState;
struct BitTorrent_Client_GTK_SessionCBData
{
  inline BitTorrent_Client_GTK_SessionCBData ()
   : acknowledgements (0)
   , controller (NULL)
   , eventSourceID (0)
   , GTKState (NULL)
   , label ()
   , pending (false)
   , sessions (NULL)
   , timeStamp ()
  {};

  unsigned int                      acknowledgements;
  BitTorrent_IControl*              controller;
  guint                             eventSourceID;
  struct Common_UI_GTKState*        GTKState;
  // *TODO*: remove this
  std::string                       label;
  bool                              pending;
  BitTorrent_Client_GUI_Sessions_t* sessions;
  std::string                       timeStamp;
};

#endif
