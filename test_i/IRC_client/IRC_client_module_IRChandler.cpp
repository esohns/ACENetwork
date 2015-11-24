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

//#include <iostream>
#include <sstream>

#include "ace/iosfwd.h"
#include "ace/OS.h"

#include "common_timer_manager.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "irc_tools.h"

#include "IRC_client_defines.h"
#include "IRC_client_network.h"

IRC_Client_Module_IRCHandler::IRC_Client_Module_IRCHandler ()
 : inherited ()
 , lock_ ()
 , subscribers_ ()
 , configuration_ ()
 , isInitialized_ (false)
 , conditionLock_ ()
 , condition_ (conditionLock_)
 , connectionIsAlive_ (false)
 , initialRegistration_ (true)
 , receivedInitialNotice_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::IRC_Client_Module_IRCHandler"));

}

IRC_Client_Module_IRCHandler::~IRC_Client_Module_IRCHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::~IRC_Client_Module_IRCHandler"));

}

bool
IRC_Client_Module_IRCHandler::initialize (const IRC_Client_ModuleHandlerConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::initialize"));

  configuration_ = configuration_in;

  // sanity check(s)
  ACE_ASSERT (inherited::stateLock_);
  ACE_ASSERT (configuration_.streamConfiguration);
  ACE_ASSERT (configuration_.streamConfiguration->messageAllocator);

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    {
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

      connectionIsAlive_ = false;
    } // end lock scope

    { // synch access to state machine
      ACE_Guard<ACE_SYNCH_NULL_MUTEX> aGuard (*inherited::stateLock_);

      inherited::state_ = REGISTRATION_STATE_NICK;
    } // end lock scope
    initialRegistration_ = true;
    receivedInitialNotice_ = false;

    { // synch access to subscribers
      ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

      subscribers_.clear ();
    } // end lock scope
  } // end IF

//   if (configuration_.automaticPong)
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("auto-answering ping messages...\n")));

  if (configuration_.subscriber)
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    subscribers_.push_back (configuration_.subscriber);
  } // end IF

  isInitialized_ = true;

  return true;
}
const IRC_Client_ModuleHandlerConfiguration&
IRC_Client_Module_IRCHandler::get () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::handleDataMessage"));

  return configuration_;
}

