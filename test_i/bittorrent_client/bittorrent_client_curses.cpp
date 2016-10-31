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
#include "stdafx.h"

// *WORKAROUND*
#include <iostream>
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary header(s) manually (see above),
//                       and prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "bittorrent_client_curses.h"

#include <string>

#include <ace/Assert.h>
#include <ace/Guard_T.h>
#include <ace/Log_Msg.h>
#include <ace/Reverse_Lock_T.h>

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_icontrol.h"

#include "bittorrent_client_network.h"

#include "test_i_defines.h"

bool
curses_start (const std::string& URI_in,
              BitTorrent_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_join"));

  ACE_UNUSED_ARG (URI_in);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (state_in.lock);

  int result = -1;
  BitTorrent_Client_CursesSessionsIterator_t iterator =
      state_in.panels.find (std::string ());
  ACE_ASSERT (iterator != state_in.panels.end ());
  PANEL* panel_p = (*iterator).second;
  ACE_ASSERT (panel_p);
  ACE_ASSERT (panel_p->win);
  WINDOW* window_p = dupwin (panel_p->win);
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dupwin(), aborting\n")));
    return false;
  } // end IF
//  result = idlok (window_p, TRUE);
//  if (result == ERR)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to idlok(), aborting\n")));
//    return false;
//  } // end IF
//  result = scrollok (window_p, TRUE);
//  if (result == ERR)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to scrollok(), aborting\n")));
//    return false;
//  } // end IF
//  immedok (window_p, TRUE);
  panel_p = new_panel (window_p);
  if (!panel_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to new_panel(), aborting\n")));

    // clean up
    result = delwin (window_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
    return false;
  } // end IF
  state_in.panels[URI_in] = panel_p;

  // switch to channel, refresh
  state_in.activePanel = state_in.panels.find (URI_in);
  ACE_ASSERT (state_in.activePanel != state_in.panels.end ());
  wbkgdset (window_p, COLOR_PAIR (0));
  result = werase (window_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to werase(), continuing\n")));
  ACE_ASSERT (state_in.log);
  result = wbkgd (state_in.log, COLOR_PAIR (0));
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wbkgd(), continuing\n")));
  result = top_panel (panel_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(), continuing\n")));
  update_panels ();
  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(), continuing\n")));

  return (result == OK);
}

void
curses_log (const std::string& URI_in,
            const std::string& text_in,
            BitTorrent_Client_CursesState& state_in,
            bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_log"));

  int result = ERR;
  WINDOW* window_p = NULL;

  if (lockedAccess_in)
  {
    result = state_in.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
  } // end IF

  BitTorrent_Client_CursesSessionsIterator_t iterator =
      state_in.panels.find (URI_in);
  if (iterator == state_in.panels.end ())
  {
    // not connected yet/not on a channel --> store
    Common_MessageStack_t message_stack;
    message_stack.push_front (text_in);
    state_in.backLog.insert (std::make_pair (URI_in, message_stack));
    goto release;
  } // end IF
  ACE_ASSERT ((*iterator).second);

  window_p = (*iterator).second->win;
  ACE_ASSERT (window_p);
//  result = wmove (window_p, window_p->_cury, 0);
//  if (result == ERR)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to wmove(), continuing\n")));
  if (window_p->_cury + 1 >= window_p->_maxy)
  {
    result = wmove (window_p,
                    window_p->_maxy, 0); // retain sanity
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to wmove(), continuing\n")));
    result = scroll (window_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to scroll(), continuing\n")));
  } // end IF
  result = waddnstr (window_p, text_in.c_str (), -1);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waddnstr(), continuing\n")));
  result = wmove (window_p,
                  window_p->_cury + 1, 0); // next line, box
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));

  // switch to channel, refresh
  ACE_ASSERT (state_in.log);
  result = wbkgd (state_in.log,
                  (URI_in.empty () ? COLOR_PAIR (TEST_I_CURSES_COLOR_LOG)
                                   : COLOR_PAIR (0)));
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wbkgd(), continuing\n")));
  result = top_panel ((*iterator).second);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(), continuing\n")));
  update_panels ();
  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(), continuing\n")));

release:
  if (lockedAccess_in)
  {
    result = state_in.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF
}

bool
curses_main (BitTorrent_Client_CursesState& state_in,
             BitTorrent_IControl* controller_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_main"));

  // sanity check(s)
  ACE_ASSERT (controller_in);

  int result = ERR;
  int ch = -1;
  std::string message_text;
  bool release = false;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_in.lock,
                                                  ACE_Acquire_Method::ACE_REGULAR);
  WINDOW* stdscr_p = NULL;
  mmask_t mouse_mask = 0;
  WINDOW* window_p = NULL;
  PANEL* panel_p = NULL;
  char* string_p = NULL;
  int result_2 = ERR;
  BitTorrent_Client_CursesMessagesIterator_t iterator;

  // step1: initialize curses

  // grab lock
  result = state_in.lock.acquire ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
    return (result == OK);
  } // end IF
  release = true;

  use_env (TRUE);
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  use_tioctl (TRUE);
#endif
//  nofilter ();

  // *NOTE*: if this fails, the program exits, which is not intended behavior
  // *TODO*: --> use newterm() instead
  //state_in.screen = initscr ();
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  state_in.screen = newterm (NULL, NULL, NULL); // use $TERM, STD_OUT, STD_IN
  if (!state_in.screen)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newterm(0x%@), aborting\n"),
                NULL));
    goto close;
  } // end IF
