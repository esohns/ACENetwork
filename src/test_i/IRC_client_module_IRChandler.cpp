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

#include "IRC_client_module_IRChandler.h"

#include <iostream>
#include <sstream>

#include "common_timer_manager.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "IRC_client_defines.h"
#include "IRC_client_tools.h"

IRC_Client_Module_IRCHandler::IRC_Client_Module_IRCHandler ()
 : inherited ()
 , lock_ ()
 , subscribers_ ()
 , allocator_ (NULL)
 , automaticPong_ (false) // *NOTE*: the idea really is not to play PONG...
 , bufferSize_ (IRC_CLIENT_BUFFER_SIZE)
 , isInitialized_ (false)
 , printPingPongDot_ (false)
 , conditionLock_ ()
 , condition_ (conditionLock_)
 , connectionIsAlive_ (false)
 , receivedInitialNotice_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::IRC_Client_Module_IRCHandler"));

}

IRC_Client_Module_IRCHandler::~IRC_Client_Module_IRCHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::~IRC_Client_Module_IRCHandler"));

}

bool
IRC_Client_Module_IRCHandler::initialize (Stream_IAllocator* allocator_in,
                                          unsigned int bufferSize_in,
                                          bool autoAnswerPings_in,
                                          bool printPingPongDot_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::initialize"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // reset state
    allocator_ = NULL;
    bufferSize_ = IRC_CLIENT_BUFFER_SIZE;
    automaticPong_ = false;
    printPingPongDot_ = false;
    isInitialized_ = false;
    {
      ACE_Guard<ACE_Thread_Mutex> aGuard (conditionLock_);

      connectionIsAlive_ = false;
    } // end lock scope
    receivedInitialNotice_ = false;
  } // end IF

  allocator_ = allocator_in;
  bufferSize_ = bufferSize_in;
  automaticPong_ = autoAnswerPings_in;
//   if (automaticPong_)
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("auto-answering ping messages...\n")));
  printPingPongDot_ = printPingPongDot_in;

  isInitialized_ = true;

  return isInitialized_;
}

void
IRC_Client_Module_IRCHandler::handleDataMessage (IRC_Client_Message*& message_inout,
                                                 bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::handleDataMessage"));

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout->getData ());

