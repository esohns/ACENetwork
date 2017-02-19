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

#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/OS_Memory.h>
#include <ace/Proactor.h>

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
 , counter_ (0) // initial count
 , inputStream_ ()
 , outputStream_ ()
 , partialWrite_ (false)
 , localSAP_ ()
 , remoteSAP_ ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , writeHandle_ (ACE_INVALID_HANDLE)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::Net_AsynchTCPSocketHandler_T"));

}

template <typename ConfigurationType>
Net_AsynchTCPSocketHandler_T<ConfigurationType>::~Net_AsynchTCPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::~Net_AsynchTCPSocketHandler_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  int result = -1;

  if (likely (writeHandle_ != ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("closing write handle (was: %d) in dtor --> check implementation !\n"),
                writeHandle_));
    result = ACE_OS::close (writeHandle_);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  writeHandle_));
  } // end IF
#endif

//  if (buffer_)
//    buffer_->release ();
}

template <typename ConfigurationType>
void
Net_AsynchTCPSocketHandler_T<ConfigurationType>::open (ACE_HANDLE handle_in,
                                                       ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::open"));

  ACE_UNUSED_ARG (messageBlock_in);

  int result = -1;
  int error = 0;
  ACE_Proactor* proactor_p = NULL;

  // sanity checks
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // step0: duplicate handle to support parallel i/o
  ACE_ASSERT (writeHandle_ == ACE_INVALID_HANDLE);
  writeHandle_ = ACE_OS::dup (handle_in);
  if (unlikely (writeHandle_ == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::dup(%d): \"%m\", aborting\n"),
                handle_in));
    goto close;
  } // end IF
#endif

  // step1: tweak socket
  // *TODO*: remove type inference
  if (likely (inherited::configuration_->socketConfiguration.bufferSize))
  {
    if (unlikely (!Net_Common_Tools::setSocketBuffer (handle_in,
                                                      SO_RCVBUF,
                                                      inherited::configuration_->socketConfiguration.bufferSize)))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                  handle_in,
                  inherited::configuration_->socketConfiguration.bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), continuing\n"),
                  handle_in,
                  inherited::configuration_->socketConfiguration.bufferSize));
#endif
    } // end IF
    if (unlikely (!Net_Common_Tools::setSocketBuffer (handle_in,
                                                      SO_SNDBUF,
                                                      inherited::configuration_->socketConfiguration.bufferSize)))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_SNDBUF,%u), continuing\n"),
                  handle_in,
                  inherited::configuration_->socketConfiguration.bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_SNDBUF,%u), continuing\n"),
                  handle_in,
                  inherited::configuration_->socketConfiguration.bufferSize));
#endif
    } // end IF
  } // end IF

  if (unlikely (!Net_Common_Tools::setNoDelay (handle_in,
                                               NET_SOCKET_DEFAULT_TCP_NODELAY)))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(0x%@,%s), aborting\n"),
                  handle_,
                  (NET_SOCKET_DEFAULT_TCP_NODELAY ? ACE_TEXT ("true")
                                                  : ACE_TEXT ("false"))));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(%d,%s), aborting\n"),
                handle_in,
                (NET_SOCKET_DEFAULT_TCP_NODELAY ? ACE_TEXT ("true")
                                                : ACE_TEXT ("false"))));
#endif
    goto close;
  } // end IF
  if (unlikely (!Net_Common_Tools::setKeepAlive (handle_in,
                                                 NET_SOCKET_DEFAULT_TCP_KEEPALIVE)))
  {
    error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(0x%@,%s), aborting\n"),
                  handle_in,
                  (NET_SOCKET_DEFAULT_TCP_KEEPALIVE ? ACE_TEXT ("true")
                                                    : ACE_TEXT ("false"))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%d,%s), aborting\n"),
                  handle_in,
                  (NET_SOCKET_DEFAULT_TCP_KEEPALIVE ? ACE_TEXT ("true")
                                                    : ACE_TEXT ("false"))));
