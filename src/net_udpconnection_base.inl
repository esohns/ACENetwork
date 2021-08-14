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

#if defined (ACE_LINUX)
#include "linux/errqueue.h"
#include "netinet/ip_icmp.h"
#endif // ACE_LINUX

#include "ace/Asynch_IO.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Proactor.h"

#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::Net_UDPConnectionBase_T (bool managed_in)
 : inherited (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::Net_UDPConnectionBase_T"));

}

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::handle_input"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  ssize_t bytes_received_i = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_INET_Addr peer_address;

  // sanity check
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration);

  // *TODO*: remove type inferences
  size_t pdu_size_i =
    inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->defaultBufferSize +
    inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->paddingBytes;
  message_block_p = inherited::allocateMessage (pdu_size_i);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to allocateMessage(%u), aborting\n"),
                this->id (),
                pdu_size_i));
    return -1; // <-- remove 'this' from dispatch
  } // end IF
  message_block_p->size (inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->defaultBufferSize);

  // read a datagram from the socket
  bytes_received_i =
    inherited::peer_.recv (message_block_p->wr_ptr (),                                                              // buffer
                           inherited::CONNECTION_BASE_T::configuration_->allocatorConfiguration->defaultBufferSize, // buffer size
                           peer_address,                                                                            // peer address
                           0);                                                                                      // flags
  //bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (),                 // buf
  //                                        inherited2::configuration_->PDUSize, // n
  //                                        0,                                   // flags
  //                                        NULL);                               // timeout
  switch (bytes_received_i)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET) &&
          (error != EPIPE)      &&      // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != EBADF)      &&
          (error != ENOTSOCK)   &&
          (error != ECONNABORTED)) // <-- connection abort()ed locally
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to SocketType::recv(): \"%m\", returning\n"),
                    this->id ()));
      message_block_p->release (); message_block_p = NULL;
      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
      message_block_p->release (); message_block_p = NULL;
      return -1; // <-- remove 'this' from dispatch
    }
    default:
    {
      // update statistic
      inherited::state_.statistic.receivedBytes += bytes_received_i;

      // adjust write pointer
      message_block_p->wr_ptr (static_cast<size_t> (bytes_received_i));

      break;
    }
  } // end SWITCH

  // enqueue the buffer into the stream for processing
  // *NOTE*: the stream assumes ownership of the buffer
  result = inherited::stream_.put (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 10058: queue has been deactivate()d
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u/%s: failed to ACE_Stream::put(): \"%m\", aborting\n"),
                  this->id (),
                  ACE_TEXT (inherited::stream_.name ().c_str ())));
    message_block_p->release (); message_block_p = NULL;
    return -1; // <-- remove 'this' from dispatch
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::handle_output"));

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);

  int result = -1;
  ssize_t bytes_sent = 0;
  ACE_Message_Block* message_block_p = NULL;

  // *NOTE*: in a threaded environment, workers could be dispatching the reactor
  //         notification queue concurrently (most notably, ACE_TP_Reactor)
  //         --> enforce proper serialization here
  // *IMPORTANT NOTE*: the ACE documentation (books) explicitly claims that
  //                   measures are in place to prevent concurrent dispatch of
  //                   the same handler for a specific handle by different
  //                   threads (*TODO*: find reference). If this is indeed true,
  //                   this test may be removed (just make sure this holds for
  //                   both:
  //                   - the socket dispatch implementation
  //                   - the (piped) notification dispatch implementation
  // *TODO*: remove type inferences
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::sendLock_, -1);
  //if (unlikely (inherited::writeBuffer_))
  //  goto continue_;

  // dequeue next chunk from the queue/stream
  // *IMPORTANT NOTE*: should NEVER block, as available outbound data has
  //                   been notified to the reactor
  result =
    (unlikely (inherited::CONNECTION_BASE_T::configuration_->useThreadPerConnection) ? inherited::getq (message_block_p, NULL)
                                                                                     : inherited::stream_.get (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    // *NOTE*: a number of issues can occur here:
    //         - connection has been closed in the meantime
    //         - queue has been deactivated
    int error = ACE_OS::last_error ();
    if ((error != EAGAIN)   && // 11   : connection has been closed
        (error != ESHUTDOWN))  // 10058: queue has been deactivated
      ACE_DEBUG ((LM_ERROR,
                  (inherited::CONNECTION_BASE_T::configuration_->useThreadPerConnection ? ACE_TEXT ("%u: failed to ACE_Task::getq(): \"%m\", aborting\n")
                                                                                        : ACE_TEXT ("%u: failed to ACE_Stream::get(): \"%m\", aborting\n")),
                  this->id ()));
    return -1; // <-- remove 'this' from dispatch
  } // end IF
//continue_:
  ACE_ASSERT (message_block_p);

//  // finished ?
//  // *TODO*: remove type inferences
//  if (message_block_p->msg_type () == ACE_Message_Block::MB_STOP)
//  {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("[0x%@]: finished sending\n"),
//                handle_in));
//#else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("[%d]: finished sending\n"),
//                handle_in));
//#endif
//
//    // clean up
//    message_block_p->release ();
//    //writeBuffer_ = NULL;
//
//    result = -1; // <-- remove 'this' from dispatch
//
//    goto continue_;
//  } // end IF

  // place data into the socket
  bytes_sent =
      inherited::HANDLER_T::send (message_block_p->rd_ptr (), // data
                                  message_block_p->length (), // #bytes to send
                                  inherited::address_,        // peer address
                                  0);//,                      // flags
                                  //NULL);                   // timeout
  switch (bytes_sent)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if ((error != ECONNRESET)   &&
          (error != ECONNABORTED) &&
          (error != EPIPE)        && // <-- connection reset by peer
          // -------------------------------------------------------------------
          (error != ENOTSOCK)     &&
          (error != EBADF))          // <-- connection abort()ed locally
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::send(%s,%u) (handle was: 0x%@): \"%m\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::address_).c_str ()),
                    message_block_p->length (),
                    handle_in));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::send(%s,%u) (handle was: %d): \"%m\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::address_).c_str ()),
                    message_block_p->length (),
                    handle_in));