//   // debug info
//   try
//   {
//     message_inout->getData()->dump_state();
//   }
//   catch (...)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("caught exception in RPG_Common_IDumpState::dump_state(), continuing\n")));
//   }

  switch (message_inout->getData ()->command.discriminator)
  {
    case IRC_Client_IRCMessage::Command::NUMERIC:
    {
//       // debug info
//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("[%u]: received \"%s\" [%u]\n"),
//                  message_inout->getID(),
//                  IRC_Client_Tools::IRCCode2String(message_inout->getData()->command.numeric).c_str(),
//                  message_inout->getData()->command.numeric));

      switch (message_inout->getData ()->command.numeric)
      {
        // *NOTE* these are the "regular" (== known) codes
        // [sent by ircd-hybrid-7.2.3 and others]...
        case IRC_Client_IRC_Codes::RPL_WELCOME:              //   1
        case IRC_Client_IRC_Codes::RPL_YOURHOST:             //   2
        case IRC_Client_IRC_Codes::RPL_CREATED:              //   3
        case IRC_Client_IRC_Codes::RPL_MYINFO:               //   4
        case IRC_Client_IRC_Codes::RPL_PROTOCTL:             //   5
        case IRC_Client_IRC_Codes::RPL_YOURID:               //  42
        case IRC_Client_IRC_Codes::RPL_STATSDLINE:           // 250
        case IRC_Client_IRC_Codes::RPL_LUSERCLIENT:          // 251
        case IRC_Client_IRC_Codes::RPL_LUSEROP:              // 252
        case IRC_Client_IRC_Codes::RPL_LUSERUNKNOWN:         // 253
        case IRC_Client_IRC_Codes::RPL_LUSERCHANNELS:        // 254
        case IRC_Client_IRC_Codes::RPL_LUSERME:              // 255
        case IRC_Client_IRC_Codes::RPL_TRYAGAIN:             // 263
        case IRC_Client_IRC_Codes::RPL_LOCALUSERS:           // 265
        case IRC_Client_IRC_Codes::RPL_GLOBALUSERS:          // 266
        case IRC_Client_IRC_Codes::RPL_USERHOST:             // 302
        case IRC_Client_IRC_Codes::RPL_UNAWAY:               // 305
        case IRC_Client_IRC_Codes::RPL_NOWAWAY:              // 306
        case IRC_Client_IRC_Codes::RPL_ENDOFWHO:             // 315
        case IRC_Client_IRC_Codes::RPL_LISTSTART:            // 321
        case IRC_Client_IRC_Codes::RPL_LIST:                 // 322
        case IRC_Client_IRC_Codes::RPL_LISTEND:              // 323
        case IRC_Client_IRC_Codes::RPL_TOPIC:                // 332
        case IRC_Client_IRC_Codes::RPL_TOPICWHOTIME:         // 333
        case IRC_Client_IRC_Codes::RPL_INVITING:             // 341
        case IRC_Client_IRC_Codes::RPL_WHOREPLY:             // 352
        case IRC_Client_IRC_Codes::RPL_NAMREPLY:             // 353
        case IRC_Client_IRC_Codes::RPL_ENDOFNAMES:           // 366
        case IRC_Client_IRC_Codes::RPL_BANLIST:              // 367
        case IRC_Client_IRC_Codes::RPL_ENDOFBANLIST:         // 368
        case IRC_Client_IRC_Codes::RPL_MOTD:                 // 372
        case IRC_Client_IRC_Codes::RPL_MOTDSTART:            // 375
        case IRC_Client_IRC_Codes::RPL_ENDOFMOTD:            // 376
        case IRC_Client_IRC_Codes::ERR_NOSUCHNICK:           // 401
        case IRC_Client_IRC_Codes::ERR_NICKNAMEINUSE:        // 433
        case IRC_Client_IRC_Codes::ERR_NEEDMOREPARAMS:       // 461
        case IRC_Client_IRC_Codes::ERR_YOUREBANNEDCREEP:     // 465
        case IRC_Client_IRC_Codes::ERR_BADCHANNAME:          // 479
        case IRC_Client_IRC_Codes::ERR_CHANOPRIVSNEEDED:     // 482
        case IRC_Client_IRC_Codes::ERR_UMODEUNKNOWNFLAG:     // 501
        {

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("[%u]: received unknown (numeric) command/reply: \"%s\" (%u), continuing\n"),
                      message_inout->getID (),
                      ACE_TEXT (IRC_Client_Tools::IRCCode2String (message_inout->getData ()->command.numeric).c_str ()),
                      message_inout->getData ()->command.numeric));

          break;
        }
      } // end SWITCH

      break;
    }
    case IRC_Client_IRCMessage::Command::STRING:
    {
      switch (IRC_Client_Tools::IRCCommandString2Type (*message_inout->getData ()->command.string))
      {
        case IRC_Client_IRCMessage::NICK:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"NICK\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::QUIT:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"QUIT\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::JOIN:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"JOIN\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::PART:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PART\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::MODE:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"MODE\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::TOPIC:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"TOPIC\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::NAMES:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"NAMES\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::LIST:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"LIST\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::INVITE:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"INVITE\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::KICK:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"KICK\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::PRIVMSG:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PRIVMSG\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::NOTICE:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"NOTICE\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          // remember first contact
          if (!receivedInitialNotice_)
            receivedInitialNotice_ = true;

          break;
        }
        case IRC_Client_IRCMessage::PING:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PING\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          // auto-answer ?
          if (automaticPong_)
          {
            // --> reply with a "PONG"

            // step0: init reply
            IRC_Client_IRCMessage* reply_struct_p = NULL;
            ACE_NEW_NORETURN (reply_struct_p,
                              IRC_Client_IRCMessage ());
            ACE_ASSERT (reply_struct_p);
            ACE_NEW_NORETURN (reply_struct_p->command.string,
                              std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::PONG)));
            ACE_ASSERT (reply_struct_p->command.string);
            reply_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
            reply_struct_p->params.push_back (message_inout->getData ()->params.back ());

            // step1: send it upstream
            sendMessage (reply_struct_p);
          } // end IF

          if (printPingPongDot_)
          {
            std::clog << '.';
          } // end IF

          break;
        }
        case IRC_Client_IRCMessage::PONG:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PONG\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
#if defined ACE_WIN32 || defined ACE_WIN64
#pragma message("applying quirk code for this compiler...")
    case IRC_Client_IRCMessage::__QUIRK__ERROR:
#else
        case IRC_Client_IRCMessage::ERROR:
