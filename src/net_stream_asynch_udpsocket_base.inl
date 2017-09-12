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

#include "ace/Asynch_IO_Impl.h"
#include "ace/Event_Handler.h"
#include "ace/Log_Msg.h"
#include "ace/Proactor.h"

#include "common_tools.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                                  const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited ()
 , inherited2 ()
 , inherited3 ()
 , inherited4 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , stream_ ()
 /////////////////////////////////////////
 , notify_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::Net_StreamAsynchUDPSocketBase_T"));

}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::reset ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::reset"));

  ACE_Message_Block message_block;
  ACE_HANDLE handle_h = inherited::SVC_HANDLER_T::handle ();

  int result = inherited::handle_close (handle_h,
                                        ACE_Event_Handler::SIGNAL_MASK);
  if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@): \"%m\", continuing\n"),
                  handle_h));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d): \"%m\", continuing\n"),
                  handle_h));
#endif
  result = inherited2::close ();
  if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(0x%@): \"%m\", continuing\n"),
                  handle_h));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(%d): \"%m\", continuing\n"),
                  handle_h));
#endif

  OWN_TYPE_T::open (ACE_INVALID_HANDLE,
                    message_block);
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::open (ACE_HANDLE handle_in,
                                                                       ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::open"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  // *TODO*: remove type inferences
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration.socketConfiguration);
  ACE_ASSERT (inherited4::configuration_->streamConfiguration);

  struct Net_UDPSocketConfiguration* socket_configuration_p =
        dynamic_cast<struct Net_UDPSocketConfiguration*> (inherited4::configuration_->socketHandlerConfiguration.socketConfiguration);
  int result = -1;
#if defined (ACE_LINUX)
  bool handle_privileges = false;
#endif
  bool handle_manager = false;
  bool handle_socket = false;
  bool decrease_reference_count = socket_configuration_p->writeOnly;

  // sanity check(s)
  ACE_ASSERT (socket_configuration_p);

  // step1: open socket ?
  // *NOTE*: even when this is a write-only connection
  //         (inherited4::configuration_.socketConfiguration.writeOnly), the
  //         base class still requires a valid handle to open the output stream
  AddressType SAP_any (static_cast<u_short> (0),
                       static_cast<ACE_UINT32> (INADDR_ANY));
  AddressType local_SAP =
    (socket_configuration_p->writeOnly ? (socket_configuration_p->sourcePort ? ACE_INET_Addr (static_cast<u_short> (socket_configuration_p->sourcePort),
                                                                                              static_cast<ACE_UINT32> (INADDR_ANY))
                                                                             : ACE_sap_any_cast (const ACE_INET_Addr&))
                                       :  socket_configuration_p->address);
#if defined (ACE_LINUX)
  // (temporarily) elevate priviledges to open system sockets
  if (!socket_configuration_p->writeOnly &&
      (local_SAP.get_port_number () <= NET_ADDRESS_MAXIMUM_PRIVILEGED_PORT))
  {
    if (!Common_Tools::setRootPrivileges ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setRootPrivileges(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    handle_privileges = true;
  } // end IF
#endif
  result =
    inherited2::open (local_SAP,                // local SAP
                      ACE_PROTOCOL_FAMILY_INET, // protocol family
                      0,                        // protocol
                      1);                       // reuse_addr
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketType::open(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (local_SAP).c_str ())));
    goto error;
  } // end IF
