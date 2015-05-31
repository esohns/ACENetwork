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

#include "common_ui_common.h"

#include "IRC_client_common.h"
//#include "IRC_client_configuration.h"
#include "IRC_client_IRCmessage.h"

// forward declaration(s)
class ACE_Thread_Mutex;
struct IRC_Client_Configuration;
class IRC_Client_IIRCControl;
class IRC_Client_GUI_Connection;

typedef std::map<std::string, IRC_Client_GUI_Connection*> connections_t;
typedef connections_t::iterator connections_iterator_t;

struct IRC_Client_GTK_CBData
{
  inline IRC_Client_GTK_CBData ()
    : configuration (NULL)
    , connections ()
    , GTKState ()
    , phoneBook ()
    , UIFileDirectory ()
  {};

  IRC_Client_Configuration* configuration;
  connections_t             connections;
  Common_UI_GTKState        GTKState;
  IRC_Client_PhoneBook      phoneBook;
  std::string               UIFileDirectory;

};

struct IRC_Client_GTK_ConnectionCBData
{
  inline IRC_Client_GTK_ConnectionCBData ()
   : away (false)
   , connection (NULL)
   , connections (NULL)
   , controller (NULL)
   , GTKState (NULL)
   , nickname ()
   , userModes ()
  {};

  // *TODO*: couldn't it be done without this one ?
  bool                       away;
  IRC_Client_GUI_Connection* connection;
  connections_t*             connections;
  IRC_Client_IIRCControl*    controller;
  Common_UI_GTKState*        GTKState;
  std::string                nickname;
  IRC_Client_UserModes_t     userModes;
};

struct IRC_Client_GTK_HandlerCBData
{
  inline IRC_Client_GTK_HandlerCBData ()
   : channelModes ()
   , connection (NULL)
   , controller (NULL)
   , GTKState (NULL)
   , id ()
   , parameters ()
  {};

  IRC_Client_ChannelModes_t  channelModes;
  IRC_Client_GUI_Connection* connection;
  IRC_Client_IIRCControl*    controller;
  Common_UI_GTKState*        GTKState;
  std::string                id;
  string_list_t              parameters;
};

#endif