#endif
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"ERROR\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::AWAY:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"AWAY\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::USERS:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"USERS\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        case IRC_Client_IRCMessage::USERHOST:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"USERHOST\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("[%u]: received unknown command (was: \"%s\"), continuing\n"),
                      message_inout->getID (),
                      ACE_TEXT (message_inout->getData ()->command.string->c_str ())));

          break;
        }
      } // end SWITCH

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("[%u]: invalid command type (was: %u), continuing\n"),
                  message_inout->getID (),
                  message_inout->getData ()->command.discriminator));

      break;
    }
  } // end SWITCH

  // refer the data back to our subscriber(s)

  // synch access to our subscribers
  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

    // *WARNING* if the user unsubscribes() within the callback
    // BAD THINGS (TM) WILL happen, because the current iter WILL be invalidated
    // --> use a slightly modified for-loop (advance first and THEN invoke the callback,
    // works for MOST containers)
    // *NOTE*: this can only happen due to the ACE_RECURSIVE_Thread_Mutex
    // we use as a lock in order to avoid deadlocks in precisely this situation...
    for (SubscribersIterator_t iter = subscribers_.begin ();
         iter != subscribers_.end ();)
    {
      try
      {
        (*iter++)->notify (*message_inout->getData ());
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in IRC_Client_INotify::notify(), continuing\n")));
      }
    } // end FOR
  } // end lock scope
}

void
IRC_Client_Module_IRCHandler::handleSessionMessage (IRC_Client_SessionMessage*& message_inout,
                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::handleSessionMessage"));

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->getType ())
  {
    case SESSION_BEGIN:
    {
      // remember connection has been opened...
      {
        ACE_Guard<ACE_Thread_Mutex> aGuard (conditionLock_);

        connectionIsAlive_ = true;

        // signal any waiter(s)
        condition_.broadcast ();
      } // end lock scope

      // refer the data back to any subscriber(s)
      //      inherited::MODULE_T* module_p = inherited::module ();
      IRC_Client_Module_IRCHandler_Module* module_p =
        dynamic_cast<IRC_Client_Module_IRCHandler_Module*> (inherited::module ());
      ACE_ASSERT (module_p);
      Stream_ModuleConfiguration_t* configuration_p = NULL;
      module_p->get (configuration_p);
      ACE_ASSERT (configuration_p);

      {
        ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

//         ACE_DEBUG((LM_DEBUG,
//                    ACE_TEXT("session starting, notifying %u subscriber(s)...\n"),
//                    subscribers_.size()));

        // *WARNING* if the user unsubscribes() within the callback
        // BAD THINGS (TM) WILL happen, because the current iter WILL be invalidated
        // --> use a slightly modified for-loop (advance first and THEN invoke the callback,
        // works for MOST containers)
        // *NOTE*: this can only happen due to the ACE_RECURSIVE_Thread_Mutex
        // we use as a lock in order to avoid deadlocks in precisely this situation...
        for (SubscribersIterator_t iter = subscribers_.begin ();
             iter != subscribers_.end ();)
        {
          try
          {
            (*iter++)->start (*configuration_p);
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in IRC_Client_INotify::start(), continuing\n")));
          }
        } // end FOR
      } // end lock scope

      break;
    }
    case SESSION_END:
    {
      // refer this information back to our subscriber(s)
      {
        ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

//         ACE_DEBUG((LM_DEBUG,
//                    ACE_TEXT("session ending, notifying %u subscriber(s)...\n"),
//                    subscribers_.size()));

        // *WARNING* if the user unsubscribes() within the callback
        // BAD THINGS (TM) WILL happen, because the current iter WILL be invalidated
        // --> use a slightly modified for-loop (advance first and THEN invoke the callback,
        // works for MOST containers)
        // *NOTE*: this can only happen due to the ACE_RECURSIVE_Thread_Mutex
        // we use as a lock in order to avoid deadlocks in precisely this situation...
        for (SubscribersIterator_t iter = subscribers_.begin ();
             iter != subscribers_.end ();)
        {
          try
          {
            (*(iter++))->end ();
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in IRC_Client_INotify::end(), continuing\n")));
          }
        } // end FOR

        if (!subscribers_.empty ())
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("removing %u subscription(s)...\n"),
                      subscribers_.size ()));

        // clean subscribers
        subscribers_.clear ();
      } // end lock scope

      // remember connection has been closed...
      {
        ACE_Guard<ACE_Thread_Mutex> aGuard (conditionLock_);

        connectionIsAlive_ = false;

        // signal any waiter(s)
        condition_.broadcast ();
      } // end lock scope

      break;
    }
    default:
    {
      // don't do anything...
      break;
    }
  } // end SWITCH
}

