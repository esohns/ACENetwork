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

#include "stream_dec_tools.h"

#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename RecordType>
HTTP_Module_Parser_T<TimePolicyType,
                     SessionMessageType,
                     ProtocolMessageType,
                     ConfigurationType,
                     RecordType>::HTTP_Module_Parser_T ()
 : inherited ()
 , sessionData_ (NULL)
 , allocator_ (NULL)
 , debugScanner_ (NET_PROTOCOL_DEFAULT_LEX_TRACE) // trace scanning ?
 , debugParser_ (NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , driver_ (NET_PROTOCOL_DEFAULT_LEX_TRACE,  // trace scanning ?
            NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , headFragment_ (NULL)
 , isDriverInitialized_ (false)
 //, lock_ ()
//, condtion_ (lock_)
 , crunchMessages_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // "crunch" messages ?
 , dataContainer_ (NULL)
 , initialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::HTTP_Module_Parser_T"));

}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename RecordType>
HTTP_Module_Parser_T<TimePolicyType,
                     SessionMessageType,
                     ProtocolMessageType,
                     ConfigurationType,
                     RecordType>::~HTTP_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::~HTTP_Module_Parser_T"));

  if (headFragment_)
    headFragment_->release ();

  if (dataContainer_)
    dataContainer_->decrease ();
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename RecordType>
bool
HTTP_Module_Parser_T<TimePolicyType,
                     SessionMessageType,
                     ProtocolMessageType,
                     ConfigurationType,
                     RecordType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (initialized_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("re-initializing...\n")));

    ACE_Message_Queue_Base* queue_p = inherited::msg_queue ();
    result = queue_p->activate ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::activate(): \"%m\", continuing\n")));

    sessionData_ = NULL;

    allocator_ = NULL;
    debugScanner_ = NET_PROTOCOL_DEFAULT_LEX_TRACE;
    debugParser_ = NET_PROTOCOL_DEFAULT_YACC_TRACE;
    if (headFragment_)
    {
      headFragment_->release ();
      headFragment_ = NULL;
    } // end IF
    isDriverInitialized_ = false;

    crunchMessages_ = HTTP_DEFAULT_CRUNCH_MESSAGES;
    if (dataContainer_)
    {
      dataContainer_->decrease ();
      dataContainer_ = NULL;
    } // end IF
    initialized_ = false;
  } // end IF

  // *NOTE*: need to clean up timer beyond this point !

  allocator_ = configuration_in.streamConfiguration->messageAllocator;
  debugScanner_ = configuration_in.traceScanning;
  debugParser_ = configuration_in.traceParsing;

  crunchMessages_ = configuration_in.crunchMessages;
  DATA_T* data_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    return false;
  } // end IF
  ACE_NEW_NORETURN (data_p->HTTPRecord,
                    RecordType ());
  if (!data_p->HTTPRecord)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));

    // clean up
    delete data_p;

    return false;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (data_p)
  ACE_NEW_NORETURN (dataContainer_,
                    DATA_CONTAINER_T (data_p));
  if (!dataContainer_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));

    // clean up
    delete data_p;

    return false;
  } // end IF
  initialized_ = true;

  return true;
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename RecordType>
void
HTTP_Module_Parser_T<TimePolicyType,
                     SessionMessageType,
                     ProtocolMessageType,
                     ConfigurationType,
                     RecordType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleDataMessage"));

  int result = -1;
  DATA_CONTAINER_T* data_container_p = NULL;
  ProtocolMessageType* message_p = NULL;
  ProtocolMessageType* message_2 = NULL;
