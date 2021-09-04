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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "irc_message.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_i_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT

#include "IRC_client_configuration.h"
#include "IRC_client_stream_common.h"

// forward declaration(s)
class IRC_Client_GUI_Connection;
class IRC_Client_GUI_MessageHandler;

typedef std::map<std::string,
                 IRC_Client_GUI_Connection*> IRC_Client_GUI_Connections_t;
typedef IRC_Client_GUI_Connections_t::iterator IRC_Client_GUI_ConnectionsIterator_t;
typedef IRC_Client_GUI_Connections_t::const_iterator IRC_Client_GUI_ConnectionsConstIterator_t;

struct IRC_Client_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#endif
{
  IRC_Client_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
#endif
#if defined (GTK_USE)
   , cursorType (GDK_LAST_CURSOR)
#endif // GTK_USE
  {}

#if defined (GTK_USE)
  GdkCursorType cursorType;
#endif // GTK_USE
};

struct IRC_Client_Configuration;
struct IRC_Client_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#endif // GTK_USE || WXWIDGETS_USE
{
  IRC_Client_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
   , configuration (NULL)
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE || WXWIDGETS_USE
   , connections ()
#if defined (GTK_USE)
   , contextId (0)
#endif // GTK_USE
   , phoneBook ()
   , progressData ()
   , UIFileDirectory ()
  {
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
    progressData.state = UIState;
#endif // GTK_USE || WXWIDGETS_USE
  }

  struct IRC_Client_Configuration*  configuration;
  IRC_Client_GUI_Connections_t      connections;
#if defined (GTK_USE)
  guint                             contextId;
#endif // GTK_USE
  struct IRC_Client_PhoneBook       phoneBook;
  struct IRC_Client_UI_ProgressData progressData;
  std::string                       UIFileDirectory;
};

struct IRC_Client_ConnectionThreadData
{
  IRC_Client_ConnectionThreadData ()
   : configuration (NULL)
   , CBData (NULL)
   , loginOptions ()
   , phonebookEntry ()
  {}

  struct IRC_Client_Configuration*  configuration;
  struct IRC_Client_UI_CBData*      CBData;
  struct IRC_LoginOptions           loginOptions;
  struct IRC_Client_ConnectionEntry phonebookEntry;
};

struct IRC_Client_UI_HandlerCBData;
struct IRC_Client_UI_ConnectionCBData
{
  IRC_Client_UI_ConnectionCBData ()
   : acknowledgements (0)
   , CBData (NULL)
   , connections (NULL)
   , controller (NULL)
#if defined (GTK_USE)
   , eventSourceId (0)
#endif // GTK_USE
   , label ()
   , pending (false)
   , timeStamp ()
  {}

  unsigned int                         acknowledgements;
  struct IRC_Client_UI_HandlerCBData*  CBData;
  IRC_Client_GUI_Connections_t*        connections;
  IRC_IControl*                        controller;
#if defined (GTK_USE)
  guint                                eventSourceId;
#endif // GTK_USE
  // *TODO*: remove this
  std::string                          label;
  bool                                 pending;
  std::string                          timeStamp;
};

struct IRC_Client_UI_HandlerCBData
{
  IRC_Client_UI_HandlerCBData ()
   : acknowledgements (0)
   , builderLabel ()
   , channelModes ()
   , connection (NULL)
   , controller (NULL)
#if defined (GTK_USE)
   , eventSourceId (0)
#endif // GTK_USE
   , handler (NULL)
   , id ()
   , parameters ()
   , pending (false)
   , timeStamp ()
  {}

  unsigned int                   acknowledgements;
  std::string                    builderLabel;
  IRC_ChannelModes_t             channelModes;
  IRC_Client_GUI_Connection*     connection;
  IRC_IControl*                  controller;
#if defined (GTK_USE)
  guint                          eventSourceId;
#endif // GTK_USE
  IRC_Client_GUI_MessageHandler* handler;
  // *TODO*: remove this
  std::string                    id;
  string_list_t                  parameters;
  bool                           pending;
  std::string                    timeStamp;
};

#endif // IRC_CLIENT_GUI_COMMON_H