bool
IRC_Client_Module_IRCHandler::registerConnection (const IRC_Client_IRCLoginOptions& loginOptions_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::registerConnection"));

  int result = -1;

  // "registering" an IRC connection implies the following 4 distinct steps:
  // --> see also RFC1459
  // 1. establish a connection (done ?!)
  // [2. wait for initial NOTICE (done ?!)]
  // 3. send PASS
  // 4. send NICK
  // 5. send USER

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  // step1: ...is done ?
  {
    ACE_Guard<ACE_Thread_Mutex> aGuard (conditionLock_);

    if (!connectionIsAlive_)
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("waiting for connection to initialize...\n")));

      // *NOTE*: can happen when trying to register IMMEDIATELY after connecting
      //         --> allow a little delay for:
      //         - connection to establish
      //         [- the initial NOTICEs to arrive]
      //         before proceeding...
      ACE_Time_Value deadline = COMMON_TIME_POLICY () +
                                ACE_Time_Value (IRC_CLIENT_IRC_MAX_WELCOME_DELAY, 0);
      result = condition_.wait (&deadline);
      if (result == -1)
      {
        int error = ACE_OS::last_error ();
        if (error != ETIME)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Thread_Condition::wait(): \"%m\", continuing\n")));
      } // end IF

      if (!connectionIsAlive_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("not (yet ?) connected, aborting\n")));
        return false;
      } // end IF

      // *TODO*: wait for NOTICE ?
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("waiting for initial NOTICE...\n")));

//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("proceeding...\n")));
    } // end IF
  } // end lock scope
  // step2: ...is done ?
//   if (!receivedInitialNotice_)
//   {
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("waiting for connection...\n")));
//
//     // *NOTE*: can happen when trying to register IMMEDIATELY after connecting
//     // --> allow a little delay for the welcome NOTICE to arrive before proceeding
//     ACE_Time_Value abs_deadline = RPG_COMMON_TIME_POLICY() + ACE_Time_Value(IRC_CLIENT_IRC_MAX_WELCOME_DELAY, 0);
//     if ((condition_.wait(&abs_deadline) == -1) &&
//         (ACE_OS::last_error() != ETIME))
//     {
//       ACE_DEBUG((LM_ERROR,
//                   ACE_TEXT("failed to ACE_Thread_Condition::wait(), aborting\n")));
//
//       return;
//     } // end IF
//
//     if (!receivedInitialNotice_)
//     {
//       ACE_DEBUG((LM_ERROR,
//                   ACE_TEXT("not (yet ?) connected, aborting\n")));
//
//       return;
//     } // end IF
//
//     ACE_DEBUG((LM_DEBUG,
//                 ACE_TEXT("proceeding...\n")));
//   } // end IF

  // step3a: initialize PASS
  IRC_Client_IRCMessage* pass_struct_p = NULL;
  ACE_NEW_NORETURN (pass_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (pass_struct_p);
  ACE_NEW_NORETURN (pass_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::PASS)));
  ACE_ASSERT (pass_struct_p->command.string);
  pass_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
  pass_struct_p->params.push_back (loginOptions_in.password);

  // step3b: send it upstream
  sendMessage (pass_struct_p);

  // step4a: initialize NICK
  IRC_Client_IRCMessage* nick_struct_p = NULL;
  ACE_NEW_NORETURN (nick_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (nick_struct_p);
  ACE_NEW_NORETURN (nick_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::NICK)));
  ACE_ASSERT (nick_struct_p->command.string);
  nick_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
  nick_struct_p->params.push_back (loginOptions_in.nick);

  // step4b: send it upstream
  sendMessage (nick_struct_p);

  // step5a: initialize USER
  IRC_Client_IRCMessage* user_struct_p = NULL;
  ACE_NEW_NORETURN (user_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (user_struct_p);
  ACE_NEW_NORETURN (user_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::USER)));
  ACE_ASSERT (user_struct_p->command.string);
  user_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
  user_struct_p->params.push_back (loginOptions_in.user.username);
  switch (loginOptions_in.user.hostname.discriminator)
  {
    case IRC_Client_IRCLoginOptions::User::Hostname::STRING:
    {
      user_struct_p->params.push_back (*loginOptions_in.user.hostname.string);

      break;
    }
    case IRC_Client_IRCLoginOptions::User::Hostname::BITMASK:
    {
      std::ostringstream converter;
      converter << static_cast<unsigned long> (loginOptions_in.user.hostname.mode);
      user_struct_p->params.push_back (std::string (converter.str ()));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid USER <hostname> parameter field type (was: %d), aborting\n"),
                  loginOptions_in.user.hostname.discriminator));
      return false;
    }
  } // end SWITCH
  user_struct_p->params.push_back (loginOptions_in.user.servername);
  user_struct_p->params.push_back (loginOptions_in.user.realname);

  // step5b: send it upstream
  sendMessage (user_struct_p);