#endif
  // *NOTE*: for some (odd) reason, newterm does not work as advertised
  //         (curscr, stdscr corrupt, return value works though)
  stdscr_p = initscr ();
  if (!stdscr_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initscr(), aborting\n")));
    goto close;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  state_in.screen = SP;
#endif
  ACE_ASSERT (state_in.screen && stdscr_p);
  string_p = longname ();
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to longname(), aborting\n")));
    goto close;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("initialized curses terminal (\"%s\")\n"),
              ACE_TEXT (string_p)));

  if (has_colors ())
  {
    result = start_color (); // intialize colors
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start_color(), aborting\n")));
      goto close;
    } // end IF

    result = init_pair (TEST_I_CURSES_COLOR_LOG,
                        COLOR_GREEN, COLOR_BLACK); // green-on-black
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to init_pair(), aborting\n")));
      goto close;
    } // end IF
    result = init_pair (TEST_I_CURSES_COLOR_STATUS,
                        COLOR_BLACK, COLOR_WHITE); // black-on-white
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to init_pair(), aborting\n")));
      goto close;
    } // end IF
  } // end IF

  result = curs_set (TEST_I_CURSES_CURSOR_MODE); // cursor mode
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to curs_set(%d), aborting\n"),
                TEST_I_CURSES_CURSOR_MODE));
    goto close;
  } // end IF
  result = nonl ();
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nonl(), aborting\n")));
    goto close;
  } // end IF

  // step2: set up initial windows
  window_p = newwin (LINES - 2, COLS,
                     0, 0);
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d), aborting\n"),
                LINES - 2, COLS));
    goto close;
  } // end IF
  state_in.log = window_p;
  result = idlok (window_p, TRUE); // hw insert/delete line feature
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to idlok(), aborting\n")));
    goto clean;
  } // end IF
  immedok (window_p, TRUE); // immediate refresh
  wbkgdset (window_p, COLOR_PAIR (TEST_I_CURSES_COLOR_LOG));
  result = box (window_p, 0, 0);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to box(), aborting\n")));
    goto clean;
  } // end IF

//#if defined (__GNUC__) && ((NCURSES_VERSION_MAJOR >= 5) && (NCURSES_VERSION_MINOR >= 9))
////   *BUG*: linux (n)curses newwin() has a bug: _maxy, and _maxx are off by _begy, _begx
//  window_p = newwin (LINES - 4 +1, COLS - 2 +1,
//                     1, 1);
  window_p = newwin (LINES - 4, COLS - 2,
                     1, 1);