#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::dropRootPrivileges ();
#endif
  handle_socket = true;
  inherited::handle (inherited2::get_handle ());
  inherited4::state_.handle = inherited2::get_handle ();

  // step2a: initialize base-class
  if (!inherited::initialize (inherited4::configuration_->socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketHandlerBase::initialize(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  // step2b: tweak socket, initialize I/O
  inherited::open (inherited2::get_handle (),
                   messageBlock_in);

  // step5: register with the connection manager (if any) ?
  if (!inherited4::isRegistered_)
  {
#if defined (__GNUG__)
    if (!inherited4::registerc (this))
#else
    if (!inherited4::registerc ())
#endif
    {
      // *NOTE*: perhaps max# connections has been reached
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
      goto error;
    } // end IF
  } // end IF
  handle_manager = true;

  // step3: initialize/start stream ?
  // step3a: connect the stream head message queue with this handler; the queue
  //         will forward outbound data to handle_output ()
  inherited4::configuration_->streamConfiguration->configuration_.notificationStrategy =
    this;

  // step3d: initialize stream ?
  if (stream_.isRunning ())
  {
    // *NOTE*: connection already initialized, do not adjust the reference count
    decrease_reference_count = false;
  } // end IF
  else
  {
    if (!stream_.initialize (*inherited4::configuration_->streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize processing stream, aborting\n")));
      goto error;
    } // end IF
    // update session data
    const typename StreamType::SESSION_DATA_CONTAINER_T& session_data_container_r =
      stream_.getR ();
    typename StreamType::SESSION_DATA_T& session_data_r =
      const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_r.getR ());
    ACE_ASSERT (session_data_r.lock);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
      session_data_r.connectionState = &(inherited4::state_);
    } // end lock scope
    //stream_.dump_state ();

    // step3e: start stream
    stream_.start ();
    if (!stream_.isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start processing stream, aborting\n")));
      goto error;
    } // end IF
  } // end IF

  // step4: start reading ?
  if (!socket_configuration_p->writeOnly)
  {
    if (messageBlock_in.length () == 0)
      inherited::initiate_read_dgram ();
    else
    {
      // forward data argument
      ACE_Message_Block* message_block_p = messageBlock_in.duplicate ();
      if (!message_block_p)
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
        goto error;
      } // end IF
      // fake a result to emulate regular behavior
      ACE_Proactor* proactor_p = inherited::proactor ();
      ACE_ASSERT (proactor_p);
      ACE_Asynch_Read_Dgram_Result_Impl* fake_result_p =
        proactor_p->create_asynch_read_dgram_result (inherited::proxy (),                  // handler proxy
                                                     inherited2::get_handle (),            // socket handle
                                                     message_block_p,                      // buffer
                                                     message_block_p->size (),             // #bytes to read
                                                     0,                                    // flags
                                                     PF_INET,                              // protocol family
                                                     NULL,                                 // ACT
                                                     ACE_INVALID_HANDLE,                   // event
                                                     0,                                    // priority
                                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
      if (!fake_result_p)
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Proactor::create_asynch_read_dgram_result: \"%m\", aborting\n")));
        goto error;
      } // end IF
      size_t bytes_transferred = message_block_p->length ();
      // <complete> for Accept would have already moved the <wr_ptr>
      // forward; update it to the beginning position
      message_block_p->wr_ptr (message_block_p->wr_ptr () - bytes_transferred);
      // invoke 'this' (see handle_read_stream())
      fake_result_p->complete (message_block_p->length (), // bytes read
                               1,                          // success
                               NULL,                       // ACT
                               0);                         // error

      // clean up
      delete fake_result_p;
    } // end ELSE
    // *NOTE*: registered with the proactor at this point
    //         --> data may start arriving at handle_input ()
  } // end IF

  // step5: 'float' the connection
  if (decrease_reference_count)
  {
    this->decrease ();
    //ACE_ASSERT (this->count () == 1); // connection manager (+ stream module(s))
  } // end IF
  else
  {
    //ACE_ASSERT (this->count () == 2); // connection manager + read operation
    //                                     (+ stream module(s))
  } // end ELSE

  inherited4::state_.status = NET_CONNECTION_STATUS_OK;

  return;

error:
  stream_.stop (true); // <-- wait for completion

#if defined (ACE_LINUX)
  if (handle_privileges)
    Common_Tools::dropRootPrivileges ();
