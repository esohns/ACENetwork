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
#include "ace/Global_Macros.h"
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"

#include "common.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_defines.h"
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
 , stream_ ()
 , allocator_ (NULL)
 , sendLock_ ()
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

  int result = -1;
  bool handle_reactor = false;
  bool handle_manager = false;
  bool handle_stream = false;
  AddressType local_SAP;
  ICONNECTOR_T* iconnector_p = NULL;
  ILISTENER_T* ilistener_p = NULL;
  AddressType peer_SAP;
  enum Net_TransportLayerType transport_layer_e = this->transportLayer ();
  // *TODO*: remove type inferences
  const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  typename StreamType::SESSION_DATA_T* session_data_p = NULL;
  ConfigurationType* configuration_p = NULL;

  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZING;

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
          iconnector_p = static_cast<ICONNECTOR_T*> (arg_in);
          ACE_ASSERT (iconnector_p);
          configuration_p =
              &const_cast<ConfigurationType&> (iconnector_p->getR ());
          break;
        }
        case NET_ROLE_SERVER:
        {
          ilistener_p = static_cast<ILISTENER_T*> (arg_in);
          ACE_ASSERT (ilistener_p);
          configuration_p =
              &const_cast<ConfigurationType&> (ilistener_p->getR ());
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
      iconnector_p = static_cast<ICONNECTOR_T*> (arg_in);
      ACE_ASSERT (iconnector_p);
      configuration_p =
          &const_cast<ConfigurationType&> (iconnector_p->getR ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: invalid/unknown transport layer (was: %d), aborting\n"),
                  id (),
                  this->transportLayer ()));
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
  result = inherited::open (configuration_p);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    ACE_UNUSED_ARG (error);
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to HandlerType::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  inherited2::state_.handle = inherited::get_handle ();

  // step2: initialize/start stream
  // step2a: connect the stream head message queue with the reactor notification
  //         pipe ?
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  //if (!inherited2::configuration_->useThreadPerConnection)
  inherited2::configuration_->streamConfiguration->configuration->notificationStrategy =
    static_cast<HandlerType*> (this);
  if (unlikely (!stream_.initialize (*(inherited2::configuration_->streamConfiguration),
                                     inherited2::state_.handle)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF
  if (unlikely (!stream_.initialize_2 (static_cast<HandlerType*> (this),
                                       ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head"))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to initialize processing stream (name was: \"%s\"), aborting\n"),
                id (),
                ACE_TEXT (stream_.name ().c_str ())));
    goto error;
  } // end IF

  // update session data
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

  // step2d: start stream
  stream_.start ();
  if (unlikely (!stream_.isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to start processing stream, aborting\n"),
                id ()));
    goto error;
  } // end IF
  handle_stream = true;

  if (unlikely (!inherited::registerWithReactor ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to register with the reactor, aborting\n"),
                id ()));
    goto error;
  } // end IF
  handle_reactor = true;

  // step3: register with the connection manager ?
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

  inherited2::state_.status = NET_CONNECTION_STATUS_OK;
  stream_.notify (STREAM_SESSION_MESSAGE_CONNECT,
                  true);

  return 0;

