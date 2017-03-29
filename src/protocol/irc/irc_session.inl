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

#include <ace/Assert.h>
#include <ace/FILE_Addr.h>
#include <ace/FILE_Connector.h>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/OS_Memory.h>

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_tools.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "net_common.h"
#include "net_macros.h"

#include "irc_codes.h"
#include "irc_defines.h"
#include "irc_tools.h"

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::IRC_Session_T (ConnectionManagerType* interfaceHandle_in,
                                             const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
 , close_ (false)
 , inputHandler_ (NULL)
 , logToFile_ (IRC_SESSION_DEF_LOG)
 , output_ (ACE_STREAMBUF_SIZE)
 , shutDownOnEnd_ (true) // *TODO*: allow more sessions
 , UIState_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::IRC_Session_T"));

}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::~IRC_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::~IRC_Session_T"));

  int result = -1;

  if (inputHandler_)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *NOTE*: the input handler runs in a dedicated (detached) thread; there is
    //         really nothing that can be done to stop it, as the thread id is
    //         unknown. The thread will return gracefully (calls handle_close ()
    //         and delete(s) itself) once ACE_STDIN is closed
    //         (see ACE_Event_Handler.cpp:252, IRC_client_inputhandler.cpp:107)
    //         --> nothing to be done here...
#else
    result = inputHandler_->handle_close (ACE_INVALID_HANDLE,
                                          ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_Client_InputHandler::handle_close(): \"%m\", continuing\n")));

    delete inputHandler_;
#endif
  } // end IF

  if (close_)
  {
    //result = file_.close ();
    //if (result == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_FILE_Stream::close(): \"%m\", continuing\n")));
    result = output_.close ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_Client_IOStream_t::close(): \"%m\", continuing\n")));
  } // end IF
}

//template <typename ConnectionType>
//const IRC_ConnectionState&
//IRC_Session_T<ConnectionType>::state () const
//{
//  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::state"));
//
//  return inherited::state_;
//}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::start (Stream_SessionId_t sessionID_in,
                                     const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::start"));

  ACE_UNUSED_ARG (sessionID_in);
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
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module \"%s\" not found, returning\n"),
                ACE_TEXT (IRC_CLIENT_HANDLER_MODULE_NAME)));

    // close connection
    inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

    return;
  } // end IF
  inherited::state_.controller =
//    dynamic_cast<ControllerType*> (const_cast<typename inherited::CONNECTION_BASE_T::STREAM_T::MODULE_T*> (module_p)->writer ());
      dynamic_cast<IRC_IControl*> (const_cast<typename inherited::STREAM_CONNECTION_BASE_T::STREAM_T::MODULE_T*> (module_p)->writer ());
  if (!inherited::state_.controller)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<IRC_IControl*> failed, returning\n"),
                module_p->name ()));

    // close connection
    inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

    return;
  } // end ELSE

  // step0b: set initial nickname
  ConfigurationType* configuration_p = NULL;
  //if (!inherited::manager_)
  //{
  //  const IRC_Client_Configuration& configuration_r = inherited::get ();
  //  configuration_p = &const_cast<IRC_Client_Configuration&> (configuration_r);
  //} // end IF
  //else
    configuration_p = inherited::CONNECTION_BASE_T::configuration_;
  // sanity check(s)
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->protocolConfiguration);
  inherited::state_.nickName =
      configuration_p->protocolConfiguration->loginOptions.nickname;

  // step1: initialize output
  //UIState_ = configuration_p->cursesState;
  logToFile_ = configuration_p->logToFile;
  if (logToFile_)
  {
    std::string file_name =
        Common_File_Tools::getLogDirectory (ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_NAME));
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
    result = connector.connect (output_,
                                address,
                                NULL,
                                ACE_Addr::sap_any,
                                O_CREAT | O_TEXT | O_TRUNC | O_WRONLY,
                                ACE_DEFAULT_FILE_PERMS);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_FILE_Connector::connect(\"%s\"): \"%m\", returning\n"),
                  ACE_TEXT (file_name.c_str ())));

      // close connection
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

      return;
    } // end IF
    close_ = true;
  } // end IF
  else
    output_.set_handle (ACE_STDOUT);

  // step2: initialize input
  if (!configuration_p->cursesState &&
      !inputHandler_)
  {
    ACE_NEW_NORETURN (inputHandler_,
                      InputHandlerType (&(inherited::state_),
                                        configuration_p->useReactor));
    if (!inputHandler_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

      // close connection
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

      return;
    } // end IF
    InputHandlerConfigurationType input_handler_configuration;
    //input_handler_configuration. = &state_;
    // *TODO*: remove type inference
    input_handler_configuration.connectionConfiguration =
      configuration_p;
    if (!inputHandler_->initialize (input_handler_configuration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IRC_Client_InputHandler::initialize(): \"%m\", returning\n")));

      // close connection
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

      return;
    } // end IF
  } // end IF
}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::notify (Stream_SessionId_t sessionID_in,
                                      const Stream_SessionMessageType& sessionEvent_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::end"));

  ACE_UNUSED_ARG (sessionID_in);

  int result = -1;

  // --> raise a signal
  if (shutDownOnEnd_)
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

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::notify (Stream_SessionId_t sessionID_in,
                                      const MessageType& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // *TODO*: remove type inference
  const IRC_Record& record_r = message_in.get ();
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
          log (record_r);
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
          log (record_r);

          break;
        }
        case IRC_Codes::RPL_NOWAWAY:          // 306
        {
          log (record_r);

          break;
        }
        case IRC_Codes::RPL_ENDOFWHO:         // 315
        {
          log (record_r);

          inherited::state_.isFirstMessage = true;

          break;
        }
        case IRC_Codes::RPL_LISTSTART:        // 321
        {
          // *WARNING*: falls through !
        }
        case IRC_Codes::RPL_LISTEND:          // 323
        {
          log (record_r);

          break;
        }
        case IRC_Codes::RPL_LIST:             // 322
        {
          // convert <# visible>
          IRC_ParametersIterator_t iterator = record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 3);
          std::advance (iterator, 2);
          std::stringstream converter;
          int num_members = 0;
          converter << *iterator;
          converter >> num_members;
          iterator--;

          break;
        }
        case IRC_Codes::RPL_NOTOPIC:          // 331
        case IRC_Codes::RPL_TOPIC:            // 332
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
          std::stringstream converter;
          std::string::size_type ws_position = 0;
          ws_position = record_r.parameters_.back ().find (' ', 0);
          converter << record_r.parameters_.back ().substr (0, ws_position);
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

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting nicknames: \"%s\"...\n"),
          //            ACE_TEXT (record_r.params.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string nick;
          string_list_t list;
