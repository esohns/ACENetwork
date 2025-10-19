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

#include "ace/Assert.h"
#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Reverse_Lock_T.h"

#include "net_macros.h"

#include "irc_icontrol.h"
#include "irc_record.h"
#include "irc_tools.h"

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
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);
        state_r.finished = true;
      } // end lock scope

      // close connection --> closes session --> closes program
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
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
      if (unlikely (result == ERR))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to getmouse(), aborting\n")));
        return false;
      } // end IF
      char buffer_a[BUFSIZ];
      ACE_OS::snprintf (buffer_a, sizeof (char[BUFSIZ]),
                        ACE_TEXT_ALWAYS_CHAR ("0x%08lx"),
                        mouse_event.bstate);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("mouse @(%d,%d), state: %s\n"),
                  mouse_event.x, mouse_event.y, buffer_a));
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

      // (switch channel,) refresh
      state_r.activePanel++;
      if (state_r.activePanel == state_r.panels.end ())
        state_r.activePanel = state_r.panels.find (std::string ());
      ACE_ASSERT (state_r.activePanel != state_r.panels.end ());
      result = top_panel ((*state_r.activePanel).second);
      if (result == ERR)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to top_panel(), continuing\n")));
      update_panels ();
      result = doupdate ();
      if (unlikely (result == ERR))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to doupdate(), continuing\n")));

      // reset focus to input window
      touchwin (state_r.input);
      wrefresh (state_r.input);

      break;
    }
    case '\r': // Apple
    case '\n': // Win32 / UNIX
    case KEY_ENTER:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case PADENTER:
