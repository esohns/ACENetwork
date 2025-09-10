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

#include "ace/config-macros.h"
#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"
#include "ace/Synch_Traits.h"

#include "common_parser_defines.h"

#include "common_timer_manager_common.h"

#include "stream_iallocator.h"

#include "net_defines.h"
#include "net_macros.h"

#include "http_bisector.h"

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
          typename TimerManagerType>
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::HTTP_Module_Bisector_T (typename inherited::ISTREAM_T* stream_in,
                                                                  bool autoStart_in,
                                                                  bool generateSessionMessages_in)
 : inherited (stream_in,
              autoStart_in,
              generateSessionMessages_in)
 , bufferState_ (NULL)
 , numberOfFrames_ (0)
 , context_ (NULL)
 , buffer_ (NULL)
 , messageLength_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::HTTP_Module_Bisector_T"));

  if (HTTP_Bisector_lex_init (&context_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init(): \"%m\", continuing\n")));

  // trace ?
  HTTP_Bisector_set_debug ((COMMON_PARSER_DEFAULT_LEX_TRACE ? 1 : 0),
                           context_);
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
          typename TimerManagerType>
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::~HTTP_Module_Bisector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::~HTTP_Module_Bisector_T"));

  // fini scanner context
  if (context_)
    HTTP_Bisector_lex_destroy (context_);

  // clean up any unprocessed (chained) buffer(s)
  if (buffer_)
    buffer_->release ();
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
          typename TimerManagerType>
bool
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::initialize"));

//  bool result = false;

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (inherited::initialized_)
  {
    numberOfFrames_ = 0;
    if (bufferState_)
    {
      HTTP_Bisector__delete_buffer (bufferState_,
                                    context_);
      bufferState_ = NULL;
    } // end IF
    if (buffer_)
      buffer_->release ();
    buffer_ = NULL;
    messageLength_ = 0;
//     currentBufferIsResized_ = false;
  } // end IF

  // *NOTE*: need to clean up timer beyond this point !

  //// initialize scanner context
  //if (IRC_Bisectlex_init_extra (&numberOfFrames_, // extra data
  //                              &context_))  // scanner context handle
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IRCBisectlex_init_extra(): \"%m\", aborting\n")));

  //  // clean up
  //  COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_);
  //  statisticCollectHandlerID_ = 0;

  //  return false;
  //} // end IF
  HTTP_Bisector_set_debug ((configuration_in.traceScanning ? 1 : 0),
                           context_);

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
          typename TimerManagerType>
void
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::handleDataMessage"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // initialize return value(s), default behavior is to pass all messages along
  // --> don't want that !
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // perhaps part of this message has already been received ?
  if (buffer_)
  {
    message_block_p = buffer_->cont ();
    if (message_block_p)
    {
      while (message_block_p->cont ()) // skip to end
        message_block_p = message_block_p->cont ();
    } // end IF
    else
      message_block_p = buffer_;
    message_block_p->cont (message_inout); // chain the buffer
  } // end IF
  else
    buffer_ = message_inout;

  // scan the byte-stream for frame bounds "\r\n"

  // *NOTE*: the scanner splits sequences of >= 2 bytes (.*\r\n)
  //         --> make sure a minimum amount of data has been received
  //         --> more sanity check(s)
  //if (buffer_->total_length () < HTTP_FRAME_BOUNDARY_SIZE)
  //  return; // don't have enough data, cannot proceed

  // OK, initialize scanner...

  //// *NOTE*: in order to accomodate flex, the current (!) buffer needs two
  ////         trailing '\0' characters (--> make sure it has this capacity)
  //ACE_ASSERT (message_inout->space () >= IRC_FLEX_BUFFER_BOUNDARY_SIZE);
//   if (buffer_->space () < IRC_FLEX_BUFFER_BOUNDARY_SIZE)
//   {
//     // *sigh*: (try to) resize it then...
//     if (buffer_->size (buffer_->size () + IRC_FLEX_BUFFER_BOUNDARY_SIZE))
//     {
//       ACE_DEBUG ((LM_ERROR,
//                   ACE_TEXT ("failed to ACE_Message_Block::size(%u), returning\n"),
//                   (buffer_->size () + IRC_FLEX_BUFFER_BOUNDARY_SIZE)));
//       return;
//     } // end IF
//     myCurrentBufferIsResized = true;
//
//     // *WARNING*: beyond this point, resize the buffer back to its original
//     //            length...
//     // *NOTE*: this is safe, as realloc() just crops the trailing bytes again
//   } // end IF
  //*(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  //*(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;

  //if (!scan_begin (message_inout->rd_ptr (),
  //                 message_inout->length () + IRC_FLEX_BUFFER_BOUNDARY_SIZE))

  message_block_p = message_inout;
  if (!scan_begin (message_block_p->rd_ptr (),
                   message_block_p->length ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scan_begin(%@, %u), returning\n"),
                message_block_p->rd_ptr (),
                message_block_p->length ()));
    return;
  } // end IF

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: scanning %u byte(s)\n\"%s\"\n"),
//               buffer_->getID (),
//               buffer_->length (),
//               ACE_TEXT (std::string (buffer_->rd_ptr (), buffer_->length ()).c_str ())));

  // scan it !
  numberOfFrames_ = 0;
  bool finished_scanning = false;
  int scanned_bytes = 0;
  int scanned_chunk = 0;