//  RecordType* record_p = NULL;
  bool release_inbound_message = true; // message_inout
  bool release_message = false; // message_p

  // initialize return value(s)
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_inout->capacity () - message_inout->length () >=
              NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  {
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    if (!headFragment_)
    {
      headFragment_ = message_inout;
      //message_2 = headFragment_;
    } // end IF
    else
    {
      for (message_2 = headFragment_;
           message_2->cont ();
           message_2 = dynamic_cast<ProtocolMessageType*> (message_2->cont ()));
      message_2->cont (message_inout);

      //// just signal the parser (see below for an explanation)
      //result = condition_.broadcast ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%s\", continuing\n"),
      //              inherited::mod_->name ()));
    } // end ELSE
    //ACE_ASSERT (message_2);

    message_p = headFragment_;
    //  dynamic_cast<ProtocolMessageType*> (message_2->duplicate ());
    //if (!message_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%s\", returning\n"),
    //              inherited::mod_->name ()));
    //  goto error;
    //} // end IF
    //release_message = true;

    // sanity check(s)
    ACE_ASSERT (dataContainer_);

    if (message_p->isInitialized ())
    {
      dataContainer_->decrease ();
      dataContainer_ =
        &const_cast<DATA_CONTAINER_T&> (message_p->get ());
      dataContainer_->increase ();
      DATA_T& data_r = const_cast<DATA_T&> (dataContainer_->get ());
      if (!data_r.HTTPRecord)
      {
        ACE_NEW_NORETURN (data_r.HTTPRecord,
                          RecordType ());
        if (!data_r.HTTPRecord)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end IF
    } // end IF
  } // end lock scope
  ACE_ASSERT (message_p);
  message_inout = NULL;
  release_inbound_message = false;

//  // "crunch" messages for easier parsing ?
//  if (crunchMessages_ &&
//      headFragment_->cont ())
//  {
////     message->dump_state();
//
//    // step1: get a new message buffer
//    message_p = allocateMessage (HTTP_BUFFER_SIZE);
//    if (!message_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to allocate message(%u), returning\n"),
//                  inherited::mod_->name (),
//                  HTTP_BUFFER_SIZE));
//      goto error;
//    } // end IF
//
//    // step2: copy available data
//    for (ACE_Message_Block* message_block_p = headFragment_;
//         message_block_p;
//         message_block_p = message_block_p->cont ())
//    {
//      ACE_ASSERT (message_block_p->length () <= message_p->space ());
//      result = message_p->copy (message_block_p->rd_ptr (),
//                                message_block_p->length ());
//      if (result == -1)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
//                    inherited::mod_->name ()));
//
//        // clean up
//        message_p->release ();
//
//        goto error;
//      } // end IF
//    } // end FOR
//
//    // clean up
//    headFragment_->release ();
//    headFragment_ = message_p;
//  } // end IF

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // sanity check(s)
    ACE_ASSERT (dataContainer_);

    // initialize driver ?
    if (!isDriverInitialized_)
    {
      DATA_T& data_r = const_cast<DATA_T&> (dataContainer_->get ());
      ACE_ASSERT (data_r.HTTPRecord);
  
      // *TODO*: find a way to resume parsing after end-of-file to support
      //         passive parser modules
      driver_.initialize (*data_r.HTTPRecord,
                          debugScanner_,
                          debugParser_,
                          inherited::msg_queue (),
                          crunchMessages_,
                          true);
      isDriverInitialized_ = true;
    } // end IF

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->getID (),
    //              message_p->length ()));

    if (!driver_.parse (message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to HTTP_ParserDriver::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->getID ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully

    if (!driver_.hasFinished ())
      goto continue_; // --> wait for more data to arrive

    // set session data
    // *TODO*: move this somewhere else
    DATA_T& data_r = const_cast<DATA_T&> (dataContainer_->get ());

    // sanity check(s)
    ACE_ASSERT (data_r.HTTPRecord);
    ACE_ASSERT (sessionData_);

    HTTP_HeadersIterator_t iterator =
      data_r.HTTPRecord->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
    if (iterator != data_r.HTTPRecord->headers.end ())
    {
      sessionData_->format =
        HTTP_Tools::Encoding2CompressionFormat ((*iterator).second);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: set compression format: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (sessionData_->format).c_str ())));
    } // end IF

    // make sure the whole fragment chain references the same data record
    data_container_p = dataContainer_;
    headFragment_->initialize (data_container_p,
                               NULL);
    message_2 = dynamic_cast<ProtocolMessageType*> (headFragment_->cont ());
    while (message_2)
    {
      dataContainer_->increase ();
      data_container_p = dataContainer_;
      message_2->initialize (data_container_p,
                             NULL);
      message_2 = dynamic_cast<ProtocolMessageType*> (message_2->cont ());

      // *NOTE*: new data fragments may have arrived by now
      if (message_2 == message_p)
        break;
    } // end WHILE
    isDriverInitialized_ = false;
    dataContainer_ = NULL;

    // allocate a new data record
    DATA_T* data_p = NULL;
    ACE_NEW_NORETURN (data_p,
                      DATA_T ());
    if (!data_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory, returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    ACE_NEW_NORETURN (data_p->HTTPRecord,
                      HTTP_Record ());
    if (!data_p->HTTPRecord)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory, returning\n"),
                  inherited::mod_->name ()));

      // clean up
      delete data_p;

      goto error;
    } // end IF
    // *IMPORTANT NOTE*: fire-and-forget API (data_p)
    ACE_NEW_NORETURN (dataContainer_,
                      DATA_CONTAINER_T (data_p));
    if (!dataContainer_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory, returning\n"),
                  inherited::mod_->name ()));

      // clean up
      delete data_p;

      goto error;
    } // end IF
  } // end lock scope

  // *NOTE*: the message has been parsed successfully
  //         --> pass the data (chain) downstream
  {
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    //// *NOTE*: new data fragments may have arrived by now
    ////         --> set the next head fragment ?
    //message_2 = dynamic_cast<ProtocolMessageType*> (message_p->cont ());
    //if (message_2)
    //  message_p->cont (NULL);

    result = inherited::put_next (headFragment_, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));

      // clean up
      headFragment_->release ();

      goto error;
    } // end IF
    headFragment_ = (message_2 ? message_2 : NULL);
  } // end lock scope

