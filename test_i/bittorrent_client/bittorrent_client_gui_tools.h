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

#ifndef BITTORRENT_CLIENT_GUI_TOOLS_H
#define BITTORRENT_CLIENT_GUI_TOOLS_H

//#include <string>

#include "ace/Global_Macros.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "bittorrent_client_gui_common.h"

class BitTorrent_Client_UI_Tools
{
 public:
//  // *WARNING*: these must be called with
//  //            BitTorrent_Client_UI_CBData::Common_UI_GTKState::lock held !
//  static bool current (const std::string&,                         // (connection-) timestamp
//                       const BitTorrent_Client_GUI_Connections_t&, // connections
//                       std::string&,                               // return value: nickname
//                       std::string&);                              // return value: channel
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  // *WARNING*: this requires gdk_threads_enter()/leave() protection !
  static BitTorrent_Client_GUI_Session_t* current (Common_UI_GTK_State_t&,                   // GTK state
                                                   const BitTorrent_Client_GUI_Sessions_t&); // sessions
#endif // GTK_USE
#endif // GUI_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_UI_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~BitTorrent_Client_UI_Tools ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_UI_Tools (const BitTorrent_Client_UI_Tools&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_UI_Tools& operator= (const BitTorrent_Client_UI_Tools&))
};

#endif