#endif

  if (handle_socket)
  {
    result = inherited::handle_close (inherited4::state_.handle,
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(): \"%m\", continuing\n")));
  } // end IF
  inherited4::state_.handle = ACE_INVALID_HANDLE;
  inherited4::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

  if (handle_manager)
    inherited4::deregister (); // <-- should 'delete this'

  if (decrease_reference_count)
    this->decrease ();
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  Stream_Base_t* upstream_p = stream_.upStream ();
  Stream_Base_t* stream_p = (upstream_p ? upstream_p : &stream_);
  ACE_Message_Block* message_block_p, *message_block_2, *message_block_3 = NULL;
  size_t bytes_sent = 0;
  ssize_t result_2 = 0;

  // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
  //                   been notified
  result = stream_p->get (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  ACE_ASSERT (message_block_p);

  // fragment the data ?
  message_block_2 = message_block_p;
  unsigned int length = 0;
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
    result_2 =
      inherited::outputStream_.send (message_block_p,                      // data
                                     bytes_sent,                           // #bytes sent
                                     0,                                    // flags
                                     inherited::address_,                  // remote address (ignored)
                                     NULL,                                 // ACT
                                     0,                                    // priority
                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
    if (unlikely (result_2 == -1))
    {
      int error = ACE_OS::last_error ();
      // *WARNING*: this could fail on multi-threaded proactors
      if (error == EAGAIN)
        goto send; // 11: happens on Linux

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
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                               ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;

  // *IMPORTANT NOTE*: when control reaches here, the socket handle has already
  //                   gone away, i.e. no new data will be accepted by the
  //                   kernel / forwarded by the proactor
  //                   --> finish processing: flush all remaining outbound data
  //                       and wait for all workers within the stream
  //                   [--> cancel any pending asynchronous operations]

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
  // *TODO*: consider cancel()ling any pending write operations

  // step2: invoke base-class maintenance
  // *NOTE*: if the connection has been close()d locally, the socket is already
  //         closed at this point
  result = inherited::handle_close (inherited::handle (),
                                    mask_in);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::handle_close(): \"%m\", continuing\n")));

  // step3: deregister with the connection manager (if any)
  if (likely (inherited4::isRegistered_))
    inherited4::deregister ();

  return result;
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                       AddressType& localSAP_out,
                                                                       AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::info"));

  int result = -1;
  int error = 0;

  handle_out = inherited2::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out.reset ();

  result = inherited2::get_local_addr (localSAP_out);
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
  if (likely (inherited::writeOnly_))
    remoteSAP_out = inherited::address_;
  else
  {
    result = inherited2::get_remote_addr (remoteSAP_out);
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
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_ConnectionId_t
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::id"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return reinterpret_cast<Net_ConnectionId_t> (inherited::handle ());
#else
  return static_cast<Net_ConnectionId_t> (inherited::handle ());
#endif
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

  ACE_UNUSED_ARG (arg_in);

  // *TODO*: find a better way to do this
  inherited4* inherited_p = this;
  inherited_p->close ();

  return 0;
}
template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

  int result = -1;

  // step1: shutdown operations
  ACE_HANDLE handle = inherited::handle ();

  // *TODO*: remove type inference
  if (inherited::writeOnly_)
    result = handle_close (handle,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
  else
    result = inherited::handle_close (handle,
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(): \"%m\", continuing\n")));

  //  step2: release the socket handle
  if (handle != ACE_INVALID_HANDLE)
  {
    result = inherited2::close ();
    if (result == -1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to SocketType::close(0x%@): \"%m\", continuing\n"),
                  reinterpret_cast<size_t> (handle)));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to SocketType::close(%d): \"%m\", continuing\n"),
                  handle));