continue_:
error:
  if (release_inbound_message)
  {
    ACE_ASSERT (message_inout);
    message_inout->release ();
    message_inout = NULL;
  } // end IF
  if (release_message)
  {
    ACE_ASSERT (message_p);
    message_p->release ();
  } // end IF
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename RecordType>
void
HTTP_Module_Parser_T<TimePolicyType,
                     SessionMessageType,
                     ProtocolMessageType,
                     ConfigurationType,
                     RecordType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);

      const typename SessionMessageType::DATA_T& session_data_container_r =
        message_inout->get ();
      sessionData_ =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_r.get ());

      break;
    }
    case STREAM_SESSION_END:
    {
      // *NOTE*: a parser thread may be waiting for additional (entity)
      //         fragments to arrive
      //         --> tell it to return
      ACE_Message_Queue_Base* queue_p = inherited::msg_queue ();
      result = queue_p->pulse ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue_Base::pulse(): \"%m\", continuing\n")));

      sessionData_ = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename RecordType>
ProtocolMessageType*
HTTP_Module_Parser_T<TimePolicyType,
                     SessionMessageType,
                     ProtocolMessageType,
                     ConfigurationType,
                     RecordType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::allocateMessage"));

  // initialize return value(s)
  ProtocolMessageType* message_p = NULL;

  if (allocator_)
  {
allocate:
    try
    {
      message_p =
        static_cast<ProtocolMessageType*> (allocator_->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p && !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      ProtocolMessageType (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ProtocolMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ProtocolMessageType: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}

////////////////////////////////////////////////////////////////////////////////

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::HTTP_Module_ParserH_T ()
 : inherited (NULL,  // lock handle
              // *NOTE*: the current (pull-)parser needs to be active because
              //         yyparse() will not return until the entity has been
              //         received and processed completely; otherwise, it would
              //         tie one dispatch thread during this time (deadlock for
              //         single-threaded reactors/proactor scenarios)
              true,  // active by default
              true,  // auto-start !
              false, // do not run the svc() routine on start (passive mode)
              false) // do not push session messages
 , sessionData_ (NULL)
 , debugScanner_ (NET_PROTOCOL_DEFAULT_LEX_TRACE) // trace scanning ?
 , debugParser_ (NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , driver_ (NET_PROTOCOL_DEFAULT_LEX_TRACE,  // trace scanning ?
            NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , headFragment_ (NULL)
 , isDriverInitialized_ (false)
 , crunchMessages_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // "crunch" messages ?
 , dataContainer_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::HTTP_Module_ParserH_T"));

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
          typename StatisticContainerType,
          typename RecordType>
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::~HTTP_Module_ParserH_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::~HTTP_Module_ParserH_T"));

  if (headFragment_)
    headFragment_->release ();

  if (dataContainer_)
    dataContainer_->decrease ();
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
          typename StatisticContainerType,
          typename RecordType>
bool
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::initialize"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (inherited::initialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    debugScanner_ = NET_PROTOCOL_DEFAULT_LEX_TRACE;
    debugParser_ = NET_PROTOCOL_DEFAULT_YACC_TRACE;
    if (headFragment_)
    {
      headFragment_->release ();
      headFragment_ = NULL;
    } // end IF
    isDriverInitialized_ = false;

    crunchMessages_ = HTTP_DEFAULT_CRUNCH_MESSAGES;
    if (dataContainer_)
    {
      dataContainer_->decrease ();
      dataContainer_ = NULL;
    } // end IF
  } // end IF

  debugScanner_ = configuration_in.traceScanning;
  debugParser_ = configuration_in.traceParsing;

  crunchMessages_ = configuration_in.crunchMessages;
  DATA_T* data_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    return false;
  } // end IF
  ACE_NEW_NORETURN (data_p->HTTPRecord,
                    HTTP_Record ());
  if (!data_p->HTTPRecord)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));

    // clean up
    delete data_p;

    return false;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (data_p)
  ACE_NEW_NORETURN (dataContainer_,
                    DATA_CONTAINER_T (data_p));
  if (!dataContainer_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));

    // clean up
    delete data_p;

    return false;
  } // end IF

  // OK: all's well...
  result = inherited::initialize (configuration_in);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return result;
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
          typename StatisticContainerType,
          typename RecordType>
