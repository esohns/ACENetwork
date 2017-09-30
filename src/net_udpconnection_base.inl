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

#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        StreamType,
                        TimerManagerType,
                        UserDataType>::Net_UDPConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::Net_UDPConnectionBase_T"));

}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        StreamType,
                        TimerManagerType,
                        UserDataType>::Net_UDPConnectionBase_T ()
 : inherited (NULL,
              ACE_Time_Value (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0))
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::Net_UDPConnectionBase_T"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
int
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        StreamType,
                        TimerManagerType,
                        UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::handle_input"));

  int result = -1;
  ssize_t bytes_received = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_INET_Addr peer_address;

  // sanity check
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_);
  ACE_ASSERT (inherited::CONNECTION_BASE_T::configuration_->streamConfiguration);

  // *TODO*: remove type inference
  message_block_p =
    inherited::allocateMessage (inherited::CONNECTION_BASE_T::configuration_->streamConfiguration->allocatorConfiguration_.defaultBufferSize);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to allocateMessage(%u), aborting\n"),
                this->id (),
                inherited::CONNECTION_BASE_T::configuration_->streamConfiguration->allocatorConfiguration_.defaultBufferSize));
    return -1; // <-- remove 'this' from dispatch
  } // end IF

  // read a datagram from the socket
  bytes_received =
    inherited::peer_.recv (message_block_p->wr_ptr (),   // buffer
                           message_block_p->capacity (), // buffer size
                           peer_address,                 // peer address
                           0);                           // flags
  //bytes_received = inherited::peer_.recv (buffer_p->wr_ptr (),                 // buf
  //                                        inherited2::configuration_->PDUSize, // n
  //                                        0,                                   // flags
  //                                        NULL);                               // timeout
  switch (bytes_received)
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
                    ACE_TEXT ("failed to SocketType::recv(): \"%m\", returning\n")));

      // clean up
      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
      // clean up
      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    default:
    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[0x%@]: received %d byte(s)\n"),
//                  handle_in,
//                  bytes_received));
//#else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[%d]: received %d byte(s)\n"),
//                  handle_in,
//                  bytes_received));
//#endif

      // adjust write pointer
      message_block_p->wr_ptr (static_cast<size_t> (bytes_received));

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

    // clean up
    message_block_p->release ();

    return -1; // <-- remove 'this' from dispatch
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
int
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        StreamType,
                        TimerManagerType,
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
    (unlikely (inherited::CONNECTION_BASE_T::configuration_->socketHandlerConfiguration.useThreadPerConnection) ? inherited::getq (message_block_p, NULL)
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
                  (inherited::CONNECTION_BASE_T::configuration_->socketHandlerConfiguration.useThreadPerConnection ? ACE_TEXT ("%u: failed to ACE_Task::getq(): \"%m\", aborting\n")
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
      inherited::peer_.send (message_block_p->rd_ptr (), // data
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

      // clean up
      message_block_p->release ();
      //writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
      // clean up
      message_block_p->release ();
      //writeBuffer_ = NULL;

      result = -1; // <-- remove 'this' from dispatch

      break;
    }
    default:
    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("0x%@: sent %d bytes\n"),
//                  handle_in,
//                  bytes_sent));
//#else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%d: sent %d bytes\n"),
//                  handle_in,
//                  bytes_sent));
//#endif

      // finished with this buffer ?
      message_block_p->rd_ptr (static_cast<size_t> (bytes_sent));
      ACE_ASSERT (!message_block_p->length ());
      //if (unlikely (message_block_p->length ()))
      //  break; // there's more data

      // clean up
      message_block_p->release ();
      //writeBuffer_ = NULL;

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
          typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                        HandlerType,
                        ConfigurationType,
                        StateType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        StreamType,
                        TimerManagerType,
                        UserDataType>::reset ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_UDPConnectionBase_T::reset"));

  int result = -1;

  result = inherited::HANDLER_T::SVC_HANDLER_T::close ();
  if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_Svc_Handler::close(): \"%m\", continuing\n"),
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
  } // end IF
  inherited::writeHandle_ = ACE_INVALID_HANDLE;

  inherited::open (inherited::CONNECTION_BASE_T::configuration_);
}

