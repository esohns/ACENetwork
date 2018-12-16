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

#ifndef BITTORRENT_CLIENT_CURSES_H
#define BITTORRENT_CLIENT_CURSES_H

#include <map>
#include <string>

#include "ace/config-lite.h"
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

#include "ace/Synch_Traits.h"

#include "common.h"

#include "bittorrent_client_common.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"

// forward declaration(s)
struct BitTorrent_Client_SessionState;

typedef std::map<std::string, struct panel*> BitTorrent_Client_CursesSessions_t;
typedef BitTorrent_Client_CursesSessions_t::iterator BitTorrent_Client_CursesSessionsIterator_t;

typedef std::map<std::string, Common_MessageStack_t> BitTorrent_Client_CursesMessages_t;
typedef BitTorrent_Client_CursesMessages_t::iterator BitTorrent_Client_CursesMessagesIterator_t;

struct BitTorrent_Client_CursesState
{
  BitTorrent_Client_CursesState ()
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
  }

  // curses
  BitTorrent_Client_CursesSessionsIterator_t activePanel;
  WINDOW*                                    input;
  WINDOW*                                    log;
  BitTorrent_Client_CursesSessions_t         panels;
  SCREEN*                                    screen;
  WINDOW*                                    status;

  // dispatch loop
  bool                                       finished;
  ACE_SYNCH_MUTEX                            lock;

  ////////////////////////////////////////

  // session
  BitTorrent_Client_CursesMessages_t         backLog;
  struct BitTorrent_Client_SessionState*     sessionState;
};

bool curses_download (const std::string&,                     // metainfo file URI
                      struct BitTorrent_Client_CursesState&); // state
void curses_log (const std::string&,                    // metainfo file URI
                 const std::string&,                    // text
                 struct BitTorrent_Client_CursesState&, // state
                 bool = true);                          // lock ?
bool curses_main (struct BitTorrent_Client_CursesState&, // state
                  BitTorrent_Client_IControl_t*);        // controller
bool curses_stop (const std::string&,                     // metainfo file URI
                  struct BitTorrent_Client_CursesState&); // state
bool curses_upload (const std::string&,                     // metainfo file URI
                    struct BitTorrent_Client_CursesState&); // state

#endif
