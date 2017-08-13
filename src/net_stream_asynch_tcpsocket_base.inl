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
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::Net_StreamAsynchTCPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                                  const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited ()
 , inherited2 ()
 , inherited3 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , stream_ ()
 , notify_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::Net_StreamAsynchTCPSocketBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::open (ACE_HANDLE handle_in,
                                                                       ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::open"));

  int result = -1;
  bool handle_manager = false;
  bool handle_socket = false;
  // *TODO*: remove type inferences
  const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
      NULL;
  typename StreamType::SESSION_DATA_T* session_data_p = NULL;

  // step1: initialize base-class, tweak socket, initialize I/O, ...
  ACE_ASSERT (inherited3::configuration_);
  ACE_ASSERT (inherited3::configuration_->streamConfiguration);

  // *TODO*: remove type inferences
  if (unlikely (!inherited::initialize (inherited3::configuration_->socketHandlerConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::initialize(), aborting\n")));
    goto error;
  } // end IF
  inherited::open (handle_in, messageBlock_in);
  handle_socket = true;

  // step4: register with the connection manager (if any)
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (!inherited3::registerc ())
//#else
  if (unlikely (!inherited3::registerc (this)))
//#endif
  {
    // *NOTE*: (most probably) maximum number of connections has been reached
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    goto error;
  } // end IF
  handle_manager = true;

  // step2: initialize/start stream
  // step2a: connect the stream head message queue with this handler; the queue
  //         will forward outbound data to handle_output ()
  inherited3::configuration_->streamConfiguration->configuration_.notificationStrategy =
    this;

  // *TODO*: remove type inferences
  inherited3::configuration_->streamConfiguration->configuration_.sessionID =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    reinterpret_cast<size_t> (handle_in); // (== socket handle)
#else
    static_cast<size_t> (handle_in); // (== socket handle)
#endif
  if (unlikely (!stream_.initialize (*(inherited3::configuration_->streamConfiguration))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto error;
  } // end IF
  session_data_container_p = &stream_.get ();
  ACE_ASSERT (session_data_container_p);
  session_data_p =
    &const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->get ());
  session_data_p->connectionState =
    &const_cast<StateType&> (inherited3::state ());
  //stream_.dump_state ();

  // step2d: start stream
  stream_.start ();
  if (unlikely (!stream_.isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto error;
  } // end IF

  // step3: start reading (need to pass any data ?)
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

  inherited3::state_.status = NET_CONNECTION_STATUS_OK;

  return;

error:
  stream_.stop (true,  // wait for completion ?
                true); // locked access ?

  if (handle_socket)
  {
    result = inherited::handle_close (inherited::handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(): \"%m\", continuing\n")));
  } // end IF

  if (handle_manager)
    inherited3::deregister ();

  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  int error = 0;
  Stream_Base_t* stream_p = stream_.upStream ();
  ACE_Message_Block* message_block_p = NULL;

  // sanity check(s)
  if (likely (!stream_p))
    stream_p = &stream_;
  ACE_ASSERT (stream_p);

//  if (!inherited::buffer_)
//  {
    // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
    //                   been notified
//    result = stream_p->get (inherited::buffer_, NULL);
    result = stream_p->get (message_block_p, NULL);
    if (unlikely (result == -1))
    { // *NOTE*: most probable reason: socket has been closed by the peer, which
      //         close()s the processing stream (see: handle_close()),
      //         shutting down the message queue
      error = ACE_OS::last_error ();
      if (error != ESHUTDOWN) // 108: happens on Linux
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
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
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
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

  // step0a: set state
  // *IMPORTANT NOTE*: set the state early to avoid deadlock in
  //                   waitForCompletion() (see below), which may be implicitly
  //                   invoked by stream_.finished()
  // *TODO*: remove type inference
  if (inherited3::state_.status != NET_CONNECTION_STATUS_CLOSED)
    inherited3::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;

  // step0b: notify stream ?
  if (likely (notify_))
  {
    notify_ = false;
    stream_.notify (STREAM_SESSION_MESSAGE_DISCONNECT,
                    true);
  } // end IF

  // step1: shut down the processing stream
  stream_.flush (false,  // do not flush inbound data
                 false,  // do not flush session messages
                 false); // flush upstream (if any)
  stream_.wait (true,   // wait for worker(s) (if any)
                false,  // wait for upstream (if any)
                false); // wait for downstream (if any)

  // *NOTE*: pending socket operations are notified by the kernel and will
  //         return automatically
  // *TODO*: consider cancel()ling pending write operations

  // step2: invoke base-class maintenance
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (unlikely (result == -1))
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::handle_close(): \"%m\", continuing\n")));

  // step3: deregister with the connection manager (if any)
  if (likely (inherited3::isRegistered_))
    inherited3::deregister ();

  return result;
}

