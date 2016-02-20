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
#include "common_tools.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename ConfigurationType>
Net_AsynchUDPSocketHandler_T<ConfigurationType>::Net_AsynchUDPSocketHandler_T ()
 : inherited ()
 , inherited2 ()
 , inherited3 (NULL,                          // event handler handle
               ACE_Event_Handler::WRITE_MASK) // mask
 , counter_ (0) // initial count
 , inputStream_ ()
 , outputStream_ ()
 , writeOnly_ (false)
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

  ACE_UNUSED_ARG (messageBlock_in);

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->socketConfiguration);

  // step0: initialize base class
  ACE_Proactor* proactor_p = ACE_Proactor::instance ();
  ACE_ASSERT (proactor_p);
  inherited2::proactor (proactor_p);
  if (handle_in != ACE_INVALID_HANDLE)
    inherited2::handle (handle_in);

  writeOnly_ = inherited::configuration_->socketConfiguration->writeOnly;
  if (!writeOnly_)
  {
    // step1: tweak socket
    if (inherited::configuration_->socketConfiguration->bufferSize)
      if (!Net_Common_Tools::setSocketBuffer (handle_in,
                                              SO_RCVBUF,
                                              inherited::configuration_->socketConfiguration->bufferSize))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                    handle_in,
                    inherited::configuration_->socketConfiguration->bufferSize));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), continuing\n"),
                    handle_in,
                    inherited::configuration_->socketConfiguration->bufferSize));
#endif
      } // end IF

    // *PORTABILITY*: (currently,) MS Windows (TM) UDP sockets do not support
    //                SO_LINGER
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (!Net_Common_Tools::setLinger (handle_in,
                                      inherited::configuration_->socketConfiguration->linger,
                                      -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%d,%s,-1), aborting\n"),
                  handle_in,
                  (inherited::configuration_->socketConfiguration->linger ? ACE_TEXT ("true")
                                                                          : ACE_TEXT ("false"))));
      return;
    } // end IF
#endif

    // step2: initialize input stream
    result = inputStream_.open (*this,
                                handle_in,
                                NULL,
                                proactor_p);
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize input stream (handle was: 0x%@), aborting\n"),
                  handle_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize input stream (handle was: %d), aborting\n"),
                  handle_in));
#endif
      return;
    } // end IF
  } // end IF

  // step3: initialize output stream
  result = outputStream_.open (*this,
                               handle_in,
                               NULL,
                               proactor_p);
  if (result == -1)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize output stream (handle was: 0x%@), aborting\n"),
                  handle_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize output stream (handle was: %d), aborting\n"),
                  handle_in));
#endif

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

  ACE_UNUSED_ARG (remoteAddress_in);
  ACE_UNUSED_ARG (localAddress_in);

//  localSAP_ = localAddress_in;
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
  if (!writeOnly_)
  {
    result = inputStream_.cancel ();
    if (result)
    {
      int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (error != ERROR_IO_PENDING) // 997: happens on Win32
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Asynch_Read_Dgram::cancel(): \"%m\" (errno was: %d), continuing\n"),
                    error));
    } // end IF
  } // end IF
  else
    result = 0;
  int result_2 = outputStream_.cancel ();
  if ((result_2 != 0) && (result_2 != 1))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ERROR_IO_PENDING) // 997: happens on Win32
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::cancel(): \"%m\" (errno was: %d), continuing\n"),
                  error));
  } // end IF

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
  ACE_HANDLE handle = inherited2::handle ();

  try
  {
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
  }
  catch (...)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchUDPSocketHandler_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                handle,
                ACE_Event_Handler::ALL_EVENTS_MASK));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchUDPSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                handle,
                ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
    result = -1;
  }
  if (result == -1)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                handle,
                ACE_Event_Handler::ALL_EVENTS_MASK));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                handle,
                ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
  } // end IF
}