#endif
    //inherited2::set_handle (handle); // debugging purposes only !
  } // end IF

  // *TODO*: remove type inference
  inherited4::state_.status = NET_CONNECTION_STATUS_CLOSED;
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::waitForCompletion"));

  // *WARNING*: this assumes that no new data will be dispatched to the stream
  //            in the meantime (i.e. make sure the stream has been stop()ped,
  //            or this will:
  //            - {waitForThreads_in && stream thread count > 0}: block
  //            - otherwise: wait for the next idle period)

  // step1: wait for the stream to flush
  stream_.wait (waitForThreads_in,
                false,             // don't wait for upstream modules
                false);            // don't wait for downstream modules
  // --> all data has been dispatched (here: to the proactor/kernel)

  // step2: wait for any scheduled asynchronous operations to complete
  inherited::counter_.wait (0);
  // --> all data has been sent
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::dump_state"));

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
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::collect"));

  try {
    return stream_.collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::report"));

  try {
    return stream_.report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_read_dgram (const ACE_Asynch_Read_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_read_dgram"));

  int result = -1;
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection closed/reset ? --> not an error
    error = result_in.error ();
    if ((error != EBADF)                   && // 9    : local close (), happens on Linux
        (error != EPIPE)                   && // 32
        (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
        (error != ECONNRESET))                // 10054: happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (0x%@): \"%s\", aborting\n"),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (%d): \"%s\", aborting\n"),
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
      if ((error != EBADF)                   && // 9    : local close (), happens on Linux
          (error != EPIPE)                   && // 32
          (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
          (error != ECONNRESET))                // 10054: happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to read from input stream (0x%@): \"%s\", aborting\n"),
                      result_in.handle (),
                      ACE::sock_error (static_cast<int> (error))));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to read from input stream (%d): \"%s\", aborting\n"),
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
//                   result_in.handle (),
//                   result_in.bytes_transferred ()));

      // push the buffer onto the stream for processing
      result = stream_.put (result_in.message_block (), NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", aborting\n")));
        break;
      } // end IF

      // start next read
      if (!inherited::initiate_read_dgram ())
      {
        error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if ((error != ENXIO)               && // happens on Win32
            (error != EFAULT)              && // *TODO*: happens on Win32
            (error != ERROR_UNEXP_NET_ERR) && // *TODO*: happens on Win32
            (error != ERROR_NETNAME_DELETED)) // happens on Win32
#else
        if (error)
#endif
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to HandlerType::initiate_read_dgram(): \"%m\", aborting\n")));
        break;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  ACE_Message_Block* message_block_p = result_in.message_block ();
  if (message_block_p)
    message_block_p->release ();

  result = handle_close (inherited::handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Svc_Handler::handle_close(): \"%m\", continuing\n")));

  this->decrease ();
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_write_dgram (const ACE_Asynch_Write_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_write_dgram"));

  inherited::handle_write_dgram (result_in);
  this->decrease ();
}

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                                  const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited4 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , stream_ ()
 /////////////////////////////////////////
 , notify_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::Net_StreamAsynchUDPSocketBase_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::open (ACE_HANDLE handle_in,
                                                                       ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::open"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration);
  ACE_ASSERT (inherited4::configuration_->streamConfiguration);

  int result = -1;
  // *TODO*: remove type inferences
  const typename StreamType::SESSION_DATA_T* session_data_p = NULL;

  // step1: open socket
  result =
      inherited2::open (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->netlinkAddress,   // local SAP
                        ACE_PROTOCOL_FAMILY_NETLINK,                                                                   // protocol family
                        inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->netlinkProtocol); // protocol
                        // NETLINK_USERSOCK);                                             // protocol
                        //NETLINK_GENERIC);                                              // protocol
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    std::string local_address;
    // *TODO*: find a replacement for ACE_INET_Addr::addr_to_string
//    if (local_SAP.addr_to_string (buffer,
//                                  sizeof (buffer)) == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    local_address = buffer;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to SocketType::open(%s): \"%m\", aborting\n"),
                ACE_TEXT (local_address.c_str ())));
    goto close;
  } // end IF

  // step2a: initialize base-class
  if (!inherited::initialize (*inherited4::configuration_->socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketHandlerBase::initialize(): \"%m\", aborting\n")));
    goto close;
  } // end IF

  // step2b: tweak socket, initialize I/O
  inherited::open (inherited2::get_handle (), messageBlock_in);

  // step3: initialize/start stream
  // step3a: connect the stream head message queue with this handler; the queue
  //         will forward outbound data to handle_output ()
  inherited4::configuration_->streamConfiguration->configuration_.notificationStrategy =
    this;

  // *TODO*: this clearly is a design glitch
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited4::configuration_->streamConfiguration->sessionID =
    reinterpret_cast<unsigned int> (inherited2::get_handle ()); // (== socket handle)
#else
  inherited4::configuration_->streamConfiguration->sessionID =
    static_cast<unsigned int> (inherited2::get_handle ()); // (== socket handle)
