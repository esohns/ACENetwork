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

#include "ace/ACE.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"
#endif // GTK_SUPPORT

#include "net_macros.h"

#include "irc_tools.h"

#include "IRC_client_network.h"
#include "IRC_client_stream_common.h"
#include "IRC_client_tools.h"

#if defined (GTK_SUPPORT)
#include "IRC_client_gui_callbacks.h"
#endif // GTK_SUPPORT
#include "IRC_client_gui_defines.h"
#include "IRC_client_gui_messagehandler.h"

template <enum Common_UI_FrameworkType GUIType>
IRC_Client_GUI_Connection_T<GUIType>::IRC_Client_GUI_Connection_T (IRC_Client_GUI_Connections_t* connections_in,
                                                                   struct IRC_Client_UI_HandlerCBData* CBData_in,
                                                                   const std::string& label_in,
                                                                   const std::string& UIFileDirectory_in)
 : CBData_ ()
 , closing_ (false)
 , isFirstUsersMsg_ (true)
 , sessionState_ (NULL)
 , UIFileDirectory_ (UIFileDirectory_in)
 , lock_ ()
 , messageHandlers_ ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::IRC_Client_GUI_Connection_T"));

  // sanity check(s)
  ACE_ASSERT (connections_in);
  ACE_ASSERT (CBData_in);
  if (!Common_File_Tools::isDirectory (UIFileDirectory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: \"%s\"): not a directory, returning\n"),
                ACE_TEXT (UIFileDirectory_in.c_str ())));
    return;
  } // end IF

  // initialize cb data
  CBData_.connections = connections_in;
  CBData_.CBData = CBData_in;
  //   CBData_.nick.clear(); // cannot set this now...
  CBData_.label = label_in;
  ACE_TCHAR time_stamp[27]; // ISO-8601 format
  ACE_OS::memset (&time_stamp, 0, sizeof (time_stamp));
  ACE_TCHAR* result_p = ACE::timestamp (COMMON_TIME_NOW,
                                        time_stamp,
                                        sizeof (time_stamp),
                                        false);
  if (result_p == NULL)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::timestamp(): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_.timeStamp = ACE_TEXT_ALWAYS_CHAR (time_stamp);
}

