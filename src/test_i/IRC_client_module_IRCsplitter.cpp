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

#include "IRC_client_module_IRCsplitter.h"

#include "ace/OS_Memory.h"

#include "common_timer_manager_common.h"

#include "stream_iallocator.h"

#include "net_macros.h"

#include "IRC_client_defines.h"

IRC_Client_Module_IRCSplitter::IRC_Client_Module_IRCSplitter ()
 : inherited (false, // inactive by default
              false) // DON'T auto-start !
 , crunchMessages_ (false)
 , statisticCollectHandler_ (ACTION_COLLECT,
                             this,
                             false)
 , statisticCollectHandlerID_ (-1)
 , scannerContext_ (NULL)
 , currentBufferState_ (NULL)
 , currentNumFrames_ (0)
 , currentMessage_ (NULL)
 , currentBuffer_ (NULL)
 , currentMessageLength_ (0)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::IRC_Client_Module_IRCSplitter"));

  if (IRC_Client_IRCBisect_lex_init (&scannerContext_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init(): \"%m\", continuing\n")));

  // trace ?
  IRC_Client_IRCBisect_set_debug ((IRC_CLIENT_DEF_LEX_TRACE ? 1 : 0),
                                  scannerContext_);
}

IRC_Client_Module_IRCSplitter::~IRC_Client_Module_IRCSplitter ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::~IRC_Client_Module_IRCSplitter"));

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

  // fini scanner context
  if (scannerContext_)
    IRC_Client_IRCBisect_lex_destroy (scannerContext_);

  // clean up any unprocessed (chained) buffer(s)
  if (currentMessage_)
    currentMessage_->release ();
}

bool
IRC_Client_Module_IRCSplitter::initialize (Stream_IAllocator* allocator_in,
                                           bool isActive_in,
                                           Stream_State_t* state_in,
                                           /////////////////////////////////////
                                           bool crunchMessages_in,
                                           unsigned int statisticCollectionInterval_in,
                                           bool traceScanning_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::initialize"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // clean up
    crunchMessages_ = false;
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
    currentNumFrames_ = 0;
    if (currentBufferState_)
    {
      IRC_Client_IRCBisect__delete_buffer (currentBufferState_,
                                           scannerContext_);
      currentBufferState_ = NULL;
    } // end IF
    if (currentMessage_)
      currentMessage_->release ();
    currentMessage_ = NULL;
    currentBuffer_ = NULL;
    currentMessageLength_ = 0;
//     currentBufferIsResized_ = false;
    isInitialized_ = false;
  } // end IF

  // set base class initializer(s)
  inherited::allocator_ = allocator_in;
  inherited::isActive_ = isActive_in;
  inherited::state_ = state_in;

  crunchMessages_ = crunchMessages_in;

  if (statisticCollectionInterval_in)
  {
    // schedule regular statistics collection...
    ACE_Time_Value interval (statisticCollectionInterval_in, 0);
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
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
//                statisticCollectHandlerID_,
//                statisticCollectionInterval_in));
  } // end IF

  // *NOTE*: need to clean up timer beyond this point !

  //// initialize scanner context
  //if (IRCBisectlex_init_extra(&currentNumFrames_, // extra data
  //                            &scannerContext_))  // scanner context handle
  //{
  //  ACE_DEBUG((LM_ERROR,
  //             ACE_TEXT("failed to IRCBisectlex_init_extra(): \"%m\", aborting\n")));

  //  // clean up
  //  COMMON_TIMERMANAGER_SINGLETON::instance()->cancel (statisticCollectHandlerID_);
  //  statisticCollectHandlerID_ = 0;

  //  return false;
  //} // end IF
  IRC_Client_IRCBisect_set_debug ((traceScanning_in ? 1 : 0),
                                  scannerContext_);

  isInitialized_ = true;

  return true;
}