//template <typename ConfigurationType,
//          typename UserDataType,
//          typename StateType,
//          typename ITransportLayerType,
//          typename StatisticContainerType,
//          typename StreamType,
//          typename HandlerType>
//void
//Net_StreamAsynchTCPSocketBase_T<ConfigurationType,
//                                UserDataType,
//                                StateType,
//                                ITransportLayerType,
//                                StatisticContainerType,
//                                StreamType,
//                                HandlerType>::act (const void* act_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::act"));
//
//  //inherited2::configuration_ = *reinterpret_cast<ConfigurationType*> (const_cast<void*> (act_in));
//}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                       AddressType& localSAP_out,
                                                                       AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::info"));

  handle_out = inherited::handle ();
  localSAP_out = inherited::localSAP_;
  remoteSAP_out = inherited::remoteSAP_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_ConnectionId_t
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<Net_ConnectionId_t> (inherited::handle ());
#else
  return static_cast<Net_ConnectionId_t> (inherited::handle ());
#endif
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::close"));

  ACE_UNUSED_ARG (arg_in);

  // *TODO*: find a better way to do this
  //inherited3::close ();
  inherited3* iconnection_p = this;
  iconnection_p->close ();

  return 0;
}
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
//                                ModuleConfigurationType>::close (u_long arg_in)
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::close"));

//  ACE_UNUSED_ARG (arg_in);
  int result = -1;

  // step1: cancel i/o operation(s), release (write) socket handle, ...
  ACE_HANDLE handle = inherited::handle ();
  result = inherited::handle_close (handle,
                                    ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
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
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_AsynchTCPSocketHandler_T::handle_close(): \"%m\", continuing\n")));
  } // end IF

  // *IMPORTANT NOTE*: wake up the open read operation; it will shutdown the
  //                   connection
  // *TODO*: simple cancellation should be enough (see below, line 462)
  //         --> try with CancelIoEx() ?
  //  step2: release the socket handle
  if (handle != ACE_INVALID_HANDLE)
  {
    result = ACE_OS::closesocket (handle);
    if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
                  reinterpret_cast<size_t> (handle)));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                  handle));
#endif
    //inherited::handle (ACE_INVALID_HANDLE);
  } // end IF

  // *TODO*: remove type inference
  inherited3::state_.status = NET_CONNECTION_STATUS_CLOSED;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::waitForCompletion"));

  // step1: wait until the stream becomes idle
  stream_.wait (false,  // don't wait for any worker thread(s)
                false,  // don't wait for upstream modules
                false); // don't wait for downstream modules
  // --> stream data has been processed

  // step2: wait for any asynchronous operations to complete ?
  if (inherited3::state_.status == NET_CONNECTION_STATUS_OK)
  {
    inherited::counter_.wait (0);
    // --> all data has been dispatched to the kernel (socket)

    // *TODO*: different platforms may implement methods by which successful
    //         placing of the data onto the wire can be established
    //         (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
    ACE_HANDLE handle = inherited::handle ();
    if (handle != ACE_INVALID_HANDLE)
    {
      bool no_delay = Net_Common_Tools::getNoDelay (handle);
      Net_Common_Tools::setNoDelay (handle, true);
      Net_Common_Tools::setNoDelay (handle, no_delay);
    } // end IF
#endif
  } // end IF

  // step3: wait for stream processing to complete ?
  if (waitForThreads_in)
    stream_.wait (true,
                  false,  // don't wait for upstream modules
                  false); // don't wait for downstream modules
  // --> stream processing has finished
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::collect"));

  try {
    return stream_.collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::report"));

  try {
    stream_.report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchTCPSocketBase_T::dump_state"));

  // debug info
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
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
//                   ACE_TEXT ("[%d]: received %u bytes...\n"),
//                   result.handle (),
//                   result.bytes_transferred ()));

      // push the buffer onto the stream for processing
      result = stream_.put (&result_in.message_block (), NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", returning\n")));
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
          typename StatisticHandlerType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchTCPSocketBase_T<HandlerType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
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
  if (inherited3::state_.status == NET_CONNECTION_STATUS_OK)
  {
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
  } // end IF

continue_:
  this->decrease ();
}