template <typename ConfigurationType>
int
Net_AsynchUDPSocketHandler_T<ConfigurationType>::notify (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::notify"));

  int result = -1;
  ACE_HANDLE handle = inherited2::handle ();

  try
  {
    result = handle_output (handle);
  }
  catch (...)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchUDPSocketHandler_T::handle_output(0x%@): \"%m\", continuing\n"),
                handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_AsynchUDPSocketHandler_T::handle_output(%d): \"%m\", continuing\n"),
                handle));
#endif
    result = -1;
  }
  if (result == -1)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_output(0x%@): \"%m\", continuing\n").
                handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_output(%d): \"%m\", continuing\n"),
                handle));
#endif

    int result_2 = handle_close (handle,
                                 ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result_2 == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  handle,
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle,
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
    } // end IF
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

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <typename ConfigurationType>
bool
Net_AsynchUDPSocketHandler_T<ConfigurationType>::initiate_read_dgram ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::initiate_read_dgram"));

  ssize_t result = -1;
  size_t bytes_to_read = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // step1: allocate a data buffer
  ACE_Message_Block* message_block_p =
      allocateMessage (inherited::configuration_->PDUSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::allocateMessage(%u), aborting\n"),
                inherited::configuration_->PDUSize));
    return false;
  } // end IF

  // step2: start (asynchronous) read...
  bytes_to_read = message_block_p->size ();
  int error = 0;
receive:
  result =
    inputStream_.recv (message_block_p,                      // buffer
                       bytes_to_read,                        // #bytes to read
                       0,                                    // flags
                       ACE_PROTOCOL_FAMILY_INET,             // protocol family
                       NULL,                                 // ACT
                       0,                                    // priority
                       COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (result == -1)
  {
    error = ACE_OS::last_error ();
    // *WARNING*: this could fail on multi-threaded proactors
    if (error == EAGAIN) goto receive; // 11: happens on Linux
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENXIO)                && // happens on Win32
        (error != EFAULT)               && // *TODO*: happens on Win32
        (error != ERROR_UNEXP_NET_ERR)  && // *TODO*: happens on Win32
        (error != ERROR_NETNAME_DELETED))  // happens on Win32
#else
    if (error)
#endif
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Dgram::recv(%u): \"%m\", aborting\n"),
                bytes_to_read));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  return true;
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (handle was: 0x%@): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to write to output stream (handle was: %d): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE_OS::strerror (error))));
#endif

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to write to output stream (0x%@): \"%s\", aborting\n"),
                    result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to write to output stream (%d): \"%s\", aborting\n"),
                    result_in.handle (),
                    ACE_TEXT (ACE_OS::strerror (error))));
#endif

      close = true;

      break;
    }
    // *** GOOD CASES ***
    default:
    {
      // *TODO*: handle short writes (more) gracefully
      if (result_in.bytes_to_write () != bytes_transferred)
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("stream (0x%@): sent %u/%u byte(s) only, aborting\n"),
                    result_in.handle (),
                    bytes_transferred,
                    result_in.bytes_to_write ()));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("stream (%d): sent %u/%u byte(s) only, aborting\n"),
                    result_in.handle (),
                    bytes_transferred,
                    result_in.bytes_to_write ()));
#endif
        close = true;
      } // end IF

      break;
    }
  } // end SWITCH

  // clean up
  ACE_Message_Block* message_block_p = result_in.message_block ();
  ACE_ASSERT (message_block_p);
  message_block_p->release ();

  if (close)
  {
    result = handle_close (result_in.handle (),
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  result_in.handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchUDPSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                  result_in.handle (),
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
    } // end IF
  } // end IF

  counter_.decrease ();
}

template <typename ConfigurationType>
ACE_Message_Block*
Net_AsynchUDPSocketHandler_T<ConfigurationType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPSocketHandler_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  if (inherited::configuration_->messageAllocator)
  {
allocate:
    try
    {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited::configuration_->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited::configuration_->messageAllocator->block ())
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
    if (inherited::configuration_->messageAllocator)
    {
      if (inherited::configuration_->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}