//          bool is_operator = false;
          do
          {
            current_position =
              record_r.parameters_.back ().find (' ', last_position);
            nick =
              record_r.parameters_.back ().substr (last_position,
              (((current_position == std::string::npos) ? record_r.parameters_.back ().size ()
              : current_position) - last_position));

            //// check whether user is a channel operator
            //if (nick.find (nickname_) != std::string::npos)
            //  is_operator = ((nick[0] == '@') &&
            //  (nick.size () == (nickname_.size () + 1)));

            list.push_back (nick);

            // advance
            last_position = current_position + 1;
          } while (current_position != std::string::npos);

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
        case IRC_Codes::ERR_BADCHANNAME:      // 479
        case IRC_Codes::ERR_CHANOPRIVSNEEDED: // 482
        case IRC_Codes::ERR_UMODEUNKNOWNFLAG: // 501
        {
          log (record_r);

          if ((record_r.command_.numeric == IRC_Codes::ERR_NOSUCHNICK)       ||
              (record_r.command_.numeric == IRC_Codes::ERR_UNKNOWNCOMMAND)   ||
              (record_r.command_.numeric == IRC_Codes::ERR_ERRONEUSNICKNAME) ||
              (record_r.command_.numeric == IRC_Codes::ERR_NICKNAMEINUSE)    ||
              (record_r.command_.numeric == IRC_Codes::ERR_NOTREGISTERED)    ||
              (record_r.command_.numeric == IRC_Codes::ERR_ALREADYREGISTRED) ||
              (record_r.command_.numeric == IRC_Codes::ERR_YOUREBANNEDCREEP) ||
              (record_r.command_.numeric == IRC_Codes::ERR_BADCHANNAME)      ||
              (record_r.command_.numeric == IRC_Codes::ERR_CHANOPRIVSNEEDED) ||
              (record_r.command_.numeric == IRC_Codes::ERR_UMODEUNKNOWNFLAG))
            error (record_r); // show in statusbar as well...

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown (numeric) command/reply (was: \"%s\" (%u)), continuing\n"),
                      ACE_TEXT (IRC_Tools::Command2String (record_r.command_.numeric).c_str ()),
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
        IRC_Tools::Command2Type (*record_r.command_.string);
      switch (command)
      {
        case IRC_Record::NICK:
        {
          // remember changed nick name
          std::string nick_name = inherited::state_.nickName;
          inherited::state_.nickName = record_r.parameters_.front ();

          // *WARNING*: falls through !
        }
        case IRC_Record::USER:
        case IRC_Record::QUIT:
        {
          log (record_r);

          if ((record_r.prefix_.origin == inherited::state_.nickName) &&
              (command == IRC_Record::QUIT))
            error (record_r); // --> show on statusbar as well

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
            if ((inherited::state_.channel.empty ()) &&
                UIState_)
              if (!curses_join (channel,
                                *UIState_))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to curses_join(\"%s\"), continuing\n"),
                            ACE_TEXT (channel.c_str ())));

            inherited::state_.channel = channel;

            break;
          } // end IF

          // someone joined a common channel...
          log (record_r);

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
            if ((!inherited::state_.channel.empty ()) &&
                UIState_)
              if (!curses_part (channel,
                                *UIState_))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to curses_part(\"%s\"), continuing\n"),
                            ACE_TEXT (channel.c_str ())));

            inherited::state_.channel.clear ();
            break;
          } // end IF

          // someone left a common channel...
          log (record_r);

          break;
        }
        case IRC_Record::MODE:
        {
          // there are two possibilities:
          // - user mode message
          // - channel mode message
          log (record_r);

          // retrieve mode string
          IRC_ParametersIterator_t iterator =
            record_r.parameters_.begin ();
          iterator++;

          if (record_r.parameters_.front () == inherited::state_.nickName)
          {
            // --> user mode
            IRC_Tools::merge (record_r.parameters_.back (),
                              inherited::state_.userModes);
          } // end IF
          else
          {
            // --> channel mode
            IRC_Tools::merge (*iterator,
                              inherited::state_.channelModes);
          } // end ELSE

          break;
        }
        case IRC_Record::TOPIC:
        {
          log (record_r);
          break;
        }
        case IRC_Record::KICK:
        {
          log (record_r);

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
//          std::string target_id;
          if (inherited::state_.nickName == record_r.parameters_.front ())
          {
            // --> send to private conversation handler

            // part of an existing conversation ?
          } // end IF

          log (record_r.parameters_.back (),
               message_text);

          break;
        }
        case IRC_Record::NOTICE:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case IRC_Record::__QUIRK__ERROR:
#else
        case IRC_Record::ERROR:
#endif
        case IRC_Record::AWAY:
        {
          log (record_r);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
          if (command == IRC_Record::__QUIRK__ERROR)
#else
          if (command == IRC_Record::ERROR)
#endif
            error (record_r); // --> show on statusbar as well...

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
template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::notify (Stream_SessionId_t sessionID_in,
                                      const SessionMessageType& sessionrecord_r)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionrecord_r);
}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
int
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::open"));

  // step0: initialize this connection
  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener
  const SocketHandlerConfigurationType* socket_handler_configuration_p = NULL;
  switch (this->role ())
  {
    case NET_ROLE_CLIENT:
    {
      typename inherited::ICONNECTOR_T* iconnector_p = NULL;

      // work around ACE code here
      if ((inherited::dispatch () == COMMON_DISPATCH_REACTOR) &&
          (inherited::transportLayer () == NET_TRANSPORTLAYER_TCP))
      {
        //ACE_CONNECTOR_T* connector_p =
        //  static_cast<ACE_CONNECTOR_T*> (arg_in);
        //ACE_ASSERT (connector_p);
        //iconnector_p = dynamic_cast<ICONNECTOR_T*> (connector_p);
        iconnector_p =
            static_cast<typename inherited::ICONNECTOR_T*> (arg_in);
        //if (!iconnector_p)
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to dynamic_cast<Net_IConnector_T*> (argument was: %@), aborting\n"),
        //              connector_p));
        //  return -1;
        //} // end IF
      } // end IF
      else
        iconnector_p =
            static_cast<typename inherited::ICONNECTOR_T*> (arg_in);
      ACE_ASSERT (iconnector_p);
      socket_handler_configuration_p = &iconnector_p->get ();
      break;
    }
    case NET_ROLE_SERVER:
    {
      typename inherited::ILISTENER_T* ilistener_p =
          static_cast<typename inherited::ILISTENER_T*> (arg_in);
      ACE_ASSERT (ilistener_p);
      socket_handler_configuration_p = &ilistener_p->get ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown role (was: %d), aborting\n"),
                  this->role ()));
      return -1;
    }
  } // end SWITCH
  ACE_ASSERT (socket_handler_configuration_p);

  ModuleHandlerConfigurationType* module_handler_configuration_p = NULL;
  if (!inherited::manager_)
  {
    // *TODO*: remove type inference
    ACE_ASSERT (socket_handler_configuration_p->userData);
    module_handler_configuration_p =
      socket_handler_configuration_p->userData->moduleHandlerConfiguration;
    ACE_ASSERT (module_handler_configuration_p);
  } // end IF
  else
  {
    ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
    module_handler_configuration_p =
        inherited::CONNECTION_BASE_T::configuration_->moduleHandlerConfiguration;
  } // end ELSE
  ACE_ASSERT (module_handler_configuration_p);

  module_handler_configuration_p->subscriber = this;
  module_handler_configuration_p->userData =
      socket_handler_configuration_p->userData;
  ACE_ASSERT (module_handler_configuration_p->userData);
