#include "stdafx.h"

#include "IRC_client_IRCsession.h"

#include "ace/Assert.h"
#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/OS_Memory.h"

#include "common_defines.h"
#include "common_file_tools.h"
#include "common_tools.h"

#include "net_common.h"
#include "net_macros.h"

#include "IRC_common.h"

#include "IRC_client_curses.h"
#include "IRC_client_defines.h"
#include "IRC_client_network.h"
#include "IRC_client_tools.h"

template <typename ConnectionType>
IRC_Client_IRCSession_T<ConnectionType>::IRC_Client_IRCSession_T (IRC_Client_IConnection_Manager_t* interfaceHandle_in,
                                                                  unsigned int statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
 , close_ (false)
 , inputHandler_ (NULL)
 , logToFile_ (IRC_CLIENT_SESSION_DEF_LOG)
 , output_ (ACE_STREAMBUF_SIZE)
 , shutdownOnEnd_ (true) // *TODO*: allow more sessions
 , state_ ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::IRC_Client_IRCSession_T"));

}

template <typename ConnectionType>
IRC_Client_IRCSession_T<ConnectionType>::~IRC_Client_IRCSession_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::~IRC_Client_IRCSession_T"));

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

template <typename ConnectionType>
void
IRC_Client_IRCSession_T<ConnectionType>::start (const IRC_Client_StreamModuleConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::start"));

  int result = -1;

  // step0a: retrieve controller handle
  const IRC_Client_Stream& stream_r = inherited::stream ();
  const typename inherited::CONNECTION_BASE_T::STREAM_T::MODULE_T* module_p = NULL;
  for (typename inherited::CONNECTION_BASE_T::STREAM_T::ITERATOR_T iterator (stream_r);
       (iterator.next (module_p) != 0);
       iterator.advance ())
    if (ACE_OS::strcmp (module_p->name (),
                        ACE_TEXT_ALWAYS_CHAR (IRC_HANDLER_MODULE_NAME)) == 0)
      break;
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module \"%s\" not found, returning\n"),
                ACE_TEXT (IRC_HANDLER_MODULE_NAME)));

    // close connection
    inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

    return;
  } // end IF
  state_.controller =
    dynamic_cast<IRC_Client_IIRCControl*> (const_cast<typename inherited::CONNECTION_BASE_T::STREAM_T::MODULE_T*> (module_p)->writer ());
  if (!state_.controller)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<IRC_Client_IIRCControl*> failed, returning\n"),
                ACE_TEXT (module_p->name ())));

    // close connection
    inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

    return;
  } // end ELSE

  // step0b: set initial nickname
  IRC_Client_Configuration* configuration_p = NULL;
  if (!inherited::manager_)
  {
    const IRC_Client_Configuration& configuration_r = inherited::get ();
    configuration_p = &const_cast<IRC_Client_Configuration&> (configuration_r);
  } // end IF
  else
    configuration_p = &(inherited::CONNECTION_BASE_T::configuration_);
  // sanity check(s)
  ACE_ASSERT (configuration_p);
  state_.nickname =
    configuration_p->protocolConfiguration.loginOptions.nickname;

  // step1: initialize output
  state_.cursesState = configuration_p->cursesState;
  logToFile_ = configuration_p->logToFile;
  if (logToFile_)
  {
    std::string filename = Common_File_Tools::getLogDirectory ();
    filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    filename += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_SESSION_LOG_FILENAME_PREFIX);
    filename += COMMON_LOG_FILENAME_SUFFIX;
    ACE_FILE_Addr address;
    result = address.set (ACE_TEXT (filename.c_str ()));
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_FILE_Addr::set(\"%s\"): \"%m\", returning\n"),
                  ACE_TEXT (filename.c_str ())));

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
                  ACE_TEXT (filename.c_str ())));

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
                      IRC_Client_InputHandler (&state_,
                                               configuration_p->useReactor));
    if (!inputHandler_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

      // close connection
      inherited::close (NET_CONNECTION_CLOSE_REASON_INITIALIZATION);

      return;
    } // end IF
    IRC_Client_InputHandlerConfiguration input_handler_configuration;
    input_handler_configuration.IRCSessionState = &state_;
    input_handler_configuration.streamConfiguration =
      &configuration_p->streamConfiguration.streamConfiguration;
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