//#endif
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d), aborting\n"),
                LINES - 4, COLS - 2));
    goto clean;
  } // end IF
  panel_p = new_panel (window_p);
  if (!panel_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to new_panel(), aborting\n")));
    goto clean;
  } // end IF
  state_in.panels[std::string ()] = panel_p;
  state_in.activePanel = state_in.panels.begin ();

//  result = move_panel (panel_p, 1, 1);
//  if (result == ERR)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to move_panel(), aborting\n")));
//    goto clean;
//  } // end IF
  result = idlok (window_p, TRUE); // hw insert/delete line feature
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to idlok(), aborting\n")));
    goto clean;
  } // end IF
  result = scrollok (window_p, TRUE); // scrolling feature
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scrollok(), aborting\n")));
    goto clean;
  } // end IF
  immedok (window_p, TRUE); // immediate refresh
  wbkgdset (window_p, COLOR_PAIR (TEST_I_CURSES_COLOR_LOG));

  // status window
  state_in.status = newwin (1, COLS,
                            stdscr_p->_maxy - 1, 0);
  if (!state_in.status)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d), aborting\n"),
                1, COLS));
    goto clean;
  } // end IF
  immedok (state_in.status, TRUE);
  result = wbkgd (state_in.status, COLOR_PAIR (TEST_I_CURSES_COLOR_STATUS));
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wbkgd(), aborting\n")));
    goto clean;
  } // end IF

  // input window
  state_in.input = newwin (1, COLS,
                           stdscr_p->_maxy, 0);
  if (!state_in.input)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d), aborting\n"),
                1, COLS));
    goto clean;
  } // end IF
  result = wmove (state_in.input, 0, 0); // move the cursor to the beginning
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), aborting\n")));
    goto clean;
  } // end IF
  immedok (state_in.input, TRUE);

  // step2a: initialize input
  result = noecho (); // disable local echo
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to noecho(), aborting\n")));
    goto clean;
  } // end IF
  result = raw (); // disable line buffering, special character processing
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to raw(), aborting\n")));
    goto clean;
  } // end IF
  result = keypad (state_in.input, TRUE); // enable function/arrow/... keys
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to keypad(), aborting\n")));
    goto clean;
  } // end IF
  result = meta (state_in.input, TRUE); // 8-bit characters
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to meta(), aborting\n")));
    goto clean;
  } // end IF
  mouse_mask = mousemask (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  if (mouse_mask == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to mousemask(), aborting\n")));
    goto clean;
  } // end IF

  // step3b: handle input
  while (true)
  {
    // step3ba: done ?
    if (state_in.finished)
      break; // done

    // step3bb: write backlog
    for (BitTorrent_Client_CursesMessagesIterator_t iterator = state_in.backLog.begin ();
         iterator != state_in.backLog.end ();
         ++iterator)
      for (Common_MessageStackConstReverseIterator_t iterator_2 = (*iterator).second.rbegin ();
           iterator_2 != (*iterator).second.rend ();
           ++iterator_2)
      curses_log ((*iterator).first,
                  *iterator_2,
                  state_in,
                  false); // don't lock
    state_in.backLog.clear ();

    // step3bc: get user input
    { // *IMPORTANT NOTE*: release lock while waiting for input
      ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock);

      ch = wgetch (state_in.input);
    } // end lock scope
    switch (ch)
    {
      case 27: // ESC
      {
        // close connection --> closes session --> closes program
        BITTORRENT_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case CTL_TAB:
#endif
      case '\t':
      {
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//        if (1) // CTRL-TAB ?
//          goto default_key;
#endif
        state_in.activePanel++;
        if (state_in.activePanel == state_in.panels.end ())
          state_in.activePanel = state_in.panels.find (std::string ());
        ACE_ASSERT (state_in.activePanel != state_in.panels.end ());

        // (switch channel,) refresh
        ACE_ASSERT (state_in.log);
        result =
            wbkgd (state_in.log,
                   ((*state_in.activePanel).first.empty () ? COLOR_PAIR (TEST_I_CURSES_COLOR_LOG)
                                                           : COLOR_PAIR (0)));
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wbkgd(), continuing\n")));
        result = top_panel ((*state_in.activePanel).second);
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to top_panel(), continuing\n")));
        update_panels ();
        result = doupdate ();
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to doupdate(), continuing\n")));

        break;
      }
      case '\r': // Apple
      case '\n': // Win32 / UNIX
      case KEY_ENTER:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case PADENTER:
#endif
      {
        // sanity check
        if (message_text.empty () ||
            (*state_in.activePanel).first.empty ())
          break; // nothing to do

        // step1: send the message
        {
          ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_in.sessionState->lock);

          // sanity check (s)
          if (state_in.sessionState->channel.empty ())
          {
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("not in a channel, continuing\n")));
            break;
          } // end IF
        } // end lock scope
        try {
          controller_in->send (receivers, message_text);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in BitTorrent_Client_IIRCControl::send(), aborting\n")));
          goto clean;
        }

        // step2: echo to the local channel window
        curses_log ((*state_in.activePanel).first,
                    message_text,
                    state_in,
                    false);

        // step3: clear the input window
        result = wmove (state_in.input, 0, 0); // reset the cursor
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wmove(), continuing\n")));
        result = wclrtoeol (state_in.input);
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wclrtoeol(), continuing\n")));
//        result = wrefresh (state_in.input);
//        if (result == ERR)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to wrefresh(), continuing\n")));

        message_text.clear ();

        break;
      }
      default:
      {
//default_key:
        result = wechochar (state_in.input, ch);
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wechochar(), continuing\n")));

        message_text += static_cast<char> (ch);

        break;
      }
      case KEY_MOUSE:
      {
        MEVENT mouse_event;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        result = nc_getmouse (&mouse_event);
#else
        result = getmouse (&mouse_event);
#endif
        if (result == ERR)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to getmouse(), returning\n")));
          break;
        } // end IF
        //mouse_mask = getmouse ();
        break;
      }
    } // end SWITCH
  } // end WHILE

  // clean up
