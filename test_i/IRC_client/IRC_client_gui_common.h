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
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"

#include "irc_message.h"

#include "IRC_client_configuration.h"
#include "IRC_client_stream_common.h"

// forward declaration(s)
class IRC_Client_GUI_Connection;
class IRC_Client_GUI_MessageHandler;

typedef std::map<std::string,
                 IRC_Client_GUI_Connection*> IRC_Client_GUI_Connections_t;
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
  struct Common_UI_GTKState*        GTKState;
  IRC_Client_GUI_PendingActions_t   pendingActions;
};

struct IRC_Client_GTK_CBData
 : Common_UI_GTKState
{
  inline IRC_Client_GTK_CBData ()
   : Common_UI_GTKState ()
   , configuration (NULL)
   , connections ()
   , contextID (0)
   , phoneBook ()
   , progressData ()
   , UIFileDirectory ()
  {};

  struct IRC_Client_Configuration*   configuration;
  IRC_Client_GUI_Connections_t       connections;
  guint                              contextID;
  struct IRC_Client_PhoneBook        phoneBook;
  struct IRC_Client_GTK_ProgressData progressData;
  std::string                        UIFileDirectory;
};

struct IRC_Client_ConnectionThreadData
{
  inline IRC_Client_ConnectionThreadData ()
   : configuration (NULL)
   , CBData (NULL)
   , loginOptions ()
   , phonebookEntry ()
  {};

  struct IRC_Client_Configuration*  configuration;
  struct IRC_Client_GTK_CBData*     CBData;
  struct IRC_LoginOptions           loginOptions;
  struct IRC_Client_ConnectionEntry phonebookEntry;
};

struct IRC_Client_GTK_ConnectionCBData
{
  inline IRC_Client_GTK_ConnectionCBData ()
   : acknowledgements (0)
   , connections (NULL)
   , controller (NULL)
   , eventSourceID (0)
   , GTKState (NULL)
   , label ()
   , pending (false)
   , timeStamp ()
  {};

  unsigned int                  acknowledgements;
  IRC_Client_GUI_Connections_t* connections;
  IRC_IControl*                 controller;
  guint                         eventSourceID;
  struct Common_UI_GTKState*    GTKState;
  // *TODO*: remove this
  std::string                   label;
  bool                          pending;
  std::string                   timeStamp;
};

struct IRC_Client_GTK_HandlerCBData
{
  inline IRC_Client_GTK_HandlerCBData ()
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

  unsigned int                   acknowledgements;
  std::string                    builderLabel;
  IRC_ChannelModes_t             channelModes;
  IRC_Client_GUI_Connection*     connection;
  IRC_IControl*                  controller;
  guint                          eventSourceID;
  struct Common_UI_GTKState*     GTKState;
  IRC_Client_GUI_MessageHandler* handler;
  // *TODO*: remove this
  std::string                    id;
  string_list_t                  parameters;
  bool                           pending;
  std::string                    timeStamp;
};

typedef Common_UI_GtkBuilderDefinition_T<struct IRC_Client_GTK_CBData> IRC_Client_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct IRC_Client_GTK_CBData> IRC_Client_GTK_Manager_t;
typedef ACE_Singleton<IRC_Client_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> IRC_CLIENT_UI_GTK_MANAGER_SINGLETON;

#endif
