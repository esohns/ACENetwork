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

#include "ace/OS.h"
#include "ace/OS_Memory.h"
#include "ace/Proactor.h"

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename ConfigurationType>
Net_AsynchUDPSocketHandler_T<ConfigurationType>::Net_AsynchUDPSocketHandler_T ()
//: inherited ()
 : /*inherited2 ()
,*/ inherited3 (NULL,                          // event handler handle
                ACE_Event_Handler::WRITE_MASK) // mask
// , inputStream_ ()
// , outputStream_ ()
// , localSAP_ ()
// , remoteSAP_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::Net_AsynchUDPSocketHandler_T"));

}

template <typename ConfigurationType>
Net_AsynchUDPSocketHandler_T<ConfigurationType>::~Net_AsynchUDPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::~Net_AsynchUDPSocketHandler_T"));

}

template <typename ConfigurationType>
void
Net_AsynchUDPSocketHandler_T<ConfigurationType>::open (ACE_HANDLE handle_in,
                                                       ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::open"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);

  // step0: initialize baseclass
  inherited2::handle (handle_in);

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

      // clean up
      handle_close (handle_in,
                    ACE_Event_Handler::ALL_EVENTS_MASK);

      return;
    } // end IF
  if (!Net_Common_Tools::setLinger (handle_in,
                                    inherited::configuration_.socketConfiguration.linger,
                                    -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s, -1) (handle was: %d), aborting\n"),
                (inherited::configuration_.socketConfiguration.linger ? ACE_TEXT ("true")
                                                                      : ACE_TEXT ("false")),
                handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF

  // step2: init i/o streams
  ACE_Proactor* proactor_p = ACE_Proactor::instance ();
  ACE_ASSERT (proactor_p);
  inherited2::proactor (proactor_p);
  result = inputStream_.open (*this,
                              handle_in,
                              NULL,
                              proactor_p);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to init input stream (handle was: %d), aborting\n"),
                handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF
  result = outputStream_.open (*this,
                               handle_in,
                               NULL,
                               proactor_p);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to init output stream (handle was: %d), aborting\n"),
                handle_in));

    // clean up
    handle_close (handle_in,
                  ACE_Event_Handler::ALL_EVENTS_MASK);

    return;
  } // end IF
}

template <typename ConfigurationType>
void
Net_AsynchUDPSocketHandler_T<ConfigurationType>::addresses (const ACE_INET_Addr& remoteAddress_in,
                                                            const ACE_INET_Addr& localAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::addresses"));

  localSAP_ = localAddress_in;
//  remoteSAP_ = remoteAddress_in;
}

template <typename ConfigurationType>
int
Net_AsynchUDPSocketHandler_T<ConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                               ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);
  ACE_UNUSED_ARG (mask_in);

  int result = -1;

  // *BUG*: Posix_Proactor.cpp:1575 should read:
  //        int rc = ::aio_cancel (result->handle_, result);
  result = inputStream_.cancel ();
//  if ((result != 0) && (result != 1))
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Dgram::cancel(): \"%m\" (result was: %d), continuing\n"),
                result));
  int result_2 = outputStream_.cancel ();
//  if ((result_2 != 0) && (result_2 != 1))
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::cancel(): \"%m\" (result was: %d), continuing\n"),
                result));

//  return ((((result != 0) && (result != 1)) ||
//           ((result_2 != 0) && (result_2 != 1))) ? -1
//                                                 : 0);
  return ((result == -1) || (result_2 == -1) ? -1
                                             : 0);
}

template <typename ConfigurationType>
void
Net_AsynchUDPSocketHandler_T<ConfigurationType>::handle_wakeup ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::handle_wakeup"));

  int result = -1;

  try
  {
    result = handle_close (inherited2::handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchUDPSocketHandler_T::handle_close(): \"%m\", continuing\n")));
    result = -1;
  }
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(): \"%m\", continuing\n")));
}

template <typename ConfigurationType>
int
Net_AsynchUDPSocketHandler_T<ConfigurationType>::notify (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::notify"));

  int result = -1;

  try
  {
    result = handle_output (inherited2::handle ());
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchUDPSocketHandler_T::handle_output(): \"%m\", continuing\n")));
    result = -1;
  }
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_output(): \"%m\", aborting\n")));

    int result_2 = handle_close (inherited2::handle (),
                                 ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(): \"%m\", continuing\n")));
  } // end IF

  return result;
}
template <typename ConfigurationType>
int
Net_AsynchUDPSocketHandler_T<ConfigurationType>::notify (ACE_Event_Handler* handler_in,
                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::notify"));

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

template <typename ConfigurationType>
void
Net_AsynchUDPSocketHandler_T<ConfigurationType>::initiate_read_dgram ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::initiate_read_dgram"));

  int result = -1;
  size_t bytes_to_read = 0;

  // step1: allocate a data buffer
  ACE_Message_Block* message_block_p =
      allocateMessage (inherited::configuration_.bufferSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::allocateMessage(%u), aborting\n"),
                inherited::configuration_.bufferSize));
    goto close;
  } // end IF

  // step2: start (asynchronous) read...
  bytes_to_read = message_block_p->size ();
  result = inputStream_.recv (message_block_p,                      // buffer
                              bytes_to_read,                        // #bytes to read
                              0,                                    // flags
                              ACE_PROTOCOL_FAMILY_INET,             // protocol family
                              NULL,                                 // ACT
                              0,                                    // priority
                              COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Dgram::recv(%u): \"%m\", aborting\n"),
                message_block_p->size ()));

    // clean up
    message_block_p->release ();

    goto close;
  } // end IF

  return;

close:
  result = handle_close (inherited2::handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(): \"%m\", continuing\n")));
}

template <typename ConfigurationType>
void
Net_AsynchUDPSocketHandler_T<ConfigurationType>::handle_write_dgram (const ACE_Asynch_Write_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::handle_write_dgram"));

  int result = -1;
  bool close = false;
  size_t bytes_transferred = result_in.bytes_transferred ();
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result != 1)
  {
    // connection closed/reset (by peer) ? --> not an error
    error = result_in.error ();
    if ((error != ECONNRESET) &&
        (error != EPIPE)      &&
        (error != EBADF)) // 9 happens on Linux (local close())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (%d): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE_OS::strerror (error))));

    close = true;
  } // end IF

  switch (bytes_transferred)
  {
    case -1:
    case 0:
    {
      // connection closed/reset (by peer) ? --> not an error
      error = result_in.error ();
      if ((error != ECONNRESET) &&
          (error != EPIPE)      &&
          (error != EBADF)) // 9 happens on Linux (local close())
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
      // *TODO*: handle short writes (more) gracefully
      if (result_in.bytes_to_write () != bytes_transferred)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("stream (%d): sent %u/%u byte(s) only, aborting"),
                    result_in.handle (),
                    bytes_transferred,
                    result_in.bytes_to_write ()));

        close = true;
      } // end IF

      break;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ()->release ();

  if (close)
  {
    result = handle_close (inherited2::handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(): \"%m\", continuing\n")));
  } // end IF
}

template <typename ConfigurationType>
ACE_Message_Block*
Net_AsynchUDPSocketHandler_T<ConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited::configuration_.messageAllocator)
  {
allocate:
    try
    {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited::configuration_.messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited::configuration_.messageAllocator->block ())
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
    if (inherited::configuration_.messageAllocator)
    {
      if (inherited::configuration_.messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}