#endif
      } // end IF
#if defined (ACE_LINUX)
      if (inherited::errorQueue_)
        this->processErrorQueue ();
#endif
      message_block_p->release (); message_block_p = NULL;
      result = -1; // <-- remove 'this' from dispatch
      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
      message_block_p->release (); message_block_p = NULL;
      result = -1; // <-- remove 'this' from dispatch
      break;
    }
    default:
    { // update statistic
      inherited::state_.statistic.sentBytes += bytes_sent;

      // finished with this buffer ?
      message_block_p->rd_ptr (static_cast<size_t> (bytes_sent));
      ACE_ASSERT (!message_block_p->length ());
      //if (unlikely (message_block_p->length ()))
      //  break; // there's more data
      message_block_p->release (); message_block_p = NULL;
      result = 0;
      break;
    }
  } // end SWITCH

  // immediately reschedule sending ?
  //  if ((writeBuffer_ == NULL) && inherited::msg_queue ()->is_empty ())
  //  {
  //    if (inherited::reactor ()->cancel_wakeup (this,
  //                                              ACE_Event_Handler::WRITE_MASK) == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Reactor::cancel_wakeup(): \"%m\", continuing\n")));
  //  } // end IF
  //  else
  //if (unlikely (writeBuffer_))
  //  result = 1;
  //if (inherited::reactor ()->schedule_wakeup (this,
  //                                            ACE_Event_Handler::WRITE_MASK) == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_Reactor::schedule_wakeup(): \"%m\", continuing\n")));

