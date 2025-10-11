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

#include <sstream>

#include "ace/Assert.h"
#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/OS_Memory.h"

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_string_tools.h"
#include "common_tools.h"

#include "common_log_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H
#include "net_common.h"
#include "net_macros.h"

#include "irc_codes.h"
#include "irc_defines.h"
#include "irc_icontrol.h"
#include "irc_tools.h"

#include "IRC_client_configuration.h"
#if defined (CURSES_SUPPORT)
#include "IRC_client_curses.h"
#endif // CURSES_SUPPORT

template <typename ConnectionType>
IRC_Client_Session_T<ConnectionType>::IRC_Client_Session_T (bool managed_in)
 : inherited (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Session_T::IRC_Client_Session_T"));

}

template <typename ConnectionType>
void
IRC_Client_Session_T<ConnectionType>::start (Stream_SessionId_t sessionId_in,
                                             const struct IRC_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Session_T::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

  int result = -1;

  // step0a: retrieve controller handle
  const typename inherited::STREAM_CONNECTION_BASE_T::STREAM_T& stream_r =
      inherited::stream ();
  const typename inherited::STREAM_CONNECTION_BASE_T::STREAM_T::MODULE_T* module_p =
      NULL;
  for (typename inherited::STREAM_CONNECTION_BASE_T::STREAM_T::ITERATOR_T iterator (stream_r);
       (iterator.next (module_p) != 0);
       iterator.advance ())
    if (ACE_OS::strcmp (module_p->name (),
                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_HANDLER_MODULE_NAME)) == 0)
      break;
  if (unlikely (!module_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module \"%s\" not found, returning\n"),
                ACE_TEXT (IRC_CLIENT_HANDLER_MODULE_NAME)));

    // close connection
    inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

    return;
  } // end IF

  inherited::state_.controller =
    dynamic_cast<IRC_IControl*> (const_cast<typename inherited::STREAM_CONNECTION_BASE_T::STREAM_T::MODULE_T*> (module_p)->writer ());
  if (unlikely (!inherited::state_.controller))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<IRC_IControl*> failed, returning\n"),
                module_p->name ()));

    // close connection
    inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

    return;
  } // end ELSE

    // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->protocolConfiguration);

  inherited::state_.protocolConfiguration =
    inherited::CONNECTION_BASE_T::configuration_->protocolConfiguration;

  // step0b: set initial nickname
  inherited::state_.nickName =
    inherited::CONNECTION_BASE_T::configuration_->protocolConfiguration->loginOptions.nickname;

  // step1: initialize output
  if (inherited::CONNECTION_BASE_T::configuration_->logToFile)
  {
    std::string file_name =
      Common_Log_Tools::getLogDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME));
    file_name += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    file_name += ACE_TEXT_ALWAYS_CHAR (IRC_SESSION_LOG_FILENAME_PREFIX);
    file_name += COMMON_LOG_FILENAME_SUFFIX;
    ACE_FILE_Addr address;
    result = address.set (ACE_TEXT (file_name.c_str ()));
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", returning\n"),
                  ACE_TEXT (file_name.c_str ())));

      // close connection
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

      return;
    } // end IF
    ACE_FILE_Connector connector;
    result = connector.connect (inherited::output_,
                                address,
                                NULL,
                                ACE_Addr::sap_any,
                                O_CREAT | O_TEXT | O_TRUNC | O_WRONLY,
                                ACE_DEFAULT_FILE_PERMS);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", returning\n"),
                  ACE_TEXT (file_name.c_str ())));

      // close connection
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

      return;
    } // end IF
    inherited::close_ = true;
  } // end IF
  else
    inherited::output_.set_handle (ACE_STDOUT);

  // step2: initialize input
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->UIState);
  if ((inherited::CONNECTION_BASE_T::configuration_->UIState->type == COMMON_UI_FRAMEWORK_CONSOLE) &&
      !inherited::inputHandler_)
  {
    ACE_NEW_NORETURN (inherited::inputHandler_,
                      IRC_Client_InputHandler (&(inherited::state_),
                                               (inherited::CONNECTION_BASE_T::configuration_->dispatch == COMMON_EVENT_DISPATCH_REACTOR)));
    if (unlikely (!inherited::inputHandler_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);
      return;
    } // end IF
    struct IRC_Client_InputHandlerConfiguration input_handler_configuration;
    input_handler_configuration.connectionConfiguration =
      inherited::CONNECTION_BASE_T::configuration_;
    if (!inherited::inputHandler_->initialize (input_handler_configuration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_Client_InputHandler::initialize(): \"%m\", returning\n")));
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);
      return;
    } // end IF
  } // end IF
}

