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

//// *WORKAROUND*
//#include <iostream>
//using namespace std;
//// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
////                   a problem in conjunction with the standard include headers
////                   when ACE_USES_OLD_IOSTREAMS is defined
////                   --> include the necessary header(s) manually (see above),
////                       and prevent ace/iosfwd.h from causing any harm
//#define ACE_IOSFWD_H

#include "IRC_client_curses.h"

#include <string>

#include "ace/Assert.h"
#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Reverse_Lock_T.h"

#include "net_macros.h"

#include "irc_icontrol.h"
#include "irc_record.h"

#include "IRC_client_network.h"
#include "IRC_client_stream_common.h"

bool
curses_input (struct Common_UI_Curses_State* state_in,
              int inputCharacter_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_input"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct IRC_Client_CursesState& state_r =
    *static_cast<struct IRC_Client_CursesState*> (state_in);
  ACE_ASSERT (state_r.controller);

  int result = ERR;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock,
                                                  ACE_Acquire_Method::ACE_REGULAR);

  switch (inputCharacter_in)
  {
    case 27: // ESC
    {
      state_r.finished = true;

      // close connection --> closes session --> closes program
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case CTL_TAB:
#endif // ACE_WIN32 || ACE_WIN64
    case '\t':
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//        if (1) // CTRL-TAB ?
//          goto default_key;
#endif // ACE_WIN32 || ACE_WIN64

      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);

      state_r.activePanel++;
      if (state_r.activePanel == state_r.panels.end ())
        state_r.activePanel = state_r.panels.find (std::string ());
      ACE_ASSERT (state_r.activePanel != state_r.panels.end ());

      // (switch channel,) refresh
      ACE_ASSERT (state_r.log);
      result =
          wbkgd (state_r.log,
                  ((*state_r.activePanel).first.empty () ? COLOR_PAIR (IRC_CLIENT_CURSES_COLOR_LOG)
                                                         : COLOR_PAIR (0)));
      if (result == ERR)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to wbkgd(), continuing\n")));
      result = top_panel ((*state_r.activePanel).second);
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
#endif // ACE_WIN32 || ACE_WIN64
    {
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);
        // sanity check
        if (state_r.message.empty () ||
            (*state_r.activePanel).first.empty ())
          break; // nothing to do

        // step1: send the message
        { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.sessionState->lock, false);
          // sanity check (s)
          if (state_r.sessionState->channel.empty ())
          {
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("not in a channel, continuing\n")));
            break;
          } // end IF

          state_r.receivers.clear ();
          state_r.receivers.push_front (state_r.sessionState->channel);
        } // end lock scope
        try {
          state_r.controller->send (state_r.receivers, state_r.message);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in IRC_IControl::send(), aborting\n")));
          return false;
        }
      } // end lock scope

      // step2: echo to the local channel window
      { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock, false);
        curses_log ((*state_r.activePanel).first,
                    state_r.message,
                    state_r,
                    false);
      } // end lock scope

      // step3: clear the input window
      result = wmove (state_r.input, 0, 0); // reset the cursor
      if (result == ERR)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to wmove(), continuing\n")));
      result = wclrtoeol (state_r.input);
      if (result == ERR)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to wclrtoeol(), continuing\n")));
//        result = wrefresh (state_in.input);
//        if (result == ERR)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to wrefresh(), continuing\n")));

      state_r.message.clear ();

      break;
    }
    default:
    {
//default_key:
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);

      result = wechochar (state_r.input, inputCharacter_in);
      if (result == ERR)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to wechochar(), continuing\n")));

      state_r.message += static_cast<char> (inputCharacter_in);

      break;
    }
    case KEY_MOUSE:
    {
      MEVENT mouse_event;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = nc_getmouse (&mouse_event);
#else
      result = getmouse (&mouse_event);
#endif // ACE_WIN32 || ACE_WIN64
      if (result == ERR)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to getmouse(), aborting\n")));
        return false;
      } // end IF
      //mouse_mask = getmouse ();
      break;
    }
  } // end SWITCH

  return true;
}

bool
curses_init (struct Common_UI_Curses_State* state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_init"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct IRC_Client_CursesState& state_r =
    *static_cast<struct IRC_Client_CursesState*> (state_in);
  ACE_ASSERT (!state_r.screen);
  ACE_ASSERT (!state_r.std_window);

  int result = ERR;
  char* string_p = NULL;
  WINDOW* window_p = NULL;
  PANEL* panel_p = NULL;
  mmask_t mouse_mask = 0;

  // lock state
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);

  use_env (TRUE);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  use_tioctl (TRUE);