//continue_:
  return result;
}

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::info (ACE_HANDLE& handle_out,
                                             ACE_INET_Addr& localSAP_out,
                                             ACE_INET_Addr& peerSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::info"));

  int result = -1;
  int error = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);

  handle_out =
    (inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly ? inherited::writeHandle_
                                                                                 : inherited::peer_.get_handle ());
  localSAP_out.reset ();
  peerSAP_out.reset ();

  if (likely (!inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly))
  {
    result = inherited::peer_.get_local_addr (localSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (error != ENOTSOCK) // 10038: Win32: handle is not a socket (i.e. socket already closed)
#else
      if (error != EBADF)    //     9: Linux: socket already closed
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end IF
  if (unlikely (inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly))
    peerSAP_out = inherited::address_;
  else
  {
    result = inherited::peer_.get_remote_addr (peerSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ENOTSOCK) && // 10038: Win32: socket already closed
          (error != ENOTCONN))   // 10057: Win32: not connected
#else
      if ((error != EBADF) &&  //   9: Linux: socket already closed
          (error != ENOTCONN)) // 107: Linux: not connected
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram::get_remote_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end ELSE
}

template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::reset ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::reset"));

  int result = -1;

  result = inherited::HANDLER_T::SVC_HANDLER_T::close ();
  if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_Svc_Handler::close(): \"%m\", continuing\n"),
                  this->id ()));
  if (likely (inherited::HANDLER_T::writeHandle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::closesocket (inherited::writeHandle_);
    if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                    id (), inherited::HANDLER_T::writeHandle_));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                    this->id (), inherited::writeHandle_));
#endif
  } // end IF
  inherited::HANDLER_T::writeHandle_ = ACE_INVALID_HANDLE;

  inherited::open (inherited::CONNECTION_BASE_T::configuration_);
}

#if defined (ACE_LINUX)
template <ACE_SYNCH_DECL,
          typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        SocketHandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        StreamType,
                        UserDataType>::processErrorQueue ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::processErrorQueue"));

  ssize_t result = -1;

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  struct iovec iovec_a[1];
  iovec_a[0].iov_base = buffer;
  iovec_a[0].iov_len = sizeof (buffer);
  ACE_INET_Addr socket_address;
  ACE_TCHAR buffer_2[BUFSIZ];
  ACE_OS::memset (buffer_2, 0, sizeof (buffer_2));
//  result = inherited::peer_.recv (iovec_a, 1,
//                                  socket_address,
//                                  MSG_ERRQUEUE);

  struct msghdr msghdr_s;
  msghdr_s.msg_iov = iovec_a;
  msghdr_s.msg_iovlen = 1;
#if defined (ACE_HAS_SOCKADDR_MSG_NAME)
  msghdr_s.msg_name =
      static_cast<struct sockaddr*> (socket_address.get_addr ());
#else
  msghdr_s.msg_name = static_cast<char*> (socket_address.get_addr ());
#endif /* ACE_HAS_SOCKADDR_MSG_NAME */
  msghdr_s.msg_namelen = socket_address.get_addr_size ();

#if defined (ACE_HAS_4_4BSD_SENDMSG_RECVMSG)
  msghdr_s.msg_control = buffer_2;
  msghdr_s.msg_controllen = sizeof (buffer_2);
#elif !defined ACE_LACKS_SENDMSG
  msghdr_s.msg_accrights = 0;
  msghdr_s.msg_accrightslen = 0;
#endif /* ACE_HAS_4_4BSD_SENDMSG_RECVMSG */

  result = ACE_OS::recvmsg (inherited::peer_.get_handle (),
                            &msghdr_s,
                            MSG_ERRQUEUE | MSG_WAITALL);
  if (result == -1)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::recv(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
//                inherited::peer_.get_handle ()));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%t: failed to ACE_OS::recvmsg(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
                inherited::peer_.get_handle ()));
    return;
  } // end IF
  socket_address.set_size (msghdr_s.msg_namelen);
  socket_address.set_type (static_cast<struct sockaddr_in*> (socket_address.get_addr ())->sin_family);

  struct sock_extended_err* sock_err_p = NULL;
  for (struct cmsghdr* cmsghdr_p = CMSG_FIRSTHDR (&msghdr_s);
       cmsghdr_p;
       cmsghdr_p = CMSG_NXTHDR (&msghdr_s, cmsghdr_p))
  {
    if ((cmsghdr_p->cmsg_level != SOL_IP) || // IPPROTO_IP
        (cmsghdr_p->cmsg_type  != IP_RECVERR))
      continue;

    sock_err_p =
        reinterpret_cast<struct sock_extended_err*> (CMSG_DATA (cmsghdr_p));
    if (!sock_err_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%t: failed to retrieve socket error: \"%m\", continuing\n"),
                  inherited::peer_.get_handle ()));
      continue;
    } // end IF

    switch (sock_err_p->ee_origin)
    {
      case SO_EE_ORIGIN_NONE:
        break;
      case SO_EE_ORIGIN_LOCAL:
        break;
      case SO_EE_ORIGIN_ICMP:
      {
        switch (sock_err_p->ee_type)
        {
          case ICMP_NET_UNREACH:
            break;
          case ICMP_HOST_UNREACH:
            break;
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%t: invalid/unknown ICMP error (was: %d), continuing\n"),
                        sock_err_p->ee_type));
            break;
          }
        } // end SWITCH
        break;
      }
      case SO_EE_ORIGIN_ICMP6:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: invalid/unknown error origin (was: %d), continuing\n"),
                    sock_err_p->ee_origin));
        break;
      }
    } // end SWITCH
  } // end FOR
}
#endif

