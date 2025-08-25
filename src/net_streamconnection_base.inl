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
#include "ace/Svc_Handler.h"
#include "ace/Task.h"
#include "ace/Thread.h"
#include "ace/Time_Value.h"

#include "stream_common.h"
#include "stream_defines.h"
#include "stream_itask.h"

#include "stream_net_common.h"

#include "net_connection_configuration.h"
#include "net_common_tools.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::Net_StreamConnectionBase_T (bool managed_in)
 : inherited ()
 , inherited2 (managed_in)
 , allocator_ (NULL)
 , sendLock_ ()
 , stream_ ()
 , writeBuffer_ (NULL)
 , notify_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::Net_StreamConnectionBase_T"));

}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::~Net_StreamConnectionBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::~Net_StreamConnectionBase_T"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);

  // wait for any worker(s)
  // *TODO*: remove type inference
  if (unlikely (inherited2::configuration_->useThreadPerConnection))
  {
    result = ACE_Task_Base::wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_Task_Base::wait(): \"%m\", continuing\n"),
                  id ()));
  } // end IF

  if (unlikely (writeBuffer_))
    writeBuffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::open"));

  // *IMPORTANT NOTE*: this ensures that 'this' will not be 'delete'd while in
  //                   this method --> make sure to decrease the count again
  inherited2::increase ();

  int result = -1;
  bool handle_reactor = false;
  bool handle_manager = false;
  bool handle_stream = false;
  AddressType local_SAP;
  typename inherited2::ICONNECTOR_T* iconnector_p = NULL;
  typename inherited2::ILISTENER_T* ilistener_p = NULL;
  AddressType peer_SAP;
  enum Net_TransportLayerType transport_layer_e = this->transportLayer ();
  const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  typename StreamType::SESSION_DATA_T* session_data_p = NULL;
  ConfigurationType* configuration_p = NULL;
  bool is_udp_write_only_b = false;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock, -1);
    inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZING;
  } // end lock scope

  // step0: initialize connection base
  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener

  switch (transport_layer_e)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    {
      switch (this->role ())
      {
        case NET_ROLE_CLIENT:
        {
          iconnector_p =
              static_cast<typename inherited2::ICONNECTOR_T*> (arg_in);
          ACE_ASSERT (iconnector_p);
          configuration_p =
              &const_cast<ConfigurationType&> (iconnector_p->getR ());
          break;
        }
        case NET_ROLE_SERVER:
        {
          ilistener_p = static_cast<typename inherited2::ILISTENER_T*> (arg_in);
          ACE_ASSERT (ilistener_p);
          configuration_p =
              &const_cast<ConfigurationType&> (ilistener_p->getR ());

          inherited2::listener_ = ilistener_p;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: invalid/unknown role (was: %d), aborting\n"),
                      id (),
                      this->role ()));
          goto error;
        }
      } // end SWITCH
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      iconnector_p = static_cast<typename inherited2::ICONNECTOR_T*> (arg_in);
      ACE_ASSERT (iconnector_p);
      configuration_p =
        &const_cast<ConfigurationType&> (iconnector_p->getR ());
      Net_UDPSocketConfiguration_t* socket_configuration_p =
        (Net_UDPSocketConfiguration_t*)&configuration_p->socketConfiguration;
      is_udp_write_only_b = socket_configuration_p->writeOnly;

      if (!is_udp_write_only_b) // server-side ?
        inherited2::connector_ = iconnector_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), aborting\n"),
                  id (),
                  transport_layer_e));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (configuration_p);
  if (unlikely (!inherited2::initialize (*configuration_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_ConnectionBase_T::initialize(): \"%m\", aborting\n"),
                id ()));
    goto error;
  } // end IF

  // step1: initialize/tweak socket
  if (unlikely (!inherited::initialize (configuration_p->socketConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_SocketHandlerBase_T::initialize(): \"%m\", aborting\n"),
                id ()));
    goto error;
  } // end IF
  result = inherited::open (&configuration_p->socketConfiguration);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    ACE_UNUSED_ARG (error);
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to HandlerType::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock, -1);
    inherited2::state_.handle = inherited::get_handle ();
  } // end lock scope

  // step2: register with the connection manager ?
  if (likely (inherited2::isManaged_))
  {
    if (unlikely (!inherited2::register_ ()))
    {
      // *NOTE*: (most probably) maximum number of connections has been reached
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("failed to Net_ConnectionBase_T::register_(), aborting\n")));
      goto error;
    } // end IF
    handle_manager = true;
  } // end IF

  // step3: initialize/start stream
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_ == configuration_p);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration->configuration_);
  //if (!inherited2::configuration_->useThreadPerConnection)
  // step3a: connect the stream head message queue with the reactor notification
  //         pipe
  inherited2::configuration_->streamConfiguration->configuration_->notificationStrategy =
    static_cast<HandlerType*> (this);
  if (unlikely (!stream_.initialize (*(inherited2::configuration_->streamConfiguration),
                                     inherited2::state_.handle)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                inherited2::state_.handle,
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF
  if (unlikely (!stream_.initialize_2 (static_cast<HandlerType*> (this),
                                       ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                inherited2::state_.handle,
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF

  // step3b: update session data
  // *TODO*: remove type inferences
  session_data_container_p = &stream_.getR_2 ();
  ACE_ASSERT (session_data_container_p);
  session_data_p =
    &const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->getR ());
  ACE_ASSERT (session_data_p->lock);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *(session_data_p->lock), -1);
    session_data_p->connectionStates.insert (std::make_pair (inherited2::state_.handle,
                                                             &(inherited2::state_)));
  } // end lock scope

  // step3d: start stream
  stream_.start ();
  //if (unlikely (!stream_.isRunning ()))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%u: failed to start processing stream, aborting\n"),
  //              id ()));
  //  goto error;
  //} // end IF
  handle_stream = true;

  // step4: start receiving ?
  if (likely (!inherited2::configuration_->delayRead) &&
              !is_udp_write_only_b)
  {
    if (unlikely (!inherited::registerWithReactor ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to register with the reactor, aborting\n"),
                  id ()));
      goto error;
    } // end IF
    handle_reactor = true;
  } // end IF

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock, -1);
    inherited2::state_.status = NET_CONNECTION_STATUS_OK;
  } // end lock scope
  stream_.notify (STREAM_SESSION_MESSAGE_CONNECT,
                  true); // recurse upstream (if any) ?

  inherited2::decrease ();

  return 0;