//   // step5a: initialize JOIN
//   IRC_Client_IRCMessage* join_struct = NULL;
//   ACE_NEW_NORETURN(join_struct,
//                    IRC_Client_IRCMessage());
//   ACE_ASSERT(join_struct);
//   ACE_NEW_NORETURN(join_struct->command.string,
//                    std::string(IRC_Client_Message::CommandType2String(IRC_Client_IRCMessage::JOIN)));
//   ACE_ASSERT(join_struct->command.string);
//   join_struct->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
// //   std::string channel_name = ACE_TEXT_ALWAYS_CHAR("#");
// //   channel_name += loginOptions_in.channel;
//   join_struct->params.push_back(loginOptions_in.channel);
//
//   // step5b: send it upstream
//   sendMessage(join_struct);

  return true;
}

void
IRC_Client_Module_IRCHandler::subscribe (IRC_Client_INotify_t* dataCallback_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::subscribe"));

  // sanity check(s)
  ACE_ASSERT (dataCallback_in);

  // synch access to subscribers
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  subscribers_.push_back (dataCallback_in);
}

void
IRC_Client_Module_IRCHandler::unsubscribe (IRC_Client_INotify_t* dataCallback_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::unsubscribe"));

  // sanity check(s)
  ACE_ASSERT (dataCallback_in);

  // synch access to subscribers
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  SubscribersIterator_t iterator = subscribers_.begin ();
  for (;
       iterator != subscribers_.end ();
       iterator++)
    if ((*iterator) == dataCallback_in)
      break;

  if (iterator != subscribers_.end ())
    subscribers_.erase (iterator);
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: %@), aborting\n"),
                dataCallback_in));
}

void
IRC_Client_Module_IRCHandler::nick (const std::string& nick_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::nick"));

  // step1: init NICK
  IRC_Client_IRCMessage* nick_struct_p = NULL;
  ACE_NEW_NORETURN (nick_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (nick_struct_p);
  ACE_NEW_NORETURN (nick_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::NICK)));
  ACE_ASSERT (nick_struct_p->command.string);
  nick_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  nick_struct_p->params.push_back (nick_in);

  // step2: send it upstream
  sendMessage (nick_struct_p);
}

void
IRC_Client_Module_IRCHandler::quit (const std::string& reason_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::quit"));

  // step1: init QUIT
  IRC_Client_IRCMessage* quit_struct_p = NULL;
  ACE_NEW_NORETURN (quit_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (quit_struct_p);
  ACE_NEW_NORETURN (quit_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::QUIT)));
  ACE_ASSERT (quit_struct_p->command.string);
  quit_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
  if (!reason_in.empty ())
    quit_struct_p->params.push_back (reason_in);

  // step2: send it upstream
  sendMessage (quit_struct_p);
}

void
IRC_Client_Module_IRCHandler::join (const string_list_t& channels_in,
                                    const string_list_t& keys_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::join"));

  // step1: init JOIN
  IRC_Client_IRCMessage* join_struct_p = NULL;
  ACE_NEW_NORETURN (join_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (join_struct_p);
  ACE_NEW_NORETURN (join_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::JOIN)));
  ACE_ASSERT (join_struct_p->command.string);
  join_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
  // compute ranges ?
  if (!keys_in.empty ())
  {
    list_item_range_t range;
    if (channels_in.size () > 1)
    {
      range.first = 0;
      range.second = (channels_in.size () - 1);
      join_struct_p->list_param_ranges.push_back (range);
    } // end IF
    if (keys_in.size () > 1)
    {
      range.first = channels_in.size ();
      range.second = (channels_in.size () + keys_in.size () - 1);
      join_struct_p->list_param_ranges.push_back (range);
    } // end IF
  } // end IF
  join_struct_p->params = channels_in;
  // append any keys
  if (!keys_in.empty ())
    join_struct_p->params.insert (join_struct_p->params.end (),
                                  keys_in.begin(),
                                  keys_in.end ());

  // step2: send it upstream
  sendMessage (join_struct_p);
}

