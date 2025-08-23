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

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Reactor.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_macros.h"

#include "net_server_defines.h"

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::Net_Server_SSL_Listener_T ()
 : inherited (NULL, // use global (default) reactor
              1)    // always accept ALL pending connections
 , inherited2 ()
 , configuration_ (NULL)
 , hasChanged_ (false)
 , isInitialized_ (false)
 , isListening_ (false)
 , isOpen_ (false)
 , isSuspended_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::Net_Server_SSL_Listener_T"));

}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::~Net_Server_SSL_Listener_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::~Net_Server_SSL_Listener_T"));

  int result = -1;

  if (isSuspended_)
  {
    result = inherited::resume ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Acceptor::resume(): \"%m\", continuing\n")));
  } // end IF

  if (isOpen_)
  {
    result = inherited::close ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Acceptor::close(): \"%m\", continuing\n")));
  } // end IF
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::handle_accept_error (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::handle_accept_error"));

  ACE_DEBUG ((LM_WARNING,
              ACE_TEXT ("failed to accept connection, continuing\n")));
  inherited::dump ();

  // *NOTE*: remain registered with the reactor
  return 0;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
bool
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::start (ACE_Time_Value* timeout_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::start"));

  ACE_UNUSED_ARG (timeout_in);

  int result = -1;

  // sanity check(s)
  if (unlikely (!isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, aborting\n")));
    return false;
  } // end IF
  if (unlikely (isListening_))
    return true; // nothing to do

  if (hasChanged_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("configuration has changed, stopping...\n")));
    hasChanged_ = false;

    if (isSuspended_)
      stop (true, true);

    if (isOpen_)
    {
      result = inherited::close ();
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Acceptor::close(): \"%m\", aborting\n")));
        isOpen_ = false;
        isListening_ = false;
        return false;
      } // end IF
      isOpen_ = false;
    } // end IF
  } // end IF

  if (isOpen_)
  {
    // already open --> resume listening

    // sanity check(s)
    ACE_ASSERT (isSuspended_);

    result = inherited::resume ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Acceptor::resume(): \"%m\", aborting\n")));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("resumed listening...\n")));

    isSuspended_ = false;
    isListening_ = true;

    return true;
  } // end IF

  // not running --> start listening
  // sanity check(s)
  ACE_ASSERT (configuration_);

  // *TODO*: remove type inferences
  if (unlikely (configuration_->socketConfiguration.useLoopBackDevice))
  {
    result =
      configuration_->socketConfiguration.address.set (configuration_->socketConfiguration.address.get_port_number (), // port
                                                       INADDR_LOOPBACK,                                                // IP address
                                                       1,                                                              // encode ?
                                                       0);                                                             // map ?
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to AddressType::set(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF
  result =
    inherited::open (configuration_->socketConfiguration.address, // local address
                     ACE_Reactor::instance (),                    // reactor handle
                     ACE_NONBLOCK,                                // flags (use non-blocking sockets)
                     //0,                                         // flags (*NOTE*: default is blocking sockets)
                     1,                                           // accept all pending connections
                     1);                                          // try to re-use address
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Acceptor::open(): \"%m\", aborting\n")));
    return false;
  } // end IF
  isOpen_ = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: started listening: %s\n"),
              inherited::get_handle (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: started listening: %s\n"),
              inherited::get_handle (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  isListening_ = true;

  return true;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::stop (bool,
                                               bool)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::stop"));

  // sanity check(s)
  if (!isListening_)
    return; // nothing to do
  ACE_ASSERT (isOpen_);

  int result = inherited::suspend ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Acceptor::suspend(): \"%m\", returning\n")));
    return;
  } // end IF
  isSuspended_ = true;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("suspended listening...\n")));

  isListening_ = false;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
bool
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  hasChanged_ = true;
  isInitialized_ = true;

  return true;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::disconnect (ACE_HANDLE handle_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::disconnect"));

  ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, inherited2::lock_);
  for (SUBSCRIBERS_CONST_ITERATOR_T iterator = subscribers_.begin ();
       iterator != subscribers_.end ();
       iterator++)
  {
    try {
      (*iterator)->disconnect (handle_in);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("exception caught from subscriber, continuing\n")));
    }
  } // end FOR
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::dump_state"));

  int result = -1;

  ACE_TCHAR* buffer_p = NULL;
  result = inherited::info (&buffer_p, BUFSIZ);
  if (unlikely ((result == -1) || !buffer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Acceptor::info(): \"%m\", returning\n")));
    return;
  } // end IF
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%s\n"),
              buffer_p));

  // clean up
  delete [] buffer_p; buffer_p = NULL;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  // *TODO*: remove type inferences
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (true)); // managed ?
  if (unlikely (!handler_out))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          StreamType,
                          UserDataType>::activate_svc_handler (HandlerType* svc_handler)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::activate_svc_handler"));

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Acceptor.cpp:334

  int result = 0;

  // See if we should enable non-blocking I/O on the <svc_handler>'s
  // peer.
  if (ACE_BIT_ENABLED (this->flags_,
    ACE_NONBLOCK))
  {
    if (svc_handler->peer ().enable (ACE_NONBLOCK) == -1)
      result = -1;
  }
  // Otherwise, make sure it's disabled by default.
  else if (svc_handler->peer ().disable (ACE_NONBLOCK) == -1)
    result = -1;

  ILISTENER_T* ilistener_p = this;
  if (result == 0 && svc_handler->open (ilistener_p) == -1)
    result = -1;

  if (result == -1)
    // The connection was already made; so this close is a "normal" close
    // operation.
    svc_handler->close (NORMAL_CLOSE_OPERATION);
  else if (result == 0)
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::MUTEX, aGuard, inherited2::lock_, -1);
    for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
         iterator != subscribers_.end ();
         iterator++)
    {
      try {
        (*iterator)->connect (svc_handler->peer ().get_handle ());
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("exception caught from subscriber, continuing\n")));
      }
    } // end FOR
  } // end ELSE IF | lock scope

  return result;
}
