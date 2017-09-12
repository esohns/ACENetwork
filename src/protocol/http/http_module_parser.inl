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

#include "stream_dec_common.h"
#include "stream_dec_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     SessionMessageType>::HTTP_Module_Parser_T (ISTREAM_T* stream_in)
#else
                     SessionMessageType>::HTTP_Module_Parser_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , inherited2 (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_LEXER_DFA_TABLES_FILENAME), // scanner tables file (if any)
               NET_PROTOCOL_PARSER_DEFAULT_LEX_TRACE,                     // trace scanning ?
               NET_PROTOCOL_PARSER_DEFAULT_YACC_TRACE)                    // trace parsing ?
 , headFragment_ (NULL)
 , crunch_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // strip protocol data ?
 //, lock_ ()
//, condtion_ (lock_)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::HTTP_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::~HTTP_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::~HTTP_Module_Parser_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    ACE_ASSERT (inherited::msg_queue_);
    result = inherited::msg_queue_->activate ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::activate(): \"%m\", continuing\n")));

    crunch_ = HTTP_DEFAULT_CRUNCH_MESSAGES;

    if (headFragment_)
    {
      headFragment_->release ();
      headFragment_ = NULL;
    } // end IF
  } // end IF

  crunch_ = configuration_in.crunchMessages;

  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      inherited::msg_queue_;
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to HTTP_ParserDriver_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleDataMessage"));

  DataMessageType* message_p = NULL;
  int result = -1;
  bool release_inbound_message = true; // message_inout
  bool release_message = false; // message_p

  // initialize return value(s)
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_inout->space () >= NET_PROTOCOL_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    if (!headFragment_)
      headFragment_ = message_inout;
    else
    {
      for (message_p = headFragment_;
           message_p->cont ();
           message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
      message_p->cont (message_inout);

      //// just signal the parser (see below for an explanation)
      //result = condition_.broadcast ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%s\", continuing\n"),
      //              inherited::mod_->name ()));
    } // end ELSE

    message_p = headFragment_;
  } // end lock scope
  ACE_ASSERT (message_p);
  message_inout = NULL;
  release_inbound_message = false;

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!this->parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_IParser::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully

    if (!this->hasFinished ())
      goto continue_; // --> wait for more data to arrive
  } // end lock scope

  // *NOTE*: the message has been parsed successfully
  //         --> pass the data (chain) downstream
  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
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
    headFragment_ = NULL;
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
          typename SessionMessageType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleSessionMessage"));

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

      if (headFragment_)
      {
        headFragment_->release ();
        headFragment_ = NULL;
      } // end IF

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
          typename SessionMessageType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::record (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == inherited2::record_);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (!headFragment_->isInitialized ());

  // debug info
  if (inherited2::trace_)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s"),
                ACE_TEXT (HTTP_Tools::dump (*record_inout).c_str ())));

  // set session data format
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

  HTTP_HeadersIterator_t iterator =
      record_inout->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
  if (iterator != record_inout->headers.end ())
  {
    session_data_r.format =
        HTTP_Tools::EncodingToCompressionFormat ((*iterator).second);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set compression format: \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (session_data_r.format).c_str ())));
  } // end IF

  DATA_CONTAINER_T* data_container_p, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;

  ACE_NEW_NORETURN (data_container_p,
                    DATA_CONTAINER_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    delete record_inout;
    record_inout = NULL;

    goto error;
  } // end IF
  data_container_p->set (record_inout);
  record_inout = NULL;
  data_container_2 = data_container_p;
  headFragment_->initialize (data_container_2,
                             NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  message_p = dynamic_cast<DataMessageType*> (headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           NULL);
    message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  inherited2::finished_ = true;
error:
  inherited2::record_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
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
          typename StatisticHandlerType,
          typename UserDataType>
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      StatisticHandlerType,
                      UserDataType>::HTTP_Module_ParserH_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in,                               // stream handle
              false,                                   // auto-start ? (active mode only)
              STREAM_HEADMODULECONCURRENCY_CONCURRENT, // concurrency mode
              true)                                    // generate sesssion messages ?
 , inherited2 (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_LEXER_DFA_TABLES_FILENAME), // scanner tables file (if any)
               NET_PROTOCOL_PARSER_DEFAULT_LEX_TRACE,                     // trace scanning ?
               NET_PROTOCOL_PARSER_DEFAULT_YACC_TRACE)                    // trace parsing ?
 , headFragment_ (NULL)
 , crunch_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // strip protocol data ?
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::HTTP_Module_ParserH_T"));

}