//   while (messageLength_ = scanner_.yylex ())
  do
  {
    scanned_chunk = HTTP_Bisector_lex (context_);
//    scanned_chunk = IRC_Bisectlex (context_);
    switch (scanned_chunk)
    {
      case 0:
      {
        // --> finished scanning this buffer

        // remember how much data was scanned so far...
        messageLength_ += scanned_bytes;

        // no more data at this time...
        finished_scanning = true; // done

        break;
      }
      case -1:
      {
        // found a frame border scanned_bytes bytes into the buffer

        // *IMPORTANT NOTE*: if scanned_bytes == 0, then it's the corner
        //                   case where the current buffer starts with either:
        //                   - a '\n'
        //                   - a "\r\n"
        // *NOTE*: in either case, a new frame has been found...
        if ((scanned_bytes == 0) &&
            (*message_block_p->rd_ptr () == '\n'))
        {
          scanned_bytes = 1;
          //messageLength_ += HTTP_FRAME_BOUNDARY_SIZE;
        } // end IF
        else
        {
          //scanned_bytes += HTTP_FRAME_BOUNDARY_SIZE;
          messageLength_ += scanned_bytes;
        } // end IF

//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("buffer (ID: %u, length: %u): frame boundary [#%u] @ offset %u\n\"%s\"\n"),
//                     buffer_->getID (),
//                     messageLength_,
//                     currentNumFrames,
//                     (scanned_bytes + (buffer_->rd_ptr () - buffer_->base ())),
//                     ACE_TEXT (std::string (buffer_->rd_ptr (), scanned_bytes).c_str ())));

        ACE_Message_Block* message_p = buffer_;
        if (messageLength_ < buffer_->total_length ())
        {
          // more data to scan...

          // *NOTE*: shallow-copy the current data block
          buffer_ = message_block_p->duplicate ();
          if (!buffer_)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IRC_Message::duplicate(): \"%m\", returning\n")));

            // clean up
            buffer_ = message_p;

            return;
          } // end IF
          // adjust wr_ptr (point to one-past-the-end of the current message)
          message_block_p->wr_ptr (message_block_p->rd_ptr () + scanned_bytes);
          ACE_ASSERT (messageLength_ == message_p->total_length ());
          // adjust rd_ptr (point to the beginning of the next message)
          buffer_->rd_ptr (scanned_bytes);
          message_block_p = buffer_; // move to next message
        } // end IF
        else
        {
          // no more data to scan...
          ACE_ASSERT (messageLength_ == message_p->total_length ());

          buffer_ = NULL;

          finished_scanning = true; // done
        } // end ELSE

//        if (inherited::configuration_.crunchMessages)
//          message_block_p->crunch ();

//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("processing message (ID: %u - %u byte(s))...\n"),
//                    message_block_p->getID (),
//                    message_block_p->total_length ()));

        // --> push it downstream...
        result = inherited::put_next (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

          // clean up
          message_p->release ();
        } // end IF

        // reset state
        messageLength_ = 0;
        scanned_bytes = 0;

        // ...continue scanning ?
        break;
      }
      default:
      {
        // scanned one/some character(s)...
        scanned_bytes += scanned_chunk;

        break;
      }
    } // end SWITCH
  } while (!finished_scanning);

  // clean up
  scan_end ();

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("found %u frame bound(s)...\n"),
//               currentNumFrames));
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
          typename TimerManagerType>
void
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
        message_inout->get ();
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          session_data_container_r.get ();
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
        session_data_r.sessionID;

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
          typename TimerManagerType>
bool
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step1: initialize info container POD
  data_out.bytes = 0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timestamp = COMMON_TIME_NOW;

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
          typename TimerManagerType>
bool
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::scan_begin (char* data_in,
                                                      size_t length_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (!bufferState_);

  // create/initialize a new buffer state
//  // *IMPORTANT NOTE*: length_in IS already adjusted for two trailing \0's
  // *TODO*: cannot use yy_scan_buffer(), as it modifies the input data...
  //bufferState_ = IRC_IRCBisect__scan_buffer (data_in,
  //                                                         length_in,
  //                                                         context_);
  bufferState_ =
      HTTP_Bisector__scan_bytes (data_in,
                                 length_in,
                                 context_);
  if (!bufferState_)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to yy_scan_buffer(%@,%d), aborting\n"),
                ACE_TEXT ("failed to yy_scan_bytes(%@,%d), aborting\n"),
                data_in,
                length_in));
    return false;
  } // end IF

  // *NOTE*: contrary (!) to the documentation
  // (e.g. http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html),
  //         one still needs to yy_switch_to_buffer()
  HTTP_Bisector__switch_to_buffer (bufferState_,
                                   context_);

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
          typename TimerManagerType>
void
HTTP_Module_Bisector_T<ACE_SYNCH_USE,
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
                       TimerManagerType>::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Bisector_T::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  HTTP_Bisector__delete_buffer (bufferState_,
                                context_);
  bufferState_ = NULL;
}