void
IRC_Client_Module_IRCHandler::part (const string_list_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::part"));

  // step1: init PART
  IRC_Client_IRCMessage* part_struct_p = NULL;
  ACE_NEW_NORETURN (part_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (part_struct_p);
  ACE_NEW_NORETURN (part_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::PART)));
  ACE_ASSERT (part_struct_p->command.string);
  part_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;
  part_struct_p->params = channels_in;

  // step2: send it upstream
  sendMessage (part_struct_p);
}

void
IRC_Client_Module_IRCHandler::mode (const std::string& target_in,
                                    char mode_in,
                                    bool enable_in,
                                    const string_list_t& parameters_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::mode"));

  // step1: init MODE
  IRC_Client_IRCMessage* mode_struct_p = NULL;
  ACE_NEW_NORETURN (mode_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (mode_struct_p);
  ACE_NEW_NORETURN (mode_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::MODE)));
  ACE_ASSERT (mode_struct_p->command.string);
  mode_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  // construct parameter
  std::string mode_string = (enable_in ? ACE_TEXT_ALWAYS_CHAR("+")
                                       : ACE_TEXT_ALWAYS_CHAR("-"));
  mode_string += mode_in;

  mode_struct_p->params.push_back (target_in);
  mode_struct_p->params.push_back (mode_string);
  // append any parameters
  mode_struct_p->params.insert (mode_struct_p->params.end (),
                                parameters_in.begin (),
                                parameters_in.end ());

  // step2: send it upstream
  sendMessage (mode_struct_p);
}

void
IRC_Client_Module_IRCHandler::topic (const std::string& channel_in,
                                           const std::string& topic_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::topic"));

  // sanity check(s)
  ACE_ASSERT (!topic_in.empty ());

  // step1: init TOPIC
  IRC_Client_IRCMessage* topic_struct_p = NULL;
  ACE_NEW_NORETURN (topic_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (topic_struct_p);
  ACE_NEW_NORETURN (topic_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::TOPIC)));
  ACE_ASSERT (topic_struct_p->command.string);
  topic_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  topic_struct_p->params.push_back (channel_in);
  topic_struct_p->params.push_back (topic_in);

  // step2: send it upstream
  sendMessage (topic_struct_p);
}

void
IRC_Client_Module_IRCHandler::names (const string_list_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::names"));

  // step1: init NAMES
  IRC_Client_IRCMessage* names_struct_p = NULL;
  ACE_NEW_NORETURN (names_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (names_struct_p);
  ACE_NEW_NORETURN (names_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::NAMES)));
  ACE_ASSERT (names_struct_p->command.string);
  names_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  // compute range ?
  if (channels_in.size () > 1)
  {
    list_item_range_t range;
    range.first = 0;
    range.second = (channels_in.size () - 1);
    names_struct_p->list_param_ranges.push_back (range);
  } // end IF
  names_struct_p->params = channels_in;

  // step2: send it upstream
  sendMessage (names_struct_p);
}

void
IRC_Client_Module_IRCHandler::list(const string_list_t& channels_in)
{
  NETWORK_TRACE(ACE_TEXT("IRC_Client_Module_IRCHandler::list"));

  // step1: init LIST
  IRC_Client_IRCMessage* list_struct_p = NULL;
  ACE_NEW_NORETURN (list_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (list_struct_p);
  ACE_NEW_NORETURN (list_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::LIST)));
  ACE_ASSERT (list_struct_p->command.string);
  list_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  // compute range ?
  if (channels_in.size () > 1)
  {
    list_item_range_t range;
    range.first = 0;
    range.second = (channels_in.size () - 1);
    list_struct_p->list_param_ranges.push_back (range);
  } // end IF
  list_struct_p->params = channels_in;

  // step2: send it upstream
  sendMessage (list_struct_p);
}