error:
  if (handle_reactor)
    inherited::deregisterFromReactor ();
  if (handle_stream)
  {
    Stream_Net_ConnectionStatesIterator_t iterator =
      session_data_p->connectionStates.find (inherited2::state_.handle);
    ACE_ASSERT (iterator != session_data_p->connectionStates.end ());
    session_data_p->connectionStates.erase (iterator);

    stream_.stop (true,  // wait for completion ?
                  false, // wait for upstream (if any) ?
                  true); // high priority ?
  } // end IF
  if (handle_manager)
  {
    // *NOTE*: do not notify any listener/connectors in this case
    inherited2::connector_ = NULL;
    inherited2::listener_ = NULL;

    inherited2::deregister ();
  } // end IF

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock, -1);
    inherited2::state_.handle = ACE_INVALID_HANDLE;
    inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;
  } // end lock scope

  unsigned int ref_count_i = inherited2::count ();
  if (ref_count_i > 1) // :( yep, it happens...
    inherited2::decrease ();

  // *IMPORTANT NOTE*: the connector invokes close(NORMAL_CLOSE_OPERATION)
  //                   --> release the final reference there
  return -1;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::close (u_long arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::close"));

  int result = -1;

  // [*NOTE*: hereby override the default behavior of a ACE_Svc_Handler,
  // which would call handle_close() AGAIN]

  // *NOTE*: this method will be invoked
  // - by any worker after returning from svc()
  //    --> in this case, this should be a NOP (triggered from handle_close(),
  //        which was invoked by the reactor) - override the default
  //        behavior of a ACE_Svc_Handler, which would call handle_close() AGAIN
  // - by the connector when connect() fails (e.g. connection refused)
  // - by the connector/acceptor when open() fails (e.g. too many connections !)

  switch (arg_in)
  {
    // called by:
    // [- any worker from ACE_Task_Base on clean-up]
    // - connector when connect() fails (e.g. connection refused)
    // - acceptor/connector when initialization fails (i.e. open() returned -1
    //   due to e.g. too many connections). Note that there is a dangling
    //   reference in this case; it is released in inherited::close(), which
    //   invokes handle_close() (see below)
    case NORMAL_CLOSE_OPERATION:
    {
      // check specifically for the first case
      result = ACE_OS::thr_equal (ACE_Thread::self (),
                                  inherited::last_thread ());
      if (result)
      {
        result = 0;

        break;
      } // end IF

      // *WARNING*: falls through !
      ACE_FALLTHROUGH;
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. cannot connect, too many connections, ...)
    // *NOTE*: this (eventually) calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION:
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      if ((arg_in == CLOSE_DURING_NEW_CONNECTION) ||
          (arg_in == NET_CONNECTION_CLOSE_REASON_INITIALIZATION))
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock, -1);
        inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;
      } // end IF | lock scope

      Net_ConnectionId_t id_i = this->id ();
      result = inherited::close (arg_in); // --> calls handle_close()
      if (unlikely (result == -1))
      {
//        int error = ACE_OS::last_error ();
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to HandlerType::close(%u): \"%m\", continuing\n"),
                    id_i,
                    arg_in));
      } // end IF

      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid argument (was: %u), aborting\n"),
                  id (),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
int
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                        ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::handle_close"));

  int result = -1;
  bool initialization_failed_b = false;
  bool peer_closed_b = false;
  bool decrease_b = true;

  // step0a: set state
  // *IMPORTANT NOTE*: set the state early to avoid deadlock in
  //                   waitForCompletion() (see below), which may be implicitly
  //                   invoked by stream_.finished()
  // *TODO*: remove type inference
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock, -1);
    initialization_failed_b =
      (inherited2::state_.status == NET_CONNECTION_STATUS_INITIALIZATION_FAILED);
    if ((inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED) && // initialization failed ?
        (inherited2::state_.status != NET_CONNECTION_STATUS_CLOSED)                && // local close ?
        (inherited2::state_.status != NET_CONNECTION_STATUS_PEER_CLOSED))             // --> peer closed
    {
      inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
      peer_closed_b = true;
    } // end IF
    decrease_b = !inherited2::state_.closed; // i.e. only decrease reference count the first time around
    inherited2::state_.closed = true;
  } // end lock scope

  // step0b: notify stream ?
//  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock (), -1);
    if (likely (notify_ && !initialization_failed_b))
    {
      notify_ = false;
      stream_.notify (STREAM_SESSION_MESSAGE_DISCONNECT,
                      true,  // recurse upstream (if any) ?
                      true); // expedite ?
    } // end IF

    // step1: shut down the processing stream
    // *NOTE*: flush outbound data only !
    stream_.flush (false, // flush inbound data ?
                   false, // flush session messages ?
                   true); // recurse upstream (if any) ?
    stream_.idle (true,   // wait forever ?
                  false); // recurse upstream (if any) ?
    stream_.stop (true,   // wait for worker(s) (if any)
                  true,   // recurse upstream (if any) ?
                  false); // high priority ?
