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

#ifndef IRC_CLIENT_GUI_COMMON_H
#define IRC_CLIENT_GUI_COMMON_H

#include <map>
#include <string>
#include <vector>

#include "ace/OS.h"

#include "common_ui_common.h"

#include "IRC_client_common.h"
#include "IRC_client_IRCmessage.h"

// forward declaration(s)
class ACE_Thread_Mutex;
struct IRC_Client_Configuration;
class IRC_Client_IIRCControl;
class IRC_Client_GUI_Connection;

typedef std::map<std::string, IRC_Client_GUI_Connection*> IRC_Client_GUI_Connections_t;
typedef IRC_Client_GUI_Connections_t::iterator IRC_Client_GUI_ConnectionsIterator_t;
typedef IRC_Client_GUI_Connections_t::const_iterator IRC_Client_GUI_ConnectionsConstIterator_t;

typedef std::map<ACE_thread_t, guint> IRC_Client_GUI_PendingActions_t;
typedef IRC_Client_GUI_PendingActions_t::iterator IRC_Client_GUI_PendingActionsIterator_t;
typedef std::set<ACE_thread_t> IRC_Client_GUI_CompletedActions_t;
typedef IRC_Client_GUI_CompletedActions_t::iterator IRC_Client_GUI_CompletedActionsIterator_t;

struct IRC_Client_GTK_ProgressData
{
  inline IRC_Client_GTK_ProgressData ()
   : completedActions ()
   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
  {};

  IRC_Client_GUI_CompletedActions_t completedActions;
  GdkCursorType                     cursorType;
  Common_UI_GTKState*               GTKState;
  IRC_Client_GUI_PendingActions_t   pendingActions;
};

struct IRC_Client_GTK_CBData
{
  inline IRC_Client_GTK_CBData ()
   : configuration (NULL)
   , connections ()
   , contextID (0)
   , GTKState ()
   , phoneBook ()
   , progressData ()
   , UIFileDirectory ()
  {};

  IRC_Client_Configuration*    configuration;
  IRC_Client_GUI_Connections_t connections;
  guint                        contextID;
  Common_UI_GTKState           GTKState;
  IRC_Client_PhoneBook         phoneBook;
  IRC_Client_GTK_ProgressData  progressData;
  std::string                  UIFileDirectory;
};

struct IRC_Client_ConnectionThreadData
{
  inline IRC_Client_ConnectionThreadData ()
   : CBData (NULL)
   , configuration (NULL)
   , loginOptions ()
   , phonebookIterator ()
  {};

  IRC_Client_GTK_CBData*       CBData;
  IRC_Client_Configuration*    configuration;
  IRC_Client_IRCLoginOptions   loginOptions;
  IRC_Client_ServersIterator_t phonebookIterator;
};

struct IRC_Client_GTK_ConnectionCBData
{
  inline IRC_Client_GTK_ConnectionCBData ()
   : acknowledgements (0)
   , connection (NULL)
   , connections (NULL)
   , controller (NULL)
   , GTKState (NULL)
   , IRCSessionState ()
   , label ()
   , pending (false)
   , timestamp ()
  {};

  unsigned int                  acknowledgements;
  IRC_Client_GUI_Connection*    connection;
  IRC_Client_GUI_Connections_t* connections;
  IRC_Client_IIRCControl*       controller;
  Common_UI_GTKState*           GTKState;
  IRC_Client_SessionState       IRCSessionState;
  std::string                   label;
  bool                          pending;
  std::string                   timestamp;
};

struct IRC_Client_GTK_HandlerCBData
{
  inline IRC_Client_GTK_HandlerCBData ()
   : acknowledgements (0)
   , builderLabel ()
   , channelModes ()
   , connection (NULL)
   , controller (NULL)
   , GTKState (NULL)
   , id ()
   , parameters ()
   , pending (false)
  {};

  unsigned int                      acknowledgements;
  std::string                       builderLabel;
  IRC_Client_ChannelModes_t         channelModes;
  IRC_Client_GUI_Connection*        connection;
  IRC_Client_IIRCControl*           controller;
  Common_UI_GTKState*               GTKState;
  std::string                       id;
  string_list_t                     parameters;
  bool                              pending;
};

#endif
