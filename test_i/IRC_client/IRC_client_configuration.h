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

#ifndef IRC_CLIENT_CONFIGURATION_H
#define IRC_CLIENT_CONFIGURATION_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#if defined (CURSES_SUPPORT)
#include "common_ui_curses_common.h"
#include "common_ui_curses_manager.h"
#endif // CURSES_SUPPORT

#include "stream_common.h"

#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "irc_configuration.h"
#include "irc_defines.h"

#include "test_i_common.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT

#include "IRC_client_common.h"
#include "IRC_client_defines.h"
#include "IRC_client_network.h"
#include "IRC_client_stream.h"

// forward declarations
struct Net_UserData;
struct IRC_Client_CursesState;

class IRC_Client_ConnectionConfiguration;
struct IRC_Client_InputHandlerConfiguration
{
  IRC_Client_InputHandlerConfiguration ()
   : controller (NULL)
   , connectionConfiguration (NULL)
  {}

  IRC_IControl*                       controller;

  IRC_Client_ConnectionConfiguration* connectionConfiguration;
};

struct IRC_Client_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  IRC_Client_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , parserConfiguration ()
   ///////////////////////////////////////
   , connectionConfigurations ()
   ///////////////////////////////////////
   , streamConfiguration ()
   ///////////////////////////////////////
   , protocolConfiguration ()
   ///////////////////////////////////////
#if defined (CURSES_SUPPORT)
   , cursesConfiguration ()
#endif // CURSES_SUPPORT
   , encoding (IRC_PRT_DEFAULT_ENCODING)
//, groupId (COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1)
   , logToFile (IRC_CLIENT_SESSION_DEFAULT_LOG)
   ///////////////////////////////////////
   , userData ()
  {}

  // ****************************** parser *************************************
  struct Common_FlexBisonParserConfiguration parserConfiguration;
  // ****************************** socket *************************************
  Net_ConnectionConfigurations_t             connectionConfigurations;
  // ****************************** stream *************************************
  IRC_Client_StreamConfiguration_t           streamConfiguration;
  // ***************************** protocol ************************************
  struct IRC_ProtocolConfiguration           protocolConfiguration;
  // ***************************************************************************
#if defined (CURSES_SUPPORT)
  struct Common_UI_Curses_Configuration      cursesConfiguration;
#endif // CURSES_SUPPORT
  // *NOTE*: see also https://en.wikipedia.org/wiki/Internet_Relay_Chat#Character_encoding
  // *TODO*: implement support for 7-bit ASCII (as it is the most compatible
  //         encoding)
  enum IRC_CharacterEncoding                 encoding;
  //int                                        groupId;
  bool                                       logToFile;

  struct Net_UserData                        userData;
};

//////////////////////////////////////////

#if defined (CURSES_SUPPORT)
typedef Common_UI_Curses_Manager_T<ACE_MT_SYNCH,
                                   struct Common_UI_Curses_Configuration,
                                   struct IRC_Client_CursesState> Common_UI_Curses_Manager_t;
typedef ACE_Singleton<Common_UI_Curses_Manager_t,
                      ACE_MT_SYNCH::MUTEX> COMMON_UI_CURSES_MANAGER_SINGLETON;
#endif // CURSES_SUPPORT

#endif
