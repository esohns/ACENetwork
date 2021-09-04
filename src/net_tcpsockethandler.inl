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

#if defined (SSL_SUPPORT)
#include "openssl/ssl.h"
#endif // SSL_SUPPORT

#include "ace/Event_Handler.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"

#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename StreamType,
          typename ConfigurationType>
Net_TCPSocketHandler_T<ACE_SYNCH_USE,
                       StreamType,
                       ConfigurationType>::Net_TCPSocketHandler_T ()//MANAGER_T* manager_in)
 : inherited ()
 , inherited2 (NULL,                     // thread manager
               NULL,                     // message queue
               ACE_Reactor::instance ()) // reactor
 , inherited3 (ACE_Reactor::instance (),      // reactor
               this,                          // event handler
               ACE_Event_Handler::WRITE_MASK) // handle output only
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::Net_TCPSocketHandler_T"));

}

template <ACE_SYNCH_DECL,
          typename StreamType,
          typename ConfigurationType>
int
Net_TCPSocketHandler_T<ACE_SYNCH_USE,
                       StreamType,
                       ConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::open"));

  // sanity check(s)
  ConfigurationType* configuration_p =
    reinterpret_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);

  // step1: tweak socket
  ACE_HANDLE handle = inherited2::get_handle ();
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);
  if (likely (configuration_p->bufferSize))
  {
    if (unlikely (!Net_Common_Tools::setSocketBuffer (handle,
                                                      SO_RCVBUF,
                                                      configuration_p->bufferSize)))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
    if (unlikely (!Net_Common_Tools::setSocketBuffer (handle,
                                                      SO_SNDBUF,
                                                      configuration_p->bufferSize)))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_SNDBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_SNDBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
  } // end IF

  // disable Nagle's algorithm
  if (unlikely (!Net_Common_Tools::setNoDelay (handle,
                                               NET_SOCKET_DEFAULT_TCP_NODELAY)))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(%s) (handle was: 0x%@), aborting\n"),
                (NET_SOCKET_DEFAULT_TCP_NODELAY ? ACE_TEXT ("true") : ACE_TEXT ("false")),
                handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(%s) (handle was: %d), aborting\n"),
                (NET_SOCKET_DEFAULT_TCP_NODELAY ? ACE_TEXT ("true") : ACE_TEXT ("false")),
                handle));
#endif // ACE_WIN32 || ACE_WIN64
    return -1;
  } // end IF
  if (unlikely (!Net_Common_Tools::setKeepAlive (handle,
                                                 NET_SOCKET_DEFAULT_TCP_KEEPALIVE)))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: 0x%@), aborting\n"),
                (NET_SOCKET_DEFAULT_TCP_KEEPALIVE ? ACE_TEXT ("true") : ACE_TEXT ("false")),
                handle));
#else
        ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: %d), aborting\n"),
                (NET_SOCKET_DEFAULT_TCP_KEEPALIVE ? ACE_TEXT ("true") : ACE_TEXT ("false")),
                handle));
#endif // ACE_WIN32 || ACE_WIN64
    return -1;
  } // end IF
  if (unlikely (!Net_Common_Tools::setLinger (handle,
                                              configuration_p->linger,
                                              std::numeric_limits<unsigned short>::max ())))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s,-1) (handle was: 0x%@), aborting\n"),
                (configuration_p->linger ? ACE_TEXT ("true") : ACE_TEXT ("false")),
                handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s, -1) (handle was: %d), aborting\n"),
                (configuration_p->linger ? ACE_TEXT ("true") : ACE_TEXT ("false")),
                handle));
#endif // ACE_WIN32 || ACE_WIN64
    return -1;
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename StreamType,
          typename ConfigurationType>
