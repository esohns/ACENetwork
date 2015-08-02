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

#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/OS_Memory.h"
#include "ace/Proactor.h"

#include "common_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename ConfigurationType>
Net_AsynchTCPSocketHandler_T<ConfigurationType>::Net_AsynchTCPSocketHandler_T ()
 : inherited ()
 , inherited2 ()
 , inherited3 (NULL,                          // event handler handle
               ACE_Event_Handler::WRITE_MASK) // mask
 , inputStream_ ()
 , outputStream_ ()
 , localSAP_ ()
 , remoteSAP_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::Net_AsynchTCPSocketHandler_T"));

}

template <typename ConfigurationType>
Net_AsynchTCPSocketHandler_T<ConfigurationType>::~Net_AsynchTCPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::~Net_AsynchTCPSocketHandler_T"));

}

template <typename ConfigurationType>
void
Net_AsynchTCPSocketHandler_T<ConfigurationType>::open (ACE_HANDLE handle_in,
                                                       ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::open"));

  int result = -1;
  int error = 0;
  ACE_Proactor* proactor_p = NULL;

  // sanity checks
  ACE_ASSERT (inherited::configuration_.socketConfiguration);

  // step1: tweak socket
  // *TODO*: remove type inference
  if (inherited::configuration_.socketConfiguration->bufferSize)
    if (!Net_Common_Tools::setSocketBuffer (handle_in,
                                            SO_RCVBUF,
                                            inherited::configuration_.socketConfiguration->bufferSize))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%u) (handle was: %d), aborting\n"),
                  inherited::configuration_.socketConfiguration->bufferSize,
                  handle_in));
      goto close;
    } // end IF
  if (!Net_Common_Tools::setNoDelay (handle_in,
                                     NET_SOCKET_DEFAULT_TCP_NODELAY))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(%s) (handle was: %d), aborting\n"),
                (NET_SOCKET_DEFAULT_TCP_NODELAY ? ACE_TEXT ("true")
                                                : ACE_TEXT ("false")),
                handle_in));
    goto close;
  } // end IF
  if (!Net_Common_Tools::setKeepAlive (handle_in,
                                       NET_SOCKET_DEFAULT_TCP_KEEPALIVE))
  {
    error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: %d), aborting\n"),
                  (NET_SOCKET_DEFAULT_TCP_KEEPALIVE ? ACE_TEXT ("true")
                                                    : ACE_TEXT ("false")),
                  handle_in));
    goto close;
  } // end IF
  if (!Net_Common_Tools::setLinger (handle_in,
                                    inherited::configuration_.socketConfiguration->linger,
                                    -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s, -1) (handle was: %d), aborting\n"),
                  (inherited::configuration_.socketConfiguration->linger ? ACE_TEXT ("true")
                                                                         : ACE_TEXT ("false")),
                  handle_in));
    goto close;
  } // end IF

  // step2: initialize i/o streams
  proactor_p = ACE_Proactor::instance ();
  ACE_ASSERT (proactor_p);
  inherited2::proactor (proactor_p);
  result = inputStream_.open (*this,       // event handler
                              handle_in,   // handle
                              NULL,        // completion key
                              proactor_p); // proactor
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::open(%d): \"%m\", aborting\n"),
                handle_in));
    goto close;
  } // end IF
  result = outputStream_.open (*this,       // event handler
                               handle_in,   // handle
                               NULL,        // completion key
                               proactor_p); // proactor
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Stream::open(%d): \"%m\", aborting\n"),
                handle_in));
    goto close;
  } // end IF

  return;

close:
  result = handle_close (handle_in,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_close(): \"%m\", continuing\n")));
}

template <typename ConfigurationType>
void
Net_AsynchTCPSocketHandler_T<ConfigurationType>::addresses (const ACE_INET_Addr& remoteAddress_in,
                                                            const ACE_INET_Addr& localAddress_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::addresses"));

  localSAP_ = localAddress_in;
  remoteSAP_ = remoteAddress_in;
}

//template <typename ConfigurationType>
//void
//Net_AsynchTCPSocketHandler_T<ConfigurationType>::act (const void* act_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::act"));
//
//  const ConfigurationType* configuration_p =
//    reinterpret_cast<const ConfigurationType*> (act_in);
//  ACE_ASSERT (configuration_p);
//  inherited::initialize (*configuration_p);
//}