template <enum Common_UI_FrameworkType GUIType>
IRC_Client_GUI_Connection_T<GUIType>::~IRC_Client_GUI_Connection_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::~IRC_Client_GUI_Connection_T"));

}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::initialize (struct IRC_SessionState* sessionState_in,
                                                  IRC_IControl* controller_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::initialize"));

  ACE_ASSERT (controller_in);
  ACE_ASSERT (sessionState_in);

  CBData_.controller = controller_in;
  sessionState_ = sessionState_in;
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::finalize (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::finalize"));

  if (lockedAccess_in)
  {
  } // end IF

  // clean up message handlers
  const IRC_Client_UI_HandlerCBData* ui_cb_data_p = NULL;
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::start (Stream_SessionId_t sessionId_in,
                                             const IRC_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::notify (Stream_SessionId_t sessionId_in,
                                              const Stream_SessionMessageType& sessionEvent_in,
                                              bool expedite_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE(ACE_TEXT("IRC_Client_GUI_Connection_T::end"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection \"%s\" closed/lost\n"),
              ACE_TEXT (CBData_.label.c_str ())));

//  // *NOTE*: this is the final invocation from the controller
//  //         --> unsubscribe anyway
//  if (CBData_.controller)
//  {
//    try {
//      CBData_.controller->unsubscribe (this);
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in IRC_IControl::unsubscribe(%@), continuing\n"),
//                  this));
//    }
//    CBData_.controller = NULL;
//  } // end IF

  close ();
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::notify (Stream_SessionId_t sessionId_in,
                                              const IRC_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

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

          // remember nickname
          ACE_ASSERT (sessionState_);
          sessionState_->nickName = record_r.parameters_.front ();

          // *WARNING*: falls through !
        }
        case IRC_Codes::RPL_YOURHOST:         //   2
        case IRC_Codes::RPL_CREATED:          //   3
        case IRC_Codes::RPL_MYINFO:           //   4
        case IRC_Codes::RPL_PROTOCTL:         //   5
        case IRC_Codes::RPL_SNOMASK:          //   8
        case IRC_Codes::RPL_YOURID:           //  20
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
          //            ACE_TEXT (record_r.parameters_.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string record;
          string_list_t list;
          do
          {
            current_position =
                record_r.parameters_.back ().find (' ', last_position);

            record =
              record_r.parameters_.back ().substr (last_position,
                                                   (((current_position == std::string::npos) ? record_r.parameters_.back ().size ()
                                                                                             : current_position) -
                                                    last_position));

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

            log (message_string);
          } // end FOR

          break;
        }
        case IRC_Codes::RPL_UNAWAY:           // 305
        {
          // *WARNING*: needs the lock protection, otherwise there is a race...
          ACE_ASSERT (sessionState_);
          sessionState_->away = false;

          log (record_r);

          break;
        }
        case IRC_Codes::RPL_NOWAWAY:          // 306
        {
          // *WARNING*: needs the lock protection, otherwise there is a race...
          ACE_ASSERT (sessionState_);
          sessionState_->away = true;

          log (record_r);

          break;
        }
        case IRC_Codes::RPL_ENDOFWHO:         // 315
        {
          //log (message_in);

          isFirstUsersMsg_ = true;

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
          break;
        }
        case IRC_Codes::RPL_NOTOPIC:          // 331
        case IRC_Codes::RPL_TOPIC:            // 332
        case IRC_Codes::RPL_TOPICWHOTIME:     // 333
        {
          IRC_ParametersIterator_t iterator_2 =
            record_r.parameters_.begin ();
          iterator_2++;

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (*iterator_2);
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT ((*iterator_2).c_str ())));
            break;
          } // end IF

          if ((record_r.command_.numeric == IRC_Codes::RPL_NOTOPIC) ||
              (record_r.command_.numeric == IRC_Codes::RPL_TOPIC))
            (*handler_iterator).second->setTopic (record_r.parameters_.back ());

          break;
        }
        case IRC_Codes::RPL_WHOREPLY:         // 352
        {
          // bisect user information from parameter strings
          IRC_ParametersIterator_t iterator_2 =
            record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 8);
          std::advance (iterator_2, 5); // nick position
          std::string nickname = *iterator_2;
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

          break;
        }
        case IRC_Codes::RPL_NAMREPLY:         // 353
        {
          // bisect (WS-separated) nicknames from the final parameter string

          // *NOTE*: UnrealIRCd 3.2.10.4 has trailing whitespace...
          std::string& back =
              const_cast<IRC_Record&> (record_r).parameters_.back ();

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting nicknames: \"%s\"...\n"),
          //            ACE_TEXT (back.c_str ())));

          // step1: trim trailing WS
          size_t position = back.find_last_not_of (' ', std::string::npos);
          if (position != std::string::npos)
            back.erase (++position, std::string::npos);

          std::istringstream converter (back);
          std::string nickname;
          string_list_t list;
          bool is_operator = false;
          while (!converter.eof ())
          {
            converter >> nickname;

            // check whether user is a channel operator
            ACE_ASSERT (sessionState_);
            if (nickname.find (sessionState_->nickName) != std::string::npos)
              is_operator = ((nickname[0] == '@') &&
                             (nickname.size () == (sessionState_->nickName.size () + 1)));

            list.push_back (nickname);
          } // end WHILE

          // retrieve channel name
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 3);
          std::advance (param_iterator, 2);

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (*param_iterator);
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT ((*param_iterator).c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->members (list, false);

          // user is operator ? --> set channel mode accordingly
          if (is_operator)
          {
            // *NOTE*: ops always have a voice...
            std::string op_mode = ACE_TEXT_ALWAYS_CHAR ("+ov");
            (*handler_iterator).second->setModes (op_mode,
                                                  std::string (), // none
                                                  false);
          } // end IF

          break;
        }
        case IRC_Codes::RPL_ENDOFNAMES:       // 366
        {
          // retrieve channel name
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
          param_iterator++;

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (*param_iterator);
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT ((*param_iterator).c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->endMembers (false);

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
            error (record_r, // show in statusbar as well...
                   false);

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown (numeric) command/reply (was: \"%s\" (%u)), continuing\n"),
                      ACE_TEXT (IRC_Tools::CommandToString (record_r.command_.numeric).c_str ()),
                      record_r.command_.numeric));

          message_in.dump_state ();

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
        case IRC_Record::NICK:
        {
          // remember changed nickname...
          ACE_ASSERT (sessionState_);
          std::string old_nickname = sessionState_->nickName;
          sessionState_->nickName = record_r.parameters_.front ();

          // step2: update channel tab nickname label(s)
          for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
               iterator != messageHandlers_.end ();
               iterator++)
          {
            if ((*iterator).second->isServerLog ())
              continue;

            (*iterator).second->update (old_nickname);
          } // end FOR

          // *WARNING*: falls through !
        }
        case IRC_Record::USER:
        case IRC_Record::QUIT:
        {
          log (record_r);

          ACE_ASSERT (sessionState_);
          if ((record_r.prefix_.origin == sessionState_->nickName) &&
              (command == IRC_Record::QUIT))
            error (record_r, // --> show on statusbar as well...
                   false);

          break;
        }
        case IRC_Record::JOIN:
        {
          log (record_r);

          // there are two possibilities:
          // - reply from a successful join request
          // - stranger entering the channel

          // reply from a successful join request ?
          ACE_ASSERT (sessionState_);
          if (record_r.prefix_.origin == sessionState_->nickName)
          {
            createMessageHandler (record_r.parameters_.front (),
                                  false);

//            // query channel members
//            string_list_t channels;
//            channels.push_back (record_r.parameters_.front ());
//            ACE_ASSERT (CBData_.controller);
//            try {
//              CBData_.controller->names (channels);
//            } catch (...) {
//              ACE_DEBUG ((LM_ERROR,
//                          ACE_TEXT ("caught exception in IRC_Client_IIRCControl::names(), continuing\n")));
//            }

            break;
          } // end IF

          // someone joined a common channel...

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.back ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.back ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->add (record_r.prefix_.origin,
                                           false);

          break;
        }
        case IRC_Record::PART:
        {
          log (record_r);

          // there are two possibilities:
          // - reply from a (successful) part request
          // - someone left a common channel

          // reply from a successful part request ?
          ACE_ASSERT (sessionState_);
          if (record_r.prefix_.origin == sessionState_->nickName)
          {
            terminateMessageHandler (record_r.parameters_.back (),
                                     false);
            break;
          } // end IF

          // someone left a common channel...

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.back ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.back ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->remove (record_r.prefix_.origin,
                                              false);

          break;
        }
        case IRC_Record::MODE:
        {
          log (record_r);

          // there are two possibilities:
          // - user mode message
          // - channel mode message

          // retrieve mode string
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
          param_iterator++;

          ACE_ASSERT (sessionState_);
          if (record_r.parameters_.front () == sessionState_->nickName)
          {
            // --> user mode
            // *WARNING*: needs the lock protection, otherwise there is a race...
            CBData_.acknowledgements +=
              IRC_Tools::merge (record_r.parameters_.back (),
                                       sessionState_->userModes);
          } // end IF
          else
          {
            // --> channel mode

            // retrieve message handler
            MESSAGE_HANDLERSITERATOR_T handler_iterator =
              messageHandlers_.find (record_r.parameters_.front ());
            if (handler_iterator == messageHandlers_.end ())
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                          ACE_TEXT (record_r.parameters_.front ().c_str ())));
              break;
            } // end IF

            // *WARNING*: needs the lock protection, otherwise there is a race...
            (*handler_iterator).second->setModes (*param_iterator,
                                                  ((*param_iterator == record_r.parameters_.back ()) ? std::string ()
                                                                                                  : record_r.parameters_.back ()),
                                                  false);
          } // end ELSE

          break;
        }
        case IRC_Record::TOPIC:
        {
          log (record_r);

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.front ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.front ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->setTopic (record_r.parameters_.back ());

          break;
        }
        case IRC_Record::KICK:
        {
          log (record_r);

          // retrieve nickname string
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
          param_iterator++;

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.front ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.front ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->remove (*param_iterator,
                                              false);

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
          std::string target_id;
          ACE_ASSERT (sessionState_);
          if (sessionState_->nickName == record_r.parameters_.front ())
          {
            // --> send to private conversation handler

            // part of an existing conversation ?

            // retrieve message handler
            if (messageHandlers_.find (record_r.prefix_.origin) == messageHandlers_.end ())
              createMessageHandler (record_r.prefix_.origin,
                                    false);
          } // end IF

          // channel/nick message ?
          forward (((sessionState_->nickName == record_r.parameters_.front ()) ? record_r.prefix_.origin
                                                                               : record_r.parameters_.front ()),
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
            error (record_r, // --> show on statusbar as well...
                   false);

          break;
        }
        case IRC_Record::PING:
          break;
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("unknown/invalid command (was: \"%s\"), continuing\n"),
                      ACE_TEXT (record_r.command_.string->c_str ())));

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
                  record_r.command_.discriminator));
      break;
    }
  } // end SWITCH
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::notify (Stream_SessionId_t sessionId_in,
                                              const IRC_Client_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE(ACE_TEXT("IRC_Client_GUI_Connection_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionMessage_in);
}

template <enum Common_UI_FrameworkType GUIType>
IRC_Client_GUI_IMessageHandler*
IRC_Client_GUI_Connection_T<GUIType>::getHandler (const std::string& id_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::getHandler"));

  // existing conversation ? --> retrieve message handler

  MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.find (id_in);
  if (iterator != messageHandlers_.end ())
    return (*iterator).second;

  return NULL;
}

//void
//IRC_Client_GUI_Connection_T::current (std::string& nickname_out,
//                                    std::string& channel_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::current"));
//
//  // sanity check(s)
//  ACE_ASSERT (sessionState_);
//
//  nickname_out = sessionState_->nickName;
//  // *TODO*: keep this information synchronized
//  channel_out = sessionState_->channel;
//}

template <enum Common_UI_FrameworkType GUIType>
IRC_Client_GUI_IMessageHandler*
IRC_Client_GUI_Connection_T<GUIType>::getActiveHandler (bool lockedAccess_in,
                                                        bool gdkLockedAccess_in) const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::getActiveHandler"));

  ACE_UNUSED_ARG (gdkLockedAccess_in);

  IRC_Client_GUI_IMessageHandler* return_value = NULL;

  if (lockedAccess_in)
  {
  } // end IF

  if (lockedAccess_in)
  {
  } // end IF

  return return_value;
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::close"));

  struct IRC_Client_UI_HandlerCBData* cb_data_p = NULL;

  // clean up message handlers
  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
         iterator != messageHandlers_.end ();
         iterator++)
    {
      cb_data_p =
          &const_cast<struct IRC_Client_UI_HandlerCBData&> ((*iterator).second->getR ());
      ACE_ASSERT (cb_data_p);
    } // end FOR
    messageHandlers_.clear ();
    closing_ = true;
  } // end lock scope
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::forward (const std::string& channel_in,
                                               const std::string& messageText_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::forward"));

  // --> pass to channel log

  // retrieve message handler
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    if (closing_)
      return; // done

    MESSAGE_HANDLERSITERATOR_T handler_iterator =
      messageHandlers_.find (channel_in);
    if (handler_iterator == messageHandlers_.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                  ACE_TEXT (channel_in.c_str ())));
      return;
    } // end IF

    (*handler_iterator).second->queueForDisplay (messageText_in);
  } // end lock scope
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::log (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::log"));

  // --> pass to server log

  // retrieve message handler
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    if (closing_)
      return; // done

    MESSAGE_HANDLERSITERATOR_T handler_iterator =
      messageHandlers_.find (std::string ());
    ACE_ASSERT (handler_iterator != messageHandlers_.end ());
    (*handler_iterator).second->queueForDisplay (message_in);
  } // end lock scope
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::log (const IRC_Record& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::log"));

  // --> pass to server log

  // retrieve message handler
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    if (closing_)
      return; // done

    MESSAGE_HANDLERSITERATOR_T handler_iterator =
      messageHandlers_.find (std::string ());
    ACE_ASSERT (handler_iterator != messageHandlers_.end ());
    (*handler_iterator).second->queueForDisplay (IRC_Tools::RecordToString (message_in));
  } // end lock scope
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::error (const IRC_Record& message_in,
                                             bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::error"));

  ACE_UNUSED_ARG (message_in);

  if (lockedAccess_in)
  {
  } // end IF

  if (lockedAccess_in)
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::channels (string_list_t& channels_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::channels"));

  // initialize return value
  channels_out.clear ();

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
         iterator != messageHandlers_.end ();
         iterator++)
      if (IRC_Tools::isValidChannelName ((*iterator).first))
        channels_out.push_back ((*iterator).first);
  } // end lock scope
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::createMessageHandler (const std::string& id_in,
                                                            bool lockedAccess_in,
                                                            bool gdkLockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::createMessageHandler"));

  ACE_UNUSED_ARG (gdkLockedAccess_in);

  // sanity check(s)
  ACE_ASSERT (CBData_.connections);

  struct IRC_Client_UI_HandlerCBData* cb_data_p = NULL;
  IMESSAGE_HANDLER_T* message_handler_p = NULL;

  if (lockedAccess_in)
  {
  } // end IF

  // create new IRC_Client_GUI_MessageHandler
  ACE_NEW_NORETURN (cb_data_p,
                    struct IRC_Client_UI_HandlerCBData ());
  if (!cb_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
//    gdk_threads_leave ();
    goto clean_up;
  } // end IF
  *cb_data_p = *CBData_.CBData;
  //  gdk_threads_enter ();
  ACE_NEW_NORETURN (message_handler_p,
                    MESSAGE_HANDLER_T (this,
                                       CBData_.controller,
                                       id_in,
                                       UIFileDirectory_,
                                       CBData_.timeStamp,
                                       cb_data_p));
  if (!message_handler_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
//    gdk_threads_leave ();
    goto clean_up;
  } // end IF
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    messageHandlers_.insert (std::make_pair (id_in, message_handler_p));
    // check whether this is the first channel of the first connection
    // --> enable corresponding widget(s) in the main UI
    if ((CBData_.connections->size () == 1) &&
        (messageHandlers_.size () == 2)) // server log + first channel
    {
    } // end IF
  } // end lock scope