void
IRC_Client_Module_IRCHandler::handleDataMessage (IRC_Message*& message_inout,
                                                 bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  const IRC_Record* message_data_p = message_inout->getData ();
  ACE_ASSERT (message_data_p);
  ACE_ASSERT (configuration_.protocolConfiguration);

//   try
//   {
//     message_data_p->dump_state ();
//   }
//   catch (...)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("caught exception in Common_IDumpState::dump_state(), continuing\n")));
//   }

  switch (message_data_p->command.discriminator)
  {
    case IRC_Record::Command::NUMERIC:
    {
//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("[%u]: received \"%s\" [%u]\n"),
//                  message_inout->getID(),
//                  IRC_Client_Tools::IRCCode2String(message_inout->getData()->command.numeric).c_str(),
//                  message_inout->getData()->command.numeric));

      switch (message_data_p->command.numeric)
      {
        // *NOTE* these are the "regular" (== known) codes
        // [sent by ircd-hybrid-7.2.3 and others]...
        case IRC_Codes::RPL_WELCOME:              //   1
        {
          if (initialRegistration_)
          {
            initialRegistration_ = false;
            if (!inherited::change (REGISTRATION_STATE_FINISHED))
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to Common_IStateMachine_T::change(\"%s\"), continuing\n"),
                          ACE_TEXT (inherited::state2String (REGISTRATION_STATE_FINISHED).c_str ())));
          } // end IF

          // *WARNING*: falls through !
        }
        case IRC_Codes::RPL_YOURHOST:             //   2
        case IRC_Codes::RPL_CREATED:              //   3
        case IRC_Codes::RPL_MYINFO:               //   4
        case IRC_Codes::RPL_PROTOCTL:             //   5
        case IRC_Codes::RPL_SNOMASK:              //   8
        case IRC_Codes::RPL_YOURID:               //  42
        case IRC_Codes::RPL_STATSDLINE:           // 250
        case IRC_Codes::RPL_LUSERCLIENT:          // 251
        case IRC_Codes::RPL_LUSEROP:              // 252
        case IRC_Codes::RPL_LUSERUNKNOWN:         // 253
        case IRC_Codes::RPL_LUSERCHANNELS:        // 254
        case IRC_Codes::RPL_LUSERME:              // 255
        case IRC_Codes::RPL_TRYAGAIN:             // 263
        case IRC_Codes::RPL_LOCALUSERS:           // 265
        case IRC_Codes::RPL_GLOBALUSERS:          // 266
        case IRC_Codes::RPL_USERHOST:             // 302
        case IRC_Codes::RPL_UNAWAY:               // 305
        case IRC_Codes::RPL_NOWAWAY:              // 306
        case IRC_Codes::RPL_ENDOFWHO:             // 315
        case IRC_Codes::RPL_LISTSTART:            // 321
        case IRC_Codes::RPL_LIST:                 // 322
        case IRC_Codes::RPL_LISTEND:              // 323
        case IRC_Codes::RPL_TOPIC:                // 332
        case IRC_Codes::RPL_TOPICWHOTIME:         // 333
        case IRC_Codes::RPL_INVITING:             // 341
        case IRC_Codes::RPL_WHOREPLY:             // 352
        case IRC_Codes::RPL_NAMREPLY:             // 353
        case IRC_Codes::RPL_ENDOFNAMES:           // 366
        case IRC_Codes::RPL_BANLIST:              // 367
        case IRC_Codes::RPL_ENDOFBANLIST:         // 368
        case IRC_Codes::RPL_MOTD:                 // 372
        case IRC_Codes::RPL_MOTDSTART:            // 375
        case IRC_Codes::RPL_ENDOFMOTD:            // 376
        case IRC_Codes::RPL_HOSTHIDDEN:           // 396
        case IRC_Codes::ERR_NOSUCHNICK:           // 401
        case IRC_Codes::ERR_NOTEXTTOSEND:         // 412
        case IRC_Codes::ERR_UNKNOWNCOMMAND:       // 421
        case IRC_Codes::ERR_NOMOTD:               // 422
        case IRC_Codes::ERR_ERRONEUSNICKNAME:     // 432
        case IRC_Codes::ERR_NICKNAMEINUSE:        // 433
        case IRC_Codes::ERR_NOTREGISTERED:        // 451
        case IRC_Codes::ERR_NEEDMOREPARAMS:       // 461
        case IRC_Codes::ERR_ALREADYREGISTRED:     // 462
        case IRC_Codes::ERR_YOUREBANNEDCREEP:     // 465
        case IRC_Codes::ERR_BADCHANNAME:          // 479
        case IRC_Codes::ERR_CHANOPRIVSNEEDED:     // 482
        case IRC_Codes::ERR_UMODEUNKNOWNFLAG:     // 501
          break;
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("[%u]: received unknown (numeric) command/reply: \"%s\" (%u), continuing\n"),
                      message_inout->getID (),
                      ACE_TEXT (IRC_Tools::Command2String (message_data_p->command.numeric).c_str ()),
                      message_data_p->command.numeric));
          break;
        }
      } // end SWITCH

      break;
    }
    case IRC_Record::Command::STRING:
    {
      switch (IRC_Tools::Command2Type (*message_data_p->command.string))
      {
        case IRC_Record::NICK:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"NICK\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::QUIT:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"QUIT\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::JOIN:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"JOIN\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::PART:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PART\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::MODE:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"MODE\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::TOPIC:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"TOPIC\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::NAMES:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"NAMES\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::LIST:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"LIST\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::INVITE:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"INVITE\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::KICK:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"KICK\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::PRIVMSG:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PRIVMSG\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::NOTICE:
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
        case IRC_Record::PING:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PING\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));

          // auto-answer ?
          if (configuration_.protocolConfiguration->automaticPong)
          {
            // --> reply with a "PONG"

            // step0: initialize reply
            IRC_Record* reply_p = NULL;
            ACE_NEW_NORETURN (reply_p,
                              IRC_Record ());
            if (!reply_p)
            {
              ACE_DEBUG ((LM_CRITICAL,
                          ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
              break;
            } // end IF
            ACE_NEW_NORETURN (reply_p->command.string,
                              std::string (IRC_Message::Command2String (IRC_Record::PONG)));
            if (!reply_p->command.string)
            {
              ACE_DEBUG ((LM_CRITICAL,
                          ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

              // clean up
              reply_p->decrease ();

              break;
            } // end IF
            reply_p->command.discriminator =
              IRC_Record::Command::STRING;
            reply_p->params.push_back (message_data_p->params.back ());

            // step1: send it upstream
            sendMessage (reply_p);
          } // end IF

          if (configuration_.protocolConfiguration->printPingDot)
            std::clog << '.';

          break;
        }
        case IRC_Record::PONG:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"PONG\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_data_p->params.back().c_str()));
          break;
        }
#if defined ACE_WIN32 || defined ACE_WIN64
#pragma message("applying quirk code for this compiler...")
    case IRC_Record::__QUIRK__ERROR:
#else
        case IRC_Record::ERROR:
#endif
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"ERROR\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_data_p->params.back().c_str()));
          break;
        }
        case IRC_Record::AWAY:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"AWAY\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::USERS:
        {
//           ACE_DEBUG((LM_DEBUG,
//                      ACE_TEXT("[%u]: received \"USERS\": \"%s\"\n"),
//                      message_inout->getID(),
//                      message_inout->getData()->params.back().c_str()));
          break;
        }
        case IRC_Record::USERHOST:
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
                      ACE_TEXT (message_data_p->command.string->c_str ())));
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
                  message_data_p->command.discriminator));
      break;
    }
  } // end SWITCH

  // forward the data to subscriber(s)

  // synch access to subscribers
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

    // *WARNING* if the user unsubscribes() within the callback
    // BAD THINGS (TM) WILL happen, because the current iter WILL be invalidated
    // --> use a slightly modified for-loop (advance first and THEN invoke the
    //     callback, works for MOST containers)
    // *NOTE*: this can only happen due to the ACE_RECURSIVE_Thread_Mutex
    //         used as a lock in order to avoid deadlocks in precisely this
    //         situation...
    for (SubscribersIterator_t iterator = subscribers_.begin ();
         iterator != subscribers_.end ();
        )
    {
      try
      {
        (*iterator++)->notify (*message_data_p);
      }
      catch (...)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in IRC_Client_INotify::notify(): \"%m\", continuing\n")));
      }
    } // end FOR
  } // end lock scope
}