/////////////////////////////////////////

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
Net_AsynchUDPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::Net_AsynchUDPConnectionBase_T (bool managed_in)
 : inherited (managed_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::Net_AsynchUDPConnectionBase_T"));

}

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_AsynchUDPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::open (ACE_HANDLE handle_in,
                                                   ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::open"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);

  // step1: initialize asynchronous I/O
  inherited::state_.status = NET_CONNECTION_STATUS_INITIALIZING;
  inherited::open (handle_in,
                   messageBlock_in);
  if (unlikely (inherited::state_.status != NET_CONNECTION_STATUS_OK))
    goto error;

  // step2: start reading ? (need to pass any data ?)
  if (unlikely (inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly))
  { ACE_ASSERT (!messageBlock_in.length ());
    goto continue_;
  } // end IF

  if (likely (!messageBlock_in.length ()))
  {
    if (unlikely (!initiate_read ()))
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
#endif // ACE_WIN32 || ACE_WIN64
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to Net_IAsynchSocketHandler::initiate_read(): \"%m\", aborting\n"),
                    this->id ()));
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

continue_:
  ACE_ASSERT (this->count () >= 2); // connection manager, read operation
                                    // (+ stream module(s))

  if (unlikely (inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly))
    this->decrease (); // there is no read operation

  return;

error:
  result = inherited::handle_close (handle_in,
                                    ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchStreamConnectionBase_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                handle_in, ACE_Event_Handler::ALL_EVENTS_MASK));
#else
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_AsynchStreamConnectionBase_T::handle_close(%d,%d): \"%m\", continuing\n"),
                handle_in, ACE_Event_Handler::ALL_EVENTS_MASK));
#endif // ACE_WIN32 || ACE_WIN64
}

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
int
Net_AsynchUDPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  // *TODO*: always retrieve data from inherited::stream_
  typename StreamType::ISTREAM_T::STREAM_T* stream_p =
      inherited::stream_.upstream (true);
  ACE_Message_Block* message_block_p = NULL;// , * message_block_2, * message_block_3 = NULL;
//  unsigned int length = 0;
  size_t bytes_sent = 0;
  ssize_t result_2 = 0;

  // *TODO*: this is obviously flaky; remove ASAP
  // sanity check(s)
  if (likely (!stream_p))
    stream_p = &(inherited::stream_);
  ACE_ASSERT (stream_p);

  // dequeue message from the stream
  // *IMPORTANT NOTE*: this should NEVER block as available outbound data has
  //                   been notified
  result = stream_p->get (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN) // 108
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u/%s: failed to ACE_Stream::get(): \"%m\", aborting\n"),
                  this->id (),
                  ACE_TEXT (inherited::stream_.name ().c_str ())));
    return -1;
  } // end IF
  ACE_ASSERT (message_block_p);

  // fragment the data ?
  //message_block_2 = message_block_p;
  //do
  //{
  //  length = message_block_2->length ();
  //  if (length > inherited::CONNECTION_BASE_T::configuration_->PDUSize)
  //  {
  //    message_block_3 = message_block_2->duplicate ();
  //    if (unlikely (!message_block_3))
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
  //      message_block_p->release (); message_block_p = NULL;
  //      return -1;
  //    } // end IF
  //    message_block_2->cont (message_block_3);

  //    message_block_2->length (inherited::CONNECTION_BASE_T::configuration_->PDUSize);
  //    message_block_3->rd_ptr (inherited::CONNECTION_BASE_T::configuration_->PDUSize);

  //    message_block_2 = message_block_3;
  //    continue;
  //  } // end IF

  //  message_block_2 = message_block_2->cont ();
  //} while (message_block_2);

  // start (asynchronous) write(s)
  //do
  //{
    //length = 0;
    //message_block_2 = message_block_p;
    //while (length < inherited::CONNECTION_BASE_T::configuration_->PDUSize)
    //{
    //  length += message_block_2->length ();
    //  message_block_2 = message_block_2->cont ();
    //  if (!message_block_2)
    //    break;
    //} // end WHILE
    //if (message_block_2)
    //{
    //  message_block_3 = message_block_p;
    //  while (message_block_3->cont () != message_block_2)
    //    message_block_3 = message_block_3->cont ();
    //  message_block_3->cont (NULL);
    //} // end IF

    this->increase ();
    inherited::counter_.increase ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