clean_up:
  if (lockedAccess_in)
  {
  } // end IF
}

template <enum Common_UI_FrameworkType GUIType>
void
IRC_Client_GUI_Connection_T<GUIType>::terminateMessageHandler (const std::string& id_in,
                                                               bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection_T::terminateMessageHandler"));

  if (lockedAccess_in)
  {
  } // end IF

  // retrieve message handler
  struct IRC_Client_UI_HandlerCBData* cb_data_p = NULL;
  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    MESSAGE_HANDLERSITERATOR_T iterator_2 = messageHandlers_.find (id_in);
    if (iterator_2 == messageHandlers_.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no handler for id (was: \"%s\"), returning\n"),
                  ACE_TEXT (id_in.c_str ())));
      goto clean_up;
    } // end IF
    ACE_ASSERT ((*iterator_2).second);
    cb_data_p =
        &const_cast<struct IRC_Client_UI_HandlerCBData&> ((*iterator_2).second->getR ());
    messageHandlers_.erase (iterator_2);
  } // end lock scope
  ACE_ASSERT (cb_data_p);

  // check whether this was the last handler of the last connection
  // --> disable corresponding widgets in the main UI
  if ((CBData_.connections->size () == 1) &&
      (messageHandlers_.size () == 1)) // server log
  {
  } // end IF

clean_up:
  if (lockedAccess_in)
  {
  } // end IF
}
