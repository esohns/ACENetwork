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
#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "net_macros.h"

#include "http_defines.h"
#include "http_record.h"

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HTTP_Module_Parser_T<LockType,
                     TaskSynchType,
                     TimePolicyType,
                     SessionMessageType,
                     ProtocolMessageType,
                     ConfigurationType,
                     StreamStateType,
                     SessionDataType,
                     SessionDataContainerType,
                     StatisticContainerType>::HTTP_Module_Parser_T ()
 : inherited (NULL,  // lock handle
              false, // inactive by default
              false, // DON'T auto-start !
              false) // do not run the svc() routine on start
 , statisticCollectHandler_ (ACTION_COLLECT,
                             this,
                             false)
 , statisticCollectHandlerID_ (-1)
 , debugScanner_ (HTTP_DEFAULT_LEX_TRACE) // trace scanning ?
 , debugParser_ (HTTP_DEFAULT_YACC_TRACE) // trace parsing ?
 , driver_ (HTTP_DEFAULT_LEX_TRACE,  // trace scanning ?
            HTTP_DEFAULT_YACC_TRACE) // trace parsing ?
 , isDriverInitialized_ (false)
 , crunchMessages_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // "crunch" messages ?
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::HTTP_Module_Parser_T"));

}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
HTTP_Module_Parser_T<LockType,
  TaskSynchType,
  TimePolicyType,
  SessionMessageType,
  ProtocolMessageType,
  ConfigurationType,
  StreamStateType,
  SessionDataType,
  SessionDataContainerType,
  StatisticContainerType>::~HTTP_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::~HTTP_Module_Parser_T"));

  // clean up timer if necessary
  if (statisticCollectHandlerID_ != -1)
  {
    int result =
     COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_);
    if (result <= 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statisticCollectHandlerID_));
  } // end IF
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
HTTP_Module_Parser_T<LockType,
  TaskSynchType,
  TimePolicyType,
  SessionMessageType,
  ProtocolMessageType,
  ConfigurationType,
  StreamStateType,
  SessionDataType,
  SessionDataContainerType,
  StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (statisticCollectHandlerID_ != -1)
    {
      int result =
       COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_);
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    statisticCollectHandlerID_));
      statisticCollectHandlerID_ = -1;
    } // end IF

    debugScanner_ = HTTP_DEFAULT_LEX_TRACE;
    debugParser_ = HTTP_DEFAULT_YACC_TRACE;
    isDriverInitialized_ = false;
    crunchMessages_ = HTTP_DEFAULT_CRUNCH_MESSAGES;

    isInitialized_ = false;
  } // end IF

  if (configuration_in.streamConfiguration->statisticReportingInterval)
  {
    // schedule regular statistics collection...
    ACE_Time_Value interval (STREAM_STATISTIC_COLLECTION, 0);
    ACE_ASSERT (statisticCollectHandlerID_ == -1);
    ACE_Event_Handler* handler_p = &statisticCollectHandler_;
    statisticCollectHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule (handler_p,                        // event handler
                                                            NULL,                             // act
                                                            COMMON_TIME_POLICY () + interval, // first wakeup time
                                                            interval);                        // interval
    if (statisticCollectHandlerID_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule(): \"%m\", aborting\n")));
      return false;
    } // end IF
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
//                 statisticCollectHandlerID_,
//                 statisticCollectionInterval_in));
  } // end IF

  // *NOTE*: need to clean up timer beyond this point !

  debugScanner_ = configuration_in.traceScanning;
  debugParser_ = configuration_in.traceParsing;
  crunchMessages_ = configuration_in.crunchMessages;

  // OK: all's well...
  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return isInitialized_;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
