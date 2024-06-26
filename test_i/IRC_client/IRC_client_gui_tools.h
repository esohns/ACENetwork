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

#ifndef IRC_CLIENT_GUI_TOOLS_H
#define IRC_CLIENT_GUI_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_common.h"
#endif // GTK_SUPPORT

#include "IRC_client_gui_common.h"

// forward declarations
class IRC_Client_GUI_IConnection;

class IRC_Client_UI_Tools
{
 public:
  // *WARNING*: these members must be called with
  //            IRC_Client_UI_CBData::Common_UI_GTKState_t::lock held !
  static bool current (const std::string&,                  // (connection-) timestamp
                       const IRC_Client_GUI_Connections_t&, // connections
                       std::string&,                        // return value: nickname
                       std::string&);                       // return value: channel
#if defined (GTK_SUPPORT)
  // *WARNING*: this requires gdk_threads_enter()/leave() protection !
  static IRC_Client_GUI_IConnection* current (Common_UI_GTK_State_t&,               // GTK state
                                              const IRC_Client_GUI_Connections_t&); // connections
#endif // GTK_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_UI_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~IRC_Client_UI_Tools ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_UI_Tools (const IRC_Client_UI_Tools&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_UI_Tools& operator= (const IRC_Client_UI_Tools&))
};

#endif