/////////////////////////////////////////

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              HandlerConfigurationType,
                              StreamType,
                              TimerManagerType,
                              UserDataType>::Net_AsynchUDPConnectionBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                            const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::Net_AsynchUDPConnectionBase_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              HandlerConfigurationType,
                              StreamType,
                              TimerManagerType,
                              UserDataType>::Net_AsynchUDPConnectionBase_T ()
 : inherited (NULL,
              ACE_Time_Value (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0))
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::Net_AsynchUDPConnectionBase_T"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
int
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              HandlerConfigurationType,
                              StreamType,
                              TimerManagerType,
                              UserDataType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  Stream_Base_t* stream_p = inherited::stream_.upStream ();
  ACE_Message_Block* message_block_p, *message_block_2, *message_block_3 = NULL;
  unsigned int length = 0;
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
  result = inherited::stream_.get (message_block_p, NULL);
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
  message_block_2 = message_block_p;
  do
  {
    length = message_block_2->length ();
    if (length > inherited::PDUSize_)
    {
      message_block_3 = message_block_2->duplicate ();
      if (unlikely (!message_block_3))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));

        // clean up
        message_block_p->release ();

        return -1;
      } // end IF
      message_block_2->cont (message_block_3);

      message_block_2->length (inherited::PDUSize_);
      message_block_3->rd_ptr (inherited::PDUSize_);

      message_block_2 = message_block_3;
      continue;
    } // end IF

    message_block_2 = message_block_2->cont ();
  } while (message_block_2);

  // start (asynchronous) write(s)
  do
  {
    length = 0;
    message_block_2 = message_block_p;
    while (length < inherited::PDUSize_)
    {
      length += message_block_2->length ();
      message_block_2 = message_block_2->cont ();
      if (!message_block_2)
        break;
    } // end WHILE
    if (message_block_2)
    {
      message_block_3 = message_block_p;
      while (message_block_3->cont () != message_block_2)
        message_block_3 = message_block_3->cont ();
      message_block_3->cont (NULL);
    } // end IF

    this->increase ();
    inherited::counter_.increase ();
send:
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
#endif
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::send(%u): \"%m\", aborting\n"),
                  message_block_p->total_length ()));

      // clean up
      message_block_p->release ();
      this->decrease ();
      inherited::counter_.decrease ();

      return -1;
    } // end IF
    message_block_p = message_block_2;
  } while (message_block_p);

  return 0;
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              HandlerConfigurationType,
                              StreamType,
                              TimerManagerType,
                              UserDataType>::info (ACE_HANDLE& handle_out,
                                                   ACE_INET_Addr& localSAP_out,
                                                   ACE_INET_Addr& peerSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchUDPConnectionBase_T::info"));

  int result = -1;
  int error = 0;
  struct Net_UDPSocketConfiguration* socket_configuration_p = NULL;
  const HandlerConfigurationType& handler_configuration_r =
    inherited::getR_2 ();

  socket_configuration_p =
    dynamic_cast<struct Net_UDPSocketConfiguration*> (handler_configuration_r.socketConfiguration);

  // sanity check(s)
  ACE_ASSERT (socket_configuration_p);

  handle_out = inherited::SOCKET_T::get_handle ();
  localSAP_out.reset ();
  peerSAP_out.reset ();

  result = inherited::SOCKET_T::get_local_addr (localSAP_out);
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
  if (unlikely (socket_configuration_p->writeOnly))
    peerSAP_out = inherited::address_;
  else
  {
    result = inherited::SOCKET_T::get_remote_addr (peerSAP_out);
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

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename TimerManagerType,
          typename UserDataType>
void
Net_AsynchUDPConnectionBase_T<HandlerType,
                              ConfigurationType,
                              StateType,
                              StatisticContainerType,
                              HandlerConfigurationType,
                              StreamType,
                              TimerManagerType,
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
