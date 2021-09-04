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

#include <ace/Log_Msg.h>

#include "common_timer_manager_common.h"

#include "stream_dec_tools.h"

#include "net_macros.h"

#include "ppspp_common.h"
#include "ppspp_defines.h"
#include "ppspp_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename RecordType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     RecordType>::HTTP_Module_Parser_T ()
 : inherited ()
 , allocator_ (NULL)
 , debugScanner_ (NET_PROTOCOL_DEFAULT_LEX_TRACE) // trace scanning ?
 , debugParser_ (NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , driver_ (NET_PROTOCOL_DEFAULT_LEX_TRACE,  // trace scanning ?
            NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , headFragment_ (NULL)
 , isDriverInitialized_ (false)
 //, lock_ ()
//, condtion_ (lock_)
 , crunch_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // strip protocol data ?
 , dataContainer_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::HTTP_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename RecordType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     RecordType>::~HTTP_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::~HTTP_Module_Parser_T"));

  if (headFragment_)
    headFragment_->release ();

  if (dataContainer_)
    dataContainer_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename RecordType>
bool
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     RecordType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("re-initializing...\n")));

    ACE_ASSERT (inherited::msg_queue_);
    result = inherited::msg_queue_->activate ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::activate(): \"%m\", continuing\n")));

    allocator_ = NULL;
    debugScanner_ = NET_PROTOCOL_DEFAULT_LEX_TRACE;
    debugParser_ = NET_PROTOCOL_DEFAULT_YACC_TRACE;
    if (headFragment_)
    {
      headFragment_->release ();
      headFragment_ = NULL;
    } // end IF
    isDriverInitialized_ = false;

    crunch_ = HTTP_DEFAULT_CRUNCH_MESSAGES;
    if (dataContainer_)
    {
      dataContainer_->decrease ();
      dataContainer_ = NULL;
    } // end IF

    inherited::isInitialized_ = false;
  } // end IF

  // *NOTE*: need to clean up timer beyond this point !

  allocator_ = configuration_in.streamConfiguration->messageAllocator;
  debugScanner_ = configuration_in.traceScanning;
  debugParser_ = configuration_in.traceParsing;

  crunch_ = configuration_in.crunchMessages;
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

  return inherited::initialize (configuration_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename RecordType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     RecordType>::handleDataMessage (DataMessageType*& message_inout,
                                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleDataMessage"));

  int result = -1;
  DATA_CONTAINER_T* data_container_p = NULL;
  DataMessageType* message_p = NULL;
  DataMessageType* message_2 = NULL;
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
           message_2 = dynamic_cast<DataMessageType*> (message_2->cont ()));
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
    //  dynamic_cast<DataMessageType*> (message_2->duplicate ());
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
                          inherited::msg_queue_,
                          true,
                          true);
      isDriverInitialized_ = true;
    } // end IF

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!driver_.parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_ParserDriver::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
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
    ACE_ASSERT (inherited::sessionData_);

    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

    HTTP_HeadersIterator_t iterator =
      data_r.HTTPRecord->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
    if (iterator != data_r.HTTPRecord->headers.end ())
    {
      session_data_r.format =
        HTTP_Tools::Encoding2CompressionFormat ((*iterator).second);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: set compression format: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (session_data_r.format).c_str ())));
    } // end IF

    // make sure the whole fragment chain references the same data record
    data_container_p = dataContainer_;
    headFragment_->initialize (data_container_p,
                               NULL);
    message_2 = dynamic_cast<DataMessageType*> (headFragment_->cont ());
    while (message_2)
    {
      dataContainer_->increase ();
      data_container_p = dataContainer_;
      message_2->initialize (data_container_p,
                             NULL);
      message_2 = dynamic_cast<DataMessageType*> (message_2->cont ());

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
    //message_2 = dynamic_cast<DataMessageType*> (message_p->cont ());
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename RecordType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     RecordType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleSessionMessage"));

  //int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_END:
    {
      //// *NOTE*: (in a 'passive' scenario,) a parser thread may be waiting for
      ////         additional (entity) fragments to arrive
      ////         --> tell it to return
      //ACE_ASSERT (inherited::msg_queue_);
      //result = inherited::msg_queue_->pulse ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_Message_Queue_Base::pulse(): \"%m\", continuing\n")));

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename RecordType>
DataMessageType*
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     RecordType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_p = NULL;

  if (allocator_)
  {
allocate:
    try {
      message_p =
        static_cast<DataMessageType*> (allocator_->malloc (requestedSize_in));
    } catch (...) {
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
                      DataMessageType (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate DataMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate DataMessageType: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}

////////////////////////////////////////////////////////////////////////////////

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::HTTP_Module_ParserH_T ()
 : inherited (NULL,  // lock handle
              false, // auto-start ?
              true)  // generate sesssion messages ?
 , debugScanner_ (NET_PROTOCOL_DEFAULT_LEX_TRACE) // trace scanning ?
 , debugParser_ (NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , driver_ (NET_PROTOCOL_DEFAULT_LEX_TRACE,  // trace scanning ?
            NET_PROTOCOL_DEFAULT_YACC_TRACE) // trace parsing ?
 , headFragment_ (NULL)
 , isDriverInitialized_ (false)
 , crunch_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // strip protocol data ?
 , dataContainer_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::HTTP_Module_ParserH_T"));

}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
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
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
bool
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (inherited::isInitialized_)
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

    crunch_ = HTTP_DEFAULT_CRUNCH_MESSAGES;
    if (dataContainer_)
    {
      dataContainer_->decrease ();
      dataContainer_ = NULL;
    } // end IF

    inherited::isInitialized_ = false;
  } // end IF

  debugScanner_ = configuration_in.traceScanning;
  debugParser_ = configuration_in.traceParsing;

  crunch_ = configuration_in.crunchMessages;
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

  return inherited::initialize (configuration_in);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
void
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::handleDataMessage (DataMessageType*& message_inout,
                                                      bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleDataMessage"));

  int result = -1;
  DATA_CONTAINER_T* data_container_p = NULL;
  DataMessageType* message_p = NULL;
  DataMessageType* message_2 = NULL;
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
           message_2 = dynamic_cast<DataMessageType*> (message_2->cont ()));
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
    //  dynamic_cast<DataMessageType*> (message_2->duplicate ());
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
                          inherited::msg_queue_,
                          true,
                          true);
      isDriverInitialized_ = true;
    } // end IF

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!driver_.parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_ParserDriver::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
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
    ACE_ASSERT (inherited::sessionData_);

    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

    HTTP_HeadersIterator_t iterator =
      data_r.HTTPRecord->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
    if (iterator != data_r.HTTPRecord->headers.end ())
    {
      session_data_r.format =
        HTTP_Tools::Encoding2CompressionFormat ((*iterator).second);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: set compression format: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (session_data_r.format).c_str ())));
    } // end IF

    // make sure the whole fragment chain references the same data record
    data_container_p = dataContainer_;
    headFragment_->initialize (data_container_p,
                               NULL);
    message_2 = dynamic_cast<DataMessageType*> (headFragment_->cont ());
    while (message_2)
    {
      dataContainer_->increase ();
      data_container_p = dataContainer_;
      message_2->initialize (data_container_p,
                             NULL);
      message_2 = dynamic_cast<DataMessageType*> (message_2->cont ());

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
    //message_2 = dynamic_cast<DataMessageType*> (message_p->cont ());
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

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
void
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                         bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleSessionMessage"));

//  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      const SessionDataType& session_data_r = inherited::sessionData_->get ();

      // retain session ID for reporting
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;

      // start profile timer...
      //profile_.start ();

      break;
    }
    //// *NOTE*: the stream has been link()ed, the message contains the (merged)
    ////         upstream session data --> retain a reference
    //case STREAM_SESSION_MESSAGE_LINK:
    //{
    //  // *NOTE*: relax the concurrency policy in this case
    //  // *TODO*: this isn't very reliable
    //  inherited::concurrent_ = true;

    //  break;
    //}
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited::lock_);

        if (inherited::sessionEndProcessed_) break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // *NOTE*: in passive 'concurrent' scenarios, there is no 'worker' thread
      //         running svc()
      //         --> do not signal completion in this case
      // *TODO*: remove type inference
      if (inherited::thr_count_ || inherited::runSvcOnStart_)
        this->inherited::TASK_BASE_T::stop (false, // wait ?
                                            false, // high priority ?
                                            true); // locked access ?

      break;
    }
    default:
      break;
  } // end SWITCH
}

//template <typename LockType,
//          typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//DataMessageType*
//HTTP_Module_ParserH_T<LockType,
//                     TaskSynchType,
//                     TimePolicyType,
//                     SessionMessageType,
//                     DataMessageType,
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
//  DataMessageType* message_p = NULL;
//
//  if (inherited::configuration_->streamConfiguration->messageAllocator)
//  {
//allocate:
//    try {
//      message_p =
//        static_cast<DataMessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
//    } catch (...) {
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
//                      DataMessageType (requestedSize_in));
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
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename RecordType>
bool
HTTP_Module_ParserH_T<LockType,
                      TaskSynchType,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      RecordType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::collect"));

  // step1: initialize info container POD
  data_out.capturedFrames = 0;
  data_out.droppedFrames = 0;
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
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
//          typename DataMessageType,
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
//                     DataMessageType,
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
//          typename DataMessageType,
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
//                     DataMessageType,
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
//  // step3: send the data downstream
//  // *NOTE*: fire-and-forget session_data_container_p
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       *inherited::sessionData_,
//                                       inherited::configuration_->streamConfiguration->messageAllocator);
//}
