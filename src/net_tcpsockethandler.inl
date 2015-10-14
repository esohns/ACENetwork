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

#include "ace/Event_Handler.h"
#include "ace/Log_Msg.h"
#include "ace/Reactor.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <typename ConfigurationType>
Net_TCPSocketHandler_T<ConfigurationType>::Net_TCPSocketHandler_T ()//MANAGER_T* manager_in)
 : inherited ()
 , inherited2 (NULL,                     // thread manager
               NULL,                     // message queue
               ACE_Reactor::instance ()) // reactor
 , notificationStrategy_ (ACE_Reactor::instance (),      // reactor
                          this,                          // event handler
                          ACE_Event_Handler::WRITE_MASK) // handle output only
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::Net_TCPSocketHandler_T"));

}

template <typename ConfigurationType>
Net_TCPSocketHandler_T<ConfigurationType>::~Net_TCPSocketHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::~Net_TCPSocketHandler_T"));

  //int result = -1;

  //// need to close the socket (see handle_close() below) ?
  //if (inherited2::reference_counting_policy ().value () ==
  //    ACE_Event_Handler::Reference_Counting_Policy::ENABLED)
  //{
  //  result = inherited2::peer_.close ();
  //  if (result == -1)
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to ACE_SOCK_IO::close(): \"%m\", continuing\n")));
  //} // end IF
}

//ACE_Event_Handler::Reference_Count
//Net_TCPSocketHandler_T::add_reference (void)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::add_reference"));

//  inherited::increase ();

//  //return inherited::add_reference ();
//  return 0;
//}

//ACE_Event_Handler::Reference_Count
//Net_TCPSocketHandler_T::remove_reference (void)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::remove_reference"));

//  // *NOTE*: may "delete this"
//  inherited2::decrease ();

//  //// *NOTE*: may "delete this"
//  //return inherited::remove_reference ();
//  return 0;
//}

template <typename ConfigurationType>
int
Net_TCPSocketHandler_T<ConfigurationType>::open (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_TCPSocketHandler_T::open"));

  ConfigurationType* configuration_p =
    static_cast<ConfigurationType*> (arg_in);
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->socketConfiguration);

  // initialize return value
  int result = -1;

  // sanity check(s)
  ACE_ASSERT (arg_in);

  // step1: tweak socket
  // *TODO*: remove type inferences
  ACE_HANDLE handle = inherited2::get_handle ();
  ACE_ASSERT (handle != ACE_INVALID_HANDLE);
  if (configuration_p->socketConfiguration->bufferSize)
  {
    if (!Net_Common_Tools::setSocketBuffer (handle,
                                            SO_RCVBUF,
                                            configuration_p->socketConfiguration->bufferSize))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(SO_RCVBUF,%u) (handle was: %d), aborting\n"),
                  configuration_p->socketConfiguration->bufferSize,
                  handle));
      return -1;
    } // end IF
    if (!Net_Common_Tools::setSocketBuffer (handle,
                                            SO_SNDBUF,
                                            configuration_p->socketConfiguration->bufferSize))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::setSocketBuffer(SO_SNDBUF,%u) (handle was: %d), aborting\n"),
                  configuration_p->socketConfiguration->bufferSize,
                  handle));
      return -1;
    } // end IF
  } // end IF
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
                                    configuration_p->socketConfiguration->linger,
                                    std::numeric_limits<unsigned short>::max ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::setLinger(%s, -1) (handle was: %d), aborting\n"),
                (configuration_p->socketConfiguration->linger ? ACE_TEXT ("true")
                                                              : ACE_TEXT ("false")),
                handle));
    return -1;
  } // end IF

  // register with the reactor
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
  // *NOTE*: let the reactor manage this handler...
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

template <typename ConfigurationType>
int
Net_TCPSocketHandler_T<ConfigurationType>::handle_close (ACE_HANDLE handle_in,
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
      int error = ACE_OS::last_error ();
      if ((mask_in == ACE_Event_Handler::ALL_EVENTS_MASK) &&
          ((error == ETIMEDOUT) ||   // 110: failed to connect: timed out
           (error == ECONNREFUSED))) // 111: failed to connect: connection refused
      {
        // *IMPORTANT NOTE*: the connection hasn't been open()ed / registered
        //                   --> remove the (final) reference manually
        // *NOTE*: the dtor (deregisters from the reactor and) close()s the
        //         stream handle
        inherited2::reference_counting_policy ().value (ACE_Event_Handler::Reference_Counting_Policy::ENABLED);
        result = inherited2::remove_reference ();
        ACE_ASSERT (result == 0);
        break;
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
          if ((mask_in == ACE_Event_Handler::ALL_EVENTS_MASK) &&
              (error == ENOENT)) // <-- user abort during initialization
                                 //     (prior to registration)
            result = 0;
          else
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@, %d): \"%m\", aborting\n"),
                        this,
                        mask_in));
        } // end IF
      } // end IF

      break;
    }
    default:
    {
// *PORTABILITY*: this isn't entirely portable...
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
#endif

      break;
    }
  } // end SWITCH

  return result;
}
