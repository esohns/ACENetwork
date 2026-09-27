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

#include "common_string_tools.h"

#include "stream_dec_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "ws_common.h"
#include "ws_defines.h"
#include "ws_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::WebSocket_Module_Parser_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 (this)
 , headFragment_ (NULL)
 , dataBytesToSkip_ (0)
 , dataSize_ (0)
 , handshakeComplete_ (false)
 , queue_ (0,    // max # slots --> unlimited
           NULL) // notification handle
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::WebSocket_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::~WebSocket_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::~WebSocket_Module_Parser_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
bool
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::initialize (const ConfigurationType& configuration_in,
                                                        Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    if (headFragment_)
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF

    dataBytesToSkip_ = 0;
    dataSize_ = 0;
    handshakeComplete_ = false;
    queue_.activate ();
    queue_.flush (true); // flush any messages
  } // end IF

  ACE_ASSERT (!configuration_in.parserConfiguration->messageQueue);
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
    &queue_;
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to WebSocket_ParserDriver_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      NULL;
    return false;
  } // end IF
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
    NULL;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
void
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::handleDataMessage"));

  passMessageDownstream_out = false;

  int result = queue_.enqueue_tail (message_inout, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::enqueue_tail(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF

  return;

error:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
void
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  bool high_priority_b = false;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      else if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u inbound data messages\n"),
                    inherited::mod_->name (),
                    result));

      high_priority_b = true;
      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *NOTE*: this prevents a race condition in svc()
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        inherited::threadCount_ = 1;
        bool lock_activate_was_b = inherited::TASK_BASE_T::TASK_BASE_T::lockActivate_;
        inherited::lockActivate_ = false;
        if (unlikely (inherited::open (NULL) == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_Task_Base_T::open(), aborting\n"),
                      inherited::mod_->name ()));
          inherited::lockActivate_ = lock_activate_was_b;
          inherited::threadCount_ = 0;
          goto error;
        } // end IF
        inherited::lockActivate_ = lock_activate_was_b;
        inherited::threadCount_ = 0;
        ACE_ASSERT (!inherited::threadIds_.empty ());
      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      stop (true,             // wait ?
            high_priority_b); // high priority ?

      if (headFragment_)
      {
        headFragment_->release (); headFragment_ = NULL;
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
          typename SessionMessageType,
          typename ParserDriverType>
void
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::record (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::record"));

  // invoke base-class implementation
  inherited::record (record_inout);

  handshakeComplete_ = true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
void
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::record (struct WebSocket_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (headFragment_);

  //if (unlikely (inherited2::configuration_->debugParser))
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s"),
  //              ACE_TEXT (WebSocket_Tools::dump (*record_inout).c_str ())));

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  DATA_CONTAINER_T* data_container_p, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;
  DATA_T* data_p = NULL;
  ACE_Message_Block* message_block_p = headFragment_;
  size_t bytes_to_skip = 0;
  size_t total_length, available_data;

  ACE_NEW_NORETURN (data_p,
                    DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  *data_p = *record_inout;

  ACE_NEW_NORETURN (data_container_p,
                    DATA_CONTAINER_T (data_p,
                                      true)); // delete ?
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  data_container_2 = data_container_p;
  headFragment_->initialize (data_container_2,
                             session_data_r.sessionId,
                             NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  //message_p = static_cast<DataMessageType*> (headFragment_->cont ());
  //while (message_p)
  //{
  //  data_container_p->increase ();
  //  data_container_2 = data_container_p;
  //  message_p->initialize (data_container_2,
  //                         message_p->sessionId (),
  //                         NULL);
  //  if (message_p->cont () == NULL)
  //    message_block_p = message_p;
  //  message_p = static_cast<DataMessageType*> (message_p->cont ());
  //} // end WHILE

  // frame the content
  message_block_p = headFragment_;
  data_p = &const_cast<DATA_T&> (data_container_p->getR ());

  // *IMPORTANT NOTE*: the parsers' offset points to the begining of the data
  bytes_to_skip = inherited2::offset ();
  do
  { ACE_ASSERT (message_block_p);
    available_data = message_block_p->length ();
    if (bytes_to_skip <= available_data)
      break;
    bytes_to_skip -= available_data;
    message_block_p->rd_ptr (available_data);
    message_block_p = message_block_p->cont ();
  } while (true);
  message_block_p->rd_ptr (bytes_to_skip);
  //if (!message_block_p->length ())
  //  message_block_p = message_block_p->cont ();

  // *NOTE*: might not have received ALL of the body; OTOH may have received
  //         MORE than the body (if the client sent more data than specified
  //         in the content length header; i.e. the next response may already
  //         have been (partially) received and appended to the fragment chain
  //         --> do this in the handleDataMessage() method
  total_length = headFragment_->total_length ();
  // ACE_ASSERT (total_length >= total_data);

  bytes_to_skip = data_p->payloadSize;

  ACE_Message_Block* message_block_2 = NULL;
  do
  { ACE_ASSERT (message_block_p);
    available_data = message_block_p->length ();
    if (bytes_to_skip <= available_data)
      break;
    bytes_to_skip -= available_data;
    message_block_p = message_block_p->cont ();
  } while (true);
  if (bytes_to_skip < available_data)
  {
    message_block_2 = message_block_p->duplicate ();
    message_block_2->cont (message_block_p->cont ());
    message_block_p->cont (message_block_2);
    message_block_p->length (bytes_to_skip);
    message_block_2->rd_ptr (bytes_to_skip);
  } // end IF
  else
  {
    ACE_ASSERT (bytes_to_skip == available_data);
  } // end ELSE

error:
  ;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
int
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::put (ACE_Message_Block* messageBlock_in,
                                                  ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::put"));

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
    { // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->parserConfiguration);
      if (likely (!inherited::configuration_->parserConfiguration->notifyProgress))
        break;

      typename SessionMessageType::DATA_T* session_data_container_p =
        inherited::sessionData_;
      // *IMPORTANT NOTE*: send 'step data' session message so downstream modules know
      //                   that some data has arrived ?
      if (likely (session_data_container_p))
      {
        session_data_container_p->increase ();
        typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());
        session_data_r.bytes += messageBlock_in->total_length ();
      } // end IF
      if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP_DATA,
                                                   session_data_container_p,
                                                   NULL,
                                                   false))) // expedited ?
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                    inherited::mod_->name (),
                    STREAM_SESSION_MESSAGE_STEP_DATA));
      break;
    }
    default:
      break;
  } // end SWITCH

  return inherited::put (messageBlock_in,
                         timeValue_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
void
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::stop (bool waitForCompletion_in,
                                                   bool highPriority_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::stop"));

  ACE_Message_Block* message_block_p = NULL;
  int result;

  // enqueue a control message
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = (highPriority_in ? queue_.enqueue_head (message_block_p, NULL) :
                              queue_.enqueue_tail (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::%s(): \"%m\", continuing\n"),
                inherited::mod_->name (),
                (highPriority_in ? ACE_TEXT ("enqueue_head") : ACE_TEXT ("enqueue_tail"))));
    message_block_p->release (); message_block_p = NULL;
  } // end IF  
  message_block_p = NULL;

  if (waitForCompletion_in)
  {
    Common_ITask* itask_p = this;
    itask_p->wait (true); // wait for message queue(s) ?
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
int
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  ACE_Message_Block* message_block_p = NULL;
  int result;
  int error = -1;

  do
  {
    result = queue_.dequeue_head (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      //if (error == ETIME)
      //  goto continue_;

      if (unlikely (error != ESHUTDOWN))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Message_Queue_T::dequeue_head(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
      result = 0; // OK, queue has been deactivate()d
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      message_block_p->release (); message_block_p = NULL;
      break; // done
    } // end IF

    if (likely (handshakeComplete_))
      dispatch_2 (message_block_p);
    else
      inherited::dispatch (message_block_p);
    message_block_p = NULL;
  } while (true);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
void
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::dispatch_2 (ACE_Message_Block* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::dispatch_2"));

  ACE_Message_Block* message_block_p = NULL, *message_block_2 = NULL;
  int result = -1;
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;
  size_t payload_length_i, total_length_i;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->parserConfiguration);

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_in->capacity () - message_in->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_in->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_in->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!headFragment_)
    headFragment_ = static_cast<DataMessageType*> (message_in);
  else
  {
    for (message_block_p = headFragment_;
         message_block_p->cont ();
         message_block_p = message_block_p->cont ());
    message_block_p->cont (message_in);
  } // end ELSE
  message_block_p = headFragment_;
  ACE_ASSERT (message_block_p);

  // OK: parse the message (fragment)
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: parsing message (id:%u (%u byte(s))...\n"),
  //            inherited::mod_->name (),
  //            static_cast<DataMessageType*> (message_block_p)->id (),
  //            message_block_p->total_length ()));

parse:
  if (!inherited2::parse (message_block_p))
  { // *NOTE*: most probable reason: connection
    //         has been closed --> session end
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: failed to WebSocket_IParser::parse() (message id was: %u), returning\n"),
                inherited::mod_->name (),
                static_cast<DataMessageType*> (message_block_p)->id ()));
    headFragment_->release (); headFragment_ = NULL;
    return;
  } // end IF
  // the message fragment has been parsed successfully

  if (!inherited2::hasFinished ())
  {
    if (!inherited2::switchBuffer (false)) // do not begin()(, will be done in parse())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to WebSocket_IParser::switchBuffer(), returning\n"),
                  inherited::mod_->name ()));
      return; // --> wait for more data to arrive
    } // end IF
    message_block_p = inherited2::fragment_;
    goto parse;
  } // end IF

  // *NOTE*: the complete message has been parsed successfully,
  //         but the headFragment_ MAY have additional data appended to it
  //         --> re-frame the message body, if necessary
  payload_length_i = getPayloadLength ();
  total_length_i = headFragment_->total_length ();
  if (payload_length_i >= total_length_i)
  {
    message_block_p = headFragment_;
    headFragment_ = NULL;
    goto continue_;
  } // end IF
  ACE_ASSERT (total_length_i > payload_length_i);

  message_block_p = Stream_Tools::get (payload_length_i,
                                       headFragment_,
                                       message_block_2);
  ACE_ASSERT (message_block_p);
  headFragment_ = static_cast<DataMessageType*> (message_block_2);