void
IRC_Client_Module_IRCHandler::invite (const std::string& nick_in,
                                            const std::string& channel_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::invite"));

  // step1: init INVITE
  IRC_Client_IRCMessage* invite_struct_p = NULL;
  ACE_NEW_NORETURN (invite_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (invite_struct_p);
  ACE_NEW_NORETURN (invite_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::INVITE)));
  ACE_ASSERT (invite_struct_p->command.string);
  invite_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  invite_struct_p->params.push_back (nick_in);
  invite_struct_p->params.push_back (channel_in);

  // step2: send it upstream
  sendMessage (invite_struct_p);
}

void
IRC_Client_Module_IRCHandler::kick (const std::string& channel_in,
                                          const std::string& nick_in,
                                          const std::string& comment_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::kick"));

  // step1: init KICK
  IRC_Client_IRCMessage* kick_struct_p = NULL;
  ACE_NEW_NORETURN (kick_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (kick_struct_p);
  ACE_NEW_NORETURN (kick_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::KICK)));
  ACE_ASSERT (kick_struct_p->command.string);
  kick_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  kick_struct_p->params.push_back (channel_in);
  kick_struct_p->params.push_back (nick_in);
  if (!comment_in.empty ())
    kick_struct_p->params.push_back (comment_in);

  // step2: send it upstream
  sendMessage (kick_struct_p);
}

void
IRC_Client_Module_IRCHandler::send (const string_list_t& receivers_in,
                                          const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::send"));

  // sanity check
  if (message_in.empty ())
    return; // nothing to do...

  // step1: init PRIVMSG
  IRC_Client_IRCMessage* msg_struct_p = NULL;
  ACE_NEW_NORETURN (msg_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (msg_struct_p);
  ACE_NEW_NORETURN (msg_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::PRIVMSG)));
  ACE_ASSERT (msg_struct_p->command.string);
  msg_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  // compute range ?
  if (receivers_in.size () > 1)
  {
    list_item_range_t range;
    range.first = 0;
    range.second = (receivers_in.size () - 1);
    msg_struct_p->list_param_ranges.push_back (range);
  } // end IF
  msg_struct_p->params = receivers_in;
  msg_struct_p->params.push_back (message_in);

  // step2: send it upstream
  sendMessage (msg_struct_p);
}

void
IRC_Client_Module_IRCHandler::who (const std::string& name_in,
                                   bool operatorsOnly_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::who"));

  // step1: init WHOIS
  IRC_Client_IRCMessage* who_struct_p = NULL;
  ACE_NEW_NORETURN (who_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (who_struct_p);
  ACE_NEW_NORETURN (who_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::WHO)));
  ACE_ASSERT (who_struct_p->command.string);
  who_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  who_struct_p->params.push_back (name_in);
  if (operatorsOnly_in)
    who_struct_p->params.push_back (std::string ("o"));

  // step2: send it upstream
  sendMessage (who_struct_p);
}

void
IRC_Client_Module_IRCHandler::whois (const std::string& servername_in,
                                           const string_list_t& nicknames_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::whois"));

  // step1: init WHOIS
  IRC_Client_IRCMessage* whois_struct_p = NULL;
  ACE_NEW_NORETURN (whois_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (whois_struct_p);
  ACE_NEW_NORETURN (whois_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::WHOIS)));
  ACE_ASSERT (whois_struct_p->command.string);
  whois_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  whois_struct_p->params = nicknames_in;
  if (!servername_in.empty ())
    whois_struct_p->params.push_front (servername_in);

  // step2: send it upstream
  sendMessage (whois_struct_p);
}

void
IRC_Client_Module_IRCHandler::whowas (const std::string& nick_in,
                                      unsigned int count_in,
                                      const std::string& servername_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::whowas"));

  // step1: init WHOWAS
  IRC_Client_IRCMessage* whowas_struct_p = NULL;
  ACE_NEW_NORETURN (whowas_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (whowas_struct_p);
  ACE_NEW_NORETURN (whowas_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::WHOWAS)));
  ACE_ASSERT (whowas_struct_p->command.string);
  whowas_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  whowas_struct_p->params.push_back (nick_in);
  if (count_in)
  {
    std::stringstream converter;
    converter << count_in;
    std::string count_string;
    converter >> count_string;
    whowas_struct_p->params.push_back (count_string);
  } // end IF
  if (!servername_in.empty ())
    whowas_struct_p->params.push_back (servername_in);

  // step2: send it upstream
  sendMessage (whowas_struct_p);
}