//  } // end lock scope

  switch (mask_in)
  {
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed (receive failed)
//    case ACE_Event_Handler::ACCEPT_MASK:
    case ACE_Event_Handler::WRITE_MASK:      // --> socket has been closed (send failed) (DevPoll)
    case ACE_Event_Handler::EXCEPT_MASK:     // --> socket has been closed (send failed)
    case ACE_Event_Handler::ALL_EVENTS_MASK: // --> connect failed (e.g. connection refused) /
                                             //     accept failed (e.g. too many connections) /
                                             //     select failed (EBADF see Select_Reactor_T.cpp) /
                                             //     user abort
      break;
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: Net_StreamConnectionBase_T::handle_close(0x%@,%d) called for unknown reasons --> check implementation !, continuing\n"),
                  id (),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: Net_StreamConnectionBase_T::handle_close(%d,%d) called for unknown reasons --> check implementation !, continuing\n"),
                  id (),
                  handle_in,
                  mask_in));
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
  } // end SWITCH

  // step3: invoke base class maintenance
  bool deregister = inherited2::isRegistered_;
  // *IMPORTANT NOTE*: retain the socket handle:
  //                   - otherwise this fails for the usecase "accept failed"
  //                     (see above)
  //                   - until deregistered from the manager
  result = inherited::handle_close (handle_in,
                                    mask_in);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to HandlerType::handle_close(0x%@,%d): \"%m\", aborting\n"),
                id (),
                handle_in,
                mask_in));
#else
    if (error != EBADF) // 9: local close
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to HandlerType::handle_close(%d,%d): \"%m\", aborting\n"),
                  id (),
                  handle_in,
                  mask_in));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
//  inherited::set_handle (handle_in); // reset for debugging purposes

  // step4: deregister with the connection manager (if any)
  if (likely (deregister))
    inherited2::deregister ();

  if (decrease_b)
    this->decrease ();

  return result;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
bool
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::collect (StatisticContainerType& statistic_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::collect"));

  statistic_out = inherited2::state_.statistic;

  typename StreamType::STATISTIC_T stream_statistic_s;
  stream_.collect (stream_statistic_s);

  statistic_out += stream_statistic_s;

  return true;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  info (handle,
        local_address,
        peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: 0x%@]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: %d]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::info (ACE_HANDLE& handle_out,
                                                AddressType& localSAP_out,
                                                AddressType& remoteSAP_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::info"));

  int result = -1;
  int error = 0;

  handle_out = inherited::get_handle ();
  result = inherited::peer_.get_local_addr (localSAP_out);
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: TCP: socket already closed
#else
    if (error != EBADF)    // 9    : Linux: socket already closed
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_SOCK::get_local_addr(): \"%m\", continuing\n"),
                  id ()));
  } // end IF
  result = inherited::peer_.get_remote_addr (remoteSAP_out);
  // *NOTE*: peer may have disconnected already --> not an error
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENOTSOCK) && // 10038: (TCP) socket already closed
        (error != ENOTCONN))   // 10057: (UDP) socket is not connected
#else
    if ((error != EBADF) &&    // 9    : Linux: socket already closed
        (error != ENOTCONN))   // 107  : Linux: perhaps UDP ?
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_SOCK::get_remote_addr(): \"%m\", continuing\n"),
                  id ()));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::abort"));

  int result = -1;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    if (likely (inherited2::state_.status == NET_CONNECTION_STATUS_OK))
      inherited2::state_.status = NET_CONNECTION_STATUS_CLOSED;
  } // end lock scope

  // wake up any open read operation; it will release the connection
  ACE_HANDLE handle = inherited::get_handle ();
  if (likely (handle != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::shutdown (handle, ACE_SHUTDOWN_BOTH);
    if (unlikely (result == -1))
    {
      int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to ACE_OS::shutdown(0x%@): \"%m\", continuing\n"),
                  id (),
                  handle));
#else
      if ((error != EBADF) &&  // 9  : local close
          (error != ENOTCONN)) // 107: UDP ?
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::shutdown(%d): \"%m\", continuing\n"),
                    id (),
                    handle));
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF

    result = ACE_OS::closesocket (handle);
    if (unlikely (result == -1))
    {
      int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (error != ENOTSOCK) // 10038: local close
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::closesocket(0x%@): \"%m\", continuing\n"),
                    id (),
                    handle));
#else
      if (error != EBADF) // 9: local close
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                    id (),
                    handle));
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
  } // end IF

  // handle any (UDP) write-only connections
  enum Net_TransportLayerType transport_layer_e = this->transportLayer ();
  bool is_udp_write_only_b = false;
  switch (transport_layer_e)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
      break;
    case NET_TRANSPORTLAYER_UDP:
    { ACE_ASSERT (inherited2::configuration_);
      Net_UDPSocketConfiguration_t* socket_configuration_p =
        (Net_UDPSocketConfiguration_t*)&inherited2::configuration_->socketConfiguration;
      is_udp_write_only_b = socket_configuration_p->writeOnly;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), returning\n"),
                  id (),
                  transport_layer_e));
      return;
    }
  } // end SWITCH
  if (unlikely (is_udp_write_only_b))
  {
    result = handle_close (ACE_INVALID_HANDLE,
                           ACE_Event_Handler::ALL_EVENTS_MASK);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to HandlerType::handle_close(): \"%m\", continuing\n"),
                  id ()));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  stream_.wait (waitForThreads_in,
                false,             // wait for upstream ?
                false);            // wait for downstream ?

  // *NOTE*: all data has been dispatched to the reactor (i.e. kernel)

  // step2: wait for the kernel to place the data onto the wire
  // *TODO*: platforms may implement different methods by which this can be
  //         established (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  // *TODO*: remove type inference
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
    {
      ACE_HANDLE handle = inherited::get_handle ();
      ACE_ASSERT (handle != ACE_INVALID_HANDLE);
      bool no_delay = Net_Common_Tools::getNoDelay (handle);
      Net_Common_Tools::setNoDelay (handle, true);
      Net_Common_Tools::setNoDelay (handle, no_delay);
    } // end IF
  } // end lock scope
