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

#include "IRC_client_curses.h"

#include <string>

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Reverse_Lock_T.h"

#include "panel.h"

#include "net_macros.h"

#include "IRC_client_iIRCControl.h"
#include "IRC_client_IRCmessage.h"

bool
curses_join (const std::string& channel_in,
             IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_join"));

  ACE_UNUSED_ARG (channel_in);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (state_in.lock);

  int result = -1;
  int max_y, max_x;
  PANEL* panel_p = state_in.panels[0];
  ACE_ASSERT (panel_p);
  getmaxyx (panel_p->win, max_y, max_x);
  WINDOW* window_p = newwin (max_y, max_x, 0, 0);
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d): \"%m\", aborting\n"),
                max_y, max_x));
    return false;
  } // end IF
  result = idlok (window_p, TRUE);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to idlok(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = scrollok (window_p, TRUE);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scrollok(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = leaveok (window_p, TRUE);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to leaveok(): \"%m\", aborting\n")));
    return false;
  } // end IF
  immedok (window_p, TRUE);
  //result = werase (window_p);
  //if (result == ERR)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to werase(): \"%m\", aborting\n")));
  //  return false;
  //} // end IF
  result = box (window_p, 0, 0);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to box(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = wmove (window_p, 1, 1);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(): \"%m\", aborting\n")));
    return false;
  } // end IF
  panel_p = new_panel (window_p);
  if (!panel_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to new_panel(): \"%m\", aborting\n")));
    return false;
  } // end IF
  else
    state_in.panels[1] = panel_p;
  result = top_panel (panel_p);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(): \"%m\", aborting\n")));
    return false;
  } // end IF
  result = show_panel (panel_p);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to show_panel(): \"%m\", aborting\n")));
    return false;
  } // end IF
  update_panels ();
  result = doupdate ();
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

void
curses_log (const std::string& text_in,
            IRC_Client_CursesState& state_in,
            bool logToChannel_in,
            bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_log"));

  int result = ERR;

  if (lockedAccess_in)
  {
    result = state_in.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
  } // end IF

  PANEL* panel_p = (logToChannel_in ? state_in.panels[1]
                                    : state_in.panels[0]);
  WINDOW* window_p = (panel_p ? panel_p->win : NULL);
  if (!window_p)
  {
    // not connected yet/not on a channel --> store
    state_in.backLog.push_front (text_in);
    goto release;
  } // end IF
  ACE_ASSERT (window_p);

  result = wmove (window_p, getcury (window_p), 1); // box
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(): \"%m\", continuing\n")));
  result = waddstr (window_p, text_in.c_str ());
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waddstr(): \"%m\", continuing\n")));
  result = wmove (window_p, getcury (window_p) + 1, 0); // next line
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(): \"%m\", continuing\n")));
  result = top_panel (panel_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(): \"%m\", continuing\n")));
  result = show_panel (panel_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to show_panel(): \"%m\", continuing\n")));
  update_panels ();

  //result = wredrawln (state_in.status, getcury (state_in.status), 1);
  //if (result == ERR)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to wredrawln(): \"%m\", continuing\n")));
  //result = wredrawln (state_in.input, getcury (state_in.input), 1);
  //if (result == ERR)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to wredrawln(): \"%m\", continuing\n")));

  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(): \"%m\", continuing\n")));

  state_in.activePanel = (logToChannel_in ? 1 : 0);

release:
  if (lockedAccess_in)
  {
    result = state_in.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF
}

void
curses_main (IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_main"));

  int result = ERR;
  int ch = -1;
  std::string message_text;
  string_list_t receivers;
  bool release = false;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_in.lock,
                                                  ACE_Acquire_Method::ACE_REGULAR);
  int color_pair_log = 1;
  int color_pair_status = 2;
  WINDOW* stdscr_p = NULL;
  mmask_t mouse_mask = 0;
  WINDOW* window_p = NULL;
  PANEL* panel_p = NULL;
  int terminal_y, terminal_x;

  // sanity checks
  ACE_ASSERT (state_in.IRCSessionState);

  // step1: initialize curses

  // grab lock
  result = state_in.lock.acquire ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", returning\n")));
    return;
  } // end IF
  else
    release = true;

  use_env (FALSE);

  // *NOTE*: if this fails, the program exits, which is not intended behavior
  // *TODO*: --> use newterm() instead
  //state_in.screen = initscr ();
  FILE* std_out; FILE* std_in;
  std_out = ACE_OS::fdopen (ACE_STDOUT, ACE_TEXT ("w"));
  if (!std_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::fdopen(%d): \"%m\", returning\n"),
                ACE_STDOUT));
    goto release;
  } // end IF
  std_in = ACE_OS::fdopen (ACE_STDIN, ACE_TEXT ("r"));
  if (!std_in)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::fdopen(%d): \"%m\", returning\n"),
                ACE_STDIN));
    goto close;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: for some (odd) reason, newterm does not work as advertised
  //         (curscr, stdscr corrupt, return value works though)
  stdscr_p = initscr ();
  state_in.screen = SP;