send:
#endif // ACE_WIN32 || ACE_WIN64
    // *NOTE*: this is a fire-and-forget API for message_block_p
    result_2 =
      inherited::outputStream_.send (message_block_p,                      // data
                                     bytes_sent,                           // #bytes sent
                                     0,                                    // flags
                                     inherited::address_,                  // peer address
                                     NULL,                                 // asynchronous completion token
                                     0,                                    // priority
                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
    if (unlikely (result_2 == -1))
    {
      int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // *WARNING*: this could fail on multi-threaded proactors
      if (error == EAGAIN) // 11   : happens on Linux
        goto send;
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::send(%u): \"%m\", aborting\n"),
                  message_block_p->total_length ()));

      // clean up
      message_block_p->release ();
      this->decrease ();
      inherited::counter_.decrease ();

      return -1;
    } // end IF
  //  message_block_p = message_block_2;
  //} while (message_block_p);

  return 0;
}

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_AsynchUDPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::info (ACE_HANDLE& handle_out,
                                                   ACE_INET_Addr& localSAP_out,
                                                   ACE_INET_Addr& peerSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::info"));

  int result = -1;
  int error = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);

  handle_out =
    (inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly ? inherited::writeHandle_
                                                                                 : inherited::HANDLER_T::SOCKET_T::get_handle ());
  localSAP_out.reset ();
  peerSAP_out.reset ();

  if (likely (!inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly))
  {
    result = inherited::HANDLER_T::SOCKET_T::get_local_addr (localSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (error != ENOTSOCK) // 10038: Win32: handle is not a socket (i.e. socket already closed)
#else
      if (error != EBADF)    //     9: Linux: socket already closed
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end IF
  if (unlikely (inherited::CONNECTION_BASE_T::configuration_->socketConfiguration.writeOnly))
    peerSAP_out = inherited::address_;
  else
  {
    result = inherited::HANDLER_T::SOCKET_T::get_remote_addr (peerSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((error != ENOTSOCK) && // 10038: Win32: socket already closed
          (error != ENOTCONN))   // 10057: Win32: not connected
#else
      if ((error != EBADF) &&  //   9: Linux: socket already closed
          (error != ENOTCONN)) // 107: Linux: not connected
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Dgram::get_remote_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end ELSE
}

template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_AsynchUDPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::reset ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::reset"));

  int result = -1;
  ACE_Message_Block message_block;
  ACE_HANDLE handle_h = inherited::SOCKET_T::get_handle ();

  // sanity check(s)
  ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);

  result = inherited::handle_close (handle_h,
                                    ACE_Event_Handler::SIGNAL_MASK);
  if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to HandlerType::handle_close(0x%@,%d): \"%m\", continuing\n"),
                  id (),
                  handle_h, ACE_Event_Handler::SIGNAL_MASK));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to HandlerType::handle_close(%d,%d): \"%m\", continuing\n"),
                  this->id (),
                  handle_h, ACE_Event_Handler::SIGNAL_MASK));
#endif
  result = inherited::SOCKET_T::close ();
  if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to HandlerType::SocketType::close(): \"%m\", continuing\n"),
                  this->id ()));
  if (likely (inherited::writeHandle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::closesocket (inherited::writeHandle_);
    if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                    id (), inherited::writeHandle_));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                    this->id (), inherited::writeHandle_));
