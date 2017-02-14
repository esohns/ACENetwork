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
#include <ace/Stream.h>

#include "common_tools.h"

#include "stream_common.h"

#include "net_defines.h"
#include "net_macros.h"

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                                  const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited4 (interfaceHandle_in,
               statisticCollectionInterval_in)
 , stream_ (ACE_TEXT_ALWAYS_CHAR (NET_STREAM_DEFAULT_NAME))
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::Net_StreamAsynchUDPSocketBase_T"));

}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::~Net_StreamAsynchUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::~Net_StreamAsynchUDPSocketBase_T"));

}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
#if defined (ACE_LINUX)
  bool handle_priviledges = false;
#endif
  bool handle_manager = false;
  bool handle_socket = false;
  //// *TODO*: remove type inferences
  //const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
  //    NULL;
  //const typename StreamType::SESSION_DATA_T* session_data_p = NULL;

  // step1: open socket ?
  // *NOTE*: even when this is a write-only connection
  //         (inherited4::configuration_.socketConfiguration.writeOnly), the
  //         base class still requires a valid handle to open the output stream
  ACE_INET_Addr SAP_any (static_cast<u_short> (0),
                         static_cast<ACE_UINT32> (INADDR_ANY));
  ACE_INET_Addr local_SAP =
    (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly ? SAP_any
                                                                                            : inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->address);
#if defined (ACE_LINUX)
  // (temporarily) elevate priviledges to open system sockets
  if (!inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly &&
      (local_SAP.get_port_number () <= NET_ADDRESS_MAXIMUM_PRIVILEDGED_PORT))
  {
    if (!Common_Tools::setRootPriviledges ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setRootPriviledges(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    handle_priviledges = true;
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
                ACE_TEXT (Net_Common_Tools::IPAddress2String (local_SAP).c_str ())));
    goto error;
  } // end IF
#if defined (ACE_LINUX)
  if (handle_priviledges)
    Common_Tools::dropRootPriviledges ();