#endif // ACE_LINUX
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::set (enum Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::set"));

  bool result = false;

  // sanity check(s)
  HandlerConfigurationType socket_handler_configuration;
  //// *TODO*: remove type inference
  //if (configuration->socketConfiguration)
  //  socket_configuration = *configuration->socketConfiguration;

  try {
    result = initialize (this->dispatch (),
                         role_in,
                         socket_handler_configuration);
                         //*configuration->socketConfiguration);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::send"));

  int result = -1;
  Stream_Module_t* module_p = NULL, *module_2 = NULL;
  Stream_Task_t* task_p = NULL;
  Stream_ITask* itask_p = NULL;

  // *NOTE*: feed the data into the stream (queue of the tail module 'reader'
  //         task; Note: 'tail' module to support potential 'streamer' modules)
  module_p = stream_.head ();
  ACE_ASSERT (module_p);
  do
  {
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);
    itask_p = dynamic_cast<Stream_ITask*> (task_p);
    if (unlikely (itask_p &&
                  itask_p->isAggregator ()))
      break; // aggregator
    module_2 = module_p->next ();
    ACE_ASSERT (module_2);
    if (unlikely (!ACE_OS::strcmp (module_2->name (),
                                   ACE_TEXT ("ACE_Stream_Tail")) ||
                  !ACE_OS::strcmp (module_2->name (),
                                   ACE_TEXT (STREAM_MODULE_TAIL_NAME))))
      break;
    module_p = module_2;
  } while (true);
  ACE_ASSERT (module_p);
  task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  result = task_p->put (message_inout, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: failed to ACE_Task::put(): \"%m\", returning\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                module_p->name ()));
    goto error;
  } // end IF

  return;

error:
  message_inout->release (); message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
bool
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::wait (StreamStatusType state_in,
                                                const ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::wait"));

  Stream_Module_t* top_module_p = NULL;
  int result = stream_.top (top_module_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                id ()));
    return false;
  } // end IF
  ACE_ASSERT (top_module_p);

  Stream_IStateMachine_t* istatemachine_p =
      dynamic_cast<Stream_IStateMachine_t*> (top_module_p->writer ());
  if (unlikely (!istatemachine_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: failed to dynamic_cast<Stream_IStateMachine_t>(0x%@), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                top_module_p->writer ()));
    return false;
  } // end IF

  bool result_2 = false;
  try {
    result_2 = istatemachine_p->wait (state_in,
                                      timeValue_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: caught exception in Stream_IStateMachine_t::wait(%d,%T#), continuing\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                state_in,
                timeValue_in));
  }

  return result_2;
}

template <ACE_SYNCH_DECL,
          typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
ACE_Message_Block*
Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                           HandlerType,
                           AddressType,
                           ConfigurationType,
                           StateType,
                           StatisticContainerType,
                           HandlerConfigurationType,
                           StreamType,
                           StreamStatusType,
                           UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration->configuration_);

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  id (),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->configuration_->messageAllocator->block ())
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
    if (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                    id ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                  id ()));
  } // end IF

  return message_block_p;
}

/////////////////////////////////////////

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::Net_AsynchStreamConnectionBase_T (bool managed_in)
 : inherited ()
 , inherited2 (managed_in)
 , allocator_ (NULL)
 , stream_ ()
 , notify_ (true)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::Net_AsynchStreamConnectionBase_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::act (const void* act_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::act"));

  // initialize this connection

  // *NOTE*: client-side: arg_in is a handle to the connector
  //         server-side: arg_in is a handle to the listener
  ConfigurationType* configuration_p = NULL;
  HandlerConfigurationType* socket_handler_configuration_p = NULL;
  typename inherited2::ICONNECTOR_T* iconnector_p = NULL;
  typename inherited2::ILISTENER_T* ilistener_p = NULL;
  switch (this->transportLayer ())
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    {
      switch (this->role ())
      {
        case NET_ROLE_CLIENT:
        {
          iconnector_p =
              static_cast<typename inherited2::ICONNECTOR_T*> (const_cast<void*> (act_in));
          ACE_ASSERT (iconnector_p);
          configuration_p =
            &const_cast<ConfigurationType&> (iconnector_p->getR ());
          break;
        }
        case NET_ROLE_SERVER:
        {
          ilistener_p =
              static_cast<typename inherited2::ILISTENER_T*> (const_cast<void*> (act_in));
          ACE_ASSERT (ilistener_p);
          configuration_p =
            &const_cast<ConfigurationType&> (ilistener_p->getR ());

          inherited2::listener_ = ilistener_p;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: invalid/unknown role (was: %d), returning\n"),
                      id (),
                      this->role ()));
          return;
        }
      } // end SWITCH

      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      iconnector_p =
          static_cast<typename inherited2::ICONNECTOR_T*> (const_cast<void*> (act_in));
      ACE_ASSERT (iconnector_p);
      configuration_p =
        &const_cast<ConfigurationType&> (iconnector_p->getR ());

      configuration_p = &const_cast<ConfigurationType&> (iconnector_p->getR ());
      Net_UDPSocketConfiguration_t* socket_configuration_p =
        (Net_UDPSocketConfiguration_t*)&configuration_p->socketConfiguration;
      bool is_udp_write_only_b = socket_configuration_p->writeOnly;

      if (!is_udp_write_only_b) // server-side ?
        inherited2::connector_ = iconnector_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), returning\n"),
                  id (),
                  this->transportLayer ()));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (configuration_p);
  socket_handler_configuration_p = &configuration_p->socketConfiguration;

  if (unlikely (!inherited::initialize (*socket_handler_configuration_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_SocketHandlerBase_T::initialize(): \"%m\", returning\n"),
                id ()));
    return;
  } // end IF
