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

// *WORKAROUND*
#include <iostream>
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary header(s) manually (see above),
//                       and prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "bittorrent_client_inputhandler.h"

#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/Proactor.h>
#include <ace/Reactor.h>

#include "net_macros.h"

#include "bittorrent_client_network.h"

BitTorrent_Client_InputHandler::BitTorrent_Client_InputHandler (BitTorrent_Client_SessionState* state_in,
                                                                bool useReactor_in)
 : inherited (NULL,                           // reactor
              ACE_Event_Handler::LO_PRIORITY) // priority
 , configuration_ ()
 , currentReadBuffer_ (NULL)
 , registered_ (false)
 , state_ (state_in)
 , useReactor_ (useReactor_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_InputHandler::BitTorrent_Client_InputHandler"));

}

BitTorrent_Client_InputHandler::~BitTorrent_Client_InputHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_InputHandler::~BitTorrent_Client_InputHandler"));

  int result = -1;

  if (currentReadBuffer_)
    currentReadBuffer_->release ();

  if (registered_)
  {
    ACE_HANDLE handle = inherited::get_handle ();
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::handle_close(%u): \"%m\", continuing\n"),
                  handle));
  } // end IF
}

bool
BitTorrent_Client_InputHandler::initialize (const BitTorrent_Client_InputHandlerConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_InputHandler::initialize"));

  int result = 0;

  configuration_ = configuration_in;

  // sanity check(s)
  ACE_ASSERT (state_);

  if (useReactor_)
  {
    ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
    ACE_ASSERT (thread_manager_p);
    ACE_Reactor* reactor_p = ACE_Reactor::instance ();
    ACE_ASSERT (reactor_p);
    result = ACE_Event_Handler::register_stdin_handler (this,
                                                        reactor_p,
                                                        thread_manager_p,
                                                        THR_DETACHED);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::register_stdin_handler(): \"%m\", aborting\n")));
    else
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("started input handler...\n")));
      registered_ = true;
    } // end ELSE
  } // end IF

  return (result != -1);
}

int
BitTorrent_Client_InputHandler::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_InputHandler::handle_input"));

  int result = -1;

  if (!currentReadBuffer_)
  { // allocate a message buffer
    currentReadBuffer_ =
      allocateMessage (configuration_.streamConfiguration->bufferSize);
    if (!currentReadBuffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                  configuration_.streamConfiguration->bufferSize));
      return -1;
    } // end IF
  } // end IF
  ACE_ASSERT (currentReadBuffer_);

  // read some data from STDIN
  ssize_t bytes_received = ACE_OS::read (handle_in,
                                         currentReadBuffer_->wr_ptr (),
                                         currentReadBuffer_->size () - 1); // \0
  switch (bytes_received)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - STDIN closed (application shutdown)
      int error = ACE_OS::last_error ();
      ACE_UNUSED_ARG (error);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::read(%d): \"%m\", aborting\n"),
                  ACE_STDIN));

      // clean up
      currentReadBuffer_->release ();
      currentReadBuffer_ = NULL;

      return -1;
    }
    // *** GOOD CASES ***
    case 0:
    {
      //       ACE_DEBUG ((LM_DEBUG,
      //                   ACE_TEXT ("[%u]: STDIN was closed...\n"),
      //                   handle_in));

      // clean up
      currentReadBuffer_->release ();
      currentReadBuffer_ = NULL;

      return -1;
    }
    default:
    {
      //       ACE_DEBUG ((LM_DEBUG,
      //                   ACE_TEXT ("[%u]: received %u bytes...\n"),
      //                   handle_in,
      //                   bytes_received));

      // adjust write pointer
      currentReadBuffer_->wr_ptr (bytes_received);

      break;
    }
  } // end SWITCH
  //*currentReadBuffer_->wr_ptr () = 0; // 0-terminate string

  //// locate linefeed
  //char* result_2 =
  //  ACE_OS::strchr (currentReadBuffer_->rd_ptr (), '\n');
  //if (!result_2)
  //  return 0; // done

  //// compute length
  //unsigned int length = result_2 - currentReadBuffer_->rd_ptr () + 1;

  // copy message data, remove trailing [CR]LF
  std::string message_text (currentReadBuffer_->rd_ptr (),
                            currentReadBuffer_->length ());
  // *NOTE*:
  // Win32: \r\n
  // UNIX:  \n
  // Apple: \r
  static std::string newline = ACE_TEXT_ALWAYS_CHAR ("\r\n");
  size_t found = message_text.find_last_not_of (newline);
  if (found != std::string::npos)
    message_text.erase (found + 1);
  else
    message_text.clear (); // all newline

  // send the message
  ACE_ASSERT (configuration_.controller);
  string_list_t receivers;
  ACE_ASSERT (state_);
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (state_->lock);

    // sanity check (s)
    if (state_->channel.empty ())
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("not in a channel, returning\n")));
      goto done;
    } // end IF

    receivers.push_front (state_->channel);
  } // end lock scope
  try
  {
    configuration_.controller->send (receivers, message_text);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_Client_IIRCControl::send(), aborting\n")));
    return -1;
  }

done:
  // crunch buffer
  currentReadBuffer_->rd_ptr (currentReadBuffer_->length ());
  result = currentReadBuffer_->crunch ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::crunch(): \"%m\", aborting\n")));

  return result;
}

int
BitTorrent_Client_InputHandler::handle_close (ACE_HANDLE handle_in,
                                              ACE_Reactor_Mask closeMask_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_InputHandler::handle_close"));

  ACE_UNUSED_ARG (closeMask_in);

  int result = 0;

  // avoid re-iteration in dtor...
  registered_ = false;

#if defined (ACE_WIN32)
  // *NOTE*: called from dedicated thread context, see ACE_Event_Handler.cpp
  delete this;
#else
  if (useReactor_)
  {
    ACE_Reactor* reactor_p = ACE_Reactor::instance ();
    ACE_ASSERT (reactor_p);
    result = ACE_Event_Handler::remove_stdin_handler (reactor_p,
                                                      NULL);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::remove_stdin_handler(): \"%m\", aborting\n")));
  } // end IF
#endif

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stopped input handler...\n")));

  return result;
}

ACE_Message_Block*
BitTorrent_Client_InputHandler::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_InputHandler::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (configuration_.streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      message_block_p =
        static_cast<ACE_Message_Block*> (configuration_.streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !configuration_.streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      ACE_Message_Block (requestedSize_in,
                                         ACE_Message_Block::MB_DATA,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                         ACE_Time_Value::zero,
                                         ACE_Time_Value::max_time,
                                         NULL,
                                         NULL));
  if (!message_block_p)
  {
    if (configuration_.streamConfiguration->messageAllocator)
    {
      if (configuration_.streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}