#endif
  handle_socket = true;
  inherited::handle (inherited2::get_handle ());

  // step2a: initialize base-class
  if (!inherited::initialize (*inherited4::configuration_->socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_SocketHandlerBase::initialize(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  // step2b: tweak socket, initialize I/O
  inherited::open (inherited::handle (), messageBlock_in);

  // step5: register with the connection manager (if any)
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
  handle_manager = true;

  // step3: initialize/start stream

  // step3a: connect stream head message queue with a notification pipe/queue ?
  if (!inherited4::configuration_->socketHandlerConfiguration->useThreadPerConnection)
    inherited4::configuration_->streamConfiguration->notificationStrategy =
      this;

  // step3d: initialize stream
  // *TODO*: this clearly is a design glitch
  inherited4::configuration_->streamConfiguration->sessionID =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      reinterpret_cast<size_t> (inherited::handle ()); // (== socket handle)
#else
      static_cast<size_t> (inherited::handle ()); // (== socket handle)
#endif
  if (!stream_.initialize (*inherited4::configuration_->streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, aborting\n")));
    goto error;
  } // end IF
  //session_data_container_p = stream_.get ();
  //ACE_ASSERT (session_data_container_p);
  //session_data_p = &session_data_container_p->get ();
  //// *TODO*: remove type inferences
  //const_cast<typename StreamType::SESSION_DATA_T*> (session_data_p)->connectionState =
  //    &const_cast<StateType&> (inherited4::state ());
  //stream_.dump_state ();

  // step3e: start stream
  stream_.start ();
  if (!stream_.isRunning ())
  {
    // *NOTE*: most likely, this happened because the stream failed to
    //         initialize
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, aborting\n")));
    goto error;
  } // end IF

  // step4: start reading (need to pass any data ?)
  if (!inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly)
  {
    if (messageBlock_in.length () == 0)
      inherited::initiate_read_dgram ();
    else
    {
      ACE_Message_Block* message_block_p = messageBlock_in.duplicate ();
      if (!message_block_p)
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
        goto error;
      } // end IF
      // fake a result to emulate regular behavior...
      ACE_Proactor* proactor_p = inherited::proactor ();
      ACE_ASSERT (proactor_p);
      ACE_Asynch_Read_Dgram_Result_Impl* fake_result_p =
        proactor_p->create_asynch_read_dgram_result (inherited::proxy (),                  // handler proxy
                                                     inherited::handle (),                 // socket handle
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
      // invoke ourselves (see handle_read_stream)
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
  if (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly)
    this->decrease (); // float the connection (connection manager)
                       //ACE_ASSERT (this->count () == 2); // connection manager & read operation
                       //                                     (+ stream module(s))

  inherited4::state_.status = NET_CONNECTION_STATUS_OK;

  return;

error:
  stream_.stop (true); // <-- wait for completion

#if defined (ACE_LINUX)
  if (handle_priviledges)
    Common_Tools::dropRootPriviledges ();
#endif

  if (handle_socket)
  {
    result = inherited::handle_close (inherited::handle (),
                                      ACE_Event_Handler::ALL_EVENTS_MASK);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HandlerType::handle_close(): \"%m\", continuing\n")));
  } // end IF

  if (handle_manager)
    inherited4::deregister (); // <-- should 'delete this'

  this->decrease ();
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_output (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_output"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration);

  int result = -1;
  Stream_Base_t* stream_p = (stream_.upStream () ? stream_.upStream ()
                                                 : &stream_);
  ACE_Message_Block* message_block_p = NULL;

  // *IMPORTANT NOTE*: this should NEVER block, as available outbound data has
  //                   been notified
  result = stream_p->get (message_block_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::get(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  ACE_ASSERT (message_block_p);

  // start (asynchronous) write
  this->increase ();
  inherited::counter_.increase ();
  size_t bytes_to_send = message_block_p->length ();
send:
  ssize_t result_2 =
    inherited::outputStream_.send (message_block_p,                      // data
                                   bytes_to_send,                        // #bytes to send
                                   0,                                    // flags
                                   inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->address, // remote address (ignored)
                                   NULL,                                 // ACT
                                   0,                                    // priority
                                   COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (result_2 == -1)
  {
    int error = ACE_OS::last_error ();
    // *WARNING*: this could fail on multi-threaded proactors
    if (error == EAGAIN) goto send; // 11: happens on Linux

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Write_Dgram::send(%u): \"%m\", aborting\n"),
                message_block_p->length ()));

    // clean up
    message_block_p->release ();
    this->decrease ();
    inherited::counter_.decrease ();

    return -1;
  } // end IF

  return 0;
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                                               ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::handle_close"));

  ACE_UNUSED_ARG (handle_in);

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration);

  // step1: wait for all workers within the stream (if any)
  stream_.stop (true,  // <-- wait for completion
                true); // locked access ?

  // step2: purge any pending notifications ?
  // *WARNING: do this here, while still holding on to the current write buffer
  int result = -1;
  if (unlikely (!inherited4::configuration_->socketHandlerConfiguration->useThreadPerConnection))
  {
    Stream_Iterator_t iterator (stream_);
    const Stream_Module_t* module_p = NULL;
    result = iterator.next (module_p);
    if (result == 1)
    {
      ACE_ASSERT (module_p);
      Stream_Task_t* task_p =
          const_cast<Stream_Module_t*> (module_p)->writer ();
      ACE_ASSERT (task_p);
      result = task_p->flush (ACE_Task_Flags::ACE_FLUSHALL);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_MessageQueue::flush(): \"%m\", continuing\n")));
      if (result > 0)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("flushed %d messages...\n")));
    } // end IF
  } // end IF

  // step3: invoke base-class maintenance ?
  // *NOTE*: if the connection has been close()d locally, the socket is already
  //         closed at this point
  if (inherited4::state_.status != NET_CONNECTION_STATUS_CLOSED)
  {
    result = inherited::handle_close (inherited::handle (),
                                      mask_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to SocketHandlerType::handle_close(): \"%m\", continuing\n")));
  } // end IF

  // step4: deregister with the connection manager (if any)
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

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration);

  handle_out = inherited2::get_handle ();
  localSAP_out.reset ();
  remoteSAP_out.reset ();

  result = inherited2::get_local_addr (localSAP_out);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  if (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly)
    remoteSAP_out =
        inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->address;
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Notification_Strategy*
Net_StreamAsynchUDPSocketBase_T<HandlerType,
                                SocketType,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::notification ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::notification"));

  return this;
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                  ACE_TEXT ("failed to SocketType::close(%u): \"%m\", continuing\n"),
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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  //        --> all data has been dispatched (here: to the proactor/kernel)
  stream_.wait (waitForThreads_in,
                false,             // don't wait for upstream modules
                false);            // don't wait for downstream modules

  // step2: wait for the asynchronous operations to complete
  inherited::counter_.wait (0);
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                                         sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    local_address_string = ACE_TEXT_ALWAYS_CHAR (buffer);
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  std::string peer_address_string;
  result = peer_address.addr_to_string (buffer,
                                        sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
  else
    peer_address_string = ACE_TEXT_ALWAYS_CHAR (buffer);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u [%d]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address_string.c_str ()),
              ACE_TEXT (peer_address_string.c_str ())));
}