#endif
    } // end IF
    goto close;
  } // end IF
  if (unlikely (!Net_Common_Tools::setLinger (handle_in,
                                              inherited::configuration_->socketConfiguration.linger,
                                              std::numeric_limits<unsigned short>::max ())))
  {
    error = ACE_OS::last_error ();
    if (error != ENOTSOCK) // <-- socket has been closed asynchronously
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(0x%@,%s,-1), aborting\n"),
                  handle_in,
                  (inherited::configuration_->socketConfiguration.linger ? ACE_TEXT ("true")
                                                                         : ACE_TEXT ("false"))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setLinger(%d,%s,-1), aborting\n"),
                  handle_in,
                  (inherited::configuration_->socketConfiguration.linger ? ACE_TEXT ("true")
                                                                         : ACE_TEXT ("false"))));
#endif
    } // end IF
    goto close;
  } // end IF

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  COMMTIMEOUTS timeouts;
//  ACE_OS::memset (&timeouts, 0, sizeof (COMMTIMEOUTS));
//  if (!::GetCommTimeouts (handle_in,
//                          &timeouts))
//  {
//    DWORD error = ::GetLastError ();
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to GetCommTimeouts(0x%@): \"%s\", aborting\n"),
//                handle_in,
//                ACE_TEXT (Common_Tools::error2String (error).c_str ())));
//    goto close;
//  } // end IF
//#endif

  // step2: initialize i/o streams
  //Common_IRefCount* iref_count_p = this;
  //ACE_ASSERT (iref_count_p);
  result = inputStream_.open (*this,       // event handler
                              handle_in,   // handle
                              NULL,        // completion key
                              proactor_p); // proactor
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::open(0x%@): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::open(%d): \"%m\", aborting\n"),
                handle_in));
#endif
    goto close;
  } // end IF
  result = outputStream_.open (*this,        // event handler
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               handle_in,    // handle
#else
                               writeHandle_, // handle
#endif
                               NULL,         // completion key
                               proactor_p);  // proactor
  if (unlikely (result == -1))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Stream::open(0x%@): \"%m\", aborting\n"),
                handle_in));
#else
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Stream::open(%d): \"%m\", aborting\n"),
                writeHandle_));
#endif
    goto close;
  } // end IF

  return;

close:
  result = handle_close (handle_in,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                handle_in,
                ACE_Event_Handler::ALL_EVENTS_MASK));
#else
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in,
                ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
  } // end IF
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
  if (unlikely ((result != 0) && (result != 1))) // 2: --> error ?
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENOENT)                  && // 2   : *TODO*
        (error != ENOMEM)                  && // 12  : [server: local close()] *TODO*: ?
        (error != ERROR_OPERATION_ABORTED) && // 995 : [server: local close()] *TODO*: ?
        (error != ERROR_IO_PENDING)        && // 997 :
        (error != ERROR_CONNECTION_ABORTED))  // 1236: [client: local close()]
#else
    if (error == EINPROGRESS) result = 0; // --> AIO_CANCELED
    if ((error != ENOENT)     && // 2  : *TODO*
        (error != EINVAL)     && // 22 : Linux [client: local close()]
        (error != EINPROGRESS))  // 115: happens on Linux
#endif
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\" (result was: %d), continuing\n"),
                  result));
    } // end IF
    else
      result = 0;
  } // end IF
  int result_2 = outputStream_.cancel ();
  if (unlikely ((result_2 != 0) && (result_2 != 1))) // 2: --> error ?
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENOENT)                  && // 2   : *TODO*
        (error != ENOMEM)                  && // 12  : [server: local close()] *TODO*: ?
        (error != ERROR_IO_PENDING)        && // 997 :
        (error != ERROR_CONNECTION_ABORTED))  // 1236: [client: local close()]