//  if (!inherited2::isManaged_) // *TODO*: remove this 'feature' completely
    if (unlikely (!inherited2::initialize (*configuration_p)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to Net_ConnectionBase_T::initialize(): \"%m\", returning\n"),
                  id ()));
      return;
    } // end IF
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::open (ACE_HANDLE handle_in,
                                                      ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::open"));

  // sanity check(s)
  // *NOTE*: handle_in is ACE_INVALID_HANDLE for UDP connections
  //ACE_ASSERT (handle_in == ACE_INVALID_HANDLE);

  bool handle_read = false;
  bool handle_manager = false;
  bool handle_stream = false;
  enum Net_TransportLayerType transport_layer_e = this->transportLayer ();
  const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  typename StreamType::SESSION_DATA_T* session_data_p = NULL;
  AddressType local_SAP, peer_SAP;
  bool is_udp_write_only_b = false;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZING;
  } // end lock scope

  // *NOTE*: act() has been called already, Net_ConnectionBase has been
  //         initialized

  ACE_ASSERT (inherited2::configuration_);
  switch (transport_layer_e)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
      break;
    case NET_TRANSPORTLAYER_UDP:
    {
      Net_UDPSocketConfiguration_t* socket_configuration_p =
        (Net_UDPSocketConfiguration_t*)&inherited2::configuration_->socketConfiguration;
      is_udp_write_only_b = socket_configuration_p->writeOnly;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), aborting\n"),
                  id (),
                  transport_layer_e));
      goto error;
    }
  } // end SWITCH

  // step1: initialize/tweak socket, initialize I/O, ...
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::isInitialized_);
  inherited::open (handle_in,
                   messageBlock_in);
  if (unlikely (!inherited::isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to HandlerType::open (handle was: %d), aborting\n"),
                id (),
                handle_in));
    goto error;
  } // end IF
  this->info (inherited2::state_.handle,
              local_SAP, peer_SAP);

  // step2: register with the connection manager ?
  if (likely (inherited2::isManaged_))
  {
    if (unlikely (!inherited2::register_ ()))
    {
      // *NOTE*: (most probably) maximum number of connections has been reached
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("failed to Net_ConnectionBase_T::registerc(), aborting\n")));
      goto error;
    } // end IF
    handle_manager = true;
  } // end IF

  // step3: initialize/start stream
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  if (unlikely (!stream_.initialize (*(inherited2::configuration_->streamConfiguration),
                                     inherited2::state_.handle)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF
  if (unlikely (!stream_.initialize_2 (this,
                                       ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF

  // step3b: update session data
  // *TODO*: remove type inferences
  session_data_container_p = &stream_.getR_2 ();
  session_data_p =
    &const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->getR ());
  ACE_ASSERT (session_data_p->lock);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);
    session_data_p->connectionStates.insert (std::make_pair (inherited2::state_.handle,
                                                             &(inherited2::state_)));
  } // end lock scope

  // step3c: start stream
  stream_.start ();
  //if (unlikely (!stream_.isRunning ()))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%u: failed to start processing stream, aborting\n"),
  //              id ()));
  //  goto error;
  //} // end IF
  handle_stream = true;

  // step4: start receiving ?
  if (likely (!inherited2::configuration_->delayRead &&
              !is_udp_write_only_b))
  {
    if (unlikely (!initiate_read ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to Net_ISocketHandler::initiate_read(), aborting\n"),
                  id ()));
      goto error;
    } // end IF
    handle_read = true;
  } // end IF

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    inherited2::state_.status = NET_CONNECTION_STATUS_OK;
  } // end lock scope
  stream_.notify (STREAM_SESSION_MESSAGE_CONNECT,
                  true); // recurse upstream (if any) ?

  return;

error:
  if (handle_read)
    this->cancel ();
  if (handle_stream)
    stream_.stop (true,  // wait for completion ?
                  true,  // wait for upstream (if any) ?
                  true); // high priority ?
  if (handle_manager)
  {
    // *NOTE*: do not notify any listener/connectors in this case
    inherited2::connector_ = NULL;
    inherited2::listener_ = NULL;

    inherited2::deregister ();
  } // end IF

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    inherited2::state_.handle = ACE_INVALID_HANDLE;
    inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;
  } // end lock scope
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
int
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::handle_close (ACE_HANDLE handle_in,
                                                              ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_close"));

  bool cancel_b = false;

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
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock, -1);
    if ((inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED) && // initialization failed ?
        (inherited2::state_.status != NET_CONNECTION_STATUS_CLOSED)                && // local close ?
        (inherited2::state_.status != NET_CONNECTION_STATUS_PEER_CLOSED))             // --> peer closed
      inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
    cancel_b = !inherited2::state_.closed; // i.e. only cancel the first time around
    inherited2::state_.closed = true;
  } // end lock scope

  // step0b: notify stream ?
//  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock (), -1);
    if (likely (notify_ &&
                (inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED)))
    {
      notify_ = false;
      stream_.notify (STREAM_SESSION_MESSAGE_DISCONNECT,
                      true); // recurse upstream (if any) ?
    } // end IF

    // step1: shut down the processing stream
    stream_.flush (false,  // do not flush inbound data
                   false,  // do not flush session messages
                   false); // recurse upstream (if any) ?
    stream_.idle (inherited2::state_.status == NET_CONNECTION_STATUS_PEER_CLOSED, // wait forever ?
                  false); // recurse upstream (if any) ?
    stream_.stop (true,   // wait for worker(s) (if any)
                  true,   // recurse upstream (if any) ?
                  false); // high priority ?