void
IRC_Client_Module_IRCHandler::away (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::away"));

  // step1: init AWAY
  IRC_Client_IRCMessage* away_struct_p = NULL;
  ACE_NEW_NORETURN (away_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (away_struct_p);
  ACE_NEW_NORETURN (away_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::AWAY)));
  ACE_ASSERT (away_struct_p->command.string);
  away_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  // *NOTE*: if there is no away message parameter, the semantic is to
  // "un-away" the user
  if (!message_in.empty ())
    away_struct_p->params.push_back (message_in);

  // step2: send it upstream
  sendMessage (away_struct_p);
}

void
IRC_Client_Module_IRCHandler::users (const std::string& server_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::users"));

  // step1: init USERS
  IRC_Client_IRCMessage* users_struct_p = NULL;
  ACE_NEW_NORETURN (users_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (users_struct_p);
  ACE_NEW_NORETURN (users_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::USERS)));
  ACE_ASSERT (users_struct_p->command.string);
  users_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  if (!server_in.empty ())
    users_struct_p->params.push_back (server_in);

  // step2: send it upstream
  sendMessage (users_struct_p);
}

void
IRC_Client_Module_IRCHandler::userhost (const string_list_t& nicknames_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::userhost"));

  // step1: init USERHOST
  IRC_Client_IRCMessage* userhost_struct_p = NULL;
  ACE_NEW_NORETURN (userhost_struct_p,
                    IRC_Client_IRCMessage ());
  ACE_ASSERT (userhost_struct_p);
  ACE_NEW_NORETURN (userhost_struct_p->command.string,
                    std::string (IRC_Client_Message::CommandType2String (IRC_Client_IRCMessage::USERHOST)));
  ACE_ASSERT (userhost_struct_p->command.string);
  userhost_struct_p->command.discriminator = IRC_Client_IRCMessage::Command::STRING;

  userhost_struct_p->params = nicknames_in;

  // step2: send it upstream
  sendMessage (userhost_struct_p);
}

void
IRC_Client_Module_IRCHandler::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::dump_state"));

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT(" ***** MODULE: \"%s\" state *****\n"),
//              ACE_TEXT_ALWAYS_CHAR(name())));
//
//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT(" ***** MODULE: \"%s\" state *****\\END\n"),
//              ACE_TEXT_ALWAYS_CHAR(name())));
}

IRC_Client_Message*
IRC_Client_Module_IRCHandler::allocateMessage(unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::allocateMessage"));

  // initialize return value(s)
  IRC_Client_Message* message_p = NULL;

  if (allocator_)
  {
allocate:
    try
    {
      message_p =
        static_cast<IRC_Client_Message*> (allocator_->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !allocator_->block ())
        goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      IRC_Client_Message (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}

void
IRC_Client_Module_IRCHandler::sendMessage (IRC_Client_IRCMessage*& command_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::sendMessage"));

  // step1: get a message buffer
  IRC_Client_Message* message_p = allocateMessage (bufferSize_);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(%u), returning\n"),
                bufferSize_));
    return;
  } // end IF

  // step2: attach the command
  // *NOTE*: message assumes control over the reference...
  message_p->initialize (command_in);
  // --> bye bye...
  command_in = NULL;

  // step3: send it upstream
  // *NOTE*: while there is NO way to prevent async close of the CONNECTION,
  // this does protect against async closure of the STREAM WHILE we propagate
  // our message...
  // --> grab our lock and check connectionIsAlive_
  ACE_Guard<ACE_Thread_Mutex> aGuard (conditionLock_);
  // sanity check
  if (!connectionIsAlive_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no connection - cannot send message, returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF

  if (reply (message_p, NULL) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::reply(): \"%m\", returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("pushed message...\n")));
}

Stream_Module_t*
IRC_Client_Module_IRCHandler::clone ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::clone"));

  // initialize return value(s)
  Stream_Module_t* module_p = NULL;

  ACE_NEW_NORETURN (module_p,
                    IRC_Client_Module_IRCHandler_Module (ACE_TEXT_ALWAYS_CHAR (inherited::name ()),
                    NULL));
  if (!module_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  else
  {
    IRC_Client_Module_IRCHandler* IRCHandler_impl = NULL;
    IRCHandler_impl =
      dynamic_cast<IRC_Client_Module_IRCHandler*> (module_p->writer ());
    if (!IRCHandler_impl)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCHandler> failed, aborting\n")));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
    IRCHandler_impl->initialize (allocator_,
                                 bufferSize_,
                                 automaticPong_,
                                 printPingPongDot_);
  } // end ELSE

  return module_p;
}