template <typename HandlerType,
          typename SocketType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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

  switch (result_in.bytes_transferred ())
  {
    //case -1:
    case std::numeric_limits<size_t>::max ():
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
  result_in.message_block ()->release ();

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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                                  const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited4 (interfaceHandle_in,
               statisticCollectionInterval_in)
// , buffer_ (NULL)
 //, userData_ ()
 //, stream_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::Net_StreamAsynchUDPSocketBase_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::~Net_StreamAsynchUDPSocketBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::~Net_StreamAsynchUDPSocketBase_T"));

//  if (buffer_)
//    buffer_->release ();
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
  // step3a: connect stream head message queue with a notification pipe/queue ?
  if (!inherited4::configuration_->streamConfiguration->useThreadPerConnection)
    inherited4::configuration_->streamConfiguration->notificationStrategy =
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

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  ACE_ASSERT (inherited4::configuration_->streamConfiguration);

  // step1: wait for all workers within the stream (if any)
  if (stream_.isRunning ())
    stream_.stop (true); // <-- wait for completion

  // step2: purge any pending notifications ?
  // *WARNING: do this here, while still holding on to the current write buffer
  if (!inherited4::configuration_->streamConfiguration->useThreadPerConnection)
  {
    Stream_Iterator_t iterator (stream_);
    const Stream_Module_t* module_p = NULL;
    result = iterator.next (module_p);
    if (result == 1)
    {
      ACE_ASSERT (module_p);
      Stream_Task_t* task_p =
          const_cast<Stream_Module_t*> (module_p)->writer ();
      ACE_ASSERT (task_p);
      if (task_p->msg_queue ()->flush () == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_MessageQueue::flush(): \"%m\", continuing\n")));
    } // end IF
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

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration);
  ACE_ASSERT (inherited4::configuration_->socketHandlerConfiguration->socketConfiguration);

  handle_out = inherited2::get_handle ();
//  result = localSAP_out.set (static_cast<u_short> (0),
//                             static_cast<ACE_UINT32> (INADDR_NONE));
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::set(0, %d): \"%m\", continuing\n"),
//                INADDR_NONE));
  localSAP_out = ACE_Addr::sap_any;
  if (!inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->writeOnly)
  {
    result = inherited2::get_local_addr (localSAP_out);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SOCK_Dgram::get_local_addr(): \"%m\", continuing\n")));
  } // end IF
  remoteSAP_out =
      inherited4::configuration_->socketHandlerConfiguration->socketConfiguration->peerAddress;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
          typename HandlerConfigurationType,
          typename StreamType,
          typename UserDataType,
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
ACE_Notification_Strategy*
Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_SOCK_Netlink,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>::notification ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamAsynchUDPSocketBase_T::notification"));

  return this;
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
//                                ModuleConfigurationType>::close (u_long arg_in)
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
              ACE_TEXT ("connection [id: %u [%d]]: \"%s\" <--> \"%s\"\n"),
              id (), handle,
              ACE_TEXT (local_address_string.c_str ()),
              buffer));
}

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
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
