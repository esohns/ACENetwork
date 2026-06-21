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

#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::HTTP_Module_Parser_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 (this)
 , headFragment_ (NULL)
 , bodyOrChunkBytesToSkip_ (0)
 , chunks_ ()
 , contentLengthOrChunkSize_ (0)
 , queue_ (0,    // max # slots --> unlimited
           NULL) // notification handle
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::HTTP_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::~HTTP_Module_Parser_T ()
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
          typename SessionMessageType,
          typename ParserDriverType>
bool
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::initialize (const ConfigurationType& configuration_in,
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

    if (headFragment_)
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF
    chunks_.clear ();
  } // end IF

  ACE_ASSERT (!configuration_in.parserConfiguration->messageQueue);
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue = &queue_;
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to HTTP_ParserDriver_T::initialize(), aborting\n"),
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
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::handleDataMessage (DataMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleDataMessage"));

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
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleSessionMessage"));

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
      chunks_.clear ();

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
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::record (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == &(inherited2::record_));
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (headFragment_);

  //if (unlikely (inherited2::configuration_->debugParser))
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s"),
  //              ACE_TEXT (HTTP_Tools::dump (*record_inout).c_str ())));

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
  message_p = static_cast<DataMessageType*> (headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           message_p->sessionId (),
                           NULL);
    if (message_p->cont () == NULL)
      message_block_p = message_p;
    message_p = static_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  // adjust buffer ?
  // *NOTE*: message_block_p points at the tail fragment (see above)
  // *NOTE*: only need to do this for the tail fragment; other fragments are
  //         handled in switchBuffer()
  //this->handleRealloc (message_block_p);

  // frame the content
  message_block_p = headFragment_;
  data_p = &const_cast<DATA_T&> (data_container_p->getR ());
  if (chunks_.empty ())
  {
    // *IMPORTANT NOTE*: the parsers' offset points to the begining of the body
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

    // sanity check(s)
    iterator =
      data_p->headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
    if (iterator != data_p->headers.end ())
    {
      std::istringstream converter;
      converter.str ((*iterator).second);
      converter >> bytes_to_skip;
      if (unlikely (bytes_to_skip == 0))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: content length was 0, continuing\n"),
                    inherited::mod_->name ()));
        bytes_to_skip = headFragment_->total_length ();
      } // end IF
    } // end IF
    else
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: no content length header, continuing\n"),
                  inherited::mod_->name ()));
      bytes_to_skip = headFragment_->total_length ();
    } // end ELSE
    // *NOTE*: might not have received ALL of the body; OTOH may have received
    //         MORE than the body (if the client sent more data than specified
    //         in the content length header; i.e. the next response may already
    //         have been (partially) received and appended to the fragment chain
    //         --> do this in the handleDataMessage() method
    total_length = headFragment_->total_length ();
    //ACE_ASSERT (total_length >= bytes_to_skip);
  } // end IF
  else
  { // --> chunked transfer
    ACE_Message_Block* message_block_2 = NULL;
    HTTP_ChunksConstIterator_t iterator_2 = chunks_.begin ();
    HTTP_ChunksConstIterator_t iterator_3;
    size_t total_data = 0;
    bytes_to_skip = (*iterator_2).first; // skip HTTP header + first chunk line
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
    if (!message_block_p->length ())
      message_block_p = message_block_p->cont ();
    for (;
         iterator_2 != chunks_.end ();
         ++iterator_2)
    {
      total_data += (*iterator_2).second;
      bytes_to_skip = (*iterator_2).second; // skip chunk
      if (!bytes_to_skip) // no (more) data
      {
        // skip over any trailing header(s) and end delimiter
        bytes_to_skip = inherited2::offset () - (*iterator_2).first;
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
      } // end IF
      else
      {
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
        message_block_p = message_block_p->cont ();
        // skip over chunk delimiter
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\r');
        message_block_p->rd_ptr (1);
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\n');
        message_block_p->rd_ptr (1);
        // skip over next chunk line
        iterator_3 = iterator_2;
        std::advance (iterator_3, 1);
        ACE_ASSERT (iterator_3 != chunks_.end ());
        bytes_to_skip = (*iterator_3).first -
                        ((*iterator_2).first + (*iterator_2).second + 2);
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
      } // end ELSE
    } // end FOR
    // *NOTE*: might not have received ALL of the body; OTOH may have received
    //         MORE than the body (if the client sent more data than specified
    //         in the content length header; i.e. the next response may already
    //         have been (partially) received and appended to the fragment chain
    //         --> do this in the handleDataMessage() method
    total_length = headFragment_->total_length ();
    //ACE_ASSERT (total_length >= total_data);
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
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::put (ACE_Message_Block* messageBlock_in,
                                             ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::put"));

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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::stop (bool waitForCompletion_in,
                                              bool highPriority_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::stop"));

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
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::svc"));

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

    dispatch (message_block_p);
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
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::dispatch (ACE_Message_Block* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::dispatch"));

  ACE_Message_Block *message_block_p = NULL, *message_block_2 = NULL;
  int result = -1;
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;
  size_t content_length_i, total_length_i;

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
  if (!this->parse (message_block_p))
  { // *NOTE*: most probable reason: connection
    //         has been closed --> session end
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: failed to HTTP_IParser::parse() (message id was: %u), returning\n"),
                inherited::mod_->name (),
                static_cast<DataMessageType*> (message_block_p)->id ()));
    return;
  } // end IF
  // the message fragment has been parsed successfully

  if (!this->hasFinished ())
    return; // --> wait for more data to arrive

  // *NOTE*: the complete document has been parsed successfully,
  //         but the headFragment_ MAY have additional data appended to it
  //         --> re-frame the document body, if necessary
  content_length_i = getContentLength ();
  total_length_i = headFragment_->total_length ();
  if (!content_length_i || content_length_i == total_length_i)
  {
    message_block_p = headFragment_;
    headFragment_ = NULL;
    goto continue_;
  } // end IF
  ACE_ASSERT (total_length_i > content_length_i);

  message_block_p = Stream_Tools::get (content_length_i,
                                       headFragment_,
                                       message_block_2);
  ACE_ASSERT (message_block_p);
  headFragment_ = static_cast<DataMessageType*> (message_block_2);

  //if (headFragment_)
  //{ // *TODO*: remove this ASAP
  //  bool bytes_repaired_b = false;
  //  // repair broken data; flex may (!) have clobbered the first few bytes
  //  if ((headFragment_->length () >= 1) && (*headFragment_->rd_ptr () != 'H'))
  //  { bytes_repaired_b = true;
  //    *headFragment_->rd_ptr () = 'H';
  //  } // end IF
  //  if ((headFragment_->length () >= 2) && ((*headFragment_->rd_ptr () + 1) != 'T'))
  //  { bytes_repaired_b = true;
  //    *(headFragment_->rd_ptr () + 1) = 'T';
  //  } // end IF
  //  if ((headFragment_->length () >= 3) && ((*headFragment_->rd_ptr () + 2) != 'T'))
  //  { bytes_repaired_b = true;
  //    *(headFragment_->rd_ptr () + 2) = 'T';
  //  } // end IF
  //  if ((headFragment_->length () >= 4) && ((*headFragment_->rd_ptr () + 3) != 'P'))
  //  { bytes_repaired_b = true;
  //    *(headFragment_->rd_ptr () + 3) = 'P';
  //  } // end IF
  //  if (unlikely (bytes_repaired_b))
  //    ACE_DEBUG ((LM_WARNING,
  //                ACE_TEXT ("%s: repaired HTTP header...\n"),
  //                inherited::mod_->name ()));
  //} // end IF

continue_:
  ACE_ASSERT (message_block_p);
  chunks_.clear ();

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
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType,
                     ParserDriverType>::getContentLength ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::getContentLength"));

  // sanity check(s)
  ACE_ASSERT (headFragment_);

  size_t result = 0;

  if (chunks_.empty ())
  {
    const typename DataMessageType::DATA_T& data_container_r =
      headFragment_->getR ();
    const typename DataMessageType::DATA_T::DATA_T& data_r =
      data_container_r.getR ();
    HTTP_HeadersConstIterator_t iterator =
      data_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
    if (iterator != data_r.headers.end ())
    {
      std::istringstream converter;
      converter.str ((*iterator).second);
      converter >> result;
      if (result == 0)
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: content length was 0, continuing\n"),
                    inherited::mod_->name ()));
        result = headFragment_->total_length ();
      } // end IF
    } // end IF
    else
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: no content length header, continuing\n"),
                  inherited::mod_->name ()));
      result = headFragment_->total_length ();
    } // end ELSE
  } // end IF
  else
  { // --> chunked transfer
    for (HTTP_ChunksConstIterator_t iterator_2 = chunks_.begin ();
         iterator_2 != chunks_.end ();
         ++iterator_2)
      result += (*iterator_2).second;
  } // end ELSE

  return result;
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
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      ParserDriverType>::HTTP_Module_ParserH_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 (this)
 , headFragment_ (NULL)
 , chunks_ ()
 , contentLengthOrChunkSize_ (0)
 , bodyOrChunkBytesToSkip_ (0)
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      ParserDriverType>::~HTTP_Module_ParserH_T ()
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
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
                      StatisticContainerType,
                      SessionManagerType,
                      TimerManagerType,
                      UserDataType,
                      ParserDriverType>::initialize (const ConfigurationType& configuration_in,
                                                     Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    if (headFragment_)
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF
    chunks_.clear ();
  } // end IF

  ACE_ASSERT (!configuration_in.parserConfiguration->messageQueue);
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
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
                      StatisticContainerType,
                      SessionManagerType,
                      TimerManagerType,
                      UserDataType,
                      ParserDriverType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleDataMessage"));

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  bool release_inbound_message = true; // message_inout
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;

  // initialize return value(s)
  passMessageDownstream_out = false;

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
                  ACE_TEXT ("%s: failed to HTTP_ParserDriver::parse() (message id was: %u), returning\n"),
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
  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    //// *NOTE*: new data fragments may have arrived by now
    ////         --> set the next head fragment ?
    //message_2 = dynamic_cast<DataMessageType*> (message_block_p->cont ());
    //if (message_2)
    //  message_block_p->cont (NULL);

    result = inherited::put_next (headFragment_, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      headFragment_->release (); headFragment_ = NULL;
      goto error;
    } // end IF
    headFragment_ = NULL;
  } // end lock scope

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
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleSessionMessage"));

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
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::collect"));

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
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                      ParserDriverType>::record (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::record"));

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
  //              ACE_TEXT (HTTP_Tools::dump (*record_inout).c_str ())));

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
  message_p = static_cast<DataMessageType*> (headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           headFragment_->sessionId (),
                           NULL);
    message_p = static_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  // frame the content
  data_p = &const_cast<DATA_T&> (data_container_p->getR ());
  if (chunks_.empty ())
  {
    // *IMPORTANT NOTE*: the parsers' offset points to the begining of the body
    bytes_to_skip = inherited2::offset ();
    do
    { ACE_ASSERT (message_block_p);
      if (bytes_to_skip <= message_block_p->length ())
        break;
      bytes_to_skip -= message_block_p->length ();
      message_block_p->rd_ptr (message_block_p->length ());
      message_block_p = message_block_p->cont ();
    } while (true);
    message_block_p->rd_ptr (bytes_to_skip);
    iterator =
      data_p->headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
    ACE_ASSERT (iterator != data_p->headers.end ());
    std::istringstream converter;
    converter.str ((*iterator).second);
    converter >> bytes_to_skip;
    ACE_ASSERT (headFragment_->total_length () == bytes_to_skip);
  } // end IF
  else
  { ACE_ASSERT (!chunks_.empty ());
    ACE_Message_Block* message_block_2 = NULL;
    HTTP_ChunksConstIterator_t iterator_2 = chunks_.begin ();
    HTTP_ChunksConstIterator_t iterator_3;
    unsigned int total_data = 0;
    bytes_to_skip = (*iterator_2).first; // skip HTTP header + first chunk line
    do
    { ACE_ASSERT (message_block_p);
      if (bytes_to_skip <= message_block_p->length ())
        break;
      bytes_to_skip -= message_block_p->length ();
      message_block_p->rd_ptr (message_block_p->length ());
      message_block_p = message_block_p->cont ();
    } while (true);
    message_block_p->rd_ptr (bytes_to_skip);
    for (;
         iterator_2 != chunks_.end ();
         ++iterator_2)
    {
      total_data += (*iterator_2).second;
      bytes_to_skip = (*iterator_2).second; // skip chunk
      if (!bytes_to_skip) // no (more) data
      {
        // skip over any trailing header(s) and end delimiter
        bytes_to_skip = inherited2::offset () - (*iterator_2).first;
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p->rd_ptr (message_block_p->length ());
          message_block_p = message_block_p->cont ();
        } while (true);
        message_block_p->rd_ptr (bytes_to_skip);
      } // end IF
      else
      {
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p = message_block_p->cont ();
        } while (true);
        if (bytes_to_skip < message_block_p->length ())
        {
          message_block_2 = message_block_p->duplicate ();
          message_block_2->cont (message_block_p->cont ());
          message_block_p->cont (message_block_2);
          message_block_p->length (bytes_to_skip);
          message_block_2->rd_ptr (bytes_to_skip);
        } // end IF
        message_block_p = message_block_p->cont ();
        // skip over chunk delimiter
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\r');
        message_block_p->rd_ptr (1);
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\n');
        message_block_p->rd_ptr (1);
        // skip over next chunk line
        iterator_3 = iterator_2;
        std::advance (iterator_3, 1);
        ACE_ASSERT (iterator_3 != chunks_.end ());
        bytes_to_skip = (*iterator_3).first -
                        ((*iterator_2).first + (*iterator_2).second + 2);
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p->rd_ptr (message_block_p->length ());
          message_block_p = message_block_p->cont ();
        } while (true);
        message_block_p->rd_ptr (bytes_to_skip);
      } // end ELSE
    } // end FOR
    ACE_ASSERT (headFragment_->total_length () == total_data);
    chunks_.clear ();
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
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
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
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::put"));

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