//  const IRC_ConnectionState& connection_state_r = inherited::state ();
//  module_handler_configuration_p->userData->connectionState =
//      &const_cast<IRC_ConnectionState&> (connection_state_r);

  // step1: initialize/start stream, tweak socket, register reading data with
  //        reactor, ...
  int result = inherited::open (arg_in);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ConnectionType::open(): \"%m\", continuing\n")));

  return result;
}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::open (ACE_HANDLE handle_in,
                                    ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::open"));

  // sanity checK(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);

  // step0: intialize configuration object
  ModuleHandlerConfigurationType* module_handler_configuration_p = NULL;
//  if (!inherited::manager_)
//  {
//    // *NOTE*: client-side: arg_in is a handle to the connector
//    //         server-side: arg_in is a handle to the listener
//    const IRC_Client_SocketHandlerConfiguration* handler_configuration_p = NULL;
//    switch (inherited::role ())
//    {
//      case NET_ROLE_CLIENT:
//      {
//        typename inherited::ICONNECTOR_T* iconnector_p = inherited::act ();
//        ACE_ASSERT (iconnector_p);
//        handler_configuration_p = &iconnector_p->get ();
//        break;
//      }
//      case NET_ROLE_SERVER:
//      {
//        typename inherited::ILISTENER_T* ilistener_p = inherited::act ();
//        ACE_ASSERT (ilistener_p);
//        handler_configuration_p = &ilistener_p->get ();
//        break;
//      }
//      default:
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("invalid/unknown role (was: %d), aborting\n"),
//                    inherited::role ()));
//        return -1;
//      }
//    } // end SWITCH
//    ACE_ASSERT (handler_configuration_p);
//    // *TODO*: remove type inference
//    module_handler_configuration_p =
//        handler_configuration_p->streamModuleHandlerConfiguration;
//    ACE_ASSERT (module_handler_configuration_p);
//  } // end IF
//  else
  module_handler_configuration_p =
      inherited::CONNECTION_BASE_T::configuration_->moduleHandlerConfiguration;
  // sanity check(s)
  ACE_ASSERT (module_handler_configuration_p);
  module_handler_configuration_p->subscriber = this;
//  ACE_ASSERT (module_handler_configuration_p->userData);
//  const IRC_ConnectionState& connection_state_r = inherited::state ();
//  module_handler_configuration_p->userData->connectionState =
//      &const_cast<IRC_ConnectionState&> (connection_state_r);

  // step1: initialize/start stream, tweak socket, register reading data with
  //        reactor, ...
  inherited::open (handle_in,
                   messageBlock_in);
}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::error (const IRC_Record& record_r)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::error"));

  std::string message_text = IRC_Tools::Record2String (record_r);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received error message: \"%s\"\n"),
              ACE_TEXT (message_text.c_str ())));
}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::log (const std::string& channel_in,
                                   const std::string& messageText_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::log"));

  int result = -1;

  if (UIState_)
  {
    curses_log (channel_in,     // channel
                messageText_in, // text
                *UIState_,      // state
                true);          // locked access

    if (logToFile_)
      output_ << messageText_in;
  } // end IF
  else
    output_ << messageText_in;
  result = output_.sync ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_IOStream_t::sync(): \"%m\", continuing\n")));
}

template <typename ConnectionType,
          typename SessionDataType,
//          typename ControllerType,
          typename NotificationType,
          typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType,
          typename SocketHandlerConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename StateType,
          typename ConnectionManagerType,
          typename InputHandlerType,
          typename InputHandlerConfigurationType,
          typename LogOutputType>
void
IRC_Session_T<ConnectionType,
              SessionDataType,
//              ControllerType,
              NotificationType,
              ConfigurationType,
              MessageType,
              SessionMessageType,
              SocketHandlerConfigurationType,
              ModuleHandlerConfigurationType,
              StateType,
              ConnectionManagerType,
              InputHandlerType,
              InputHandlerConfigurationType,
              LogOutputType>::log (const IRC_Record& record_r)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Session_T::log"));

  std::string message_text = IRC_Tools::Record2String (record_r);
  log (std::string (), // --> server log
       message_text);
}