template <typename ConfigurationType>
int
Net_AsynchTCPSocketHandler_T<ConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                               ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);
  ACE_UNUSED_ARG (mask_in);

  int result = -1;
  result = inputStream_.cancel ();
//  if ((result != 0) && (result != 1))
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\" (result was: %d), continuing\n"),
                result));
  int result_2 = outputStream_.cancel ();
//  if ((result_2 != 0) && (result_2 != 1))
  if (result_2 == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Stream::cancel(): \"%m\" (result was: %d), continuing\n"),
                result));

//  return ((((result != 0) && (result != 1)) ||
//           ((result_2 != 0) && (result_2 != 1))) ? -1
//                                                 : 0);
  return ((result == -1) || (result_2 == -1) ? -1
                                             : 0);
}

template <typename ConfigurationType>
int
Net_AsynchTCPSocketHandler_T<ConfigurationType>::notify (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::notify"));

  int result = -1;

  ACE_HANDLE handle = inherited2::handle ();
  try
  {
    result = handle_output (handle);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchTCPSocketHandler_T::handle_output(), continuing\n")));
    result = -1;
  }
  if (result == -1)
  {
    // *IMPORTANT NOTE*: socket may have closed in the meantime...
    int error = ACE_OS::last_error ();
    if ((error != ENOTSOCK)   && // 10038, happens on Win32
        (error != ECONNRESET) && // 10054, happens on Win32
        (error != ENOTCONN))     // 10057, happens on Win32
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_output(): \"%m\", aborting\n")));
  } // end IF

  return result;
}
template <typename ConfigurationType>
int
Net_AsynchTCPSocketHandler_T<ConfigurationType>::notify (ACE_Event_Handler* handler_in,
                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::notify"));

  ACE_UNUSED_ARG (handler_in);
  ACE_UNUSED_ARG (mask_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <typename ConfigurationType>
ACE_Message_Block*
Net_AsynchTCPSocketHandler_T<ConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::allocateMessage"));

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

template <typename ConfigurationType>
void
Net_AsynchTCPSocketHandler_T<ConfigurationType>::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::handle_write_stream"));

  int result = -1;
  size_t bytes_transferred = result_in.bytes_transferred ();
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection closed/reset (by peer) ? --> not an error
    error = result_in.error ();
    if ((error != ECONNRESET) &&
        (error != EPIPE)      &&
        (error != EBADF)) // 9 happens on Linux (local close())
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (%d): \"%s\", continuing\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE::sock_error (error))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (%d): \"%s\", continuing\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE_OS::strerror (error))));
#endif
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to write to output stream (%d): \"%s\", continuing\n"),
                    result_in.handle (),
                    ACE_TEXT (ACE::sock_error (error))));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to write to output stream (%d): \"%s\", continuing\n"),
                    result_in.handle (),
                    ACE_TEXT (ACE_OS::strerror (error))));
#endif

      break;
    }
    // *** GOOD CASES ***
    default:
    {
      // short write ?
      // *TODO*: handle short writes (more) gracefully
      if (result_in.bytes_to_write () != bytes_transferred)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("stream (%d): sent %u/%u byte(s) only, continuing\n"),
                    result_in.handle (),
                    bytes_transferred,
                    result_in.bytes_to_write ()));

      break;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ().release ();
}

template <typename ConfigurationType>
bool
Net_AsynchTCPSocketHandler_T<ConfigurationType>::initiate_read_stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::initiate_read_stream"));

  int result = -1;

  // allocate a data buffer
  ACE_Message_Block* message_block_p =
      allocateMessage (inherited::configuration_.bufferSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::allocateMessage(%u), aborting\n"),
                inherited::configuration_.bufferSize));
    return false;
  } // end IF

  // start (asynchronous) read...
  result = inputStream_.read (*message_block_p,                     // buffer
                              message_block_p->size (),             // bytes to read
                              NULL,                                 // ACT
                              0,                                    // priority
                              COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENXIO)               && // happens on Win32
        (error != EFAULT)              && // *TODO*: happens on Win32
        (error != ERROR_UNEXP_NET_ERR) && // *TODO*: happens on Win32
        (error != ERROR_NETNAME_DELETED)) // happens on Win32
#else
    if (error)
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Read_Stream::read(%u): \"%m\", aborting\n"),
                  message_block_p->size ()));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  return true;
}