#else
    if (error == EINPROGRESS) result_2 = 0; // --> AIO_CANCELED
    if ((error != ENOENT)     && // 2  : *TODO*
        (error != EBADF)      && // 9  : Linux [client: local close()]
        (error != EPIPE)      && // 32 : Linux [client: remote close()]
        (error != EINPROGRESS))  // 115: happens on Linux
#endif
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Write_Stream::cancel(): \"%m\" (result was: %d), continuing\n"),
                  result_2));
    } // end IF
    else
      result_2 = 0;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (likely (writeHandle_ != ACE_INVALID_HANDLE))
  {
    int result_3 = ACE_OS::close (writeHandle_);
    if (unlikely (result_3 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  writeHandle_));
    writeHandle_ = ACE_INVALID_HANDLE;
  } // end IF
#endif

  return ((((result   != 0) && (result   != 1)) ||
           ((result_2 != 0) && (result_2 != 1))) ? -1
                                                 : 0);
}

template <typename ConfigurationType>
int
Net_AsynchTCPSocketHandler_T<ConfigurationType>::notify (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::notify"));

  int result = -1;

  try {
    result = handle_output (inherited2::handle ());
  } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ACE_Event_Handler::handle_output(0x%@): \"%m\", continuing\n"),
                inherited2::handle ()));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ACE_Event_Handler::handle_output(%d): \"%m\", continuing\n"),
                inherited2::handle ()));
#endif
    result = -1;
  }
  if (unlikely (result == -1))
  { // *NOTE*: most probable reason: socket has been closed by the peer, which
    //         close()s the processing stream (see: handle_close()),
    //         shutting down the message queue
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENOTSOCK)   && // 10038: happens on Win32
        (error != ECONNRESET) && // 10054: happens on Win32
        (error != ENOTCONN))     // 10057: happens on Win32
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::handle_output(0x%@): \"%m\", continuing\n"),
                  inherited2::handle ()));
#else
    if (error != ESHUTDOWN) // 108: happens on Linux
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Event_Handler::handle_output(%d): \"%m\", continuing\n"),
                  inherited2::handle ()));
#endif
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  if (likely (inherited::configuration_->messageAllocator))
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited::configuration_->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));
      return NULL;
    }

    // keep retrying ?
    if (unlikely (!message_block_p &&
                  !inherited::configuration_->messageAllocator->block ()))
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
  if (unlikely (!message_block_p))
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

template <typename ConfigurationType>
void
Net_AsynchTCPSocketHandler_T<ConfigurationType>::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::handle_write_stream"));

  size_t bytes_transferred = result_in.bytes_transferred ();
  bool close = false;
  ACE_Message_Block& message_block_r = result_in.message_block ();

  // sanity check(s)
  if (unlikely (!result_in.success ()))
  {
    // connection closed/reset (by peer) ? --> not an error
    unsigned long error = result_in.error ();
    if ((error != EBADF)                   && // 9  : Linux: local close()
        (error != EPIPE)                   && // 32 : Linux: remote close()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_NETNAME_DELETED)   && // 64 : Win32: local close()
        (error != ERROR_OPERATION_ABORTED) && // 995: Win32: local close()
#endif
        (error != ECONNRESET))                // 104/10054: reset by peer

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("0x%@: failed to write to output stream: \"%s\", continuing\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%d: failed to write to output stream: \"%s\", continuing\n"),
                  result_in.handle (),
                  ACE_TEXT (ACE_OS::strerror (error))));
#endif
  } // end IF

  switch (static_cast<int> (bytes_transferred))
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      unsigned long error = result_in.error ();
      if ((error != EBADF)                   && // 9        : Linux [client: local close()]
          (error != EPIPE)                   && // 32       : Linux [client: remote close()]
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          (error != ERROR_NETNAME_DELETED)   && // 64       : Win32: local close()
          (error != ERROR_OPERATION_ABORTED) && // 995      : Win32: local close()
#endif
          (error != ECONNRESET))                // 104/10054: reset by peer
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("0x%@: failed to write to output stream: \"%s\", continuing\n"),
                        result_in.handle (),
                        ACE::sock_error (static_cast<int> (error))));