#endif // ACE_WIN32 || ACE_WIN64
//  nofilter ();

  // *NOTE*: if this fails, the program exits, which is not intended behavior
  // *TODO*: --> use newterm() instead
  //state_in.screen = initscr ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  state_r.screen = newterm (NULL, NULL, NULL); // use $TERM, STD_OUT, STD_IN
  if (!state_r.screen)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newterm(0x%@), aborting\n"),
                NULL));
    result = ERR;
    goto error;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  // *NOTE*: for some (odd) reason, newterm does not work as advertised
  //         (curscr, stdscr corrupt, return value works though)
  state_r.std_window = initscr ();
  if (!state_r.std_window)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initscr(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  state_r.screen = SP;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (state_r.screen && state_r.std_window);

  string_p = longname ();
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to longname(), aborting\n")));
    result = ERR;
    goto error;
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
      goto error;
    } // end IF

    result = init_pair (IRC_CLIENT_CURSES_COLOR_LOG,
                        COLOR_GREEN, COLOR_BLACK); // green-on-black
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to init_pair(), aborting\n")));
      goto error;
    } // end IF
    result = init_pair (IRC_CLIENT_CURSES_COLOR_STATUS,
                        COLOR_BLACK, COLOR_WHITE); // black-on-white
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to init_pair(), aborting\n")));
      goto error;
    } // end IF
  } // end IF

  result = curs_set (IRC_CLIENT_CURSES_CURSOR_MODE); // cursor mode
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to curs_set(%d), aborting\n"),
                IRC_CLIENT_CURSES_CURSOR_MODE));
    goto error;
  } // end IF
  result = nonl ();
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nonl(), aborting\n")));
    goto error;
  } // end IF

  // step2: set up initial windows
  state_r.log = newwin (LINES - 2, COLS,
                        0, 0);
  if (!state_r.log)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d), aborting\n"),
                LINES - 2, COLS));
    result = ERR;
    goto error;
  } // end IF
  result = idlok (state_r.log, TRUE); // hw insert/delete line feature
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to idlok(), aborting\n")));
    goto error;
  } // end IF
  immedok (state_r.log, TRUE); // immediate refresh
  wbkgdset (state_r.log, COLOR_PAIR (IRC_CLIENT_CURSES_COLOR_LOG));
  result = box (state_r.log, 0, 0);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to box(), aborting\n")));
    goto error;
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
    result = ERR;
    goto error;
  } // end IF
  panel_p = new_panel (window_p);
  if (!panel_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to new_panel(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF
  state_r.panels[std::string ()] = panel_p;
  state_r.activePanel = state_r.panels.begin ();

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
    goto error;
  } // end IF
  result = scrollok (window_p, TRUE); // scrolling feature
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scrollok(), aborting\n")));
    goto error;
  } // end IF
  immedok (window_p, TRUE); // immediate refresh
  wbkgdset (window_p, COLOR_PAIR (IRC_CLIENT_CURSES_COLOR_LOG));

  // status window
  state_r.status = newwin (1, COLS,
                           state_r.std_window->_maxy - 2, 0);
  if (!state_r.status)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d), aborting\n"),
                1, COLS));
    result = ERR;
    goto error;
  } // end IF
  immedok (state_r.status, TRUE);
  result = wbkgd (state_r.status, COLOR_PAIR (IRC_CLIENT_CURSES_COLOR_STATUS));
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wbkgd(), aborting\n")));
    goto error;
  } // end IF

  // input window
  state_r.input = newwin (1, COLS,
                          state_r.std_window->_maxy - 1, 0);
  if (!state_r.input)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d,%d,%d), aborting\n"),
                1, COLS, state_r.std_window->_maxy - 1, 0));
    result = ERR;
    goto error;
  } // end IF
  result = wmove (state_r.input, 0, 0); // move the cursor to the beginning
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), aborting\n")));
    goto error;
  } // end IF
  immedok (state_r.input, TRUE);

  // step2a: initialize input
  result = noecho (); // disable local echo
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to noecho(), aborting\n")));
    goto error;
  } // end IF
  result = raw (); // disable line buffering, special character processing
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to raw(), aborting\n")));
    goto error;
  } // end IF
  result = keypad (state_r.input, TRUE); // enable function/arrow/... keys
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to keypad(), aborting\n")));
    goto error;
  } // end IF
  result = meta (state_r.input, TRUE); // 8-bit characters
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to meta(), aborting\n")));
    goto error;
  } // end IF
  mouse_mask = mousemask (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  if (mouse_mask == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to mousemask(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF

  return true;

error:
  for (Common_UI_Curses_PanelsIterator_t iterator = state_r.panels.begin ();
       iterator != state_r.panels.end ();
       iterator++)
  {
    window_p = (*iterator).second->win;
    result = del_panel ((*iterator).second);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to del_panel(), continuing\n")));
    result = delwin (window_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end FOR
  if (state_r.log)
  {
    result = delwin (state_r.log);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end IF
  if (state_r.status)
  {
    result = delwin (state_r.status);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end IF
  if (state_r.input)
  {
    result = delwin (state_r.input);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end IF

  result = endwin ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to endwin(), continuing\n")));
  result = refresh (); // restore terminal
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to refresh(), continuing\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  delscreen (state_r.screen);
#endif // ACE_WIN32 || ACE_WIN64

  return false;
}

bool
curses_fini (struct Common_UI_Curses_State* state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_init"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct IRC_Client_CursesState& state_r =
    *static_cast<struct IRC_Client_CursesState*> (state_in);

  int result = ERR;
  WINDOW* window_p = NULL;

  // lock state
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);

  for (Common_UI_Curses_PanelsIterator_t iterator = state_r.panels.begin ();
       iterator != state_r.panels.end ();
       iterator++)
  {
    window_p = (*iterator).second->win;
    result = del_panel ((*iterator).second);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to del_panel(), continuing\n")));
    result = delwin (window_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
  } // end FOR
  state_r.panels.clear ();
  if (state_r.log)
  {
    result = delwin (state_r.log);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
    state_r.log = NULL;
  } // end IF
  if (state_r.status)
  {
    result = delwin (state_r.status);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
    state_r.status = NULL;
  } // end IF
  if (state_r.input)
  {
    result = delwin (state_r.input);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(), continuing\n")));
    state_r.input = NULL;
  } // end IF

  result = endwin ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to endwin(), continuing\n")));
  result = refresh (); // restore terminal
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to refresh(), continuing\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  delscreen (state_r.screen);
#endif // ACE_WIN32 || ACE_WIN64

  return true;
}

bool
curses_main (struct Common_UI_Curses_State* state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_main"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  struct IRC_Client_CursesState& state_r =
    *static_cast<struct IRC_Client_CursesState*> (state_in);

  int result = ERR;
  int ch = -1;
  bool release = false;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock,
                                                  ACE_Acquire_Method::ACE_REGULAR);
  int result_2 = ERR;
  IRC_Client_CursesMessagesIterator_t iterator;

  // step3b: handle input
  while (true)
  {
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);

    // step3ba: done ?
    if (state_r.finished)
    {
      result = OK;
      break; // done
    } // end IF

    // step3bb: write backlog
    for (IRC_Client_CursesMessagesIterator_t iterator = state_r.backLog.begin ();
         iterator != state_r.backLog.end ();
         ++iterator)
      for (IRC_Client_MessageQueueReverseIterator_t iterator_2 = (*iterator).second.rbegin ();
           iterator_2 != (*iterator).second.rend ();
           ++iterator_2)
      curses_log ((*iterator).first,
                  *iterator_2,
                  state_r,
                  false); // don't lock
    state_r.backLog.clear ();

    // step3bc: get user input
    { // *IMPORTANT NOTE*: release lock while waiting for input
      ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock, false);
      ch = wgetch (state_r.input);
      if (!curses_input (state_in,
                         ch))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::curses_input(%d), aborting\n")));
        result = ERR;
        goto clean;
      } // end IF
    } // end lock scope
  } // end WHILE

clean:
  if (result == ERR)
  {
    //Common_Tools::finalizeEventDispatch (*state_in.dispatchState,
    //                                     false);
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
  } // end IF

  return (result == OK);
}

//////////////////////////////////////////

bool
curses_join (const std::string& channel_in,
             struct IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_join"));

  ACE_UNUSED_ARG (channel_in);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (state_in.lock);

  int result = -1;
  Common_UI_Curses_PanelsIterator_t iterator =
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
  state_in.panels[channel_in] = panel_p;

  // switch to channel, refresh
  state_in.activePanel = state_in.panels.find (channel_in);
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
curses_log (const std::string& channel_in,
            const std::string& text_in,
            struct IRC_Client_CursesState& state_in,
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

  Common_UI_Curses_PanelsIterator_t iterator =
      state_in.panels.find (channel_in);
  if (iterator == state_in.panels.end ())
  {
    // not connected yet/not on a channel --> store
    IRC_Client_MessageQueue_t message_queue;
    message_queue.push_front (text_in);
    state_in.backLog.insert (std::make_pair (channel_in, message_queue));
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
                  (channel_in.empty () ? COLOR_PAIR (IRC_CLIENT_CURSES_COLOR_LOG)
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
curses_part (const std::string& channel_in,
             struct IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_part"));

  int result = ERR;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_in.lock, false);

  Common_UI_Curses_PanelsIterator_t iterator =
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
      wbkgd (state_in.log, IRC_CLIENT_CURSES_COLOR_LOG);
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