#endif
  if (!stream_.initialize (*inherited4::configuration_->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto close;
  } // end IF
  session_data_p = &stream_.sessionData ();
  const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
      &const_cast<StateType&> (inherited4::state ());
  //stream_.dump_state ();
  // *NOTE*: as soon as this returns, data starts arriving at handle_output()[/msg_queue()]
  stream_.start ();
  if (!stream_.isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto close;
  } // end IF

  // step4: start reading (need to pass any data ?)
  if (messageBlock_in.length () == 0)
    inherited::initiate_read_dgram ();
  else
  {
    ACE_Message_Block* message_block_p = messageBlock_in.duplicate ();
    if (!message_block_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
      goto close;
    } // end IF
    // fake a result to emulate regular behavior...
    ACE_Proactor* proactor_p = inherited::proactor ();
    ACE_ASSERT (proactor_p);
    ACE_Asynch_Read_Dgram_Result_Impl* fake_result_p =
      proactor_p->create_asynch_read_dgram_result (inherited::proxy (),                  // handler proxy
                                                   inherited2::get_handle (),            // socket handle
                                                   message_block_p,                      // buffer
                                                   message_block_p->size (),             // #bytes to read
                                                   0,                                    // flags
                                                   PF_INET,                              // protocol family
                                                   NULL,                                 // ACT
                                                   ACE_INVALID_HANDLE,                   // event
                                                   0,                                    // priority
                                                   COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
    if (!fake_result_p)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Proactor::create_asynch_read_dgram_result: \"%m\", aborting\n")));
      goto close;
    } // end IF
    size_t bytes_transferred = message_block_p->length ();
    // <complete> for Accept would have already moved the <wr_ptr>
    // forward; update it to the beginning position
    message_block_p->wr_ptr (message_block_p->wr_ptr () - bytes_transferred);
    // invoke ourselves (see handle_read_stream)
    fake_result_p->complete (message_block_p->length (), // bytes read
                             1,                          // success
                             NULL,                       // ACT
                             0);                         // error

    // clean up
    delete fake_result_p;
  } // end ELSE

  // step5: register with the connection manager (if any)
  if (!inherited4::registerc ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
    goto close;
  } // end IF

  return;

close:
  result = handle_close (inherited2::get_handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;
  ssize_t result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
//  if (buffer_ == NULL)
//  {
  // send next data chunk from the stream...
  // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
  // been notified
  //  result = stream_.get (buffer_, &ACE_Time_Value::zero);
  result = stream_.get (message_block_p,
                        &const_cast<ACE_Time_Value&> (ACE_Time_Value::zero));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
    return -1;
  } // end IF
//  } // end IF

  // start (asynchronous) write
  // *NOTE*: this is a fire-and-forget API for message_block...
  size_t bytes_to_send = message_block_p->size ();
  ACE_Netlink_Addr peer_address;
  peer_address.set (0, 0); // send to kernel
//  result_2 = inherited::outputStream_.send (buffer_,     // data
  result_2 =
      inherited::outputStream_.send (message_block_p,                      // data
                                     bytes_to_send,                        // #bytes to send
                                     0,                                    // flags
                                     peer_address,                         // peer address
                                     NULL,                                 // ACT
                                     0,                                    // priority
                                     COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::send(%u): \"%m\", aborting\n"),
                message_block_p->size ()));

    // clean up
//    buffer_->release ();
//    buffer_ = NULL;
    message_block_p->release ();

    return -1;
  } // end IF

  return 0;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                               ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);

  int result = -1;

  // step1: wait for all workers within the stream (if any)
  if (stream_.isRunning ())
    stream_.stop (true); // <-- wait for completion

  // step2: purge any pending data
  Stream_Iterator_t iterator (stream_);
  const Stream_Module_t* module_p = NULL;
  result = iterator.next (module_p);
  if (unlikely (result == 1))
  {
    ACE_ASSERT (module_p);
    Stream_Task_t* task_p =
        const_cast<Stream_Module_t*> (module_p)->writer ();
    ACE_ASSERT (task_p);
    if (task_p->msg_queue ()->flush () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_MessageQueue::flush(): \"%m\", continuing\n")));
  } // end IF

  // step3: invoke base-class maintenance
  result = inherited::handle_close (inherited2::get_handle (),
                                    mask_in);
  if (result == -1)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to SocketHandlerType::handle_close(): \"%m\", continuing\n")));

  // step4: deregister with the connection manager (if any)
  inherited4::deregister ();

  // step5: release a reference
  // *IMPORTANT NOTE*: may 'delete this'
  //inherited4::decrease ();
  this->decrease ();

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::info (ACE_HANDLE& handle_out,
                                                                       AddressType& localSAP_out,
                                                                       AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::info"));

  int result = -1;
  int error = 0;

  handle_out = inherited2::get_handle ();
//  result = localSAP_out.set (static_cast<u_short> (0),
//                             static_cast<ACE_UINT32> (INADDR_NONE));
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::set(0, %d): \"%m\", continuing\n"),
//                INADDR_NONE));
  localSAP_out = ACE_Addr::sap_any;
  if (likely (!inherited4::writeOnly_))
  {
    result = inherited2::get_local_addr (localSAP_out);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      if (error != EBADF) // 9: Linux: socket already closed
#endif
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_SOCK_Netlink::get_local_addr(): \"%m\", continuing\n")));
    } // end IF
  } // end IF
  remoteSAP_out = inherited4::address_;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