error:
  if (handle_reactor)
    inherited::deregisterFromReactor ();

  if (handle_stream)
    stream_.stop (true,  // wait for completion ?
                  false, // wait for upstream (if any) ?
                  true); // locked access ?

  if (handle_manager)
    inherited2::deregister ();

  inherited2::state_.handle = ACE_INVALID_HANDLE;
  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

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
    //   due to e.g. too many connections)
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
    }
    // called by external (e.g. reactor) thread wanting to close the connection
    // (e.g. cannot connect, too many connections, ...)
    // *NOTE*: this (eventually) calls handle_close() (see below)
    case CLOSE_DURING_NEW_CONNECTION:
    case NET_CONNECTION_CLOSE_REASON_INITIALIZATION:
    case NET_CONNECTION_CLOSE_REASON_USER_ABORT:
    {
      Net_ConnectionId_t id_i = this->id ();
      result = inherited::close (arg_in);
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

  // step0a: set state
  // *IMPORTANT NOTE*: set the state early to avoid deadlock in
  //                   waitForCompletion() (see below), which may be implicitly
  //                   invoked by stream_.finished()
  // *TODO*: remove type inference
  if ((inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED) && // initialization failed ?
      (inherited2::state_.status != NET_CONNECTION_STATUS_CLOSED))
    inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;

  // step0b: notify stream ?
//  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock (), -1);
    if (likely (notify_ &&
                (inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED)))
    {
      notify_ = false;
      stream_.notify (STREAM_SESSION_MESSAGE_DISCONNECT,
                      true);
    } // end IF

    // step1: shut down the processing stream
    stream_.flush (false,  // do not flush inbound data
                   false,  // do not flush session messages
                   false); // flush upstream (if any)
    stream_.idle ();
    stream_.stop (true,  // wait for worker(s) (if any)
                  false, // recurse upstream (if any)
                  true); // locked access ?
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
    {
      // sanity check(s)
      ACE_ASSERT (inherited2::configuration_);

      // step1: signal completion and wait for all processing
      // *IMPORTANT NOTE*: when the socket closes, any dispatching threads
      //                   currently servicing the socket handle will call
      //                   handle_close()
//      stream_.flush (false,  // flush inbound ?
//                     false,  // flush session messages ?
//                     false); // flush upstream ?
//      stream_.wait (true,   // wait for worker(s) (if any)
//                    false,  // wait for upstream (if any)
//                    false); // wait for downstream (if any)

      // step2: purge any pending (!) notifications ?
      // *TODO*: remove type inference
//      if (likely (!inherited2::configuration_->useThreadPerConnection))
//      { // *IMPORTANT NOTE*: in a multithreaded environment, in particular when
        //                   using a multithreaded reactor, there may still be
        //                   in-flight notifications being dispatched at this
        //                   stage. In that case, do not rely on releasing all
        //                   handler resources "manually", use reference
        //                   counting instead
//        ACE_Reactor* reactor_p = inherited::SVC_HANDLER_T::reactor ();
//        ACE_ASSERT (reactor_p);
//        result =
//            reactor_p->purge_pending_notifications (this,
//                                                    ACE_Event_Handler::ALL_EVENTS_MASK);
//        if (unlikely (result == -1))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%u: failed to ACE_Reactor::purge_pending_notifications(0x%@,%d): \"%m\", continuing\n"),
//                      id (),
//                      this,
//                      ACE_Event_Handler::ALL_EVENTS_MASK));
        //else if (result > 0)
        //  ACE_DEBUG ((LM_DEBUG,
        //              ACE_TEXT ("flushed %d notifications (handle was: %d)\n"),
        //              result,
        //              handle_in));
//      } // end IF

      break;
    }
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
  unsigned int count = inherited2::count ();
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

  if ((count > 1) ||
      ((count == 1) && !deregister))
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

  return stream_.collect (statistic_out.streamStatistic);
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
#endif
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
                           UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_StreamConnectionBase_T::close"));

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
      if (error != EBADF) // 9: local close
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
      if (error != EBADF) // 9: local close: happens on Linux
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to ACE_OS::closesocket(%d): \"%m\", continuing\n"),
                    id (),
                    handle));
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
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
  if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
  {
    ACE_HANDLE handle = inherited::get_handle ();
    ACE_ASSERT (handle != ACE_INVALID_HANDLE);
    bool no_delay = Net_Common_Tools::getNoDelay (handle);
    Net_Common_Tools::setNoDelay (handle, true);
    Net_Common_Tools::setNoDelay (handle, no_delay);
  } // end IF
