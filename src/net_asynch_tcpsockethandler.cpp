/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#include "net_asynch_tcpsockethandler.h"

#include "ace/OS.h"
#include "ace/OS_Memory.h"
#include "ace/Proactor.h"

#include "stream_defines.h"

#include "net_defines.h"
#include "net_common_tools.h"
#include "net_macros.h"

Net_AsynchTCPSocketHandler::Net_AsynchTCPSocketHandler ()
 : inherited ()
 , inherited2 ()
 , inherited3 (NULL,                          // event handler handle
               ACE_Event_Handler::WRITE_MASK) // mask
 , inputStream_ ()
 , outputStream_ ()
 , localSAP_ ()
 , remoteSAP_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::Net_AsynchTCPSocketHandler"));

}

Net_AsynchTCPSocketHandler::~Net_AsynchTCPSocketHandler ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::~Net_AsynchTCPSocketHandler"));

}

void
Net_AsynchTCPSocketHandler::open (ACE_HANDLE handle_in,
                                  ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::open"));

  int result = -1;
  int error = 0;

  // step1: tweak socket
  if (inherited::configuration_.socketConfiguration.bufferSize)
    if (!Net_Common_Tools::setSocketBuffer (handle_in,
                                            SO_RCVBUF,
                                            inherited::configuration_.socketConfiguration.bufferSize))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%u) (handle was: %d), aborting\n"),
                  inherited::configuration_.socketConfiguration.bufferSize,
                  handle_in));

      goto close;
    } // end IF
  if (!Net_Common_Tools::setNoDelay (handle_in,
                                     NET_DEFAULT_SOCKET_TCP_NODELAY))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(%s) (handle was: %d), aborting\n"),
                (NET_DEFAULT_SOCKET_TCP_NODELAY ? ACE_TEXT ("true")
                                                : ACE_TEXT ("false")),
                handle_in));

    goto close;
  } // end IF
  if (!Net_Common_Tools::setKeepAlive (handle_in,
                                       NET_DEFAULT_SOCKET_TCP_KEEPALIVE))
  {
    error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: %d), aborting\n"),
                  (NET_DEFAULT_SOCKET_TCP_KEEPALIVE ? ACE_TEXT ("true")
                                                    : ACE_TEXT ("false")),
                  handle_in));

    goto close;
  } // end IF
  if (!Net_Common_Tools::setLinger (handle_in,
                                    NET_DEFAULT_SOCKET_LINGER))
  {
    error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s) (handle was: %d), aborting\n"),
                  ((NET_DEFAULT_SOCKET_LINGER > 0) ? ACE_TEXT ("true")
                                                   : ACE_TEXT ("false")),
                  handle_in));

    goto close;
  } // end IF

  // step2: initialize i/o streams
  inherited2::proactor (ACE_Proactor::instance ());
  result = inputStream_.open (*this,                    // event handler
                              handle_in,                // handle
                              NULL,                     // ACT
                              inherited2::proactor ()); // proactor
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::open(%d): \"%m\", aborting\n"),
                handle_in));

    goto close;
  } // end IF
  result = outputStream_.open (*this,                    // event handler
                               handle_in,                // handle
                               NULL,                     // ACT
                               inherited2::proactor ()); // proactor
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Stream::open(%d): \"%m\", returning\n"),
                handle_in));

    goto close;
  } // end IF

  return;

close:
  result = handle_close (handle_in,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::handle_close(): \"%m\", continuing\n")));
}

void
Net_AsynchTCPSocketHandler::addresses (const ACE_INET_Addr& remoteAddress_in,
                                       const ACE_INET_Addr& localAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::addresses"));

  localSAP_ = localAddress_in;
  remoteSAP_ = remoteAddress_in;
}

void
Net_AsynchTCPSocketHandler::act (const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::act"));

  const Net_SocketHandlerConfiguration_t* configuration_p =
    reinterpret_cast<const Net_SocketHandlerConfiguration_t*> (act_in);
  ACE_ASSERT (configuration_p);
  inherited::initialize (*configuration_p);
}