void
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                      bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleDataMessage"));

  int result = -1;
  DATA_CONTAINER_T* data_container_p = NULL;
  ProtocolMessageType* message_p = message_inout;
  HTTP_Record* record_p = NULL;

  if (driver_.hasFinished ())
  {
    ACE_ASSERT (dataContainer_);
    dataContainer_->increase ();
    data_container_p = dataContainer_;
    // *TODO*: need to merge message data here
    message_inout->initialize (*data_container_p,
                               NULL);
    return; // done
  } // end IF

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_inout->capacity () - message_inout->length () >=
              NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!headFragment_)
    headFragment_ = message_inout;
  else
  {
    ACE_Message_Block* message_2 = headFragment_;
    for (;
         message_2->cont ();
         message_2 = message_2->cont ());
    message_2->cont (message_inout);
  } // end IF
  ACE_ASSERT (headFragment_);
  message_inout = NULL;
  passMessageDownstream_out = false;

  // "crunch" messages for easier parsing ?
  if (crunchMessages_ &&
      headFragment_->cont ())
  {
//     message->dump_state();

    // step1: get a new message buffer
    message_p = inherited::allocateMessage (HTTP_BUFFER_SIZE);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocate message(%u), returning\n"),
                  HTTP_BUFFER_SIZE));
      goto error;
    } // end IF

    // step2: copy available data
    for (ACE_Message_Block* message_block_p = headFragment_;
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

    // clean up
    headFragment_->release ();
    headFragment_ = message_p;
  } // end IF

  if (headFragment_->isInitialized ())
  {
    dataContainer_->decrease ();
    dataContainer_ =
        &const_cast<DATA_CONTAINER_T&> (headFragment_->get ());
    dataContainer_->increase ();
    DATA_T& data_r = const_cast<DATA_T&> (dataContainer_->get ());
    if (!data_r.HTTPRecord)
    {
      ACE_NEW_NORETURN (data_r.HTTPRecord,
                        HTTP_Record ());
      if (!data_r.HTTPRecord)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory, aborting\n")));
        goto error;
      } // end IF
    } // end IF
    record_p = data_r.HTTPRecord;
  } // end IF
  else
  {
    dataContainer_->increase ();
    data_container_p = dataContainer_;
    headFragment_->initialize (*data_container_p,
                               NULL);
    DATA_T& data_r = const_cast<DATA_T&> (dataContainer_->get ());
    record_p = data_r.HTTPRecord;
  } // end IF
  ACE_ASSERT (record_p);

  // initialize driver ?
  if (!isDriverInitialized_)
  {
    driver_.initialize (*record_p,
                        debugScanner_,
                        debugParser_,
                        inherited::msg_queue (),
                        crunchMessages_);
    isDriverInitialized_ = true;
  } // end IF

  // OK: parse this message

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
//              message_p->getID (),
//              message_p->length ()));

  if (!driver_.parse (headFragment_))
  { // *NOTE*: most probable cause: need more data
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("failed to HTTP_ParserDriver::parse() (message ID: %d), returning\n"),
//                message_p->getID ()));
    goto done;
  } // end IF

  // *NOTE*: the (chained) fragment has been parsed, the read pointer has been
  //         advanced to the entity body
  //         --> pass message (and following) downstream
  //ACE_ASSERT (driver_.record ());
//  driver_.record ()->dump_state ();

  // make sure the chain references the same data
  message_p = dynamic_cast<ProtocolMessageType*> (headFragment_->cont ());

  while (message_p)
  {
    dataContainer_->increase ();
    data_container_p = dataContainer_;
    // *TODO*: need to merge message data here
    message_p->initialize (*data_container_p,
                           NULL);
    message_p = dynamic_cast<ProtocolMessageType*> (message_p->cont ());
  } // end WHILE

  result = inherited::put_next (headFragment_, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task_T::put_next(): \"%m\", continuing\n")));
    goto error;
  } // end IF
  headFragment_ = NULL;