//  } // end lock scope

  // *NOTE*: pending socket operations are notified by the kernel and will
  //         return automatically
  // *TODO*: consider cancel()ling pending write operations

  // step2: invoke base-class maintenance
  // step2a: cancel pending operation(s) ?
  if (cancel_b)
    inherited::cancel ();

  // step2b: deregister with the connection manager (if any) ?
//  close_socket_b = (inherited2::count () <= 2);
  if (likely (inherited2::isRegistered_))
    inherited2::deregister ();

  // step2c: release socket handle(s) ?
  return inherited::handle_close (handle_in,
                                  mask_in);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
bool
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::collect (StatisticContainerType& statistic_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::collect"));

  statistic_out = inherited2::state_.statistic;

  typename StreamType::STATISTIC_T stream_statistic_s;
  stream_.collect (stream_statistic_s);

  statistic_out += stream_statistic_s;

  return true;
}


template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::dump_state"));

  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  AddressType local_address, peer_address;
  this->info (handle,
              local_address,
              peer_address);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: 0x%@]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection [id: %u, handle: %d]: %s <--> %s\n"),
              id (), handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (local_address).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::abort"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
      inherited2::state_.status = NET_CONNECTION_STATUS_CLOSED;
  } // end lock scope

  // step1: cancel (pending) i/o operation(s)
  inherited::cancel ();

  int result = ACE_OS::shutdown (inherited2::state_.handle,
                                 ACE_SHUTDOWN_BOTH);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: local close
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::shutdown(0x%@): \"%m\", continuing\n"),
                  inherited2::state_.handle));
#else
    if ((error != EBADF) &&  // 9  : Linux: local close()
        (error != ENOTCONN)) // 107: Linux: local close()
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::shutdown(%d): \"%m\", continuing\n"),
                  inherited2::state_.handle));
#endif // ACE_WIN32 || ACE_WIN64
   } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (likely (inherited::writeHandle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::shutdown (inherited::writeHandle_,
                               ACE_SHUTDOWN_BOTH);
    if (unlikely (result == -1))
    {
      int error = ACE_OS::last_error ();
      if ((error != EBADF) &&  // 9  : Linux: local close() *TODO*
          (error != ENOTCONN)) // 107: Linux: local close ()
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::shutdown(%d): \"%m\", continuing\n"),
                    inherited::writeHandle_));
    } // end IF
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

   bool is_udp_write_only_b = false;
   switch (this->transportLayer ())
   {
     case NET_TRANSPORTLAYER_TCP:
     case NET_TRANSPORTLAYER_SSL:
       break;
     case NET_TRANSPORTLAYER_UDP:
     {
       // sanity check(s)
       ACE_ASSERT (inherited2::configuration_);

       Net_UDPSocketConfiguration_t* socket_configuration_p =
          (Net_UDPSocketConfiguration_t*)&inherited2::configuration_->socketConfiguration;
       is_udp_write_only_b = socket_configuration_p->writeOnly;
       break;
     }
     default:
     {
       ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), continuing\n"),
                  id (),
                  this->transportLayer ()));
       break;
     }
   } // end SWITCH

   // *TODO*: is this necessary at all ? *NOTE*: inherited::handle_close calls handle_close
   if (is_udp_write_only_b)
     handle_close (inherited2::state_.handle,
                   ACE_Event_Handler::ALL_EVENTS_MASK);
   else
    inherited::handle_close (inherited2::state_.handle,
                             ACE_Event_Handler::ALL_EVENTS_MASK);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::waitForCompletion (bool waitForThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::waitForCompletion"));

  // step1: wait for the stream to flush
  stream_.wait (waitForThreads_in,
                false,             // wait for upstream ?
                false);            // wait for downstream ?

  // *NOTE*: all data has been dispatched to the reactor (i.e. kernel)

  // step2: wait for the kernel to place the data onto the wire
  // *TODO*: platforms may implement different methods by which this can be
  //         established (see also: http://stackoverflow.com/questions/855544/is-there-a-way-to-flush-a-posix-socket)
#if defined (ACE_LINUX)
  // *TODO*: remove type inference
  if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
  {
    ACE_HANDLE handle = inherited::handle ();
    ACE_ASSERT (handle != ACE_INVALID_HANDLE);
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif // ACE_LINUX
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::set (enum Net_ClientServerRole role_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::set"));

  bool result = false;

  // sanity check(s)
  HandlerConfigurationType socket_handler_configuration;
  //// *TODO*: remove type inference
  //if (configuration->socketConfiguration)
  //  socket_configuration = *configuration->socketConfiguration;

  try {
    result = initialize (this->dispatch (),
                         role_in,
                         socket_handler_configuration);
                         //*configuration->socketConfiguration);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: caught exception in Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
  }
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to Net_ITransportLayer_T::initialize(), continuing\n"),
                id ()));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::send (ACE_Message_Block*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::send"));

  int result = -1;
  Stream_Module_t* module_p = NULL, *module_2 = NULL;
  Stream_Task_t* task_p = NULL;
  Stream_ITask* itask_p = NULL;

  // *NOTE*: feed the data into the stream (queue of the tail module 'reader'
  //         task; Note: 'tail' module to support potential 'streamer' modules)
  module_p = stream_.head ();
  ACE_ASSERT (module_p);
  do
  {
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);
    itask_p = dynamic_cast<Stream_ITask*> (task_p);
    if (unlikely (itask_p &&
                  itask_p->isAggregator ()))
      break; // aggregator
    module_2 = module_p->next ();
    ACE_ASSERT (module_2);
    if (unlikely (!ACE_OS::strcmp (module_2->name (),
                                   ACE_TEXT ("ACE_Stream_Tail")) ||
                  !ACE_OS::strcmp (module_2->name (),
                                   ACE_TEXT (STREAM_MODULE_TAIL_NAME))))
      break;
    module_p = module_2;
  } while (true);
  ACE_ASSERT (module_p);
  task_p = module_p->reader ();
  ACE_ASSERT (task_p);
  result = task_p->put (message_inout, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: failed to ACE_Task::put(): \"%m\", returning\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                module_p->name ()));
    goto error;
  } // end IF

  return;

error:
  message_inout->release (); message_inout = NULL;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::waitForIdleState (bool waitForever_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::waitForIdleState"));

  // step1: wait for the streams' 'outbound' queue to turn idle
  stream_.idle (waitForever_in,
                false);         // recurse ?

  // --> outbound stream data has been processed

  // step2: wait for any asynchronous operations to complete
  inherited::counter_.wait (0);

  // --> all data has been dispatched to the kernel (socket)
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
bool
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::wait (StreamStatusType state_in,
                                                      const ACE_Time_Value* timeValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::wait"));

  Stream_Module_t* top_module_p = NULL;
  int result = stream_.top (top_module_p);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to ACE_Stream::top(): \"%m\", aborting\n"),
                id ()));
    return false;
  } // end IF
  ACE_ASSERT (top_module_p);

  Stream_IStateMachine_t* istatemachine_p =
      dynamic_cast<Stream_IStateMachine_t*> (top_module_p->writer ());
  if (unlikely (!istatemachine_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: failed to dynamic_cast<Stream_IStateMachine_t>(0x%@), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                top_module_p->writer ()));
    return false;
  } // end IF

  bool result_2 = false;
  try {
    result_2 = istatemachine_p->wait (state_in,
                                      timeValue_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u/%s/%s: caught exception in Stream_IStateMachine_t::wait(%d,%T#), continuing\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ()),
                top_module_p->name (),
                state_in,
                timeValue_in));
  }

  return result_2;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
