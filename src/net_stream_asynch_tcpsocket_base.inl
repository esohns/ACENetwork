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
#include "ace/Proactor.h"
#include "ace/Stream.h"

#include "stream_common.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::Net_StreamAsynchTCPSocketBase_T ()
 : inherited ()
 , inherited2 ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::Net_StreamAsynchTCPSocketBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::open (ACE_HANDLE handle_in,
                                                     ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::open"));

  int result = -1;
  bool handle_socket = false;
  const ConfigurationType& configuration_r = this->getR ();

  // step1: initialize base-class(es), tweak socket, initialize I/O, ...
  // *TODO*: remove type inferences
  if (unlikely (!inherited::initialize (configuration_r.socketHandlerConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::initialize(), aborting\n")));
    goto error;
  } // end IF
  inherited::open (handle_in, messageBlock_in);
  handle_socket = true;

  // step2: start reading (need to pass any data ?)
  if (likely (!messageBlock_in.length ()))
  {
    if (unlikely (!inherited::initiate_read_stream ()))
    {
      int error = ACE_OS::last_error ();
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
                    ACE_TEXT ("failed to Net_AsynchTCPSocketHandler::initiate_read_stream(): \"%m\", aborting\n")));
      goto error;
    } // end IF
  } // end IF
  else
  {
    ACE_Message_Block* duplicate_p = messageBlock_in.duplicate ();
    if (!duplicate_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    // fake a result to emulate regular behavior
    ACE_Proactor* proactor_p = inherited::proactor ();
    ACE_ASSERT (proactor_p);
    ACE_Asynch_Read_Stream_Result_Impl* fake_result_p =
        proactor_p->create_asynch_read_stream_result (inherited::proxy (),                            // handler proxy
                                                      handle_in,                                      // socket handle
                                                      *duplicate_p,                                   // buffer
                                                      static_cast<u_long> (duplicate_p->capacity ()), // (maximum) #bytes to read
                                                      NULL,                                           // ACT
                                                      ACE_INVALID_HANDLE,                             // event
                                                      0,                                              // priority
                                                      COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);           // signal number
    if (!fake_result_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Proactor::create_asynch_read_stream_result: \"%m\", aborting\n")));
      goto error;
    } // end IF
    size_t bytes_transferred = duplicate_p->length ();
    // <complete> for Accept would have already moved the <wr_ptr>
    // forward; update it to the beginning position
    duplicate_p->wr_ptr (duplicate_p->wr_ptr () - bytes_transferred);
    // invoke ourselves (see handle_read_stream())
    fake_result_p->complete (duplicate_p->length (), // bytes read
                             1,                      // success
                             NULL,                   // ACT
                             0);                     // error

    // clean up
    delete fake_result_p;
  } // end ELSE
  //ACE_ASSERT (this->count () == 2); // connection manager, read operation
  //                                     (+ stream module(s))

  return;

error:
  if (handle_socket)
  {
    result = inherited::handle_close (handle_in,
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(): \"%m\", continuing\n")));
  } // end IF

  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  int error = 0;
  ACE_Message_Block* message_block_p = NULL;

//  if (!inherited::buffer_)
//  {
    // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
    //                   been notified
//    result = inherited::getq (inherited::buffer_, NULL);
    result = inherited2::getq (message_block_p, NULL);
    if (unlikely (result == -1))
    { // *NOTE*: most probable reason: socket has been closed by the peer, which
      //         close()s the processing stream (see: handle_close()),
      //         shutting down the message queue
      error = ACE_OS::last_error ();
      if (error != ESHUTDOWN) // 108: happens on Linux
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
      return -1;
    } // end IF
//  } // end IF
//  ACE_ASSERT (inherited::buffer_);
    ACE_ASSERT (message_block_p);

  // start (asynchronous) write
  this->increase ();
  inherited::counter_.increase ();
send:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result =
//    inherited::outputStream_.writev (*inherited::buffer_,                  // data
//                                     inherited::buffer_->length (),        // bytes to write
      inherited::outputStream_.writev (*message_block_p,                     // data
                                       message_block_p->length (),           // bytes to write
                                       NULL,                                 // ACT
                                       0,                                    // priority
                                       COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
#else
  result =
//    inherited::outputStream_.write (*inherited::buffer_,                  // data
//                                    inherited::buffer_->length (),        // bytes to write
      inherited::outputStream_.write (*message_block_p,                     // data
                                      message_block_p->length (),           // bytes to write
                                      NULL,                                 // ACT
                                      0,                                    // priority
                                      COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal number
#endif
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
    // *WARNING*: this could fail on multi-threaded proactors
    if (error == EAGAIN) goto send; // 11   : happens on Linux
    if ((error != ENOTSOCK)     &&  // 10038: happens on Win32
        (error != ECONNABORTED) &&  // 10053: happens on Win32
        (error != ECONNRESET)   &&  // 10054: happens on Win32
        (error != ENOTCONN))        // 10057: happens on Win32
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Write_Stream::writev(%u): \"%m\", aborting\n"),
//                  inherited::buffer_->length ()));
                  message_block_p->length ()));

    // clean up
//    inherited::buffer_->release ();
//    inherited::buffer_ = NULL;
    message_block_p->release ();
    this->decrease ();
    inherited::counter_.decrease ();
  } // end IF
//  else if (result == 0)
//  {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("0x%@: socket was closed\n"),
//                handle_in));
//#else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%d: socket was closed\n"),
//                handle_in));
//#endif
//  } // end IF

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                             ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_close"));

  int result = -1;

  // *IMPORTANT NOTE*: when control reaches here, the socket handle has already
  //                   gone away, i.e. no new data will be accepted by the
  //                   kernel / forwarded by the proactor
  //                   --> finish processing: flush all remaining outbound data
  //                       and wait for all workers within the stream
  //                   [--> cancel any pending asynchronous operations]

  // *NOTE*: pending socket operations are notified by the kernel and will
  //         return automatically
  // *TODO*: consider cancel()ling pending write operations

  // step2: invoke base-class maintenance
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle_in,
                  mask_in));
#endif

  return result;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::close"));

  int result = -1;

  // step1: cancel i/o operation(s), release (write) socket handle, ...
  ACE_HANDLE handle = inherited::handle ();
  result = inherited::handle_close (handle,
                                    ACE_Event_Handler::ALL_EVENTS_MASK);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENOENT)          && // 2  : *TODO*
        (error != ENOMEM)          && // 12 : [server: local close()] *TODO*: ?
        (error != ERROR_IO_PENDING))  // 997:
#else
    if (error == EINPROGRESS)
      result = 0; // --> AIO_CANCELED
    if ((error != ENOENT)     && // 2  : *TODO*
        (error != EBADF)      && // 9  : Linux [client: local close()]
        (error != EINVAL)     && // 22 : Linux [client: local close()]
        (error != EPIPE)      && // 32 : Linux [client: remote close()]
        (error != EINPROGRESS))  // 115: happens on Linux
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  handle,
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  handle,
                  ACE_Event_Handler::ALL_EVENTS_MASK));
#endif
  } // end IF

  //  step2: wake up any open read operation; it will release the connection
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::closesocket (handle);
    if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                  handle));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                  handle));
#endif
  } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::waitForCompletion"));

  int result = -1;
  ACE_Time_Value one_second (1, 0);

  // sanity check(s)
  ACE_ASSERT (inherited2::msg_queue_);

  // step1: wait for the queue to flush
  while (!inherited2::msg_queue_->is_empty ())
  {
    result = ACE_OS::sleep (one_second);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
  } // end WHILE

  // step2: wait for any asynchronous operations to complete ?
  inherited::counter_.wait (0);
  // --> all data has been dispatched to the kernel (socket)

  // *TODO*: different platforms may implement methods by which successful
  //         placing of the data onto the wire can be established
  //         (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  ACE_HANDLE handle = inherited::handle ();
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::dump_state"));

  // debug info
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [0x%@]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_read_stream"));

  int result = -1;
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection closed/reset (by peer) ? --> not an error
    error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ERROR_NETNAME_DELETED)   && // 64  : peer close()
        (error != ERROR_OPERATION_ABORTED) && // 995 : local close()
        (error != ERROR_CONNECTION_ABORTED))  // 1236: local close()
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("0x%@: failed to read from input stream: \"%s\", returning\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
    if ((error != EBADF)     && // 9  : local close(), happens on Linux
        (error != EPIPE)     && // 32 : happens on Linux
        (error != ECONNRESET))  // 104: happens on Linux
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%d: failed to read from input stream: \"%s\", returning\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#endif
  } // end IF

  switch (static_cast<int> (result_in.bytes_transferred ()))
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ERROR_NETNAME_DELETED)   && // 64  : peer close()
          (error != ERROR_OPERATION_ABORTED) && // 995 : local close()
          (error != ERROR_CONNECTION_ABORTED))  // 1236: local close()
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%d: failed to read from input stream: \"%s\", returning\n"),
                    result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#else
      if ((error != EBADF)     && // 9  : local close(), happens on Linux
          (error != EPIPE)     && // 32 : happens on Linux
          (error != ECONNRESET))  // 104: happens on Linux
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%d: failed to read from input stream: \"%s\", returning\n"),
                    result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#endif
      break;
    }
    // *** GOOD CASES ***
    case 0:
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

      break;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%d]: received %u byte(s)\n"),