HTTP_Module_Parser_T<LockType,
  TaskSynchType,
  TimePolicyType,
  SessionMessageType,
  ProtocolMessageType,
  ConfigurationType,
  StreamStateType,
  SessionDataType,
  SessionDataContainerType,
  StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                            bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleDataMessage"));

  int result = -1;
  HTTP_Record* record_p = NULL;

  // "crunch" messages for easier parsing ?
  ProtocolMessageType* message_p = message_inout;
  if (crunchMessages_ &&
      message_inout->cont ())
  {
//     message->dump_state();

    // step1: get a new message buffer
    message_p = allocateMessage (HTTP_BUFFER_SIZE);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocate message(%u), returning\n"),
                  HTTP_BUFFER_SIZE));
      goto error;
    } // end IF

    // step2: copy available data
    for (ACE_Message_Block* message_block_p = message_inout;
         message_block_p;
         message_block_p = message_block_p->cont ())
    {
      ACE_ASSERT (message_block_p->length () <= message_p->space ());
      result = message_p->copy (message_block_p->rd_ptr (),
                                message_block_p->length ());
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));

        // clean up
        message_p->release ();

        goto error;
      } // end IF
    } // end FOR

    record_p = const_cast<HTTP_Record*> (message_inout->getData ());
    if (record_p)
      record_p->increase ();

    // clean up
    message_inout->release ();
    message_inout = NULL;
    passMessageDownstream_out = false;
  } // end IF
  ACE_ASSERT (message_p);

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_p->capacity () - message_p->length () >= HTTP_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_p->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_p->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() should stay as it was

  if (!record_p)
  {
    // allocate the target data container and attach it to the current message ?
    ACE_NEW_NORETURN (record_p,
                      HTTP_Record ());
    if (!record_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
      goto error;
    } // end IF
    HTTP_Record* record_2 = record_p;
    message_p->initialize (record_2);

    // *NOTE*: message has assumed control over "record_p"...
  } // end IF
  ACE_ASSERT (record_p);

  // initialize driver ?
  if (!isDriverInitialized_)
  {
    driver_.initialize (*record_p,
                        debugScanner_,
                        debugParser_);
    isDriverInitialized_ = true;
  } // end IF

  // OK: parse this message

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("parsing message(ID: %u, %u byte(s))\n\"%s\"\n"),
//               message_p->getID (),
//               message_p->length (),
//               ACE_TEXT (std::string (message_p->rd_ptr (),
//                                      message_p->length ()).c_str ())));

  if (!driver_.parse (message_p,        // data block
                      crunchMessages_)) // has data been crunched ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_ParserDriver::parse() (message ID: %d), returning\n"),
                message_p->getID ()));
    goto error;
  } // end IF

  if (!passMessageDownstream_out)
  {
    result = inherited::put_next (message_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_T::put_next(): \"%m\", continuing\n")));
      goto error;
    } // end IF
  } // end IF

  return;

error:
  message_p->release ();
  message_p = NULL;
  passMessageDownstream_out = false;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
HTTP_Module_Parser_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // retain session ID for reporting
      const SessionDataContainerType& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r =
          session_data_container_r.get ();
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;

      // start profile timer...
      //profile_.start ();

      break;
    }
    default:
    {
      // don't do anything...
      break;
    }
  } // end SWITCH
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ProtocolMessageType*
HTTP_Module_Parser_T<LockType,
  TaskSynchType,
  TimePolicyType,
  SessionMessageType,
  ProtocolMessageType,
  ConfigurationType,
  StreamStateType,
  SessionDataType,
  SessionDataContainerType,
  StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_p = NULL;

  if (inherited::configuration_.streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      message_p =
        static_cast<ProtocolMessageType*> (inherited::configuration_.streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p && !inherited::configuration_.streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      ProtocolMessageType (requestedSize_in));
  if (!message_p)
  {
    if (inherited::configuration_.streamConfiguration->messageAllocator)
    {
      if (inherited::configuration_.streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
HTTP_Module_Parser_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step1: initialize info container POD
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timestamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
HTTP_Module_Parser_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::report"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // *TODO*: support (local) reporting here as well ?
  //         --> leave this to any downstream modules...
  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
HTTP_Module_Parser_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);

//  // step1: initialize session data
//  IRC_StreamSessionData* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    IRC_StreamSessionData ());
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  //ACE_OS::memset (data_p, 0, sizeof (IRC_SessionData));
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  session_data_r.currentStatistic = statisticData_in;

//  // step2: allocate session data container
//  IRC_StreamSessionData_t* session_data_container_p = NULL;
//  // *NOTE*: fire-and-forget stream_session_data_p
//  ACE_NEW_NORETURN (session_data_container_p,
//                    IRC_StreamSessionData_t (stream_session_data_p,
//                                                    true));
//  if (!session_data_container_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

//    // clean up
//    delete stream_session_data_p;

//    return false;
//  } // end IF

  // step3: send the data downstream...
  // *NOTE*: fire-and-forget session_data_container_p
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       *inherited::sessionData_,
                                       inherited::configuration_.streamConfiguration->messageAllocator);
}