int
Net_AsynchTCPSocketHandler::handle_close (ACE_HANDLE handle_in,
                                          ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::handle_close"));

  ACE_UNUSED_ARG (mask_in);

  int result = -1;

  // clean up
  result = inputStream_.cancel ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
  result = outputStream_.cancel ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Stream::cancel(): \"%m\", continuing\n")));

  if (handle_in != ACE_INVALID_HANDLE)
  {
    result = ACE_OS::closesocket (handle_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                  handle_in));
  } // end IF

  return result;
}

int
Net_AsynchTCPSocketHandler::notify (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::notify"));

  int result = -1;

  try
  {
    result = handle_output (inherited2::handle ());
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchTCPSocketHandler::handle_output(), continuing")));

    result = -1;
  }
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::handle_output(): \"%m\", aborting\n")));

    int result_2 = handle_close (inherited2::handle (),
                                 ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::handle_close(): \"%m\", continuing\n")));
  } // end IF

  return result;
}
int
Net_AsynchTCPSocketHandler::notify (ACE_Event_Handler* handler_in,
                                    ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::notify"));

  ACE_UNUSED_ARG (handler_in);
  ACE_UNUSED_ARG (mask_in);

  // *NOTE*: should NEVER be reached !
  ACE_ASSERT (false);

#if defined (_MSC_VER)
  return -1;
#else
  ACE_NOTREACHED (return -1;)
#endif
}

ACE_Message_Block*
Net_AsynchTCPSocketHandler::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::allocateMessage"));

  // init return value(s)
  ACE_Message_Block* message_out = NULL;

  if (inherited::configuration_.messageAllocator)
    message_out =
        static_cast<ACE_Message_Block*> (inherited::configuration_.messageAllocator->malloc (requestedSize_in));
  else
    ACE_NEW_NORETURN (message_out,
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
  if (!message_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return message_out;
}

void
Net_AsynchTCPSocketHandler::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::handle_write_stream"));

  bool close = false;
  int result = -1;
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection reset (by peer) ? --> not an error
    error = result_in.error ();
    if ((error != ECONNRESET) &&
        (error != EPIPE))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (%d): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE_OS::strerror (error))));

    close = true;
  } // end IF

  switch (result_in.bytes_transferred ())
  {
    case -1:
    case 0:
    {
      // connection reset (by peer) ? --> not an error
      error = result_in.error ();
      if ((error != ECONNRESET) &&
          (error != EPIPE))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to write to output stream (%d): \"%s\", aborting\n"),
                    result_in.handle (),
                    ACE_TEXT (ACE_OS::strerror (error))));

      close = true;

      break;
    }
    // *** GOOD CASES ***
    default:
    {
      // short write ?
      if (result_in.bytes_to_write () != result_in.bytes_transferred ())
      {
        // *TODO*: handle short writes (more) gracefully
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("stream (%d): sent %u/%u byte(s) only, aborting"),
                    result_in.handle (),
                    result_in.bytes_transferred (),
                    result_in.bytes_to_write ()));

        close = true;
      } // end IF

      break;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ().release ();

  if (close)
  {
    result = handle_close (inherited2::handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::handle_close(): \"%m\", continuing\n")));
  } // end IF
}

void
Net_AsynchTCPSocketHandler::initiate_read_stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::initiate_read_stream"));

  int result = -1;

  // allocate a data buffer
  ACE_Message_Block* message_block_p = allocateMessage (STREAM_BUFFER_SIZE);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::allocateMessage(%u), returning\n"),
                STREAM_BUFFER_SIZE));

    // clean up
    result = handle_close (inherited2::handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::handle_close(): \"%m\", continuing\n")));

    return;
  } // end IF

  // start (asynchronous) read...
  result = inputStream_.read (*message_block_p,         // buffer
                              message_block_p->size (), // bytes to read
                              NULL,                     // ACT
                              0,                        // priority
                              ACE_SIGRTMIN);            // signal
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::read(%u): \"%m\", returning\n"),
                message_block_p->size ()));

    // clean up
    message_block_p->release ();
    result = handle_close (inherited2::handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::handle_close(): \"%m\", continuing\n")));
  } // end IF
}

//void
//Net_AsynchTCPSocketHandler::abort ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler::abort"));

//  handle_close (inherited::handle (),
//                ACE_Event_Handler::ALL_EVENTS_MASK);
//}