#else
  state_in.screen = newterm (NULL, std_out, std_in); // use $TERM
#endif
  if (!state_in.screen)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newterm(0x%@,%d,%d): \"%m\", returning\n"),
                NULL, ACE_STDOUT, ACE_STDIN));
    goto close;
  } // end IF
  ACE_ASSERT (stdscr_p);
  char* string_p = longname ();
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to longname(): \"%m\", returning\n")));
    goto close;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("initialized curses terminal \"%s\"\n"),
              ACE_TEXT (string_p)));

  if (has_colors ())
  {
    result = start_color (); // intialize colors
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start_color(): \"%m\", returning\n")));
      goto close;
    } // end IF

    result = init_pair (color_pair_log, COLOR_GREEN, COLOR_BLACK); // green-on-black
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to init_pair(): \"%m\", returning\n")));
      goto close;
    } // end IF
    result = init_pair (color_pair_status, COLOR_BLACK, COLOR_WHITE); // black-on-white
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to init_pair(): \"%m\", returning\n")));
      goto close;
    } // end IF
  } // end IF

  //result = raw_output (TRUE);
  //if (result == ERR)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to raw_output(TRUE): \"%m\", returning\n")));
  //  goto close;
  //} // end IF
  result = curs_set (2); // show cursor
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to curs_set(2): \"%m\", returning\n")));
    goto close;
  } // end IF
  result = nonl ();
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nonl(): \"%m\", returning\n")));
    goto close;
  } // end IF

  // step2: set up initial windows
  result = scrollok (stdscr_p, TRUE);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scrollok(): \"%m\", returning\n")));
    goto close;
  } // end IF

  // get maximum window dimensions
  getmaxyx (stdscr_p, terminal_y, terminal_x);

  window_p = newwin (terminal_y - 2, terminal_x, 0, 0);
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(): \"%m\", returning\n")));
    goto close;
  } // end IF
  result = idlok (window_p, TRUE);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to idlok(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  result = scrollok (window_p, TRUE);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scrollok(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  result = leaveok (window_p, TRUE);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to leaveok(): \"%m\", aborting\n")));
    goto clean;
  } // end IF
  immedok (window_p, TRUE);
  result = wbkgd (window_p, COLOR_PAIR (color_pair_log));
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wbkgd(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  result = wmove (window_p, 1, 0); // box
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(): \"%m\", continuing\n")));
    goto clean;
  } // end IF
  panel_p = new_panel (window_p);
  if (!panel_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to new_panel(): \"%m\", aborting\n")));
    goto close;
  } // end IF
  else
    state_in.panels[0] = panel_p;

  // status window
  state_in.status = newwin (1, terminal_x, terminal_y - 2, 0);
  if (!state_in.status)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  immedok (state_in.status, TRUE);
  //result = clearok (state_in.status, TRUE);
  //if (result == ERR)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to clearok(): \"%m\", returning\n")));
  //  goto clean;
  //} // end IF
  result = wbkgd (state_in.status, COLOR_PAIR (color_pair_status));
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wbkgd(): \"%m\", returning\n")));
    goto clean;
  } // end IF

  state_in.input = newwin (1, terminal_x, terminal_y - 1, 0);
  if (!state_in.input)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  result = wmove (state_in.input, 0, 0); // move the cursor to the beginning
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  immedok (state_in.input, TRUE);

  // step2a: draw box, move cursor position, refresh
  result = box (window_p, 0, 0);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to box(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  result = wmove (window_p, 1, 1);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(): \"%m\", returning\n")));
    goto clean;
  } // end IF

  // step3a: initialize input
  result = noecho (); // disable local echo
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to noecho(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  result = raw (); // disable line buffering, special character processing
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to raw(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  result = keypad (state_in.input, TRUE); // enable function/arrow/... keys
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to keypad(): \"%m\", returning\n")));
    goto clean;
  } // end IF
  mouse_mask = mousemask (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  if (mouse_mask == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to mousemask(): \"%m\", returning\n")));
    goto clean;
  } // end IF

  // step3b: handle input
  while (true)
  {
    // step3ba: done ?
    if (state_in.finished)
      break; // done

    // step3bb: write backlog
    for (IRC_Client_MessageQueueIterator_t iterator = state_in.backLog.rbegin ();
         iterator != state_in.backLog.rend ();
         iterator++)
      curses_log (*iterator,
                  state_in,
                  false,  // log to server log
                  false); // don't lock
    state_in.backLog.clear ();

    // step3bc: get user input
    { // *IMPORTANT NOTE*: release lock while waiting for input
      ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_MUTEX> > aGuard (reverse_lock);

      ch = wgetch (state_in.input);
    } // end lock scope
    switch (ch)
    {
      case CTL_TAB:
      {
        PANEL* panel_p = state_in.panels[state_in.activePanel];
        if (!panel_p)
          break; // done
        result = hide_panel (panel_p);
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to hide_panel(): \"%m\", continuing\n")));

        state_in.activePanel++;
        if (state_in.activePanel == (sizeof (state_in.panels) / sizeof (PANEL*)))
          state_in.activePanel = 0;

        panel_p = state_in.panels[state_in.activePanel];
        result = top_panel (panel_p);
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to top_panel(): \"%m\", continuing\n")));
        result = show_panel (panel_p);
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to show_panel(): \"%m\", continuing\n")));
        update_panels ();
        result = doupdate ();
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to doupdate(): \"%m\", continuing\n")));
        break;
      }
      case '\r': // Apple
      case '\n': // Win32 / UNIX
      case KEY_ENTER:
      case PADENTER:
      {
        // sanity check
        if (message_text.empty ())
          break; // nothing to do

        // step1: send the message
        ACE_ASSERT (state_in.IRCSessionState->controller);
        {
          ACE_Guard<ACE_SYNCH_MUTEX> aGuard (state_in.IRCSessionState->lock);

          // sanity check (s)
          if (state_in.IRCSessionState->channel.empty ())
          {
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("not in a channel, continuing\n")));
            break;
          } // end IF

          receivers.clear ();
          receivers.push_front (state_in.IRCSessionState->channel);
        } // end lock scope
        try
        {
          state_in.IRCSessionState->controller->send (receivers, message_text);
        }
        catch (...)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in IRC_Client_IIRCControl::send(), aborting\n")));
          goto clean;
        }

        // step2: echo to the channel window
        curses_log (message_text,
                    state_in,
                    true,
                    false);

        // step3: clear the input window
        result = werase (state_in.input);
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to werase(): \"%m\", continuing\n")));
        result = wmove (state_in.input, 0, 0); // move the cursor to the beginning
        if (result == ERR)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wmove(): \"%m\", continuing\n")));
        //result = wrefresh (state_in.input);
        //if (result == ERR)
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to wrefresh(): \"%m\", continuing\n")));

        message_text.clear ();

        break;
      }
      default:
      {
        wechochar (state_in.input, ch);

        message_text += static_cast<char> (ch);

        break;
      }
      case KEY_MOUSE:
      {
        MEVENT mouse_event;
        result = nc_getmouse (&mouse_event);
        if (result == ERR)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to nc_getmouse(): \"%m\", returning\n")));
          break;
        } // end IF
        //mouse_mask = getmouse ();
        break;
      }
    } // end SWITCH
  } // end WHILE
 
  // clean up