#endif
    inherited::writeHandle_ = ACE_INVALID_HANDLE;
  } // end IF

  inherited::open (ACE_INVALID_HANDLE,
                   message_block);
}

#if defined (ACE_LINUX)
template <typename SocketHandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          typename UserDataType>
void
Net_AsynchUDPConnectionBase_T<SocketHandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              StreamType,
                              UserDataType>::processErrorQueue ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::processErrorQueue"));

  ssize_t result = -1;

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  struct iovec iovec_a[1];
  iovec_a[0].iov_base = buffer;
  iovec_a[0].iov_len = sizeof (buffer);
  ACE_INET_Addr socket_address;
  ACE_TCHAR buffer_2[BUFSIZ];
  ACE_OS::memset (buffer_2, 0, sizeof (buffer_2));
//  result = inherited::peer_.recv (iovec_a, 1,
//                                  socket_address,
//                                  MSG_ERRQUEUE);

  struct msghdr msghdr_s;
  msghdr_s.msg_iov = iovec_a;
  msghdr_s.msg_iovlen = 1;
#if defined (ACE_HAS_SOCKADDR_MSG_NAME)
  msghdr_s.msg_name =
      static_cast<struct sockaddr*> (socket_address.get_addr ());
#else
  msghdr_s.msg_name = static_cast<char*> (socket_address.get_addr ());
#endif /* ACE_HAS_SOCKADDR_MSG_NAME */
  msghdr_s.msg_namelen = socket_address.get_addr_size ();

#if defined (ACE_HAS_4_4BSD_SENDMSG_RECVMSG)
  msghdr_s.msg_control = buffer_2;
  msghdr_s.msg_controllen = sizeof (buffer_2);
#elif !defined ACE_LACKS_SENDMSG
  msghdr_s.msg_accrights = 0;
  msghdr_s.msg_accrightslen = 0;
#endif /* ACE_HAS_4_4BSD_SENDMSG_RECVMSG */

  result = ACE_OS::recvmsg (inherited::peer_.get_handle (),
                            &msghdr_s,
                            MSG_ERRQUEUE | MSG_WAITALL);
  if (result == -1)
  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%t: failed to ACE_SOCK_Dgram::recv(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
//                inherited::peer_.get_handle ()));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%t: failed to ACE_OS::recvmsg(%d,MSG_ERRQUEUE): \"%m\", returning\n"),
                inherited::peer_.get_handle ()));
    return;
  } // end IF
  socket_address.set_size (msghdr_s.msg_namelen);
  socket_address.set_type (static_cast<struct sockaddr_in*> (socket_address.get_addr ())->sin_family);

  struct sock_extended_err* sock_err_p = NULL;
  for (struct cmsghdr* cmsghdr_p = CMSG_FIRSTHDR (&msghdr_s);
       cmsghdr_p;
       cmsghdr_p = CMSG_NXTHDR (&msghdr_s, cmsghdr_p))
  {
    if ((cmsghdr_p->cmsg_level != SOL_IP) || // IPPROTO_IP
        (cmsghdr_p->cmsg_type  != IP_RECVERR))
      continue;

    sock_err_p =
        reinterpret_cast<struct sock_extended_err*> (CMSG_DATA (cmsghdr_p));
    if (!sock_err_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%t: failed to retrieve socket error: \"%m\", continuing\n"),
                  inherited::peer_.get_handle ()));
      continue;
    } // end IF

    switch (sock_err_p->ee_origin)
    {
      case SO_EE_ORIGIN_NONE:
        break;
      case SO_EE_ORIGIN_LOCAL:
        break;
      case SO_EE_ORIGIN_ICMP:
      {
        switch (sock_err_p->ee_type)
        {
          case ICMP_NET_UNREACH:
            break;
          case ICMP_HOST_UNREACH:
            break;
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%t: invalid/unknown ICMP error (was: %d), continuing\n"),
                        sock_err_p->ee_type));
            break;
          }
        } // end SWITCH
        break;
      }
      case SO_EE_ORIGIN_ICMP6:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%t: invalid/unknown error origin (was: %d), continuing\n"),
                    sock_err_p->ee_origin));
        break;
      }
    } // end SWITCH
  } // end FOR
}
#endif // ACE_LINUX