template <typename ConnectionType>
const IRC_Client_SessionState&
IRC_Client_IRCSession_T<ConnectionType>::state () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::state"));

  return state_;
}

template <typename ConnectionType>
void
IRC_Client_IRCSession_T<ConnectionType>::notify (const IRC_Client_IRCMessage& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::notify"));

  switch (message_in.command.discriminator)
  {
    case IRC_Client_IRCMessage::Command::NUMERIC:
    {
      switch (message_in.command.numeric)
      {
        case IRC_Client_IRC_Codes::RPL_WELCOME:          //   1
        {
          // *NOTE*: this is the first message in any connection !

          // *WARNING*: falls through !
        }
        case IRC_Client_IRC_Codes::RPL_YOURHOST:         //   2
        case IRC_Client_IRC_Codes::RPL_CREATED:          //   3
        case IRC_Client_IRC_Codes::RPL_MYINFO:           //   4
        case IRC_Client_IRC_Codes::RPL_PROTOCTL:         //   5
        case IRC_Client_IRC_Codes::RPL_SNOMASK:          //   8
        case IRC_Client_IRC_Codes::RPL_YOURID:           //  42
        case IRC_Client_IRC_Codes::RPL_STATSDLINE:       // 250
        case IRC_Client_IRC_Codes::RPL_LUSERCLIENT:      // 251
        case IRC_Client_IRC_Codes::RPL_LUSEROP:          // 252
        case IRC_Client_IRC_Codes::RPL_LUSERUNKNOWN:     // 253
        case IRC_Client_IRC_Codes::RPL_LUSERCHANNELS:    // 254
        case IRC_Client_IRC_Codes::RPL_LUSERME:          // 255
        case IRC_Client_IRC_Codes::RPL_TRYAGAIN:         // 263
        case IRC_Client_IRC_Codes::RPL_LOCALUSERS:       // 265
        case IRC_Client_IRC_Codes::RPL_GLOBALUSERS:      // 266
        case IRC_Client_IRC_Codes::RPL_INVITING:         // 341
        {
          log (message_in);
          break;
        }
        case IRC_Client_IRC_Codes::RPL_USERHOST:         // 302
        {
          // bisect (WS-separated) userhost records from the final parameter

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting records: \"%s\"...\n"),
          //            ACE_TEXT (message_in.params.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string record;
          string_list_t list;
          do
          {
            current_position = message_in.params.back ().find (' ', last_position);

            record =
              message_in.params.back ().substr (last_position,
              (((current_position == std::string::npos) ? message_in.params.back ().size ()
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
        case IRC_Client_IRC_Codes::RPL_UNAWAY:           // 305
        {
          log (message_in);

          break;
        }
        case IRC_Client_IRC_Codes::RPL_NOWAWAY:          // 306
        {
          log (message_in);

          break;
        }
        case IRC_Client_IRC_Codes::RPL_ENDOFWHO:         // 315
        {
          log (message_in);

          state_.isFirstMessage = true;

          break;
        }
        case IRC_Client_IRC_Codes::RPL_LISTSTART:        // 321
        {
          // *WARNING*: falls through !
        }
        case IRC_Client_IRC_Codes::RPL_LISTEND:          // 323
        {
          log (message_in);

          break;
        }
        case IRC_Client_IRC_Codes::RPL_LIST:             // 322
        {
          // convert <# visible>
          IRC_Client_ParametersIterator_t param_iterator = message_in.params.begin ();
          ACE_ASSERT (message_in.params.size () >= 3);
          std::advance (param_iterator, 2);
          std::stringstream converter;
          int num_members = 0;
          converter << *param_iterator;
          converter >> num_members;
          param_iterator--;

          break;
        }
        case IRC_Client_IRC_Codes::RPL_NOTOPIC:          // 331
        case IRC_Client_IRC_Codes::RPL_TOPIC:            // 332
        case IRC_Client_IRC_Codes::RPL_TOPICWHOTIME:     // 333
          break;
        case IRC_Client_IRC_Codes::RPL_WHOREPLY:         // 352
        {
          // bisect user information from parameter strings
          IRC_Client_ParametersIterator_t iterator_2 =
            message_in.params.begin ();
          ACE_ASSERT (message_in.params.size () >= 8);
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
          ws_position = message_in.params.back ().find (' ', 0);
          converter << message_in.params.back ().substr (0, ws_position);
          converter >> hop_count;
          real_name = message_in.params.back ().substr (ws_position + 1);

          if (state_.isFirstMessage)
            state_.isFirstMessage = false;

          //// ignore own record
          //if (nick == nickname_)
          //  break;

          break;
        }
        case IRC_Client_IRC_Codes::RPL_NAMREPLY:         // 353
        {
          // bisect (WS-separated) nicknames from the final parameter string

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting nicknames: \"%s\"...\n"),
          //            ACE_TEXT (message_in.params.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string nick;
          string_list_t list;
//          bool is_operator = false;
          do
          {
            current_position =
              message_in.params.back ().find (' ', last_position);
            nick =
              message_in.params.back ().substr (last_position,
              (((current_position == std::string::npos) ? message_in.params.back ().size ()
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
          IRC_Client_ParametersIterator_t param_iterator =
            message_in.params.begin ();
          ACE_ASSERT (message_in.params.size () >= 3);
          std::advance (param_iterator, 2);

          break;
        }
        case IRC_Client_IRC_Codes::RPL_ENDOFNAMES:       // 366
        {
          // retrieve channel name
          IRC_Client_ParametersIterator_t param_iterator =
            message_in.params.begin ();
          param_iterator++;

          break;
        }
        case IRC_Client_IRC_Codes::RPL_ENDOFBANLIST:     // 368
        case IRC_Client_IRC_Codes::RPL_MOTD:             // 372
        case IRC_Client_IRC_Codes::RPL_MOTDSTART:        // 375
        case IRC_Client_IRC_Codes::RPL_ENDOFMOTD:        // 376
        case IRC_Client_IRC_Codes::ERR_NOSUCHNICK:       // 401
        case IRC_Client_IRC_Codes::ERR_UNKNOWNCOMMAND:   // 421
        case IRC_Client_IRC_Codes::ERR_NOMOTD:           // 422
        case IRC_Client_IRC_Codes::ERR_NICKNAMEINUSE:    // 433
        case IRC_Client_IRC_Codes::ERR_NOTREGISTERED:    // 451
        case IRC_Client_IRC_Codes::ERR_NEEDMOREPARAMS:   // 461
        case IRC_Client_IRC_Codes::ERR_YOUREBANNEDCREEP: // 465
        case IRC_Client_IRC_Codes::ERR_BADCHANNAME:      // 479
        case IRC_Client_IRC_Codes::ERR_CHANOPRIVSNEEDED: // 482
        case IRC_Client_IRC_Codes::ERR_UMODEUNKNOWNFLAG: // 501
        {
          log (message_in);

          if ((message_in.command.numeric == IRC_Client_IRC_Codes::ERR_NOSUCHNICK)       ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_UNKNOWNCOMMAND)   ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_NICKNAMEINUSE)    ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_NOTREGISTERED)    ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_YOUREBANNEDCREEP) ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_BADCHANNAME)      ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_CHANOPRIVSNEEDED) ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_UMODEUNKNOWNFLAG))
            error (message_in); // show in statusbar as well...

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown (numeric) command/reply (was: \"%s\" (%u)), continuing\n"),
                      ACE_TEXT (IRC_Client_Tools::IRCCode2String (message_in.command.numeric).c_str ()),
                      message_in.command.numeric));

          message_in.dump_state ();

          break;
        }
      } // end SWITCH

      break;
    }
    case IRC_Client_IRCMessage::Command::STRING:
    {
      IRC_Client_IRCMessage::CommandType command =
        IRC_Client_Tools::IRCCommandString2Type (*message_in.command.string);
      switch (command)
      {
        case IRC_Client_IRCMessage::NICK:
        {
          // remember changed nickname...
          std::string old_nick = state_.nickname;
          state_.nickname = message_in.params.front ();

          // *WARNING*: falls through !
        }
        case IRC_Client_IRCMessage::USER:
        case IRC_Client_IRCMessage::QUIT:
        {
          log (message_in);

          if ((message_in.prefix.origin == state_.nickname) &&
              (command == IRC_Client_IRCMessage::QUIT))
            error (message_in); // --> show on statusbar as well...

          break;
        }
        case IRC_Client_IRCMessage::JOIN:
        {
          // there are two possibilities:
          // - reply from a successful join request
          // - stranger entering the channel

          // reply from a successful join request ?
          if (message_in.prefix.origin == state_.nickname)
          {
            std::string channel = message_in.params.front ();
            if ((state_.channel.empty ()) &&
                state_.cursesState)
              if (!curses_join (channel,
                                *state_.cursesState))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to curses_join(\"%s\"), continuing\n"),
                            ACE_TEXT (channel.c_str ())));

            state_.channel = channel;

            break;
          } // end IF

          // someone joined a common channel...
          log (message_in);

          break;
        }
        case IRC_Client_IRCMessage::PART:
        {
          // there are two possibilities:
          // - reply from a (successful) part request
          // - someone left a common channel

          // reply from a successful part request ?
          if (message_in.prefix.origin == state_.nickname)
          {
            std::string channel = message_in.params.front ();
            if ((!state_.channel.empty ()) &&
                 state_.cursesState)
              if (!curses_part (channel,
                                *state_.cursesState))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to curses_part(\"%s\"), continuing\n"),
                            ACE_TEXT (channel.c_str ())));

            state_.channel.clear ();
            break;
          } // end IF

          // someone left a common channel...
          log (message_in);

          break;
        }
        case IRC_Client_IRCMessage::MODE:
        {
          // there are two possibilities:
          // - user mode message
          // - channel mode message
          log (message_in);

          // retrieve mode string
          IRC_Client_ParametersIterator_t param_iterator =
            message_in.params.begin ();
          param_iterator++;

          if (message_in.params.front () == state_.nickname)
          {
            // --> user mode
            IRC_Client_Tools::merge (message_in.params.back (),
                                     state_.userModes);
          } // end IF
          else
          {
            // --> channel mode
            IRC_Client_Tools::merge (*param_iterator,
                                     state_.channelModes);
          } // end ELSE

          break;
        }
        case IRC_Client_IRCMessage::TOPIC:
        {
          log (message_in);
          break;
        }
        case IRC_Client_IRCMessage::KICK:
        {
          log (message_in);

          //// retrieve nickname string
          //IRC_Client_ParametersIterator_t param_iterator =
          //  message_in.params.begin ();
          //param_iterator++;
          break;
        }
        case IRC_Client_IRCMessage::PRIVMSG:
        {
          // *TODO*: parse (list of) receiver(s)

          std::string message_text;
          if (!message_in.prefix.origin.empty ())
          {
            message_text += ACE_TEXT_ALWAYS_CHAR ("<");
            message_text += message_in.prefix.origin;
            message_text += ACE_TEXT_ALWAYS_CHAR ("> ");
          } // end IF
          message_text += message_in.params.back ();

          // private message ?
//          std::string target_id;
          if (state_.nickname == message_in.params.front ())
          {
            // --> send to private conversation handler

            // part of an existing conversation ?
          } // end IF

          log (message_in.params.back (),
               message_text);

          break;
        }
        case IRC_Client_IRCMessage::NOTICE:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case IRC_Client_IRCMessage::__QUIRK__ERROR:
#else
        case IRC_Client_IRCMessage::ERROR:
#endif
        case IRC_Client_IRCMessage::AWAY:
        {
          log (message_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
          if (command == IRC_Client_IRCMessage::__QUIRK__ERROR)
#else
          if (command == IRC_Client_IRCMessage::ERROR)
#endif
            error (message_in); // --> show on statusbar as well...

          break;
        }
        case IRC_Client_IRCMessage::PING:
          break;
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown command (was: \"%s\"), continuing\n"),
                      ACE_TEXT (message_in.command.string->c_str ())));

          message_in.dump_state ();

          break;
        }
      } // end SWITCH

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid command type (was: %u), continuing\n"),
                  message_in.command.discriminator));
      break;
    }
  } // end SWITCH
}

template <typename ConnectionType>
void
IRC_Client_IRCSession_T<ConnectionType>::end ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::end"));

  int result = -1;

  // --> raise a signal
  if (shutdownOnEnd_)
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
int
IRC_Client_IRCSession_T<ConnectionType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::open"));

  // step0: intialize configuration object
  IRC_Client_Configuration* configuration_p = NULL;
  if (!inherited::manager_)
  {
    IRC_Client_Connector_t* connector_p =
      reinterpret_cast<IRC_Client_Connector_t*> (arg_in);
    ACE_ASSERT (connector_p);
    const IRC_Client_SocketHandlerConfiguration* socket_handler_configuration_p =
      connector_p->getConfiguration ();
    ACE_ASSERT (socket_handler_configuration_p);
    //configuration_p = socket_handler_configuration_p->configuration;
  } // end IF
  else
    configuration_p = &(inherited::CONNECTION_BASE_T::configuration_);
  // sanity check(s)
  ACE_ASSERT (configuration_p);
  configuration_p->streamConfiguration.streamModuleConfiguration.subscriber =
    this;

  // step1: initialize/start stream, tweak socket, register reading data with
  //        reactor, ...
  return inherited::open (arg_in);
}

