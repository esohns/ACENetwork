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

#include "IRC_client_common.h"

typedef std::map<std::string, struct panel*> IRC_Client_CursesChannels_t;
typedef IRC_Client_CursesChannels_t::iterator IRC_Client_CursesChannelsIterator_t;

typedef std::map<std::string, IRC_Client_MessageQueue_t> IRC_Client_CursesMessages_t;
typedef IRC_Client_CursesMessages_t::iterator IRC_Client_CursesMessagesIterator_t;

struct IRC_Client_SessionState;
struct IRC_Client_CursesState
{
  inline IRC_Client_CursesState ()
   : activePanel ()
   , input (NULL)
   , log (NULL)
   , panels ()
   , screen (NULL)
   , status (NULL)
   , finished (false)
   , lock ()
   ///////////////////////////////////////
   , backLog ()
   , sessionState (NULL)
  {
    activePanel = panels.begin ();
  };

  // curses
  IRC_Client_CursesChannelsIterator_t activePanel;
  WINDOW*                             input;
  WINDOW*                             log;
  IRC_Client_CursesChannels_t         panels;
  SCREEN*                             screen;
  WINDOW*                             status;

  // dispatch loop
  bool                                finished;
  ACE_SYNCH_MUTEX                     lock;

  ////////////////////////////////////////

  // session
  IRC_Client_CursesMessages_t         backLog;
  struct IRC_Client_SessionState*     sessionState;
};

bool curses_join (const std::string&,              // channel
                  struct IRC_Client_CursesState&); // state
void curses_log (const std::string&,      // channel (empty ? server log : channel)
                 const std::string&,      // text
                 struct IRC_Client_CursesState&, // state
                 bool = true);                   // lock ?
bool curses_main (struct IRC_Client_CursesState&, // state
                  IRC_IControl*);                 // controller
bool curses_part (const std::string&,              // channel
                  struct IRC_Client_CursesState&); // state

#endif