int
Net_TCPSocketHandler_T<ACE_SYNCH_USE,
                       StreamType,
                       ConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::handle_close"));

  // initialize return value
  int result = 0;

  // *IMPORTANT NOTE*: due to reference counting, the
  // ACE_Svc_Handler::shutdown() method will crash, as it references a
  // connection recycler AFTER removing the connection from the reactor (which
  // releases (at least one) reference). In the case that "this" is the final
  // reference, this leads to a crash. (see code)
  // --> avoid invoking ACE_Svc_Handler::shutdown()
  // --> this means that "manual" cleanup is necessary (see below)

  // *IMPORTANT NOTE*: due to reference counting, the base-class function is a
  //                   NOP (see code) --> this means that clean up is necessary
  //                   on:
  // - connect failed (e.g. connection refused)
  // - accept failed (e.g. too many connections)
  // - ... ?

  switch (mask_in)
  {
    // *NOTE*: 'this' is already being removed from the reactor
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
      break; // --> nothing to do
    case ACE_Event_Handler::WRITE_MASK:      // --> socket has been closed (send failed) (DevPoll)
    case ACE_Event_Handler::EXCEPT_MASK:     // --> socket has been closed (send failed)
      break;
    case ACE_Event_Handler::ALL_EVENTS_MASK: // - connect failed (e.g. connection refused/timed out) /
                                             // - accept failed (e.g. too many connections) /
                                             // - select failed (EBADF see Select_Reactor_T.cpp) /
                                             // - user abort
                                             // - ... ?
    {
      // *NOTE*: --> client side: connection failed, no need to deregister from
      //                          the reactor
      //         --> server side: too many open connections
      int error = ACE_OS::last_error ();
      if ((error == ETIMEDOUT) ||  // 110/10060: failed to connect: timed out
          (error == ECONNREFUSED)) // 111/10061: failed to connect: connection refused
      {
        // *IMPORTANT NOTE*: the connection hasn't been open()ed / registered
        //                   --> remove the (final) reference manually
        // *NOTE*: the dtor (deregisters from the reactor and) close()s the
        //         stream handle
//        inherited2::reference_counting_policy ().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);
//        result = inherited2::remove_reference ();
//        ACE_ASSERT (result == 0);
//        break;
      } // end IF

      if (handle_in != ACE_INVALID_HANDLE)
      {
        ACE_Reactor* reactor_p = inherited2::reactor ();
        ACE_ASSERT (reactor_p);
        //      ACE_Reactor_Mask mask =
        //          ((mask_in == ACE_Event_Handler::EXCEPT_MASK) ? ACE_Event_Handler::WRITE_MASK
        //                                                       : mask_in);
        result = reactor_p->remove_handler (this,
                                            (mask_in |
                                             ACE_Event_Handler::DONT_CALL));
        if (result == -1)
        {
          error = ACE_OS::last_error ();
          if (error == ENOENT) // <-- user abort during initialization
                               //     (prior to registration)
            result = 0;
          else if (error != EPERM) // Local close
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@, %d): \"%m\", aborting\n"),
                        this,
                        mask_in));
        } // end IF
      } // end IF
      break;
    }
    default:
    {
// *PORTABILITY*: this isn't entirely portable
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %d) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %d) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
  } // end SWITCH

  // *IMPORTANT NOTE*: ensure dtor does not ACE_Svc_Handler::shutdown() again
  inherited2::closing_ = true;
  inherited2::shutdown ();

  return result;
}

//////////////////////////////////////////

#if defined (SSL_SUPPORT)
template <ACE_SYNCH_DECL,
          typename ConfigurationType>