//                   result.handle (),
//                   result.bytes_transferred ()));

      // push the buffer into the queue for processing
      result = inherited2::putq (&result_in.message_block (), NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", returning\n")));
        break;
      } // end IF

      // start next read
      if (!inherited::initiate_read_stream ())
      {
        error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if ((error != ENXIO)                 && // 6    : happens on Win32
            (error != EFAULT)                && // 14   : *TODO*: happens on Win32
            (error != ERROR_UNEXP_NET_ERR)   && // 59   : *TODO*: happens on Win32
            (error != ERROR_NETNAME_DELETED) && // 64   : happens on Win32
            (error != ENOTSOCK)              && // 10038: (asynchronous) local close
            (error != ECONNRESET))              // 10054: reset by peer
#else
        if (error != ECONNRESET) // 104: reset by peer
#endif
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to HandlerType::initiate_read_stream(): \"%m\", returning\n")));
        goto close;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ().release ();

close:
  result =
      handle_close (result_in.handle (),
                    ((result_in.bytes_transferred () == 0) ? ACE_Event_Handler::READ_MASK // peer closed the connection
                                                           : ACE_Event_Handler::ALL_EVENTS_MASK));
  if (result == -1)
  {
//    error = ACE_OS::last_error ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if ((error != ENOENT)                  && // 2   :
//        (error != ENOMEM)                  && // 12  : [server: local close()], [client: peer reset ()]
//        (error != ERROR_IO_PENDING)        && // 997 :
//        (error != ERROR_CONNECTION_ABORTED))  // 1236: [client: local close()]
//#else
//    if (error == EINPROGRESS) result = 0; // --> AIO_CANCELED
//    if ((error != ENOENT)     && // 2  :
//        (error != EPIPE)      && // 32 : Linux [client: remote close()]
//        (error != EINPROGRESS))  // 115: happens on Linux
//#endif
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n")));
  } // end IF

  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_write_stream"));

//  int result = -1;
//  size_t bytes_to_write = result_in.bytes_to_write ();

  // *NOTE*: the message block is released by the base class
  inherited::handle_write_stream (result_in);

  // partial write ?
  if (inherited::partialWrite_)
      goto continue_;

  // reschedule ?
  // *TODO*: put the buffer back into the queue and call handle_output()
  // *IMPORTANT NOTE*: this will fail if any buffers have been
  //                   dispatched in the meantime
  //                   --> i.e. works for single-threaded proactors only

//    result = handle_output (result_in.handle ());
//    if (result == -1)
//    { // *NOTE*: most probable reason: socket has been closed by the peer, which
//      //         close()s the processing stream (see: handle_close()),
//      //         shutting down the message queue
//      int error = ACE_OS::last_error ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if ((error != ENOTSOCK)   && // 10038: happens on Win32
//          (error != ECONNRESET) && // 10054: happens on Win32
//          (error != ENOTCONN))     // 10057: happens on Win32
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Event_Handler::handle_output(0x%@): \"%m\", continuing\n"),
//                    handle));
//#else
//      if (error != ESHUTDOWN) // 108: happens on Linux
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Event_Handler::handle_output(%d): \"%m\", continuing\n"),
//                    handle));
//#endif
//    } // end IF

continue_:
  this->decrease ();
}