void
IRC_Client_Module_IRCHandler::handleSessionMessage (IRC_Client_SessionMessage*& message_inout,
                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // step0: initialize session state
      // *NOTE*: the IRC protocol does not foresee a 'handshake' during session
      //         initialization for client-to-server communications.
      //         (see: RFC 1459 page 13f)
      //         In particular, this means that the initial user nickname is not
      //         'acknowledged' during the (PASS-)NICK-USER sequence
      Stream_Module_t* module_p = inherited2::module ();
      ACE_ASSERT (module_p);
      IRC_Client_IModule_t* imodule_p =
          dynamic_cast<IRC_Client_IModule_t*> (module_p);
      if (!imodule_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Stream_IModule_T*>(%@), returning\n"),
                    module_p));
        break;
      } // end IF
      const IRC_Client_ModuleHandlerConfiguration& module_handler_configuration_r =
          imodule_p->get ();
      ACE_ASSERT (module_handler_configuration_r.protocolConfiguration);
      const IRC_Client_SessionData_t& session_data_container_r =
          message_inout->get ();
      const IRC_Client_SessionData& session_data_r =
          session_data_container_r.get ();
      ACE_ASSERT (session_data_r.connectionState);
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (session_data_r.connectionState->lock);
        session_data_r.connectionState->nickName =
            module_handler_configuration_r.protocolConfiguration->loginOptions.nickName;
      } // end lock scope

      // step1: remember connection has been opened...
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

        connectionIsAlive_ = true;

        // signal any waiter(s)
        result = condition_.broadcast ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
      } // end lock scope

      // step2: announce this state to any subscriber(s)

      // forward the (current) session data to any subscriber(s)
      {
        ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

        //// initial subscriber ?
        //if (module_handler_configuration_r.subscriber)
        //  subscribers_.push_back (module_handler_configuration_r.subscriber);

//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("session starting, notifying %u subscriber(s)...\n"),
//                     subscribers_.size()));

        // *WARNING*: if the user unsubscribes() within the callback
        //            BAD THINGS (TM) may happen, because the current iterator
        //            WILL be invalidated
        //            --> use a slightly modified for-loop (advance first and
        //                THEN invoke the callback, this works for MOST
        //                container types)
        // *NOTE*: this can only happen due to the ACE_RECURSIVE_Thread_Mutex
        //         being used as a lock in order to avoid deadlocks in exactly
        //         this situation...
        for (SubscribersIterator_t iterator = subscribers_.begin ();
             iterator != subscribers_.end ();)
        {
          try
          {
            (*iterator++)->start (session_data_r);
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in IRC_Client_IStreamNotify::start(), continuing\n")));
          }
        } // end FOR
      } // end lock scope

      break;
    }
    case STREAM_SESSION_END:
    {
      // announce this information to any subscriber(s)
      {
        ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("session ending, notifying %u subscriber(s)...\n"),
//                     subscribers_.size()));

        // *WARNING*: if the user unsubscribes() within the callback
        //            BAD THINGS (TM) may happen, because the current iterator
        //            WILL be invalidated
        //            --> use a slightly modified for-loop (advance first and
        //                THEN invoke the callback, this works for MOST
        //                container types)
        // *NOTE*: this can only happen due to the ACE_RECURSIVE_Thread_Mutex
        //         being used as a lock in order to avoid deadlocks in exactly
        //         this situation...
        for (SubscribersIterator_t iterator = subscribers_.begin ();
             iterator != subscribers_.end ();)
        {
          try
          {
            (*(iterator++))->end ();
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in IRC_Client_IStreamNotify_t::end(), continuing\n")));
          }
        } // end FOR

        if (!subscribers_.empty ())
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: removing %u subscription(s)...\n"),
                      ACE_TEXT (inherited2::name ()),
                      subscribers_.size ()));
        subscribers_.clear ();
      } // end lock scope

      // remember connection has been closed...
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

        connectionIsAlive_ = false;

        // signal any waiter(s)
        result = condition_.broadcast ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
      } // end lock scope

      break;
    }
    default:
    {
      // forward the data to subscriber(s)

      // synch access to subscribers
      {
        ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

        // *WARNING* if the user unsubscribes() within the callback
        // BAD THINGS (TM) WILL happen, because the current iter WILL be invalidated
        // --> use a slightly modified for-loop (advance first and THEN invoke the
        //     callback, works for MOST containers)
        // *NOTE*: this can only happen due to the ACE_RECURSIVE_Thread_Mutex
        //         used as a lock in order to avoid deadlocks in precisely this
        //         situation...
        for (SubscribersIterator_t iterator = subscribers_.begin ();
             iterator != subscribers_.end ();
            )
        {
          try
          {
            (*iterator++)->notify (*message_inout);
          }
          catch (...)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("caught exception in IRC_Client_INotify::notify(): \"%m\", continuing\n")));
          }
        } // end FOR
      } // end lock scope

      break;
    }
  } // end SWITCH
}

