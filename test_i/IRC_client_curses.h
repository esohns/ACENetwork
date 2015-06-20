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

#include <string>

#include "ace/OS.h"

#if defined (ACE_WIN32) || defined (ACE_WIN32)
#include "curses.h"
#else
#include <ncurses.h>
#endif
#include "panel.h"

#include "IRC_client_common.h"

// forward declarations
struct IRC_Client_SessionState;

struct IRC_Client_CursesState
{
  inline IRC_Client_CursesState ()
   : activePanel (0)
   , panels ()
   , input (NULL)
   , screen (NULL)
   , status (NULL)
   , finished (false)
   , lock ()
   //////////////////////////////////////
   , backLog ()
   , IRCSessionState (NULL)
  {
    ACE_OS::memset (panels, 0, sizeof (panels));
  };

  // curses
  int                       activePanel;
  WINDOW*                   input;
  PANEL*                    panels[2]; // 0: log, 1: channel
  SCREEN*                   screen;
  WINDOW*                   status;

  // dispatch loop
  bool                      finished;
  ACE_SYNCH_MUTEX           lock;

  ///////////////////////////////////////

  // session
  IRC_Client_MessageQueue_t backLog;
  IRC_Client_SessionState*  IRCSessionState;
};

bool curses_join (const std::string&,       // channel
                  IRC_Client_CursesState&); // state
void curses_log (const std::string&,      // text
                 IRC_Client_CursesState&, // state
                 bool = true,             // channel ? : server log
                 bool = true);            // lock ?
void curses_main (IRC_Client_CursesState&); // state
bool curses_part (const std::string&,       // channel
                  IRC_Client_CursesState&); // state

#endif
