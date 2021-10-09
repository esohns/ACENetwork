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

#ifndef IRC_CLIENT_CURSES_H
#define IRC_CLIENT_CURSES_H

#include <map>
#include <string>

#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN32)
#include "curses.h"
#else
#include "ncurses.h"
// *NOTE*: the ncurses "timeout" macros conflicts with
//         ACE_Synch_Options::timeout. Since not currently used, it's safe to
//         undefine
#undef timeout
#endif // ACE_WIN32 || ACE_WIN32
#include "panel.h"

#include "common_ui_curses_common.h"

#include "IRC_client_common.h"

typedef std::map<std::string, IRC_Client_MessageQueue_t> IRC_Client_CursesMessages_t;
typedef IRC_Client_CursesMessages_t::iterator IRC_Client_CursesMessagesIterator_t;

struct IRC_SessionState;
struct IRC_Client_CursesState
 : Common_UI_Curses_State
{
  IRC_Client_CursesState ()
   : Common_UI_Curses_State ()
   , activePanel ()
   , input (NULL)
   , log (NULL)
   , panels ()
   , status (NULL)
   ///////////////////////////////////////
   , backLog ()
   , controller (NULL)
   , message ()
   , receivers ()
   , sessionState (NULL)
  {
    activePanel = panels.begin ();
  };

  // curses
  Common_UI_Curses_PanelsIterator_t activePanel;
  WINDOW*                           input;
  WINDOW*                           log;
  Common_UI_Curses_Panels_t         panels;
  WINDOW*                           status;

  ////////////////////////////////////////

  // session
  IRC_Client_CursesMessages_t       backLog;
  IRC_IControl*                     controller;
  std::string                       message; // current-
  string_list_t                     receivers; // current-
  struct IRC_SessionState*          sessionState;
};

// event hooks
bool curses_init (struct Common_UI_Curses_State*); // state
bool curses_fini (struct Common_UI_Curses_State*); // state

bool curses_input (struct Common_UI_Curses_State*, // state
                   int);                           // input character
bool curses_main (struct Common_UI_Curses_State*); // state

//////////////////////////////////////////

bool curses_join (const std::string&,              // channel
                  struct IRC_Client_CursesState&); // state
void curses_log (const std::string&,      // channel (empty ? server log : channel)
                 const std::string&,      // text
                 struct IRC_Client_CursesState&, // state
                 bool = true);                   // lock ?
bool curses_part (const std::string&,              // channel
                  struct IRC_Client_CursesState&); // state

#endif