Net_ConnectionId_t
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::id () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::id"));

  return static_cast<Net_ConnectionId_t> (inherited::handle ());
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
int
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

  ACE_UNUSED_ARG (arg_in);

  this->close ();

  return 0;
}
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::close"));

//  ACE_UNUSED_ARG (arg_in);
  int result = -1;

  // step1: shutdown operations
  ACE_HANDLE handle = inherited::handle ();
  // *NOTE*: may 'delete this'
  result = handle_close (handle,
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", aborting\n")));
                ACE_TEXT ("failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n")));

  //  step2: release the socket handle
  if (handle != ACE_INVALID_HANDLE)
  {
    int result_2 = ACE_OS::closesocket (handle);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::closesocket(%u): \"%m\", continuing\n"),
                  reinterpret_cast<size_t> (handle)));
//    inherited::handle (ACE_INVALID_HANDLE);
  } // end IF

//  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  //        --> all data has been dispatched (here: to the proactor/kernel)
  stream_.waitForCompletion (waitForThreads_in,
                             true);

  // step2: wait for the asynchronous operations to complete
  inherited::counter_.wait (0);
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
bool
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::collect"));

  try {
    return stream_.collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::collect(), aborting\n")));
  }

  return false;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::report"));

  try {
    return stream_.report ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic::report(), aborting\n")));
  }
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::dump_state"));

  int result = -1;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string local_address_string;
  result = local_address.addr_to_string (buffer,
                                         sizeof (buffer),
                                         1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address_string = buffer;
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = peer_address.addr_to_string (buffer,
                                        sizeof (buffer),
                                        1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (local_address_string.c_str ()),
              buffer));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
void
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                StatisticHandlerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_read_dgram (const ACE_Asynch_Read_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_read_dgram"));

  int result = -1;
  unsigned long error = 0;

  // sanity check
  result = result_in.success ();
  if (result == 0)
  {
    // connection closed/reset ? --> not an error
    error = result_in.error ();
    if ((error != EBADF)                   && // 9    : local close (), happens on Linux
        (error != EPIPE)                   && // 32
        (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
        (error != ECONNRESET))                // 10054: happens on Win32
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to read from input stream (%d): %u --> \"%s\", aborting\n"),
                  result_in.handle (),
                  error, ACE_TEXT (ACE_OS::strerror (error))));
  } // end IF

  switch (result_in.bytes_transferred ())
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      error = result_in.error ();
      if ((error != EBADF)                   && // 9    : local close (), happens on Linux
          (error != EPIPE)                   && // 32
          (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
          (error != ECONNRESET))                // 10054: happens on Win32
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to read from input stream (%d): %u --> \"%s\", aborting\n"),
                    result_in.handle (),
                    error, ACE_TEXT (ACE_OS::strerror (error))));
      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%u]: socket was closed by the peer...\n"),
//                   handle_));
      break;
    }
    default:
    {
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("[%d]: received %u bytes...\n"),
//                   result_in.handle (),
//                   result_in.bytes_transferred ()));

      // push the buffer onto our stream for processing
      result = stream_.put (result_in.message_block (), NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::put(): \"%m\", aborting\n")));
        break;
      } // end IF

      // start next read
      inherited::initiate_read_dgram ();

      return;
    }
  } // end SWITCH

  // clean up
  result_in.message_block ()->release ();

  result = handle_close (inherited::handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Svc_Handler::handle_close(): \"%m\", continuing\n")));
}

#endif