#else
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%d: failed to write to output stream: \"%s\", continuing\n"),
                        result_in.handle (),
                        ACE_TEXT (ACE_OS::strerror (error))));
#endif
    } // *WARNING*: control falls through here
    case 0:
    {
      if (bytes_transferred == 0)
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("0x%@: socket was closed\n"),
                    result_in.handle ()));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%d: socket was closed\n"),
                    result_in.handle ()));
#endif
      } // end IF

      close = true;

      break;
    }
    // *** GOOD CASES ***
    default:
    {
      // finished with this buffer ?
      if (unlikely (message_block_r.length ()))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("0x%@: partial write (%u/%u byte(s)), continuing\n"),
                    result_in.handle (),
                    bytes_transferred,
                    bytes_transferred + message_block_r.length ()));
#else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%d: partial write (%u/%u byte(s)), continuing\n"),
                    result_in.handle (),
                    bytes_transferred,
                    bytes_transferred + message_block_r.length ()));
#endif
        partialWrite_ = true;

        goto continue_; // done
      } // end IF

      break;
    }
  } // end SWITCH
  message_block_r.release ();

continue_:
  if (unlikely (close))
  {
    int result = handle_close (result_in.handle (),
                               ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
    {
      int error_2 = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error_2 != ENOENT)          && // 2  : *TODO*
          (error_2 != ENOMEM)          && // 12 : [server: local close()] *TODO*: ?
          (error_2 != ERROR_IO_PENDING))  // 997:
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                    result_in.handle (),
                    ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      //if (error_2 == EINPROGRESS) result = 0; // --> AIO_CANCELED
      if ((error_2 != ENOENT)     && // 2  : *TODO*
          (error_2 != EBADF)      && // 9  : Linux [client: local close()]
          (error_2 != EPIPE)      && // 32 : Linux [client: remote close()]
          (error_2 != EINPROGRESS))  // 115: happens on Linux
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_close(%d,%d): \"%m\", continuing\n"),
                    result_in.handle (),
                    ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
    } // end IF
  } // end IF

  counter_.decrease ();
}

template <typename ConfigurationType>
bool
Net_AsynchTCPSocketHandler_T<ConfigurationType>::initiate_read_stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchTCPSocketHandler_T::initiate_read_stream"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // allocate a data buffer
  ACE_Message_Block* message_block_p =
      allocateMessage (inherited::configuration_->PDUSize);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::allocateMessage(%u), aborting\n"),
                inherited::configuration_->PDUSize));
    return false;
  } // end IF

  // start (asynchronous) read
receive:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result =
    inputStream_.readv (*message_block_p,                     // buffer
                        inherited::configuration_->PDUSize,   // bytes to read
                        NULL,                                 // ACT
                        0,                                    // priority
                        COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
#else
  int result =
    inputStream_.read (*message_block_p,                     // buffer
                       inherited::configuration_->PDUSize,   // bytes to read
                       NULL,                                 // ACT
                       0,                                    // priority
                       COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
#endif
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    // *WARNING*: this could fail on multi-threaded proactors
    if (error == EAGAIN) goto receive; // 11: happens on Linux
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENXIO)                 && // 6    : happens on Win32
        (error != EFAULT)                && // 14   : *TODO*: happens on Win32
        (error != ERROR_UNEXP_NET_ERR)   && // 59   : *TODO*: happens on Win32
        (error != ERROR_NETNAME_DELETED) && // 64   : happens on Win32
        (error != ENOTSOCK)              && // 10038: local close()
        (error != ECONNRESET))              // 10054: reset by peer
#else
    if (error != ECONNRESET) // 104: reset by peer
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Read_Stream::readv(%u): \"%m\", aborting\n"),
                  inherited::configuration_->PDUSize));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  return true;
}