clean:
  for (BitTorrent_Client_CursesSessionsIterator_t iterator = state_in.panels.begin ();
       iterator != state_in.panels.end ();
       iterator++)
  {
    window_p = (*iterator).second->win;
    result_2 = del_panel ((*iterator).second);
    if (result_2 == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to del_panel(), continuing\n")));
    result_2 = delwin (window_p);
    if (result_2 == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end FOR
  if (state_in.log)
  {
    result_2 = delwin (state_in.log);
    if (result_2 == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end IF
  if (state_in.status)
  {
    result_2 = delwin (state_in.status);
    if (result_2 == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end IF
  if (state_in.input)
  {
    result_2 = delwin (state_in.input);
    if (result_2 == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end IF

close:
  result_2 = endwin ();
  if (result_2 == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to endwin(), continuing\n")));
  result_2 = refresh (); // restore terminal
  if (result_2 == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to refresh(), continuing\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  delscreen (state_in.screen);
#endif

//release:
  if (release)
  {
    result_2 = state_in.lock.release ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return (result == OK);
}

bool
curses_stop (const std::string& URI_in,
             BitTorrent_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_stop"));

  int result = ERR;

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_in.lock);

  BitTorrent_Client_CursesSessionsIterator_t iterator =
      state_in.panels.find (channel_in);
  ACE_ASSERT (iterator != state_in.panels.end ());
  WINDOW* window_p = (*iterator).second->win;
  result = del_panel ((*iterator).second);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to del_panel(), continuing\n")));
  result = delwin (window_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to delwin(), continuing\n")));
  state_in.panels.erase (iterator);

  // show server log, refresh
  state_in.activePanel = state_in.panels.find (std::string ());
  ACE_ASSERT (state_in.activePanel != state_in.panels.end ());
  ACE_ASSERT (state_in.log);
  result =
      wbkgd (state_in.log, TEST_I_CURSES_COLOR_LOG);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wbkgd(), continuing\n")));
  result = top_panel ((*state_in.activePanel).second);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(), continuing\n")));
  update_panels ();
  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(), continuing\n")));

  return (result == OK);
}