continue_:
  ACE_ASSERT (message_block_p);

  // *NOTE*: the message has been parsed/framed successfully
  //         --> pass the data (chain) downstream
  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_block_p->release ();
    goto error;
  } // end IF

  if (likely (!inherited::configuration_->parserConfiguration->notifyProgress))
    goto continue_2;

  // *IMPORTANT NOTE*: send 'step' session message so downstream modules know
  //                   that the complete document data has arrived
  if (likely (session_data_container_p))
    session_data_container_p->increase ();
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP,
                                               session_data_container_p,
                                               NULL,
                                               false))) // expedited ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::mod_->name (),
                STREAM_SESSION_MESSAGE_STEP));
  session_data_container_p = inherited::sessionData_;

continue_2:
  if (headFragment_)
  {
    message_block_p = headFragment_;
    goto parse;
  } // end IF

  return;

error:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
size_t
WebSocket_Module_Parser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ParserDriverType>::getPayloadLength ()
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_Parser_T::getPayloadLength"));

  // sanity check(s)
  ACE_ASSERT (headFragment_);
  if (unlikely (!headFragment_->isInitialized ()))
    return 0;

  const typename DataMessageType::DATA_T& data_container_r =
    headFragment_->getR ();
  const typename DataMessageType::DATA_T::DATA_T& data_r =
    data_container_r.getR ();
  return data_r.payloadSize;
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::WebSocket_Module_ParserH_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 (this)
 , headFragment_ (NULL)
 , dataBytesToSkip_ (0)
 , dataSize_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::WebSocket_Module_ParserH_T"));

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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::~WebSocket_Module_ParserH_T ()
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::~WebSocket_Module_ParserH_T"));

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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
bool
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    if (headFragment_)
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF

    dataBytesToSkip_ = 0;
    dataSize_ = 0;
  } // end IF

  ACE_ASSERT (!configuration_in.parserConfiguration->messageQueue);
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
    inherited::msg_queue_;
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize parser driver: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      NULL;
    return false;
  } // end IF
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
    NULL;

  if (unlikely (configuration_in.parserConfiguration->multiBody))
    multiBody_ = true;

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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
void
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::handleDataMessage"));

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  bool release_inbound_message = true; // message_inout
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;

  // initialize return value(s)
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->parserConfiguration);

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    if (!headFragment_)
      headFragment_ = message_inout;
    else
    {
      for (message_block_p = headFragment_;
           message_block_p->cont ();
           message_block_p = message_block_p->cont ());
      message_block_p->cont (message_inout);
    } // end ELSE
    message_block_p = headFragment_;
  } // end lock scope
  ACE_ASSERT (message_block_p);
  message_inout = NULL;
  release_inbound_message = false;

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (id:%u (%u byte(s))...\n"),
    //              dynamic_cast<DataMessageType*> (message_block_p)->id (),
    //              message_block_p->total_length ()));

    if (!this->parse (message_block_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to WebSocket_ParserDriver::parse() (message id was: %u), returning\n"),
                  inherited::mod_->name (),
                  dynamic_cast<DataMessageType*> (message_block_p)->id ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully

    if (!this->hasFinished ())
      goto continue_; // --> wait for more data to arrive
  } // end lock scope

  // *NOTE*: the message has been parsed successfully
  //         --> pass the data (chain) downstream
  message_block_p = headFragment_;
  headFragment_ = NULL;

continue_2:
  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
    goto error;
  } // end IF

  // *IMPORTANT NOTE*: send 'step' session message so downstream modules know
  //                   that the complete document data has arrived
  if (likely (session_data_container_p))
    session_data_container_p->increase ();
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP,
                                               session_data_container_p,
                                               NULL,
                                               false))) // expedited ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::mod_->name (),
                STREAM_SESSION_MESSAGE_STEP));