#endif
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
  Stream_Module_t* module_p = NULL;
  Stream_Task_t* task_p = NULL;

  // *NOTE*: feed the data into the stream (queue of the tail module 'reader'
  //         task; Note: 'tail' module to support potential 'streamer' modules)
  module_p = stream_.head ();
  ACE_ASSERT (module_p);
  do
  {
    if (unlikely (!module_p->next () || // aggregator
                  !module_p->next ()->next () || // next is aggregator
                  (!ACE_OS::strcmp (module_p->next ()->name (),
                                    ACE_TEXT ("ACE_Stream_Tail")) ||
                   !ACE_OS::strcmp (module_p->next ()->name (),
                                    ACE_TEXT (STREAM_MODULE_TAIL_NAME)))))
      break;
    module_p = module_p->next ();
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

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_->streamConfiguration->configuration->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->configuration->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  id (),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->configuration->messageAllocator->block ())
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
    if (inherited2::configuration_->streamConfiguration->configuration->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->configuration->messageAllocator->block ())
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
 , stream_ ()
 , allocator_ (NULL)
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
  const ICONNECTOR_T* iconnector_p = NULL;
  const ILISTENER_T* ilistener_p = NULL;
  switch (this->transportLayer ())
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    {
      switch (this->role ())
      {
        case NET_ROLE_CLIENT:
        {
          iconnector_p = static_cast<const ICONNECTOR_T*> (act_in);
          ACE_ASSERT (iconnector_p);
          configuration_p =
            &const_cast<ConfigurationType&> (iconnector_p->getR ());
          break;
        }
        case NET_ROLE_SERVER:
        {
          ilistener_p = static_cast<const ILISTENER_T*> (act_in);
          ACE_ASSERT (ilistener_p);
          configuration_p =
            &const_cast<ConfigurationType&> (ilistener_p->getR ());
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
      iconnector_p = static_cast<const ICONNECTOR_T*> (act_in);
      ACE_ASSERT (iconnector_p);
      configuration_p =
        &const_cast<ConfigurationType&> (iconnector_p->getR ());
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
  socket_handler_configuration_p = configuration_p;

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

  bool handle_manager = false;
  bool handle_stream = false;
  const typename StreamType::SESSION_DATA_CONTAINER_T* session_data_container_p =
    NULL;
  typename StreamType::SESSION_DATA_T* session_data_p = NULL;
  AddressType local_SAP, peer_SAP;

  // *NOTE*: act() has been called already, Net_ConnectionBase has been
  //         initialized

  // step1: initialize/tweak socket, initialize I/O, ...
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::isInitialized_);
  inherited::open (handle_in,
                   messageBlock_in);
  if (unlikely (!inherited::isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%u: failed to HandlerType::open (handle was: %u), aborting\n"),
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

  // step2: initialize/start stream
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  // step2a: connect the stream head message queue with this handler; the queue
  //         will forward outbound data to handle_output ()
  //inherited2::configuration_->streamConfiguration->configuration->notificationStrategy =
  //  this;
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock ());
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

    // step2b: update session data
    // *TODO*: remove type inferences
    session_data_container_p = &stream_.getR_2 ();
    session_data_p =
      &const_cast<typename StreamType::SESSION_DATA_T&> (session_data_container_p->getR ());
    ACE_ASSERT (session_data_p->lock);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);
      session_data_p->connectionStates.insert (std::make_pair (inherited2::state_.handle,
                                                              &(inherited2::state_)));
    } // end lock scope

    // step2c: start stream
    stream_.start ();
    if (unlikely (!stream_.isRunning ()))
    { // already closed ?
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%u: failed to start processing stream, aborting\n"),
                  id ()));
      goto error;
    } // end IF
  } // end lock scope
  handle_stream = true;

  inherited2::state_.status = NET_CONNECTION_STATUS_OK;
  stream_.notify (STREAM_SESSION_MESSAGE_CONNECT,
                  true);

  return;

error:
  inherited2::state_.handle = ACE_INVALID_HANDLE;
  inherited2::state_.status = NET_CONNECTION_STATUS_INITIALIZATION_FAILED;

  if (handle_stream)
  { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock ());
    stream_.stop (true,  // wait for completion ?
                  true); // locked access ?
  } // end IF / lock scope

  if (handle_manager)
    inherited2::deregister ();
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

  int result = -1;
  bool cancel_b = false;
  bool close_socket_b = false;

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
  if ((inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED) && // initialization failed ?
      (inherited2::state_.status != NET_CONNECTION_STATUS_CLOSED))
  {
    inherited2::state_.status = NET_CONNECTION_STATUS_PEER_CLOSED;
    cancel_b = true;
  } // end IF

  // step0b: notify stream ?