clean:
  window_p = NULL;
  panel_p = state_in.panels[1];
  if (panel_p)
  {
    window_p = panel_p->win;
    result = del_panel (panel_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to del_panel(): \"%m\", continuing\n")));
  } // end IF
  if (window_p)
  {
    result = delwin (window_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(): \"%m\", continuing\n")));
    window_p = NULL;
  } // end IF
  panel_p = state_in.panels[0];
  if (panel_p)
  {
    window_p = panel_p->win;
    result = del_panel (panel_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to del_panel(): \"%m\", continuing\n")));
  } // end IF
  if (window_p)
  {
    result = delwin (window_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(): \"%m\", continuing\n")));
  } // end IF
  if (state_in.status)
  {
    result = delwin (state_in.status);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(): \"%m\", continuing\n")));
  } // end IF
  if (state_in.input)
  {
    result = delwin (state_in.input);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to delwin(): \"%m\", continuing\n")));
  } // end IF

close:
  result = endwin ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to endwin(): \"%m\", continuing\n")));
  result = refresh (); // restore terminal
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to refresh(): \"%m\", continuing\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  delscreen (state_in.screen);
#endif
  if (std_out)
  {
    result = ACE_OS::fclose (std_out);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::flcose(%d): \"%m\", continuing\n"),
                  ACE_STDOUT));
  } // end IF
  if (std_in)
  {
    result = ACE_OS::fclose (std_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::flcose(%d): \"%m\", continuing\n"),
                  ACE_STDIN));
  } // end IF

release:
  if (release)
  {
    result = state_in.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF
}

bool
curses_part (const std::string& channel_in,
             IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_part"));

  ACE_UNUSED_ARG (channel_in);

  int result = ERR;

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (state_in.lock);

  PANEL* panel_p = state_in.panels[1];
  ACE_ASSERT (panel_p);
  WINDOW* window_p = panel_p->win;
  result = del_panel (state_in.panels[1]);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to del_panel(): \"%m\", continuing\n")));
  state_in.panels[1] = NULL;
  result = delwin (window_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to delwin(): \"%m\", continuing\n")));
  panel_p = state_in.panels[0];
  ACE_ASSERT (panel_p);
  result = top_panel (panel_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(): \"%m\", continuing\n")));
  result = show_panel (panel_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to show_panel(): \"%m\", continuing\n")));
  update_panels ();
  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(): \"%m\", continuing\n")));

  state_in.activePanel = 0;

  return (result == OK);
}