template <typename ConnectionType>
void
IRC_Client_Session_T<ConnectionType>::notify (Stream_SessionId_t sessionId_in,
                                              const enum Stream_SessionMessageType& sessionEvent_in,
                                              bool expedite_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Session_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename ConnectionType>
void
IRC_Client_Session_T<ConnectionType>::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Session_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  int result = -1;

  // --> raise a signal
  if (inherited::shutDownOnEnd_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connection %u closed/lost, shutting down\n"),
                inherited::id ()));

    result = ACE_OS::raise (SIGINT);
    if (result == -1)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                  SIGINT));
  } // end IF
}

template <typename ConnectionType>
void
IRC_Client_Session_T<ConnectionType>::notify (Stream_SessionId_t sessionId_in,
                                              const IRC_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Session_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // *TODO*: remove type inference
  const IRC_Record& record_r = message_in.getR ();
  switch (record_r.command_.discriminator)
  {
    case IRC_Record::Command::NUMERIC:
    {
      switch (record_r.command_.numeric)
      {
        case IRC_Codes::RPL_WELCOME:          //   1
        {
          // *NOTE*: this is the first message in any connection !

          // *WARNING*: falls through !
        }
        case IRC_Codes::RPL_YOURHOST:         //   2
        case IRC_Codes::RPL_CREATED:          //   3
        case IRC_Codes::RPL_MYINFO:           //   4
        case IRC_Codes::RPL_PROTOCTL:         //   5
        case IRC_Codes::RPL_SNOMASK:          //   8
        case IRC_Codes::RPL_YOURID:           //  42
        case IRC_Codes::RPL_STATSDLINE:       // 250
        case IRC_Codes::RPL_LUSERCLIENT:      // 251
        case IRC_Codes::RPL_LUSEROP:          // 252
        case IRC_Codes::RPL_LUSERUNKNOWN:     // 253
        case IRC_Codes::RPL_LUSERCHANNELS:    // 254
        case IRC_Codes::RPL_LUSERME:          // 255
        case IRC_Codes::RPL_TRYAGAIN:         // 263
        case IRC_Codes::RPL_LOCALUSERS:       // 265
        case IRC_Codes::RPL_GLOBALUSERS:      // 266
        case IRC_Codes::RPL_INVITING:         // 341
        {
          inherited::log (record_r);
          break;
        }
        case IRC_Codes::RPL_USERHOST:         // 302
        {
          // bisect (WS-separated) userhost records from the final parameter

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting records: \"%s\"...\n"),
          //            ACE_TEXT (record_r.params.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string record;
          string_list_t list;
          do
          {
            current_position = record_r.parameters_.back ().find (' ', last_position);

            record =
              record_r.parameters_.back ().substr (last_position,
              (((current_position == std::string::npos) ? record_r.parameters_.back ().size ()
              : current_position) - last_position));

            // check whether the record is empty
            if (!record.empty ())
              list.push_back (record);

            // advance
            last_position = current_position + 1;
          } while (current_position != std::string::npos);

          std::string message_string;
          for (string_list_const_iterator_t iterator = list.begin ();
               iterator != list.end ();
               iterator++)
          {
            current_position = (*iterator).find ('=', 0);
            if (current_position == std::string::npos)
            {
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("invalid USERHOST record: \"%s\", continuing\n"),
                          ACE_TEXT ((*iterator).c_str ())));
              continue;
            } // end IF

            message_string = (*iterator).substr (0, current_position);
            message_string += ACE_TEXT_ALWAYS_CHAR (" is \"");

            // advance
            last_position = current_position;

            current_position = (*iterator).find ('@', current_position);
            if (current_position == std::string::npos)
            {
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("invalid USERHOST record: \"%s\", continuing\n"),
                          ACE_TEXT ((*iterator).c_str ())));
              continue;
            } // end IF
            message_string += (*iterator).substr (last_position + 1,
                                                  (current_position - last_position - 1));
            message_string += ACE_TEXT_ALWAYS_CHAR ("\" on \"");
            message_string += (*iterator).substr (current_position + 1);
            message_string += ACE_TEXT_ALWAYS_CHAR ("\"\n");

            log (std::string (),
                 message_string);
          } // end FOR

          break;
        }
        case IRC_Codes::RPL_UNAWAY:           // 305
        {
          inherited::log (record_r);

          break;
        }
        case IRC_Codes::RPL_NOWAWAY:          // 306
        {
          inherited::log (record_r);

          break;
        }
        case IRC_Codes::RPL_ENDOFWHO:         // 315
        {
          inherited::log (record_r);

          inherited::state_.isFirstMessage = true;

          break;
        }
        case IRC_Codes::RPL_LISTSTART:        // 321
        {
          // *WARNING*: falls through !
        }
        case IRC_Codes::RPL_LISTEND:          // 323
        {
          inherited::log (record_r);

          break;
        }
        case IRC_Codes::RPL_LIST:             // 322
        {
          // convert <# visible>
          IRC_ParametersIterator_t iterator = record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 3);
          std::advance (iterator, 2);
          std::istringstream converter;
          int num_members = 0;
          converter.str (*iterator);
          converter >> num_members;
          iterator--;

          break;
        }
        case IRC_Codes::RPL_NOTOPIC:          // 331
        case IRC_Codes::RPL_TOPIC:            // 332
        { ACE_ASSERT (record_r.parameters_.size () >= 3);
          IRC_ParametersIterator_t iterator = record_r.parameters_.begin ();
          std::advance (iterator, 1);
          std::string channel = *iterator;
          std::string topic;
          if (record_r.command_.numeric == IRC_Codes::RPL_TOPIC)
          {
            std::advance (iterator, 1);
            topic = *iterator;
          } // end IF

          ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
          ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->UIState);
          if (inherited::CONNECTION_BASE_T::configuration_->UIState->type == COMMON_UI_FRAMEWORK_CURSES)
          {
#if defined (CURSES_SUPPORT)
            curses_topic (channel,
                          topic,
                          *static_cast<IRC_Client_CursesState*> (inherited::CONNECTION_BASE_T::configuration_->UIState));
#endif // CURSES_SUPPORT
          } // end IF

          break;
        }
        case IRC_Codes::RPL_TOPICWHOTIME:     // 333
          break;
        case IRC_Codes::RPL_WHOREPLY:         // 352
        {
          // bisect user information from parameter strings
          IRC_ParametersIterator_t iterator_2 =
            record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 8);
          std::advance (iterator_2, 5); // nick position
          std::string nick = *iterator_2;
          iterator_2++;
          bool away = ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("G"), 0) == 0);
          ACE_UNUSED_ARG (away);
          bool is_IRCoperator =
            ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("*"), 1) == 1);
          ACE_UNUSED_ARG (is_IRCoperator);
          bool is_operator =
            ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("@"), 2) != std::string::npos);
          ACE_UNUSED_ARG (is_operator);
          bool is_voiced =
            ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("+"), 2) != std::string::npos);
          ACE_UNUSED_ARG (is_voiced);
          unsigned int hop_count = 0;
          std::string real_name;
          std::istringstream converter;
          std::string::size_type ws_position = 0;
          ws_position = record_r.parameters_.back ().find (' ', 0);
          converter.str (record_r.parameters_.back ().substr (0, ws_position));
          converter >> hop_count;
          real_name = record_r.parameters_.back ().substr (ws_position + 1);

          if (inherited::state_.isFirstMessage)
            inherited::state_.isFirstMessage = false;

          //// ignore own record
          //if (nick == nickname_)
          //  break;

          break;
        }
        case IRC_Codes::RPL_NAMREPLY:         // 353
        {
          // bisect (WS-separated) nicknames from the final parameter string
          std::vector<std::string> nicknames_a =
            Common_String_Tools::split (record_r.parameters_.back (),
                                        ' ');

          // retrieve channel name
          IRC_ParametersIterator_t iterator =
            record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 3);
          std::advance (iterator, 2);

          break;
        }
        case IRC_Codes::RPL_ENDOFNAMES:       // 366
        {
          // retrieve channel name
          IRC_ParametersIterator_t iterator =
            record_r.parameters_.begin ();
          iterator++;

          break;
        }
        case IRC_Codes::RPL_ENDOFBANLIST:     // 368
        case IRC_Codes::RPL_MOTD:             // 372
        case IRC_Codes::RPL_MOTDSTART:        // 375
        case IRC_Codes::RPL_ENDOFMOTD:        // 376
        case IRC_Codes::RPL_HOSTHIDDEN:       // 396
        case IRC_Codes::ERR_NOSUCHNICK:       // 401
        case IRC_Codes::ERR_UNKNOWNCOMMAND:   // 421
        case IRC_Codes::ERR_NOMOTD:           // 422
        case IRC_Codes::ERR_ERRONEUSNICKNAME: // 432
        case IRC_Codes::ERR_NICKNAMEINUSE:    // 433
        case IRC_Codes::ERR_NOTREGISTERED:    // 451
        case IRC_Codes::ERR_NEEDMOREPARAMS:   // 461
        case IRC_Codes::ERR_ALREADYREGISTRED: // 462
        case IRC_Codes::ERR_YOUREBANNEDCREEP: // 465
        case IRC_Codes::ERR_BANNEDFROMCHAN:   // 474
        case IRC_Codes::ERR_BADCHANNAME:      // 479
        case IRC_Codes::ERR_CHANOPRIVSNEEDED: // 482
        case IRC_Codes::ERR_UMODEUNKNOWNFLAG: // 501
        {
          inherited::log (record_r);

          if ((record_r.command_.numeric == IRC_Codes::ERR_NOSUCHNICK)       ||
              (record_r.command_.numeric == IRC_Codes::ERR_UNKNOWNCOMMAND)   ||
              (record_r.command_.numeric == IRC_Codes::ERR_ERRONEUSNICKNAME) ||
              (record_r.command_.numeric == IRC_Codes::ERR_NICKNAMEINUSE)    ||
              (record_r.command_.numeric == IRC_Codes::ERR_NOTREGISTERED)    ||
              (record_r.command_.numeric == IRC_Codes::ERR_ALREADYREGISTRED) ||
              (record_r.command_.numeric == IRC_Codes::ERR_YOUREBANNEDCREEP) ||
              (record_r.command_.numeric == IRC_Codes::ERR_BANNEDFROMCHAN)   ||
              (record_r.command_.numeric == IRC_Codes::ERR_BADCHANNAME)      ||
              (record_r.command_.numeric == IRC_Codes::ERR_CHANOPRIVSNEEDED) ||
              (record_r.command_.numeric == IRC_Codes::ERR_UMODEUNKNOWNFLAG))
            inherited::error (record_r); // show in statusbar as well...

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown (numeric) command/reply (was: \"%s\" (%u)), continuing\n"),
                      ACE_TEXT (IRC_Tools::CommandToString (record_r.command_.numeric).c_str ()),
                      record_r.command_.numeric));

          record_r.dump_state ();

          break;
        }
      } // end SWITCH

      break;
    }
    case IRC_Record::Command::STRING:
    {
      IRC_Record::CommandType command =
        IRC_Tools::CommandToType (*record_r.command_.string);
      switch (command)
      {
        case IRC_Record::PASS:
        {
          inherited::log (record_r);
  
          break;
        }
        case IRC_Record::NICK:
        { ACE_ASSERT (!record_r.parameters_.empty ());
          // remember changed nick name
          inherited::state_.nickName = record_r.parameters_.front ();

          inherited::log (record_r);

          break;
        }
        case IRC_Record::USER:
        {
          inherited::log (record_r);

          break;
        }
        case IRC_Record::QUIT:
        {
          inherited::log (record_r);

          if (record_r.prefix_.origin == inherited::state_.nickName)
            inherited::error (record_r); // --> show on statusbar as well

          break;
        }
        case IRC_Record::JOIN:
        {
          // there are two possibilities:
          // - reply from a successful join request
          // - stranger entering the channel

          // reply from a successful join request ?
          if (record_r.prefix_.origin == inherited::state_.nickName)
          {
            std::string channel = record_r.parameters_.front ();
            // sanity check(s)
            channels_iterator_t iterator =
              std::find (inherited::state_.channels.begin (),
                         inherited::state_.channels.end (),
                         channel);
            if (unlikely (iterator != inherited::state_.channels.end ()))
            {
              ACE_DEBUG ((LM_WARNING,
                          ACE_TEXT ("already joined channel (was: \"%s\"), continuing\n"),
                          ACE_TEXT (channel.c_str ())));
              break;
            } // end IF
            inherited::state_.channels.push_back (channel);
            inherited::state_.channelModes.insert (std::make_pair (channel, 0));
            inherited::state_.activeChannel = channel;

            ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
            ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->UIState);
            if (inherited::CONNECTION_BASE_T::configuration_->UIState->type == COMMON_UI_FRAMEWORK_CURSES)
            {
#if defined (CURSES_SUPPORT)
              if (!curses_join (channel,
                                *static_cast<IRC_Client_CursesState*> (inherited::CONNECTION_BASE_T::configuration_->UIState)))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to curses_join(\"%s\"), continuing\n"),
                            ACE_TEXT (channel.c_str ())));
#endif // CURSES_SUPPORT
            } // end IF

            break;
          } // end IF

          // someone joined a common channel...
          inherited::log (record_r);

          break;
        }
        case IRC_Record::PART:
        {
          // there are two possibilities:
          // - reply from a (successful) part request
          // - someone left a common channel

          // reply from a successful part request ?
          if (record_r.prefix_.origin == inherited::state_.nickName)
          {
            std::string channel = record_r.parameters_.front ();
            channels_iterator_t iterator =
              std::find (inherited::state_.channels.begin (),
                         inherited::state_.channels.end (),
                         channel);
            if (iterator == inherited::state_.channels.end ())
            {
              ACE_DEBUG ((LM_WARNING,
                          ACE_TEXT ("not in channel (was: \"%s\"), continuing\n"),
                          ACE_TEXT (channel.c_str ())));
              break;
            } // end IF
            inherited::state_.channels.erase (iterator);
            if (inherited::state_.activeChannel == channel)
              inherited::state_.activeChannel.clear ();
            if (inherited::state_.activeChannel.empty () &&
                !inherited::state_.channels.empty ())
              inherited::state_.activeChannel =
                inherited::state_.channels.front ();
            channel_modes_iterator_t iterator_2 =
              inherited::state_.channelModes.find (channel);
            ACE_ASSERT (iterator_2 != inherited::state_.channelModes.end ());
            inherited::state_.channelModes.erase (iterator_2);

            ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
            ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->UIState);
            if (inherited::CONNECTION_BASE_T::configuration_->UIState->type == COMMON_UI_FRAMEWORK_CURSES)
            {
#if defined (CURSES_SUPPORT)
              curses_part (channel,
                           *static_cast<IRC_Client_CursesState*> (inherited::CONNECTION_BASE_T::configuration_->UIState));
#endif // CURSES_SUPPORT
            } // end IF

            break;
          } // end IF

          // someone left a common channel...
          inherited::log (record_r);

          break;
        }
        case IRC_Record::MODE:
        {
          // there are two possibilities:
          // - user mode message
          // - channel mode message
          inherited::log (record_r);

          // retrieve mode string
          IRC_ParametersIterator_t iterator =
            record_r.parameters_.begin ();
          iterator++;
          std::string channel_string;

          if (record_r.parameters_.front () == inherited::state_.nickName)
          {
            // --> user mode
            IRC_Tools::merge (record_r.parameters_.back (),
                              inherited::state_.userModes);
          } // end IF
          else
          {
            // --> channel mode
            channel_string = record_r.parameters_.front ();
            channel_modes_iterator_t iterator_2 =
              inherited::state_.channelModes.find (channel_string);
            ACE_ASSERT (iterator_2 != inherited::state_.channelModes.end ());
            IRC_Tools::merge (*iterator,
                              (*iterator_2).second);
          } // end ELSE

          ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
          ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->UIState);
          if (inherited::CONNECTION_BASE_T::configuration_->UIState->type == COMMON_UI_FRAMEWORK_CURSES)
          {
#if defined (CURSES_SUPPORT)
            curses_mode (channel_string,
                         *static_cast<IRC_Client_CursesState*> (inherited::CONNECTION_BASE_T::configuration_->UIState));
#endif // CURSES_SUPPORT
          } // end IF

          break;
        }
        case IRC_Record::TOPIC:
        {
          inherited::log (record_r);
          break;
        }
        case IRC_Record::KICK:
        {
          inherited::log (record_r);

          //// retrieve nickname string
          //IRC_ParametersIterator_t iterator =
          //  record_r.params.begin ();
          //iterator++;
          break;
        }
        case IRC_Record::PRIVMSG:
        {
          // *TODO*: parse (list of) receiver(s)

          std::string message_text;
          if (!record_r.prefix_.origin.empty ())
          {
            message_text += ACE_TEXT_ALWAYS_CHAR ("<");
            message_text += record_r.prefix_.origin;
            message_text += ACE_TEXT_ALWAYS_CHAR ("> ");
          } // end IF
          message_text += record_r.parameters_.back ();

          // private message ?
          if (inherited::state_.nickName == record_r.parameters_.front ())
          {
            ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
            ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->UIState);
            if (inherited::CONNECTION_BASE_T::configuration_->UIState->type == COMMON_UI_FRAMEWORK_CURSES)
            {
#if defined (CURSES_SUPPORT)
              curses_msg (inherited::state_.nickName,
                          message_text,
                          *static_cast<IRC_Client_CursesState*> (inherited::CONNECTION_BASE_T::configuration_->UIState));
#endif // CURSES_SUPPORT
            } // end IF

            break;
          } // end IF

          log (record_r.parameters_.front (),
               message_text);

          break;
        }
        case IRC_Record::NOTICE:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case IRC_Record::__QUIRK__ERROR:
#else
        case IRC_Record::ERROR:
#endif // ACE_WIN32 || ACE_WIN64
        case IRC_Record::AWAY:
        {
          inherited::log (record_r);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
          if (command == IRC_Record::__QUIRK__ERROR)
#else
          if (command == IRC_Record::ERROR)
#endif // ACE_WIN32 || ACE_WIN64
            inherited::error (record_r); // --> show on statusbar as well...

          break;
        }
        case IRC_Record::PING:
          break;
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown command (was: \"%s\"), continuing\n"),
                      ACE_TEXT (record_r.command_.string->c_str ())));

          record_r.dump_state ();

          break;
        }
      } // end SWITCH

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid command type (was: %u), continuing\n"),
                  record_r.command_.discriminator));
      break;
    }
  } // end SWITCH
}

template <typename ConnectionType>
void
IRC_Client_Session_T<ConnectionType>::notify (Stream_SessionId_t sessionId_in,
                                              const IRC_Client_SessionMessage& sessionrMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Session_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionrMessage_in);
}

template <typename ConnectionType>
void
IRC_Client_Session_T<ConnectionType>::log (const std::string& channel_in,
                                           const std::string& messageText_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Session_T::log"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->UIState);

  if (inherited::CONNECTION_BASE_T::configuration_->logToFile)
    inherited::output_.send (messageText_in.c_str (), messageText_in.size ());

  switch (inherited::CONNECTION_BASE_T::configuration_->UIState->type)
  {
    case COMMON_UI_FRAMEWORK_CONSOLE:
      std::cout << messageText_in;
      break;
    case COMMON_UI_FRAMEWORK_CURSES:
#if defined (CURSES_SUPPORT)
      curses_log (channel_in,                                                                                    // channel
                  messageText_in,                                                                                // text
                  *static_cast<IRC_Client_CursesState*> (inherited::CONNECTION_BASE_T::configuration_->UIState), // state
                  true);                                                                                         // locked access ?
#endif // CURSES_SUPPORT
      break;
    default:
      // *NOTE*: implemented in child classes
      ACE_ASSERT (false);
      ACE_NOTSUP;
      break;
  } // end SWITCH
}