continue_:
error:
  if (release_inbound_message)
  { ACE_ASSERT (message_block_p);
    message_inout->release (); message_inout = NULL;
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
void
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      if (headFragment_)
      {
        headFragment_->release (); headFragment_ = NULL;
      } // end IF
      chunks_.clear ();

      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      if (inherited::endSeenFromUpstream_ && // <-- there was (!) an upstream
          inherited::configuration_->stopOnUnlink)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received unlink from upstream, updating state\n"),
                    inherited::mod_->name ()));
        inherited::change (STREAM_STATE_SESSION_STOPPING);
      } // end IF
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      // const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      //   inherited::sessionData_->getR ();

      //// start profile timer
      //profile_.start ();

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (headFragment_)
      {
        headFragment_->release (); headFragment_ = NULL;
      } // end IF
      chunks_.clear ();

      if (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
      { Common_ITask* itask_p = this;
        itask_p->stop (false,  // wait ?
                       false); // high priority ?
      } // end IF

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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
bool
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::collect"));

  // step1: initialize info container POD
  data_out.capturedFrames = 0;
  data_out.droppedFrames = 0;
  data_out.bytes = 0;
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
void
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::record (struct WebSocket_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == &(inherited2::record_));
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (headFragment_);
  //ACE_ASSERT (!headFragment_->isInitialized ());

  //if (unlikely (inherited2::configuration_->debugParser))
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s"),
  //              ACE_TEXT (WebSocket_Tools::dump (*record_inout).c_str ())));

  // set session data format
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  DATA_CONTAINER_T* data_container_p, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;
  DATA_T* data_p = NULL;
  ACE_Message_Block* message_block_p = headFragment_;
  unsigned int bytes_to_skip = 0;

  ACE_NEW_NORETURN (data_p,
                    DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  *data_p = *record_inout;
  record_inout = NULL;

  ACE_NEW_NORETURN (data_container_p,
                    DATA_CONTAINER_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  data_container_p->setPR (data_p);
  data_container_2 = data_container_p;
  headFragment_->initialize (data_container_2,
                             headFragment_->sessionId (),
                             NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  //message_p = static_cast<DataMessageType*> (headFragment_->cont ());
  //while (message_p)
  //{
  //  data_container_p->increase ();
  //  data_container_2 = data_container_p;
  //  message_p->initialize (data_container_2,
  //                         headFragment_->sessionId (),
  //                         NULL);
  //  message_p = static_cast<DataMessageType*> (message_p->cont ());
  //} // end WHILE

  // frame the content
  message_block_p = headFragment_;
  data_p = &const_cast<DATA_T&> (data_container_p->getR ());

  // *IMPORTANT NOTE*: the parsers' offset points to the begining of the data
  bytes_to_skip = inherited2::offset ();
  do
  { ACE_ASSERT (message_block_p);
    available_data = message_block_p->length ();
    if (bytes_to_skip <= available_data)
      break;
    bytes_to_skip -= available_data;
    message_block_p->rd_ptr (available_data);
    message_block_p = message_block_p->cont ();
  } while (true);
  message_block_p->rd_ptr (bytes_to_skip);
  //if (!message_block_p->length ())
  //  message_block_p = message_block_p->cont ();

  // *NOTE*: might not have received ALL of the body; OTOH may have received
  //         MORE than the body (if the client sent more data than specified
  //         in the content length header; i.e. the next response may already
  //         have been (partially) received and appended to the fragment chain
  //         --> do this in the handleDataMessage() method
  total_length = headFragment_->total_length ();
  // ACE_ASSERT (total_length >= total_data);

  bytes_to_skip = data_p->payloadSize;

  ACE_Message_Block* message_block_2 = NULL;
  do
  { ACE_ASSERT (message_block_p);
    available_data = message_block_p->length ();
    if (bytes_to_skip <= available_data)
      break;
    bytes_to_skip -= available_data;
    message_block_p = message_block_p->cont ();
  } while (true);
  if (bytes_to_skip < available_data)
  {
    message_block_2 = message_block_p->duplicate ();
    message_block_2->cont (message_block_p->cont ());
    message_block_p->cont (message_block_2);
    message_block_p->length (bytes_to_skip);
    message_block_2->rd_ptr (bytes_to_skip);
  } // end IF
  else
  {
    ACE_ASSERT (bytes_to_skip == available_data);
  } // end ELSE

  inherited2::finished_ = true;

error:
  ;
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
int
WebSocket_Module_ParserH_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           ConfigurationType,
                           StreamControlType,
                           StreamNotificationType,
                           StreamStateType,
                           StatisticContainerType,
                           SessionManagerType,
                           TimerManagerType,
                           UserDataType,
                           ParserDriverType>::put (ACE_Message_Block* messageBlock_in,
                                                   ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("WebSocket_Module_ParserH_T::put"));

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
    {
      typename SessionMessageType::DATA_T* session_data_container_p =
        inherited::sessionData_;

      // *IMPORTANT NOTE*: send 'step data' session message so downstream modules know
      //                   that some data has arrived
      if (likely (session_data_container_p))
      {
        session_data_container_p->increase ();

        typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());
        session_data_r.bytes += messageBlock_in->total_length ();
      } // end IF
      if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP_DATA,
                                                   session_data_container_p,
                                                   NULL,
                                                   false))) // expedited ?
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                    inherited::mod_->name (),
                    STREAM_SESSION_MESSAGE_STEP_DATA));
      break;
    }
    default:
      break;
  } // end SWITCH

  return inherited::put (messageBlock_in,
                         timeValue_in);
}