void
IRC_Client_Module_IRCSplitter::handleDataMessage (IRC_Client_Message*& message_inout,
                                                  bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::handleDataMessage"));

  int result = -1;

  // initialize return value(s), default behavior is to pass all messages along
  // --> don't want that !
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  // perhaps we already have part of this message ?
  if (currentBuffer_)
    currentBuffer_->cont (message_inout); // chain the buffer
  currentBuffer_ = message_inout;

  // scan the bytestream for frame bounds "\r\n"

  // do we know where to start ?
  if (!currentMessage_)
    currentMessage_ = currentBuffer_; // start scanning at offset 0...

  // *NOTE*: the scanner checks sequences of >= 2 bytes (.*\r\n)
  // --> make sure we have a minimum amount of data...
  // --> more sanity check(s)
  if (currentMessage_->total_length () < IRC_CLIENT_IRC_FRAME_BOUNDARY_SIZE)
    return; // don't have enough data, cannot proceed
  if (currentBuffer_->length () < IRC_CLIENT_IRC_FRAME_BOUNDARY_SIZE)
  {
    // *sigh*: OK, so this CAN actually happen...
    // case1: if we have anything OTHER than '\n', there's nothing to do
    //        --> wait for more data
    // case2: if we have an '\n' we have to check the trailing character
    //        of the PRECEDING buffer:
    //        - if it's an '\r' --> voilà, we've found a frame boundary
    //        - else            --> wait for more data
    if (((*currentBuffer_->rd_ptr ()) == '\n') &&
        (currentMessage_ != currentBuffer_))
    {
      ACE_Message_Block* preceding_buffer = currentMessage_;
      for (;
           preceding_buffer->cont () != currentBuffer_;
           preceding_buffer = preceding_buffer->cont ());
      if (*(preceding_buffer->rd_ptr () + (preceding_buffer->length () - 1)) == '\r')
      {
        // OK, message complete !
        if (crunchMessages_)
          currentMessage_->crunch ();

        // --> push it downstream...
        result = put_next (currentMessage_, NULL);
        if (result == -1)
        {
          ACE_DEBUG((LM_ERROR,
                     ACE_TEXT("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

          // clean up
          currentMessage_->release ();
        } // end IF

        // bye bye...
        currentMessageLength_ = 0;
        currentMessage_ = NULL;
        currentBuffer_ = NULL;

        return;
      } // end IF
    } // end IF

    return; // don't have enough data, cannot proceed
  } // end IF

  // OK, initialize scanner...

  // *WARNING*: cannot use yy_scan_buffer(), as flex modifies the data... :-(
//   // *NOTE*: in order to accomodate flex, the buffer needs two trailing
//   // '\0' characters...
//   // --> make sure it has this capacity
//   if (myCurrentBuffer->space() < IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE)
//   {
//     // *sigh*: (try to) resize it then...
//     if (myCurrentBuffer->size(myCurrentBuffer->size() + IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE))
//     {
//       ACE_DEBUG((LM_ERROR,
//                  ACE_TEXT("failed to ACE_Message_Block::size(%u), aborting\n"),
//                  (myCurrentBuffer->size() + IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE)));
//
//       // what else can we do ?
//       return;
//     } // end IF
//     myCurrentBufferIsResized = true;
//
//     // *WARNING*: beyond this point, make sure we resize the buffer back
//     // to its original length...
//     // *NOTE*: this is safe, as realloc() just crops the trailing bytes again...
//   } // end IF
// //   for (int i = 0;
// //        i < IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE;
// //        i++)
// //     *(myCurrentBuffer->wr_ptr() + i) = YY_END_OF_BUFFER_CHAR;
//   *(myCurrentBuffer->wr_ptr()) = '\0';
//   *(myCurrentBuffer->wr_ptr() + 1) = '\0';
//
//   if (!scan_begin(myCurrentBuffer->rd_ptr(),
//                   myCurrentBuffer->length() + IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE))
  if (!scan_begin (currentBuffer_->rd_ptr (),
                   currentBuffer_->length ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to scan_begin(%@, %u), aborting\n"),
                currentBuffer_->rd_ptr (),
                currentBuffer_->length ()));

//     // clean up
//     if (currentBufferIsResized_)
//     {
//       if (currentBuffer_->size (currentBuffer_->size () - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE))
//         ACE_DEBUG ((LM_ERROR,
//                     ACE_TEXT ("failed to ACE_Message_Block::size(%u), continuing\n"),
//                     (currentBuffer_->size () - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE)));
//       currentBufferIsResized_ = false;
//     } // end IF

    return;
  } // end IF

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: scanning %u byte(s)\n\"%s\"\n"),
//               currentBuffer_->getID (),
//               currentBuffer_->length (),
//               ACE_TEXT (std::string (currentBuffer->rd_ptr (), currentBuffer_->length ()).c_str ())));

  // scan it !
  currentNumFrames_ = 0;
  bool finished_scanning = false;
  int scanned_bytes = 0;
  int scanned_chunk = 0;
//   while (currentMessageLength_ = scanner_.yylex ())
  do
  {
    scanned_chunk = IRC_Client_IRCBisect_lex (scannerContext_);
//    scanned_chunk = IRCBisectlex (scannerContext_);
    switch (scanned_chunk)
    {
      case 0:
      {
        // --> finished scanning this buffer
        finished_scanning = true; // no (more) frame boundaries found

        // remember how much data was scanned so far...
        currentMessageLength_ += scanned_bytes;

        break;
      }
      case -1:
      {
        // found a frame border scanned_bytes bytes into the buffer

        // *NOTE*: if scanned_bytes == 0, then it's the corner
        // case where the current buffer starts with either:
        // - a '\n'
        // - a "\r\n"
        // *NOTE*: in either case, a new frame has been found...
        if ((scanned_bytes == 0) &&
            (*currentBuffer_->rd_ptr () == '\n'))
        {
          scanned_bytes = 1;
          currentMessageLength_ += IRC_CLIENT_IRC_FRAME_BOUNDARY_SIZE;
        } // end IF
        else
        {
          scanned_bytes += IRC_CLIENT_IRC_FRAME_BOUNDARY_SIZE;
          currentMessageLength_ += scanned_bytes;
        } // end IF

//         ACE_DEBUG((LM_DEBUG,
//                    ACE_TEXT("buffer (ID: %u, length: %u): frame boundary [#%u] @ offset %u\n\"%s\"\n"),
//                    myCurrentBuffer->getID(),
//                    myCurrentMessageLength,
//                    myCurrentNumFrames,
//                    (scanned_bytes + (myCurrentBuffer->rd_ptr() - myCurrentBuffer->base())),
//                    std::string(myCurrentBuffer->rd_ptr(), scanned_bytes).c_str()));

        IRC_Client_Message* message_p = currentMessage_;
        if (currentMessageLength_ < currentMessage_->total_length ())
        {
          // more data to scan...

          // *NOTE*: copy ctor shallow-copies the current data block
          currentMessage_ =
              dynamic_cast<IRC_Client_Message*> (currentBuffer_->duplicate ());
          ACE_ASSERT (currentMessage_);
          // adjust wr_ptr (point to one-past-the-end of the current message)
          currentBuffer_->wr_ptr (currentBuffer_->rd_ptr () + scanned_bytes);
          ACE_ASSERT (currentMessageLength_ == message_p->total_length ());
          // adjust rd_ptr (point to the beginning of the next message)
          currentMessage_->rd_ptr (scanned_bytes);
        } // end IF
        else
        {
          // NO more data to scan...
          ACE_ASSERT (currentMessageLength_ == currentMessage_->total_length ());

          // set new message head
          currentMessage_ = NULL;
        } // end ELSE

        if (crunchMessages_)
          message_p->crunch ();

//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("processing message (ID: %u - %u byte(s))...\n"),
//                     message_p->getID (),
//                     message_p->total_length ()));

        // --> push it downstream...
        result = put_next (message_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

          // clean up
          message_p->release ();
        } // end IF

        // set new current buffer
        currentBuffer_ = currentMessage_;
        // reset state
        currentMessageLength_ = 0;
        scanned_bytes = 0;

        // ...continue scanning !
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
//   // *NOTE*: that even if we've sent some frames downstream in the meantime,
//   // we're still referencing the same buffer we resized earlier - it's always
//   // the new "head" message...
//   if (myCurrentBufferIsResized)
//   {
//     if (myCurrentBuffer->size(myCurrentBuffer->size() - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE))
//       ACE_DEBUG((LM_ERROR,
//                  ACE_TEXT("failed to ACE_Message_Block::size(%u), continuing\n"),
//                  (myCurrentBuffer->size() - IRC_CLIENT_FLEX_BUFFER_BOUNDARY_SIZE)));
//     myCurrentBufferIsResized = false;
//   } // end IF

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("found %u frame bound(s)...\n"),
//              myCurrentNumFrames));
}

void
IRC_Client_Module_IRCSplitter::handleSessionMessage (IRC_Client_SessionMessage*& message_inout,
                                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::handleSessionMessage"));

  // don't care (implies yes per default, if we're part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->getType ())
  {
    case SESSION_BEGIN:
    {
      // remember session ID for reporting...
      ACE_ASSERT (inherited::state_);
      inherited::state_->sessionID = message_inout->getState ()->sessionID;

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

bool
IRC_Client_Module_IRCSplitter::collect (IRC_Client_RuntimeStatistic& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::collect"));

  // just a dummy...
  ACE_UNUSED_ARG (data_out);

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step1: init info container POD
  ACE_OS::memset (&data_out, 0, sizeof (IRC_Client_RuntimeStatistic));

  // *IMPORTANT NOTE*: information is collected by the statistic module
  //                   (if any)

  // step1: send the container downstream
  if (!putStatisticsMessage (data_out,               // data container
                             COMMON_TIME_POLICY ())) // timestamp
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putSessionMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

void
IRC_Client_Module_IRCSplitter::report () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::report"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // *TODO*: support (local) reporting here as well ?
  // --> leave this to any downstream modules...
  ACE_ASSERT (false);
}

bool
IRC_Client_Module_IRCSplitter::putStatisticsMessage (const IRC_Client_RuntimeStatistic& info_in,
                                                     const ACE_Time_Value& collectionTime_in) const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::putStatisticsMessage"));

  // step1: initialize session data
  IRC_Client_SessionData* data_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    IRC_Client_SessionData);
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return false;
  } // end IF
  //ACE_OS::memset (data_p, 0, sizeof (IRC_Client_SessionData));
  data_p->currentStatistics = info_in;
  data_p->lastCollectionTimestamp = collectionTime_in;

  // step2: allocate session data container
  IRC_Client_StreamSessionData_t* session_data_p = NULL;
  ACE_NEW_NORETURN (session_data_p,
                    IRC_Client_StreamSessionData_t (data_p,
                                                          true));
  if (!session_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

    // clean up
    delete data_p;

    return false;
  } // end IF

  // step3: send the data downstream...
  // *NOTE*: "fire-and-forget"-API for session_data_p
  ACE_ASSERT (inherited::state_);
  return inherited::putSessionMessage (SESSION_STATISTICS,
                                       session_data_p,
                                       inherited::allocator_);
}

bool
IRC_Client_Module_IRCSplitter::scan_begin (char* data_in,
                                           size_t length_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (!currentBufferState_);

  // create/init a new buffer state
  // *WARNING*: cannot use yy_scan_buffer(), as flex modifies the data... :-(
//   // *WARNING*: length_in IS already adjusted for two trailing \0's
//   currentState_ = yy_scan_buffer (data_in,
//                                   length_in,
//                                   scannerContext_);
  currentBufferState_ =
      IRC_Client_IRCBisect__scan_bytes (data_in,
                                        length_in,
                                        scannerContext_);
  if (!currentBufferState_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yy_scan_bytes(%@,%d), aborting\n"),
                data_in,
                length_in));
    return false;
  } // end IF

  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
  IRC_Client_IRCBisect__switch_to_buffer (currentBufferState_,
                                          scannerContext_);

  return true;
}

void
IRC_Client_Module_IRCSplitter::scan_end ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Module_IRCSplitter::scan_end"));

  // sanity check(s)
  ACE_ASSERT (currentBufferState_);

  // clean state
  IRC_Client_IRCBisect__delete_buffer (currentBufferState_,
                                       scannerContext_);
  currentBufferState_ = NULL;
}