template <typename ConnectionType>
void
IRC_Client_IRCSession_T<ConnectionType>::open (ACE_HANDLE handle_in,
                                               ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::open"));

  // step0: intialize configuration object
  IRC_Client_Configuration* configuration_p = NULL;
  if (!inherited::manager_)
  {
//    const IRC_Client_SocketHandlerConfiguration* socket_handler_configuration_p =
//        reinterpret_cast<IRC_Client_Configuration*> (inherited::act ());
//    ACE_ASSERT (socket_handler_configuration_p);
//    configuration_p = socket_handler_configuration_p->configuration;
  } // end IF
  else
    configuration_p = &(inherited::CONNECTION_BASE_T::configuration_);
  // sanity check(s)
  ACE_ASSERT (configuration_p);
  configuration_p->streamConfiguration.streamModuleConfiguration.subscriber =
    this;

  // step1: initialize/start stream, tweak socket, register reading data with
  //        reactor, ...
  inherited::open (handle_in,
                   messageBlock_in);
}

template <typename ConnectionType>
void
IRC_Client_IRCSession_T<ConnectionType>::error (const IRC_Client_IRCMessage& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::error"));

  std::string message_text = IRC_Client_Tools::IRCMessage2String (message_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received error message: \"%s\"\n"),
              ACE_TEXT (message_text.c_str ())));
}

template <typename ConnectionType>
void
IRC_Client_IRCSession_T<ConnectionType>::log (const std::string& channel_in,
                                              const std::string& messageText_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::log"));

  int result = -1;

  if (state_.cursesState)
  {
    curses_log (channel_in,            // channel
                messageText_in,        // text
                *(state_.cursesState), // state
                true);                 // locked access

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

template <typename ConnectionType>
void
IRC_Client_IRCSession_T<ConnectionType>::log (const IRC_Client_IRCMessage& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_IRCSession_T::log"));

  std::string message_text = IRC_Client_Tools::IRCMessage2String (message_in);
  log (std::string (), // --> server log
       message_text);

}