Net_TCPSocketHandler_T<ACE_SYNCH_USE,
                       ACE_SSL_SOCK_Stream,
                       ConfigurationType>::Net_TCPSocketHandler_T ()//MANAGER_T* manager_in)
 : inherited ()
 , inherited2 (NULL,                     // thread manager
               NULL,                     // message queue
               ACE_Reactor::instance ()) // reactor
 , inherited3 (ACE_Reactor::instance (),      // reactor
               this,                          // event handler
               ACE_Event_Handler::WRITE_MASK) // handle output only
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::Net_TCPSocketHandler_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
int
Net_TCPSocketHandler_T<ACE_SYNCH_USE,
                       ACE_SSL_SOCK_Stream,
                       ConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::open"));

  // sanity check(s)
  ConfigurationType* configuration_p =
    reinterpret_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);

  // initialize return value
  int result = -1;

  // step1: tweak socket
  // *TODO*: remove type inferences
  ACE_HANDLE handle = inherited2::get_handle ();
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);
  if (configuration_p->bufferSize)
  {
    if (!Net_Common_Tools::setSocketBuffer (handle,
                                            SO_RCVBUF,
                                            configuration_p->bufferSize))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#endif
    } // end IF
    if (!Net_Common_Tools::setSocketBuffer (handle,
                                            SO_SNDBUF,
                                            configuration_p->bufferSize))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(0x%@,SO_RCVBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(%d,SO_SNDBUF,%u), continuing\n"),
                  handle,
                  configuration_p->bufferSize));
#endif
    } // end IF
  } // end IF

  // disable Nagle's algorithm
  if (!Net_Common_Tools::setNoDelay (handle,
                                     NET_SOCKET_DEFAULT_TCP_NODELAY))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setNoDelay(%s) (handle was: %d), aborting\n"),
                (NET_SOCKET_DEFAULT_TCP_NODELAY ? ACE_TEXT ("true")
                                                : ACE_TEXT ("false")),
                handle));
    return -1;
  } // end IF
  if (!Net_Common_Tools::setKeepAlive (handle,
                                       NET_SOCKET_DEFAULT_TCP_KEEPALIVE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setKeepAlive(%s) (handle was: %d), aborting\n"),
                (NET_SOCKET_DEFAULT_TCP_KEEPALIVE ? ACE_TEXT ("true")
                                                  : ACE_TEXT ("false")),
                handle));
    return -1;
  } // end IF
  if (!Net_Common_Tools::setLinger (handle,
                                    configuration_p->linger,
                                    std::numeric_limits<unsigned short>::max ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s, -1) (handle was: %d), aborting\n"),
                (configuration_p->linger ? ACE_TEXT ("true") : ACE_TEXT ("false")),
                handle));
    return -1;
  } // end IF

  // step2: tweak SSL context
  typename inherited2::stream_type& peer_stream_r = inherited2::peer ();
  struct ssl_st* ssl_p = peer_stream_r.ssl ();
  ACE_ASSERT (ssl_p);
  struct ssl_ctx_st* ssl_ctx_p = SSL_get_SSL_CTX (ssl_p);
  ACE_ASSERT (ssl_ctx_p);
  long ssl_ctx_mode_bitmask = SSL_CTX_set_mode (ssl_ctx_p,
                                                SSL_MODE_AUTO_RETRY);
  ACE_ASSERT (ssl_ctx_mode_bitmask & SSL_MODE_AUTO_RETRY);

  // step3: register with the reactor
  result = inherited2::open (arg_in);
  if (result == -1)
  {
    // *NOTE*: this can happen when the connection handle is still registered
    //         with the reactor (i.e. the reactor is still processing events on
    //         a file descriptor that has been closed and is now being reused by
    //         the system)
    // *NOTE*: more likely, this happened because the (select) reactor is out of
    //         "free" (read) slots
    int error = ACE_OS::last_error ();
    if (error)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Svc_Handler::open(0x%@/%d): \"%m\", aborting\n"),
                  arg_in, handle));
    return -1;
  } // end IF
  // *NOTE*: let the reactor manage this handler
  if (inherited2::reference_counting_policy ().value () ==
      ACE_Event_Handler::Reference_Counting_Policy::ENABLED)
    inherited2::remove_reference ();

  // *NOTE*: registered with the reactor (READ_MASK) at this point