ACE_Message_Block*
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration->configuration_);

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  id (),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->configuration_->messageAllocator->block ())
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
    if (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->configuration_->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                    id ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%u: failed to allocate memory: \"%m\", aborting\n"),
                  id ()));
  } // end IF

  return message_block_p;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
bool
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::initiate_read ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::initiate_read"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->allocatorConfiguration);

  // allocate a data buffer
  unsigned int pdu_size_i =
    inherited2::configuration_->allocatorConfiguration->defaultBufferSize +
    inherited2::configuration_->allocatorConfiguration->paddingBytes;
  ACE_Message_Block* message_block_p = this->allocateMessage (pdu_size_i);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_ISocketHandler::allocateMessage(%u), aborting\n"),
                pdu_size_i));
    return false;
  } // end IF
  message_block_p->size (inherited2::configuration_->allocatorConfiguration->defaultBufferSize);

  // delegate to base class(es), fire-and-forget message_block_p
  return inherited::initiate_read (message_block_p);
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_read_stream"));

  int result = -1;
  bool close_b = false;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    close_b =
      (inherited2::state_.status == NET_CONNECTION_STATUS_CLOSED);
  } // end lock scope
  bool release_message_block_b = true;
  size_t bytes_transferred_i = 0;

  // sanity check
  if (unlikely (!result_in.success ()))
  {
    // connection closed/reset (by peer) ? --> not an error
    unsigned long error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *NOTE*: ERROR_SEM_TIMEOUT 121
    if ((error != ERROR_NETNAME_DELETED)   && // 64  : peer close()
        (error != ERROR_OPERATION_ABORTED) && // 995 : local close()
        (error != ERROR_CONNECTION_ABORTED))  // 1236: local close()
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (handle was: 0x%@): \"%s\", continuing\n"),
                  id (), result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
    if ((error != EBADF)     && // 9  : local close(), happens on Linux
        (error != EPIPE)     && // 32 : happens on Linux
        (error != ECONNRESET))  // 104: happens on Linux
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (handle was: %d): \"%s\", continuing\n"),
                  id (), result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  bytes_transferred_i = result_in.bytes_transferred ();
  switch (static_cast<int> (bytes_transferred_i))
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      unsigned long error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely ((error != ERROR_NETNAME_DELETED)   &&  // 64  : peer close()
                    (error != ERROR_OPERATION_ABORTED) &&  // 995 : cancel()led
                    (error != ERROR_CONNECTION_ABORTED)))  // 1236: local close()
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to read from input stream (handle was: 0x%@): \"%s\", returning\n"),
                    id (), result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#else
      if (unlikely ((error != EBADF)     && // 9  : local close(), happens on Linux
                    (error != EPIPE)     && // 32 : happens on Linux
                    (error != ECONNRESET)))  // 104: happens on Linux
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to read from input stream (handle was: %d): \"%s\", returning\n"),
                    id (), result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
    // *** GOOD CASES ***
    case 0:
    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%u: socket (handle was: 0x%@) has been closed\n"),