template <ACE_SYNCH_DECL,
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
          typename StatisticHandlerType,
          typename UserDataType>
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      StatisticHandlerType,
                      UserDataType>::~HTTP_Module_ParserH_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::~HTTP_Module_ParserH_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
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
          typename StatisticHandlerType,
          typename UserDataType>
bool
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      StatisticHandlerType,
                      UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                 Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    crunch_ = HTTP_DEFAULT_CRUNCH_MESSAGES;

    if (headFragment_)
    {
      headFragment_->release ();
      headFragment_ = NULL;
    } // end IF
  } // end IF

  crunch_ = configuration_in.crunchMessages;
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      inherited::msg_queue_;
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize parser driver: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
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
          typename StatisticHandlerType,
          typename UserDataType>
void
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      StatisticHandlerType,
                      UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleDataMessage"));

  DataMessageType* message_p = NULL;
  int result = -1;
  bool release_inbound_message = true; // message_inout
  bool release_message = false; // message_p

  // initialize return value(s)
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_inout->space () >= NET_PROTOCOL_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    if (!headFragment_)
      headFragment_ = message_inout;
    else
    {
      for (message_p = headFragment_;
           message_p->cont ();
           message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
      message_p->cont (message_inout);

      //// just signal the parser (see below for an explanation)
      //result = condition_.broadcast ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%s\", continuing\n"),
      //              inherited::mod_->name ()));
    } // end ELSE

    message_p = headFragment_;
  } // end lock scope
  ACE_ASSERT (message_p);
  message_inout = NULL;
  release_inbound_message = false;

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!this->parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_ParserDriver::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully

    if (!this->hasFinished ())
      goto continue_; // --> wait for more data to arrive
  } // end lock scope

  // *NOTE*: the message has been parsed successfully
  //         --> pass the data (chain) downstream
  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
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
    headFragment_ = NULL;
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
          typename StatisticHandlerType,
          typename UserDataType>
void
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      StatisticHandlerType,
                      UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      const SessionDataType& session_data_r = inherited::sessionData_->getR ();

      // retain session ID for reporting
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->sessionData);
      ACE_ASSERT (inherited::streamState_->sessionData->lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *(inherited::streamState_->sessionData->lock));
        inherited::streamState_->sessionData->sessionId =
          session_data_r.sessionId;
      } // end lock scope

      //// start profile timer
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
      { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (headFragment_)
      {
        headFragment_->release ();
        headFragment_ = NULL;
      } // end IF

      // *TODO*: remove type inference
      if (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
        this->inherited::TASK_BASE_T::stop (false,  // wait for completion ?
                                            false); // N/A

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
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
          typename StatisticHandlerType,
          typename UserDataType>
bool
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      StatisticHandlerType,
                      UserDataType>::collect (StatisticContainerType& data_out)
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
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
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
          typename StatisticHandlerType,
          typename UserDataType>
void
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      StatisticHandlerType,
                      UserDataType>::record (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == inherited2::record_);
  ACE_ASSERT (!headFragment_->isInitialized ());

  // debug info
  if (inherited2::trace_)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s"),
                ACE_TEXT (HTTP_Tools::dump (*record_inout).c_str ())));

  // set session data format
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  HTTP_HeadersIterator_t iterator =
      record_inout->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
  if (iterator != record_inout->headers.end ())
  {
    session_data_r.format =
        HTTP_Tools::EncodingToCompressionFormat ((*iterator).second);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set compression format: \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (session_data_r.format).c_str ())));
  } // end IF

  DATA_CONTAINER_T* data_container_p, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;

  ACE_NEW_NORETURN (data_container_p,
                    DATA_CONTAINER_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    delete record_inout;
    record_inout = NULL;

    goto error;
  } // end IF
  data_container_p->setPR (record_inout);
  record_inout = NULL;
  data_container_2 = data_container_p;
  headFragment_->initialize (data_container_2,
                             headFragment_->sessionId (),
                             NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  message_p = dynamic_cast<DataMessageType*> (headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           headFragment_->sessionId (),
                           NULL);
    message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  inherited2::finished_ = true;
error:
  inherited2::record_ = NULL;
}