//   // ...register for writes (WRITE_MASK) as well
//   if (reactor ()->register_handler (this,
//                                     ACE_Event_Handler::WRITE_MASK) == -1)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to ACE_Reactor::register_handler(WRITE_MASK): \"%m\", aborting\n")));
//     return -1;
//   } // end IF

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
int
Net_TCPSocketHandler_T<ACE_SYNCH_USE,
                       ACE_SSL_SOCK_Stream,
                       ConfigurationType>::handle_close (ACE_HANDLE handle_in,
                                                         ACE_Reactor_Mask mask_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::handle_close"));

  // initialize return value
  int result = 0;

  // *IMPORTANT NOTE*: due to reference counting, the
  // ACE_Svc_Handle::shutdown() method will crash, as it references a
  // connection recycler AFTER removing the connection from the reactor (which
  // releases (at least one) reference). In the case that "this" is the final
  // reference, this leads to a crash. (see code)
  // --> avoid invoking ACE_Svc_Handle::shutdown()
  // --> this means that "manual" cleanup is necessary (see below)

  // *IMPORTANT NOTE*: due to reference counting, the base-class function is a
  // NOP (see code) --> this means that clean up is necessary on:
  // - connect failed (e.g. connection refused)
  // - accept failed (e.g. too many connections)
  // - ... ?

  int error = 0;
  switch (mask_in)
  {
    // *NOTE*: this is already being removed from the reactor (check stack)
    //         --> just return
    case ACE_Event_Handler::READ_MASK:       // --> socket has been closed
      break;
    case ACE_Event_Handler::WRITE_MASK:      // --> socket has been closed (send failed) (DevPoll)
    case ACE_Event_Handler::EXCEPT_MASK:     // --> socket has been closed (send failed)
      break;
    case ACE_Event_Handler::ALL_EVENTS_MASK: // - connect failed (e.g. connection refused/timed out) /
                                             // - accept failed (e.g. too many connections) /
                                             // - select failed (EBADF see Select_Reactor_T.cpp) /
                                             // - user abort
                                             // - ... ?
    {
      // *NOTE*: --> client side: connection failed, no need to deregister from
      //                          the reactor
      //         --> server side: too many open connections
      error = ACE_OS::last_error ();
      if (((error == ETIMEDOUT) ||   // 110/10060: failed to connect: timed out
           (error == ECONNREFUSED))) // 111/10061: failed to connect: connection refused
      {
        // *IMPORTANT NOTE*: the connection hasn't been open()ed / registered
        //                   --> remove the (final) reference manually
        // *NOTE*: the dtor (deregisters from the reactor and) close()s the
        //         socket handle
        //inherited2::reference_counting_policy ().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);
        //result = inherited2::remove_reference ();
        //ACE_ASSERT (result == 0);
        //break;
      } // end IF

      if (handle_in != ACE_INVALID_HANDLE)
      {
        ACE_Reactor* reactor_p = inherited2::reactor ();
        ACE_ASSERT (reactor_p);
        //      ACE_Reactor_Mask mask =
        //          ((mask_in == ACE_Event_Handler::EXCEPT_MASK) ? ACE_Event_Handler::WRITE_MASK
        //                                                       : mask_in);
        result = reactor_p->remove_handler (this,
                                            (mask_in |
                                             ACE_Event_Handler::DONT_CALL));
        if (unlikely (result == -1))
        {
          error = ACE_OS::last_error ();
          if (error == ENOENT) // <-- user abort during initialization
                               //     (prior to registration)
            result = 0;
          else if (error != EPERM) // Local close
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@, %d): \"%m\", aborting\n"),
                        this,
                        mask_in));
        } // end IF
      } // end IF

      break;
    }
    default:
    {
// *PORTABILITY*: this isn't entirely portable
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %@, mask: %d) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handle_close called for unknown reasons (handle: %d, mask: %d) --> check implementation !, continuing\n"),
                  handle_in,
                  mask_in));
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
  } // end SWITCH

  result = inherited2::peer_.close (); // shutdown SSL context
  if (unlikely (result == -1))
  {
    error = ACE_OS::last_error ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (error != ENOTSOCK) // 10038: local close
#else
    if (error != EBADF) // // 9: EBADF
#endif // ACE_WIN32 || ACE_WIN64
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SSL_SOCK_Stream::close(): \"%m\", aborting\n")));
    else
      result = 0;
  } // end IF

  return result;
}
#endif // SSL_SUPPORT