//  { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, stream_.getLock (), -1);
    if (likely (notify_ &&
                (inherited2::state_.status != NET_CONNECTION_STATUS_INITIALIZATION_FAILED)))
    {
      notify_ = false;
      stream_.notify (STREAM_SESSION_MESSAGE_DISCONNECT,
                      true);
    } // end IF

    // step1: shut down the processing stream
    stream_.flush (false,  // do not flush inbound data
                   false,  // do not flush session messages
                   false); // flush upstream (if any) ?
    stream_.idle ();
    stream_.stop (true,  // wait for worker(s) (if any)
                  false, // wait for upstream (if any) ?
                  true); // locked access ?
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
  result =
//    (close_socket_b ? inherited::handle_close (handle_in,
//                                               mask_in)
//                    : 0);
    inherited::handle_close (handle_in,
                             mask_in);

  return result;
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

  return stream_.collect (statistic_out.streamStatistic);
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
#endif
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
                                 UserDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::close"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::state_.lock);
    if (inherited2::state_.status == NET_CONNECTION_STATUS_OK)
      inherited2::state_.status = NET_CONNECTION_STATUS_CLOSED;
  } // end lock scope

  // step1: cancel (pending) i/o operation(s)
  inherited::cancel ();

  int result = ACE_OS::shutdown (inherited2::state_.handle, ACE_SHUTDOWN_BOTH);
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
    result = ACE_OS::shutdown (inherited::writeHandle_, ACE_SHUTDOWN_BOTH);
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
#endif
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
  Stream_Module_t* module_p = NULL;
  Stream_Task_t* task_p = NULL;

  // *NOTE*: feed the data into the stream (queue of the tail module 'reader'
  //         task; Note: 'tail' module to support potential 'streamer' modules)
  module_p = stream_.head ();
  ACE_ASSERT (module_p);
  do
  {
    if (unlikely (!module_p->next () || // aggregator
                  !module_p->next ()->next () || // next is aggregator
                  (!ACE_OS::strcmp (module_p->next ()->name (),
                                    ACE_TEXT ("ACE_Stream_Tail")) ||
                   !ACE_OS::strcmp (module_p->next ()->name (),
                                    ACE_TEXT (STREAM_MODULE_TAIL_NAME)))))
      break;
    module_p = module_p->next ();
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
                                 UserDataType>::waitForIdleState () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_AsynchStreamConnectionBase_T::waitForIdleState"));

  // step1: wait for the streams' 'outbound' queue to turn idle
  stream_.idle ();

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

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  if (inherited2::configuration_->streamConfiguration->configuration->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->configuration->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%u: caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  id (),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->configuration->messageAllocator->block ())
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
    if (inherited2::configuration_->streamConfiguration->configuration->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->configuration->messageAllocator->block ())
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
  bool close_b =
      (inherited2::state_.status == NET_CONNECTION_STATUS_CLOSED);
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
#endif
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
#endif
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
//#endif
      break;
    }
    default:
    {
      // update statistic
      inherited2::state_.statistic.receivedBytes += bytes_transferred_i;

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
      if (unlikely (!inherited::initiate_read ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%u: failed to Net_IAsynchSocketHandler::initiate_read(): \"%m\", returning\n"),
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
                  ACE_TEXT ("%u: failed to Net_StreamAsynchTCPSocketBase_T::handle_close(): \"%m\", continuing\n"),
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

  // sanity check
  if (unlikely (!result_in.success ()))
  {
    // connection closed/reset ? --> not an error
    unsigned long error = result_in.error ();
    if ((error != EBADF)                   && // 9    : local close (), happens on Linux
        (error != EPIPE)                   && // 32
        (error != 64)                      && // *TODO*: EHOSTDOWN (- 10000), happens on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        (error != ERROR_OPERATION_ABORTED) && // 995  : local close (), happens on Win32
#endif
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
#endif
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
#endif
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
#endif
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
      inherited2::state_.statistic.receivedBytes += bytes_transferred_i;

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
      break;
    }
  } // end SWITCH

  if (unlikely (!inherited::initiate_read ()))
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
                  ACE_TEXT ("%u: failed to Net_IAsynchSocketHandler::initiate_read(): \"%m\", aborting\n"),
                  id ()));
    goto error;
  } // end IF

  return;

error:
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
#endif

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