#endif // ACE_WIN32 || ACE_WIN64
    {
      IRC_CommandType_t command_e =
        IRC_Record::CommandType::IRC_COMMANDTYPE_INVALID;
      string_list_t parameters_a;
      IRC_Tools::parse (state_r.message,
                        command_e,
                        parameters_a);

      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);
        // sanity check(s)
        if (state_r.message.empty () ||                                        // --> no data
            ((*state_r.activePanel).first.empty () &&                          // --> server log ?
             (command_e == IRC_Record::CommandType::IRC_COMMANDTYPE_INVALID))) // --> not a command
          break; // nothing to do
        { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.sessionState->lock, false);
          // sanity check (s)
          if (!state_r.sessionState->channel.empty ())
          {
            state_r.receivers.clear ();
            state_r.receivers.push_front (state_r.sessionState->channel);
          } // end IF
        } // end lock scope

        // step1: parse commands
        switch (command_e)
        {
          case IRC_Record::CommandType::IRC_COMMANDTYPE_INVALID:
            break; // --> send a message to the current channel
          case IRC_Record::CommandType::PASS:
          {
            std::string password_string;
            if (parameters_a.empty ())
            { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.sessionState->lock, false);
              password_string = state_r.sessionState->protocolConfiguration->loginOptions.password;
            } // end IF
            else
              password_string = parameters_a.front ();
            try {
              state_r.controller->pass (password_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::pass(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::NICK:
          {
            std::string nickname_string;
            if (parameters_a.empty ())
            { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.sessionState->lock, false);
              nickname_string = state_r.sessionState->protocolConfiguration->loginOptions.nickname;
            } // end IF
            else
              nickname_string = parameters_a.front ();
            try {
              state_r.controller->nick (nickname_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::nick(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::USER:
          {
            std::string username_string, hostname_string, servername_string, realname_string;
            string_list_const_iterator_t iterator = parameters_a.begin ();
            if (iterator != parameters_a.end ())
            {
              username_string = *iterator;
              std::advance (iterator, 1);
            } // end IF
            else
              username_string = state_r.sessionState->protocolConfiguration->loginOptions.user.userName;

            if (iterator != parameters_a.end ())
            {
              hostname_string = *iterator;
              std::advance (iterator, 1);
            } // end IF
            else
            {
              switch (state_r.sessionState->protocolConfiguration->loginOptions.user.hostName.discriminator)
              {
                case IRC_LoginOptions::User::Hostname::STRING:
                { ACE_ASSERT (state_r.sessionState->protocolConfiguration->loginOptions.user.hostName.string);
                  hostname_string = *state_r.sessionState->protocolConfiguration->loginOptions.user.hostName.string;
                  break;
                }
                case IRC_LoginOptions::User::Hostname::MODE:
                {
                  std::ostringstream converter;
                  converter << static_cast<unsigned int> (state_r.sessionState->protocolConfiguration->loginOptions.user.hostName.mode);
                  hostname_string = converter.str ();
                  break;
                }
                default:
                {
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("invalid USER <host name> parameter field type (was: %d), aborting\n"),
                              state_r.sessionState->protocolConfiguration->loginOptions.user.hostName.discriminator));
                  return false;
                }
              } // end SWITCH
            } // end ELSE
            if (iterator != parameters_a.end ())
            {
              servername_string = *iterator;
              std::advance (iterator, 1);
            } // end IF
            else
              servername_string = state_r.sessionState->protocolConfiguration->loginOptions.user.serverName;
            if (iterator != parameters_a.end ())
              realname_string = *iterator;
            else
              realname_string = state_r.sessionState->protocolConfiguration->loginOptions.user.realName;
            try {
              state_r.controller->user (username_string,
                                        hostname_string,
                                        servername_string,
                                        realname_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::user(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::QUIT:
          {
            std::string reason_string;
            if (!parameters_a.empty ())
              reason_string = parameters_a.front ();

            try {
              state_r.controller->quit (reason_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::quit(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::JOIN:
          {
            std::string channel_string;
            if (!parameters_a.empty ())
              channel_string = parameters_a.front ();
            else
              channel_string = state_r.sessionState->protocolConfiguration->loginOptions.channel;

            // sanity check(s): has '#' prefix ?
            if (channel_string.find ('#', 0) != 0)
              channel_string.insert (channel_string.begin (), '#');

            // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
            // *TODO*: support the CHANNELLEN=xxx "feature" of the server
            if (unlikely (channel_string.size () > IRC_PRT_MAXIMUM_CHANNEL_LENGTH))
              channel_string.resize (IRC_PRT_MAXIMUM_CHANNEL_LENGTH);

            string_list_t channels_a, keys_a;
            channels_a.push_back (channel_string);
            try {
              state_r.controller->join (channels_a,
                                        keys_a);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::join(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::PART:
          {
            string_list_t channels_a;
            std::string channel_string;
            if (parameters_a.empty ())
            {
              if (unlikely ((*state_r.activePanel).first.empty ())) // server log active ?
                goto continue_; // --> ignore command
              channel_string = (*state_r.activePanel).first;
            } // end IF
            else
              channel_string = parameters_a.front ();

            // sanity check(s): has '#' prefix ?
            if (channel_string.find ('#', 0) != 0)
              channel_string.insert (channel_string.begin (), '#');

            // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
            // *TODO*: support the CHANNELLEN=xxx "feature" of the server
            if (unlikely (channel_string.size () > IRC_PRT_MAXIMUM_CHANNEL_LENGTH))
              channel_string.resize (IRC_PRT_MAXIMUM_CHANNEL_LENGTH);

            channels_a.push_back (channel_string);

            try {
              state_r.controller->part (channels_a);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::part(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::MODE:
          { ACE_ASSERT (parameters_a.size () >= 3);
            string_list_const_iterator_t iterator = parameters_a.begin ();
            std::string channel_or_nickname_string = *iterator;
            std::advance (iterator, 1);
            ACE_ASSERT (!(*iterator).empty ());
            char mode_c = (*iterator)[0];
            std::advance (iterator, 1);
            ACE_ASSERT (!(*iterator).empty ());
            std::istringstream converter;
            converter.str (*iterator);
            int enable_i;
            converter >> enable_i;
            std::advance (iterator, 1);
            string_list_t additional_parameters_a;
            std::copy (iterator, parameters_a.cend (), additional_parameters_a.begin ());

            try {
              state_r.controller->mode (channel_or_nickname_string,
                                        mode_c,
                                        (enable_i > 0),
                                        additional_parameters_a);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::mode(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::TOPIC:
          {
            string_list_const_iterator_t iterator = parameters_a.begin ();
            std::string channel_string = (*state_r.activePanel).first;
            if (!parameters_a.empty ())
              channel_string = *iterator;
            std::string topic_string;
            if (!IRC_Tools::isValidChannelName (channel_string))
            {
              topic_string = channel_string;
              channel_string = (*state_r.activePanel).first;
            } // end IF
            else if (parameters_a.size () >= 2)
            {
              std::advance (iterator, 1);
              topic_string = *iterator;
            } // end IF

            try {
              state_r.controller->topic (channel_string,
                                         topic_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::topic(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::NAMES:
          {
            string_list_t channels_a;
            std::string channel_string = (*state_r.activePanel).first;
            if (parameters_a.empty () && !channel_string.empty ())
              channels_a.push_back (channel_string);
            else
              channels_a = parameters_a;
            try {
              state_r.controller->names (channels_a);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::names(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::LIST:
          {
            string_list_t channels_a;
            std::string channel_string = (*state_r.activePanel).first;
            if (parameters_a.empty () && !channel_string.empty ())
              channels_a.push_back (channel_string);
            else
              channels_a = parameters_a;
            try {
              state_r.controller->list (channels_a);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::list(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::INVITE:
          { ACE_ASSERT (parameters_a.size () >= 2);
            string_list_const_iterator_t iterator = parameters_a.begin ();
            ACE_ASSERT (!(*iterator).empty ());
            std::string nickname_string = *iterator;
            std::advance (iterator, 1);
            ACE_ASSERT (!(*iterator).empty ());
            std::string channel_string = *iterator;

            try
            {
              state_r.controller->invite (nickname_string,
                                          channel_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::invite(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::KICK:
          { ACE_ASSERT (parameters_a.size () >= 2);
            string_list_const_iterator_t iterator = parameters_a.begin ();
            ACE_ASSERT (!(*iterator).empty ());
            std::string channel_string = *iterator;
            std::advance (iterator, 1);
            ACE_ASSERT (!(*iterator).empty ());
            std::string client_string = *iterator;
            std::advance (iterator, 1);
            std::string comment_string;
            if (iterator != parameters_a.end () &&
                !(*iterator).empty ())
              comment_string = *iterator;

            try
            {
              state_r.controller->kick (channel_string,
                                        client_string,
                                        comment_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::kick(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::PRIVMSG:
          { ACE_ASSERT (parameters_a.size () >= 2);
            string_list_const_iterator_t iterator = parameters_a.begin ();
            ACE_ASSERT (!(*iterator).empty ());
            std::string target_string = *iterator;
            string_list_t targets_a;
            targets_a.push_back (target_string);
            std::advance (iterator, 1);
            ACE_ASSERT (!(*iterator).empty ());
            std::string message_string = *iterator;

            try {
              state_r.controller->privmsg (targets_a,
                                           message_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::privmsg(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::NOTICE:
          { ACE_ASSERT (parameters_a.size () >= 2);
            string_list_const_iterator_t iterator = parameters_a.begin ();
            ACE_ASSERT (!(*iterator).empty ());
            std::string target_string = *iterator;
            string_list_t targets_a;
            targets_a.push_back (target_string);
            std::advance (iterator, 1);
            ACE_ASSERT (!(*iterator).empty ());
            std::string message_string = *iterator;

            try {
              state_r.controller->notice (targets_a,
                                          message_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::notice(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::AWAY:
          {
            std::string message_string;
            if (!parameters_a.empty ())
              message_string = parameters_a.front ();

            try {
              state_r.controller->away (message_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::away(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::USERHOST:
          {
            try {
              state_r.controller->userhost (parameters_a);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::userhost(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          case IRC_Record::CommandType::KNOCK:
          { ACE_ASSERT (parameters_a.size () >= 1);
            string_list_const_iterator_t iterator = parameters_a.begin ();
            ACE_ASSERT (!(*iterator).empty ());
            std::string channel_string = *iterator;
            std::advance (iterator, 1);
            std::string comment_string;
            if (iterator != parameters_a.end () &&
                !(*iterator).empty ())
              comment_string = *iterator;

            try
            {
              state_r.controller->knock (channel_string,
                                         comment_string);
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("caught exception in IRC_IControl::knock(), aborting\n")));
              return false;
            }

            goto continue_;
          }
          default:
            goto continue_;
        } // end SWITCH

        // step2: send the message
        try {
          state_r.controller->privmsg (state_r.receivers, state_r.message);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in IRC_IControl::privmsg(), aborting\n")));
          return false;
        }

        // step3: echo to the local channel window
        curses_log ((*state_r.activePanel).first,
                    state_r.message,
                    state_r,
                    false);

continue_:
        // step4: clear the input window
        result = wmove (state_r.input, 0, 0); // reset the cursor
        if (unlikely (result == ERR))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wmove(), continuing\n")));
        result = wclrtoeol (state_r.input);
        if (unlikely (result == ERR))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wclrtoeol(), continuing\n")));

        // step5: clean up state
        state_r.message.clear ();
      } // end lock scope

      break;
    }
    case '\b':
    case KEY_BACKSPACE:
    {
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);

      result = wechochar (state_r.input, inputCharacter_in);
      if (unlikely (result == ERR))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to wechochar(), continuing\n")));
      result = wclrtoeol (state_r.input);
        if (unlikely (result == ERR))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to wclrtoeol(), continuing\n")));

      if (!state_r.message.empty ())
        state_r.message.erase (--state_r.message.end ());

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

    result = init_pair (IRC_CLIENT_CURSES_COLOR_CHANNEL,
                        COLOR_WHITE, COLOR_BLACK); // white-on-black
    if (result == ERR)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to init_pair(), aborting\n")));
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
  // (server-)log window
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
  result = scrollok (state_r.log, TRUE); // scrolling feature
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scrollok(), aborting\n")));
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
  panel_p = new_panel (state_r.log);
  if (!panel_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to new_panel(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF
  state_r.panels[std::string ()] = panel_p;
  state_r.activePanel = state_r.panels.begin ();

  result = wmove (state_r.log,
                  getbegy (state_r.log), 3);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));
  result = waddstr (state_r.log, ACE_TEXT_ALWAYS_CHAR ("server log"));
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waddstr(), continuing\n")));

  // status window
  state_r.status = newwin (1, COLS,
                           getmaxy (state_r.std_window) - 2, 0);
  if (!state_r.status)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d), aborting\n"),
                1, COLS));
    result = ERR;
    goto error;
  } // end IF
  immedok (state_r.status, TRUE); // immediate refresh
  wbkgdset (state_r.status, COLOR_PAIR (IRC_CLIENT_CURSES_COLOR_STATUS));

  // input window
  state_r.input = newwin (1, COLS,
                          getmaxy (state_r.std_window) - 1, 0);
  if (!state_r.input)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(%d,%d,%d,%d), aborting\n"),
                1, COLS, getmaxy (state_r.std_window) - 1, 0));
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

  // mouse handling
  keypad (state_r.std_window, TRUE);
  mouse_mask = mousemask (ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  if (mouse_mask == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to mousemask(), aborting\n")));
    result = ERR;
    goto error;
  } // end IF
  // makes the terminal report mouse movement events
  printf ("\033[?1003h\n");

  return true;

error:
  for (Common_UI_Curses_PanelsIterator_t iterator = state_r.panels.begin ();
       iterator != state_r.panels.end ();
       iterator++)
  {
    window_p = panel_window ((*iterator).second);
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
    window_p = panel_window ((*iterator).second);
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
  state_r.log = NULL; // *NOTE*: the log window has already been deleted !
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

  // makes the terminal not report mouse movement events any more
  printf ("\033[?1003l\n");

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
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock,
                                                  ACE_Acquire_Method::ACE_REGULAR);
  IRC_Client_CursesMessagesIterator_t iterator;

  // step3b: handle input
  while (true)
  {
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, false);

    // step3ba: done ?
    if (state_r.finished)
    {
      result = 0/*OK*/;
      break; // done
    } // end IF

    // step3bb: write backlog
    for (IRC_Client_CursesMessagesIterator_t iterator = state_r.backLog.begin ();
         iterator != state_r.backLog.end ();
         ++iterator)
      for (IRC_Client_MessageQueueReverseIterator_t iterator_2 = (*iterator).second.rbegin ();
           iterator_2 != (*iterator).second.rend ();
           ++iterator_2)
      { // *IMPORTANT NOTE*: release lock while waiting for input
        ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock, false);
        curses_log ((*iterator).first,
                    *iterator_2,
                    state_r,
                    true);
      } // end FOR/lock scope
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

  return (result == 0/*OK*/);
}

//////////////////////////////////////////

void
curses_log (const std::string& channel_in,
            const std::string& text_in,
            struct IRC_Client_CursesState& state_in,
            bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_log"));

  int result = ERR;
  WINDOW* window_p = NULL;
  int cury_i;

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
    IRC_Client_CursesMessagesIterator_t iterator_2 =
      state_in.backLog.find (channel_in);
    if (iterator_2 != state_in.backLog.end ())
    {
      (*iterator_2).second.push_back (text_in);
      goto release;
    } // end IF
    ACE_ASSERT (iterator_2 == state_in.backLog.end ());
    IRC_Client_MessageQueue_t message_queue;
    message_queue.push_front (text_in);
    state_in.backLog.insert (std::make_pair (channel_in, message_queue));
    goto release;
  } // end IF
  ACE_ASSERT ((*iterator).second);

  window_p = panel_window ((*iterator).second);
  ACE_ASSERT (window_p);
  cury_i = getcury (window_p);
  if (cury_i == getmaxy (window_p) - 1)
  {
    result = scroll (window_p);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to scroll(), continuing\n")));
    --cury_i;
    result = wmove (window_p,
                    cury_i, 1);
    if (result == ERR)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to wmove(), continuing\n")));
  } // end IF
  ++cury_i;
  result = waddstr (window_p,
                    text_in.c_str ());
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waddstr(), continuing\n")));
  result = wclrtoeol (window_p);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wclrtoeol(), continuing\n")));

  result = box (window_p,
                0, 0);
  if (unlikely (result == ERR))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to box(), continuing\n")));
  result = wmove (window_p,
                  getbegy (window_p), 3);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));
  result = waddstr (window_p,
                    channel_in.empty () ? ACE_TEXT_ALWAYS_CHAR ("server log") : channel_in.c_str ());
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waddstr(), continuing\n")));
  curses_mode (channel_in,
               state_in);

  result = wmove (window_p,
                  cury_i, 1);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));

  // switch to channel, refresh
  state_in.activePanel = iterator;
  result = top_panel ((*iterator).second);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(), continuing\n")));
  update_panels ();
  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(), continuing\n")));

  // reset focus to input window
  touchwin (state_in.input);
  wrefresh (state_in.input);

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
curses_topic (const std::string& channel_in,
              const std::string& topic_in,
              struct IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_topic"));

  std::string message_string =
    ACE_TEXT_ALWAYS_CHAR ("*** topic for ") + channel_in + ACE_TEXT_ALWAYS_CHAR (" is: \"") + topic_in + ACE_TEXT_ALWAYS_CHAR ("\"");

  // echo to the channel window
  curses_log (channel_in,
              message_string,
              state_in,
              true);
}

bool
curses_join (const std::string& channel_in,
             struct IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_join"));

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_in.lock, false);

  int result = -1;
  PANEL* panel_p = NULL;
  WINDOW* window_p = newwin (LINES - 2, COLS,
                             0, 0);
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to newwin(), aborting\n")));
    return false;
  } // end IF
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
  immedok (window_p, TRUE);
  wbkgdset (window_p, COLOR_PAIR (IRC_CLIENT_CURSES_COLOR_CHANNEL));
  result = box (window_p, 0, 0);
  if (result == ERR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to box(), aborting\n")));
    goto error;
  } // end IF
  panel_p = new_panel (window_p);
  if (!panel_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to new_panel(), aborting\n")));
    goto error;
  } // end IF
  state_in.panels[channel_in] = panel_p;

  result = wmove (window_p,
                  getbegy (window_p), 3);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));
  result = waddstr (window_p, channel_in.c_str ());
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waddstr(), continuing\n")));

  result = wmove (window_p,
                  getbegy (window_p) + 1, 1);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));

  // switch to channel, refresh
  state_in.activePanel = state_in.panels.find (channel_in);
  ACE_ASSERT (state_in.activePanel != state_in.panels.end ());
  //result = werase (window_p);
  //if (result == ERR)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to werase(), continuing\n")));
  result = top_panel (panel_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(), continuing\n")));
  update_panels ();
  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(), continuing\n")));

  // reset focus to input window
  touchwin (state_in.input);
  wrefresh (state_in.input);

  return true;

error:
  result = delwin (window_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to delwin(), continuing\n")));
  return false;
}

void
curses_part (const std::string& channel_in,
             struct IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_part"));

  int result = ERR;

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_in.lock);

  Common_UI_Curses_PanelsIterator_t iterator =
    state_in.panels.find (channel_in);
  ACE_ASSERT (iterator != state_in.panels.end ());
  WINDOW* window_p = panel_window ((*iterator).second);
  result = del_panel ((*iterator).second);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to del_panel(), continuing\n")));
  result = delwin (window_p);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to delwin(), continuing\n")));
  state_in.panels.erase (iterator);

  // show new active channel || server log, refresh
  ACE_ASSERT (state_in.sessionState);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_in.sessionState->lock);
    ACE_ASSERT (state_in.sessionState->channel != channel_in);
    state_in.activePanel =
      state_in.panels.find (state_in.sessionState->channel);
  } // end lock scope
  ACE_ASSERT (state_in.activePanel != state_in.panels.end ());
  result = top_panel ((*state_in.activePanel).second);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to top_panel(), continuing\n")));
  update_panels ();
  result = doupdate ();
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to doupdate(), continuing\n")));

  // reset focus to input window
  touchwin (state_in.input);
  wrefresh (state_in.input);
}

void
curses_mode (const std::string& channel_in,
             struct IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_mode"));

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_in.lock);

  int result = -1;
  Common_UI_Curses_PanelsIterator_t iterator =
    state_in.panels.find (channel_in);
  ACE_ASSERT (iterator != state_in.panels.end ());
  std::string mode_string;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_in.sessionState->lock);
    if (channel_in.empty ())
      mode_string = IRC_Tools::UserModeToString (state_in.sessionState->userModes);
    else
    {
      channel_modes_iterator_t iterator_2 =
        state_in.sessionState->channelModes.find (channel_in);
      ACE_ASSERT (iterator_2 != state_in.sessionState->channelModes.end ());
      mode_string = IRC_Tools::ChannelModeToString ((*iterator_2).second);
    } // end ELSE
  } // end lock scope

  WINDOW* window_p = panel_window ((*iterator).second);
  ACE_ASSERT (window_p);

  int cury, curx;
  getyx (window_p, cury, curx);

  result = wmove (window_p,
                  getbegy (window_p), getmaxx (window_p) - (3 + static_cast<int> (mode_string.size ())));
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));
  result =
    waddstr (window_p, mode_string.c_str ());
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to waddstr(), continuing\n")));

  result = wmove (window_p,
                  cury, curx);
  if (result == ERR)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to wmove(), continuing\n")));

  // reset focus to input window
  touchwin (state_in.input);
  wrefresh (state_in.input);
}

void
curses_msg (const std::string& nickName_in,
            const std::string& message_in,
            struct IRC_Client_CursesState& state_in)
{
  NETWORK_TRACE (ACE_TEXT ("::curses_msg"));

  // echo to the active channel
  std::string channel_string;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_in.sessionState->lock);
    channel_string = state_in.sessionState->channel;
  } // end lock scope

  curses_log (channel_string,
              message_in,
              state_in,
              true);
}