bool
IRC_Client_Module_IRCHandler::registerc (const IRC_LoginOptions& loginOptions_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::registerc"));

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
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

    if (!connectionIsAlive_)
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("waiting for connection to initialize...\n")));

      // *NOTE*: can happen when trying to register IMMEDIATELY after connecting
      //         --> allow a little delay for:
      //         - connection to establish
      //         [- the initial NOTICEs to arrive]
      //         before proceeding...
      ACE_Time_Value timeout (IRC_MAX_NOTICE_DELAY, 0);
      // *NOTE*: cannot use COMMON_TIME_NOW, as this is a high precision monotonous
      //         clock... --> use standard getimeofday
      ACE_Time_Value deadline = ACE_OS::gettimeofday () + timeout;
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
                    ACE_TEXT ("connection timeout (%#T), aborting\n"),
                    &timeout));
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
  IRC_Record* message_p =
    allocateMessage (IRC_Record::PASS);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", aborting\n")));
    return false;
  } // end IF

  message_p->params.push_back (loginOptions_in.passWord);

  // step3b: send it upstream
  sendMessage (message_p);

  // step4: initialize nickname
  nick (loginOptions_in.nickName);

  // step5a: initialize user
  message_p = allocateMessage (IRC_Record::USER);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", aborting\n")));
    return false;
  } // end IF

  message_p->params.push_back (loginOptions_in.user.userName);
  switch (loginOptions_in.user.hostName.discriminator)
  {
    case IRC_LoginOptions::User::Hostname::STRING:
    {
      ACE_ASSERT (loginOptions_in.user.hostName.string);
      message_p->params.push_back (*loginOptions_in.user.hostName.string);
      break;
    }
    case IRC_LoginOptions::User::Hostname::MODE:
    {
      std::ostringstream converter;
      converter << static_cast<unsigned int> (loginOptions_in.user.hostName.mode);
      message_p->params.push_back (std::string (converter.str ()));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid USER <host name> parameter field type (was: %d), aborting\n"),
                  loginOptions_in.user.hostName.discriminator));

      // clean up
      message_p->decrease ();

      return false;
    }
  } // end SWITCH
  message_p->params.push_back (loginOptions_in.user.serverName);
  message_p->params.push_back (loginOptions_in.user.realName);

  // step5b: send it upstream
  sendMessage (message_p);

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

