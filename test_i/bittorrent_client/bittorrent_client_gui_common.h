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
#include <vector>

#include <ace/OS.h>

#include <gtk/gtk.h>

#include "common_ui_common.h"

#include "bittorrent_message.h"

#include "bittorrent_client_configuration.h"
#include "bittorrent_client_stream_common.h"

// forward declaration(s)
class BitTorrent_Client_GUI_Connection;
class BitTorrent_Client_GUI_MessageHandler;

typedef std::map<std::string, BitTorrent_Client_GUI_Connection*> BitTorrent_Client_GUI_Connections_t;
typedef BitTorrent_Client_GUI_Connections_t::iterator BitTorrent_Client_GUI_ConnectionsIterator_t;
typedef BitTorrent_Client_GUI_Connections_t::const_iterator BitTorrent_Client_GUI_ConnectionsConstIterator_t;

typedef std::map<ACE_thread_t, guint> BitTorrent_Client_GUI_PendingActions_t;
typedef BitTorrent_Client_GUI_PendingActions_t::iterator BitTorrent_Client_GUI_PendingActionsIterator_t;
typedef std::set<ACE_thread_t> BitTorrent_Client_GUI_CompletedActions_t;
typedef BitTorrent_Client_GUI_CompletedActions_t::iterator BitTorrent_Client_GUI_CompletedActionsIterator_t;

struct BitTorrent_Client_GTK_ProgressData
{
  inline BitTorrent_Client_GTK_ProgressData ()
   : completedActions ()
   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
  {};

  BitTorrent_Client_GUI_CompletedActions_t completedActions;
  GdkCursorType                     cursorType;
  Common_UI_GTKState*               GTKState;
  BitTorrent_Client_GUI_PendingActions_t   pendingActions;
};

struct BitTorrent_Client_GTK_CBData
{
  inline BitTorrent_Client_GTK_CBData ()
   : configuration (NULL)
   , connections ()
   , contextID (0)
   , GTKState ()
   , phoneBook ()
   , progressData ()
   , UIFileDirectory ()
  {};

  BitTorrent_Client_Configuration*    configuration;
  BitTorrent_Client_GUI_Connections_t connections;
  guint                        contextID;
  Common_UI_GTKState           GTKState;
  BitTorrent_Client_PhoneBook         phoneBook;
  BitTorrent_Client_GTK_ProgressData  progressData;
  std::string                  UIFileDirectory;
};

struct BitTorrent_Client_ConnectionThreadData
{
  inline BitTorrent_Client_ConnectionThreadData ()
   : configuration (NULL)
   , CBData (NULL)
   , loginOptions ()
   , phonebookEntry ()
  {};

  BitTorrent_Client_Configuration*  configuration;
  BitTorrent_Client_GTK_CBData*     CBData;
  BitTorrent_LoginOptions           loginOptions;
  BitTorrent_Client_ConnectionEntry phonebookEntry;
};

struct BitTorrent_Client_GTK_ConnectionCBData
{
  inline BitTorrent_Client_GTK_ConnectionCBData ()
   : acknowledgements (0)
   , connections (NULL)
   , controller (NULL)
   , eventSourceID (0)
   , GTKState (NULL)
   , label ()
   , pending (false)
   , timeStamp ()
  {};

  unsigned int                         acknowledgements;
  BitTorrent_Client_GUI_Connections_t* connections;
  BitTorrent_IControl*                 controller;
  guint                                eventSourceID;
  Common_UI_GTKState*                  GTKState;
  // *TODO*: remove this
  std::string                          label;
  bool                                 pending;
  std::string                          timeStamp;
};

struct BitTorrent_Client_GTK_HandlerCBData
{
  inline BitTorrent_Client_GTK_HandlerCBData ()
   : acknowledgements (0)
   , builderLabel ()
   , channelModes ()
   , connection (NULL)
   , controller (NULL)
   , eventSourceID (0)
   , GTKState (NULL)
   , handler (NULL)
   , id ()
   , parameters ()
   , pending (false)
   , timeStamp ()
  {};

  unsigned int                          acknowledgements;
  std::string                           builderLabel;
  BitTorrent_ChannelModes_t             channelModes;
  BitTorrent_Client_GUI_Connection*     connection;
  BitTorrent_IControl*                  controller;
  guint                                 eventSourceID;
  Common_UI_GTKState*                   GTKState;
  BitTorrent_Client_GUI_MessageHandler* handler;
  // *TODO*: remove this
  std::string                           id;
  string_list_t                         parameters;
  bool                                  pending;
  std::string                           timeStamp;
};

#endif