//                  id (), result_in.handle ()));
//#else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%u: socket (handle was: %d) has been closed\n"),
//                  id (), result_in.handle ()));
//#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
    default:
    {
      // update statistic
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
        inherited2::state_.statistic.receivedBytes += bytes_transferred_i;
      } // end lock scope

      // push the buffer onto the stream for processing
      result = stream_.put (&result_in.message_block (), NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u/%s: failed to ACE_Stream::put(): \"%m\", returning\n"),
                    id (), ACE_TEXT (stream_.name ().c_str ())));
        break;
      } // end IF
      release_message_block_b = false;

      // start next read ?
      if (close_b)
        break;
      if (unlikely (!this->initiate_read ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to Net_AsynchStreamConnectionBase_T::initiate_read(): \"%m\", returning\n"),
                    id ()));
        break;
      } // end IF

      return;
    }
  } // end SWITCH

  // clean up
  if (release_message_block_b)
    result_in.message_block ().release ();

  Net_ConnectionId_t id = this->id ();
  result =
      handle_close (result_in.handle (),
                    ACE_Event_Handler::ALL_EVENTS_MASK);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: local close
#else
    if (error != EBADF) // 9: local close (Linux)
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to Net_AsynchStreamConnectionBase_T::handle_close(): \"%m\", continuing\n"),
                  id));
  } // end IF

  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::handle_read_dgram (const ACE_Asynch_Read_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_read_dgram"));

  int result = -1;
  size_t bytes_transferred_i = 0;
  bool release_message_block_b = true;

  // sanity check
  if (unlikely (!result_in.success ()))
  {
    // connection closed/reset ? --> not an error
    unsigned long error = result_in.error ();
    if ((error != EBADF)                   && // 9    : local close (), happens on Linux
        (error != EPIPE)                   && // 32
        (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_MORE_DATA)         && // 234  : message block too small ?
        (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif // ACE_WIN32 || ACE_WIN64
        (error != ECONNREFUSED))              // 111: happens on Linux
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (0x%@): \"%s\", aborting\n"),
                  id (),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to read from input stream (%d): \"%s\", aborting\n"),
                  id (),
                  result_in.handle (),
                  ACE::sock_error (static_cast<int> (error))));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  bytes_transferred_i = result_in.bytes_transferred ();
  switch (static_cast<int> (bytes_transferred_i))
  {
    case -1:
    {
      // connection closed/reset (by peer) ? --> not an error
      unsigned long error = result_in.error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely ((error != EPIPE)                   && // 32
                    (error != ERROR_OPERATION_ABORTED) && // 995  : cancel()led
                    (error != ECONNRESET)              && // 10054
                    (error != EHOSTDOWN)))                // 10064
#else
      if (unlikely ((error != EBADF)                   && // 9  : local close (), happens on Linux
                    (error != EPIPE)                   && // 32 :
                    (error != ECONNREFUSED)))             // 111:
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%u: failed to read from input stream (0x%@): \"%s\", aborting\n"),
                      id (),
                      result_in.handle (),
                      ACE::sock_error (static_cast<int> (error))));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to read from input stream (%d): \"%s\", aborting\n"),
                    id (),
                    result_in.handle (),
                    ACE::sock_error (static_cast<int> (error))));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    }
    // *** GOOD CASES ***
    case 0:
    {
      goto error;
    }
    default:
    {
      // update statistic
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
        inherited2::state_.statistic.receivedBytes += bytes_transferred_i;
      } // end lock scope

      // push the buffer into the stream for processing
      result = stream_.put (result_in.message_block (), NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u/%s: failed to ACE_Stream::put(): \"%m\", aborting\n"),
                    id (),
                    ACE_TEXT (stream_.name ().c_str ())));
        goto error;
      } // end IF
      release_message_block_b = false;
      break;
    }
  } // end SWITCH

  if (unlikely (!this->initiate_read ()))
  {
    int error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if ((error != ENXIO)               && // happens on Win32
        (error != EFAULT)              && // *TODO*: happens on Win32
        (error != ERROR_UNEXP_NET_ERR) && // *TODO*: happens on Win32
        (error != ERROR_NETNAME_DELETED)) // happens on Win32
#else
    if (error)
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: failed to Net_AsynchStreamConnectionBase_T::initiate_read(): \"%m\", aborting\n"),
                  id ()));
    goto error;
  } // end IF

  return;

error:
  if (release_message_block_b)
    result_in.message_block ()->release ();

  result = handle_close (result_in.handle (),
                         ACE_Event_Handler::ALL_EVENTS_MASK);
  if (unlikely (result == -1))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(0x%@,%d): \"%m\", continuing\n"),
                result_in.handle (),
                ACE_Event_Handler::ALL_EVENTS_MASK));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_StreamAsynchUDPSocketBase_T::handle_close(%d,%d): \"%m\", continuing\n"),
                result_in.handle (),
                ACE_Event_Handler::ALL_EVENTS_MASK));
#endif // ACE_WIN32 || ACE_WIN64

  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::handle_write_stream (const ACE_Asynch_Write_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_write_stream"));

  size_t bytes_transferred_i = result_in.bytes_transferred ();

  switch (static_cast<int> (bytes_transferred_i))
  {
    case -1:
    case 0:
      break;
    default:
    {
      inherited2::state_.statistic.sentBytes += bytes_transferred_i;
      break;
    }
  } // end switch

  // *NOTE*: the message block is released by the base class
  inherited::handle_write_stream (result_in);

//  ACE_ASSERT (this->count () > 1);
  this->decrease ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename UserDataType>
void
Net_AsynchStreamConnectionBase_T<HandlerType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 StreamStatusType,
                                 UserDataType>::handle_write_dgram (const ACE_Asynch_Write_Dgram::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::handle_write_dgram"));

  size_t bytes_transferred_i = result_in.bytes_transferred ();

  switch (static_cast<int> (bytes_transferred_i))
  {
    case -1:
    case 0:
      break;
    default:
    {
      inherited2::state_.statistic.sentBytes += bytes_transferred_i;
      break;
    }
  } // end switch

  // *NOTE*: the message block is released by the base class
  inherited::handle_write_dgram (result_in);

  this->decrease ();
}