bool
IRC_Client_Module_IRCHandler::wait (const ACE_Time_Value* timeout_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::wait"));

  int result = -1;

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

  result = condition_.wait (timeout_in);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ETIME)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Condition::wait(): \"%m\", continuing\n")));
  } // end IF
  if (initialRegistration_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("registration timeout, aborting\n")));
    return false;
  } // end IF

  return true;
}

void
IRC_Client_Module_IRCHandler::subscribe (IRC_Client_IStreamNotify_t* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::subscribe"));

  // sanity check(s)
  ACE_ASSERT (interfaceHandle_in);

  // synch access to subscribers
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  subscribers_.push_back (interfaceHandle_in);
}

void
IRC_Client_Module_IRCHandler::unsubscribe (IRC_Client_IStreamNotify_t* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::unsubscribe"));

  // sanity check(s)
  ACE_ASSERT (interfaceHandle_in);

  // synch access to subscribers
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (lock_);

  SubscribersIterator_t iterator = subscribers_.begin ();
  for (;
       iterator != subscribers_.end ();
       iterator++)
    if ((*iterator) == interfaceHandle_in)
      break;

  if (iterator != subscribers_.end ())
    subscribers_.erase (iterator);
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: %@), aborting\n"),
                interfaceHandle_in));
}

void
IRC_Client_Module_IRCHandler::nick (const std::string& nickName_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::nick"));

  // step1: initialize NICK
  IRC_Record* message_p =
    allocateMessage (IRC_Record::NICK);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params.push_back (nickName_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::quit (const std::string& reason_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::quit"));

  // step1: initialize QUIT
  IRC_Record* message_p =
    allocateMessage (IRC_Record::QUIT);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  if (!reason_in.empty ())
    message_p->params.push_back (reason_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::join (const string_list_t& channels_in,
                                    const string_list_t& keys_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::join"));

  // step1: initialize JOIN
  IRC_Record* message_p =
    allocateMessage (IRC_Record::JOIN);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  // compute ranges ?
  if (!keys_in.empty ())
  {
    list_item_range_t range;
    if (channels_in.size () > 1)
    {
      range.first = 0;
      range.second = (channels_in.size () - 1);
      message_p->list_param_ranges.push_back (range);
    } // end IF
    if (keys_in.size () > 1)
    {
      range.first = channels_in.size ();
      range.second = (channels_in.size () + keys_in.size () - 1);
      message_p->list_param_ranges.push_back (range);
    } // end IF
  } // end IF
  message_p->params = channels_in;
  // append any keys
  if (!keys_in.empty ())
    message_p->params.insert (message_p->params.end (),
                              keys_in.begin (),
                              keys_in.end ());

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::part (const string_list_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::part"));

  // step1: initialize PART
  IRC_Record* message_p =
    allocateMessage (IRC_Record::PART);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params = channels_in;

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::mode (const std::string& target_in,
                                    char mode_in,
                                    bool enable_in,
                                    const string_list_t& parameters_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::mode"));

  // step1: initialize MODE
  IRC_Record* message_p =
    allocateMessage (IRC_Record::MODE);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  // construct parameter
  std::string mode_string = (enable_in ? ACE_TEXT_ALWAYS_CHAR("+")
                                       : ACE_TEXT_ALWAYS_CHAR("-"));
  mode_string += mode_in;

  message_p->params.push_back (target_in);
  message_p->params.push_back (mode_string);
  // append any parameters
  message_p->params.insert (message_p->params.end (),
                            parameters_in.begin (),
                            parameters_in.end ());

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::topic (const std::string& channel_in,
                                     const std::string& topic_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::topic"));

  // sanity check(s)
  ACE_ASSERT (!topic_in.empty ());

  // step1: initialize TOPIC
  IRC_Record* message_p =
    allocateMessage (IRC_Record::TOPIC);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params.push_back (channel_in);
  message_p->params.push_back (topic_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::names (const string_list_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::names"));

  // step1: initialize NAMES
  IRC_Record* message_p =
    allocateMessage (IRC_Record::NAMES);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  // compute range ?
  if (channels_in.size () > 1)
  {
    list_item_range_t range;
    range.first = 0;
    range.second = (channels_in.size () - 1);
    message_p->list_param_ranges.push_back (range);
  } // end IF
  message_p->params = channels_in;

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::list (const string_list_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::list"));

  // step1: initialize LIST
  IRC_Record* message_p =
    allocateMessage (IRC_Record::LIST);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  // compute range ?
  if (channels_in.size () > 1)
  {
    list_item_range_t range;
    range.first = 0;
    range.second = (channels_in.size () - 1);
    message_p->list_param_ranges.push_back (range);
  } // end IF
  message_p->params = channels_in;

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::invite (const std::string& nick_in,
                                      const std::string& channel_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::invite"));

  // step1: initialize INVITE
  IRC_Record* message_p =
    allocateMessage (IRC_Record::INVITE);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params.push_back (nick_in);
  message_p->params.push_back (channel_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::kick (const std::string& channel_in,
                                    const std::string& nick_in,
                                    const std::string& comment_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::kick"));

  // step1: initialize KICK
  IRC_Record* message_p =
    allocateMessage (IRC_Record::KICK);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params.push_back (channel_in);
  message_p->params.push_back (nick_in);
  if (!comment_in.empty ())
    message_p->params.push_back (comment_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::send (const string_list_t& receivers_in,
                                    const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::send"));

  // sanity check
  if (message_in.empty ())
    return; // nothing to do...

  // step1: initialize PRIVMSG
  IRC_Record* message_p =
    allocateMessage (IRC_Record::PRIVMSG);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  // compute range ?
  if (receivers_in.size () > 1)
  {
    list_item_range_t range;
    range.first = 0;
    range.second = (receivers_in.size () - 1);
    message_p->list_param_ranges.push_back (range);
  } // end IF
  message_p->params = receivers_in;
  message_p->params.push_back (message_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::who (const std::string& name_in,
                                   bool operatorsOnly_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::who"));

  // step1: initialize WHO
  IRC_Record* message_p =
    allocateMessage (IRC_Record::WHO);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params.push_back (name_in);
  if (operatorsOnly_in)
    message_p->params.push_back (std::string ("o"));

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::whois (const std::string& servername_in,
                                     const string_list_t& nicknames_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::whois"));

  // step1: initialize WHOIS
  IRC_Record* message_p =
    allocateMessage (IRC_Record::WHOIS);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params = nicknames_in;
  if (!servername_in.empty ())
    message_p->params.push_front (servername_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::whowas (const std::string& nick_in,
                                      unsigned int count_in,
                                      const std::string& servername_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::whowas"));

  // step1: initialize WHOWAS
  IRC_Record* message_p =
    allocateMessage (IRC_Record::WHOWAS);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params.push_back (nick_in);
  if (count_in)
  {
    std::stringstream converter;
    converter << count_in;
    std::string count_string;
    converter >> count_string;
    message_p->params.push_back (count_string);
  } // end IF
  if (!servername_in.empty ())
    message_p->params.push_back (servername_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::away (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::away"));

  // step1: initialize AWAY
  IRC_Record* message_p =
    allocateMessage (IRC_Record::AWAY);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  // *NOTE*: if there is no away message parameter, the semantic is to
  // "un-away" the user
  if (!message_in.empty ())
    message_p->params.push_back (message_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::users (const std::string& server_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::users"));

  // step1: initialize USERS
  IRC_Record* message_p =
    allocateMessage (IRC_Record::USERS);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  if (!server_in.empty ())
    message_p->params.push_back (server_in);

  // step2: send it upstream
  sendMessage (message_p);
}

void
IRC_Client_Module_IRCHandler::userhost (const string_list_t& nicknames_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::userhost"));

  // step1: initialize USERHOST
  IRC_Record* message_p =
    allocateMessage (IRC_Record::USERHOST);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(): \"%m\", returning\n")));
    return;
  } // end IF

  message_p->params = nicknames_in;

  // step2: send it upstream
  sendMessage (message_p);
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

void
IRC_Client_Module_IRCHandler::onChange (IRC_RegistrationState newState_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::allocateMessage"));

  int result = -1;

  if (newState_in == REGISTRATION_STATE_FINISHED)
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);

    result = condition_.broadcast ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Condition::broadcast(): \"%m\", continuing\n")));
  } // end IF
}

IRC_Message*
IRC_Client_Module_IRCHandler::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::allocateMessage"));

  // initialize return value(s)
  IRC_Message* message_p = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_.streamConfiguration);

  if (configuration_.streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      message_p =
        static_cast<IRC_Message*> (configuration_.streamConfiguration->messageAllocator->malloc (requestedSize_in));
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
        !configuration_.streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      IRC_Message (requestedSize_in));
  if (!message_p)
  {
    if (configuration_.streamConfiguration->messageAllocator)
    {
      if (configuration_.streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}

IRC_Record*
IRC_Client_Module_IRCHandler::allocateMessage (IRC_Record::CommandType type_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::allocateMessage"));

  // initialize return value(s)
  IRC_Record* message_p = NULL;

  ACE_NEW_NORETURN (message_p,
                    IRC_Record ());
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return NULL;
  } // end IF
  ACE_NEW_NORETURN (message_p->command.string,
                    std::string (IRC_Message::Command2String (type_in)));
  if (!message_p->command.string)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

    // clean up
    message_p->decrease ();

    return NULL;
  } // end IF
  message_p->command.discriminator = IRC_Record::Command::STRING;

  return message_p;
}

void
IRC_Client_Module_IRCHandler::sendMessage (IRC_Record*& command_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCHandler::sendMessage"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_.streamConfiguration);

  // step1: get a message buffer
  IRC_Message* message_p =
      allocateMessage (configuration_.streamConfiguration->bufferSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::allocateMessage(%u), returning\n"),
                configuration_.streamConfiguration->bufferSize));

    // clean up
    command_in->decrease ();
    command_in = NULL;

    return;
  } // end IF

  // step2: attach the command
  // *NOTE*: message assumes control over command_in
  message_p->initialize (command_in);
  // --> bye bye...
  command_in = NULL;

  // step3: send it upstream

  // *NOTE*: there is NO way to prevent asynchronous closure of the connection;
  //         this protect against closure of the stream while the message is
  //         propagated... (see line 614)
  //         --> grab lock and check connectionIsAlive_
  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (conditionLock_);
  // sanity check
  if (!connectionIsAlive_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("connection has been closed/lost - cannot send message, returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF

  result = inherited2::reply (message_p, NULL);
  if (result == -1)
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
                    IRC_Client_Module_IRCHandler_Module (ACE_TEXT_ALWAYS_CHAR (inherited2::name ()),
                                                         NULL,
                                                         true));
  if (!module_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  else
  {
    IRC_Client_Module_IRCHandler* handler_p = NULL;
    handler_p =
      dynamic_cast<IRC_Client_Module_IRCHandler*> (module_p->writer ());
    if (!handler_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCHandler> failed, aborting\n")));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
//    if (!handler_p->initialize (configuration_))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::initialize(), aborting\n")));

//      // clean up
//      delete module_p;

//      return NULL;
//    } // end IF
  } // end ELSE

  return module_p;
}