done:
  return;

error:
  headFragment_->release ();
  headFragment_ = NULL;
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
          typename StatisticContainerType,
          typename RecordType>
void
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                         bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // retain session ID for reporting
      const SessionDataContainerType& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;

      // start profile timer...
      //profile_.start ();

      break;
    }
    case STREAM_SESSION_END:
    {
      // *NOTE*: a parser thread may be waiting for additional (entity)
      //         fragments to arrive
      //         --> tell it to return
      ACE_Message_Queue_Base* queue_p = inherited::msg_queue ();
      result = queue_p->pulse ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue_Base::pulse(): \"%m\", continuing\n")));

      break;
    }
    default:
    {
      // don't do anything...
      break;
    }
  } // end SWITCH
}

//template <typename LockType,
//          typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//ProtocolMessageType*
//HTTP_Module_ParserH_T<LockType,
//                     TaskSynchType,
//                     TimePolicyType,
//                     SessionMessageType,
//                     ProtocolMessageType,
//                     ConfigurationType,
//                     StreamStateType,
//                     SessionDataType,
//                     SessionDataContainerType,
//                     StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::allocateMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
//  // initialize return value(s)
//  ProtocolMessageType* message_p = NULL;
//
//  if (inherited::configuration_->streamConfiguration->messageAllocator)
//  {
//allocate:
//    try
//    {
//      message_p =
//        static_cast<ProtocolMessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
//                  requestedSize_in));
//      return NULL;
//    }
//
//    // keep retrying ?
//    if (!message_p && !inherited::configuration_->streamConfiguration->messageAllocator->block ())
//      goto allocate;
//  } // end IF
//  else
//    ACE_NEW_NORETURN (message_p,
//                      ProtocolMessageType (requestedSize_in));
//  if (!message_p)
//  {
//    if (inherited::configuration_->streamConfiguration->messageAllocator)
//    {
//      if (inherited::configuration_->streamConfiguration->messageAllocator->block ())
//        ACE_DEBUG ((LM_CRITICAL,
//                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
//    } // end IF
//    else
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
//  } // end IF
//
//  return message_p;
//}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
bool
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      SessionMessageType,
                      ProtocolMessageType,
                      ConfigurationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::collect"));

  // step1: initialize info container POD
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <typename LockType,
//          typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//HTTP_Module_ParserH_T<LockType,
//                     TaskSynchType,
//                     TimePolicyType,
//                     SessionMessageType,
//                     ProtocolMessageType,
//                     ConfigurationType,
//                     StreamStateType,
//                     SessionDataType,
//                     SessionDataContainerType,
//                     StatisticContainerType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return);
//}

//template <typename LockType,
//          typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//bool
//HTTP_Module_ParserH_T<LockType,
//                     TaskSynchType,
//                     TimePolicyType,
//                     SessionMessageType,
//                     ProtocolMessageType,
//                     ConfigurationType,
//                     StreamStateType,
//                     SessionDataType,
//                     SessionDataContainerType,
//                     StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
//{
//  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::putStatisticMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
////  // step1: initialize session data
////  IRC_StreamSessionData* session_data_p = NULL;
////  ACE_NEW_NORETURN (session_data_p,
////                    IRC_StreamSessionData ());
////  if (!session_data_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
////    return false;
////  } // end IF
////  //ACE_OS::memset (data_p, 0, sizeof (IRC_SessionData));
//  SessionDataType& session_data_r =
//      const_cast<SessionDataType&> (inherited::sessionData_->get ());
//  session_data_r.currentStatistic = statisticData_in;
//
////  // step2: allocate session data container
////  IRC_StreamSessionData_t* session_data_container_p = NULL;
////  // *NOTE*: fire-and-forget stream_session_data_p
////  ACE_NEW_NORETURN (session_data_container_p,
////                    IRC_StreamSessionData_t (stream_session_data_p,
////                                                    true));
////  if (!session_data_container_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//
////    // clean up
////    delete stream_session_data_p;
//
////    return false;
////  } // end IF
//
//  // step3: send the data downstream...
//  // *NOTE*: fire-and-forget session_data_container_p
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       *inherited::sessionData_,
//                                       inherited::configuration_->streamConfiguration->messageAllocator);
//}
