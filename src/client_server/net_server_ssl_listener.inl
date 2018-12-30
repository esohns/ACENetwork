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
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
                          StreamType,
                          UserDataType>::Net_Server_SSL_Listener_T ()
 : inherited (NULL, // use global (default) reactor
              1)    // always accept ALL pending connections
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
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
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
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
                          StreamType,
                          UserDataType>::handle_accept_error (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::handle_accept_error"));

  ACE_DEBUG ((LM_WARNING,
              ACE_TEXT ("failed to accept connection, continuing\n")));
#if defined (_DEBUG)
  inherited::dump ();
#endif // _DEBUG

  // *NOTE*: remain registered with the reactor
  return 0;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
                          StreamType,
                          UserDataType>::start (ACE_thread_t& threadId_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::start"));

  // initialize return value(s)
  threadId_out = 0;

  int result = -1;

  // sanity check(s)
  if (unlikely (!isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (unlikely (isListening_))
    return; // nothing to do

  if (hasChanged_)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("configuration has changed, stopping...\n")));
    hasChanged_ = false;

    if (isSuspended_)
      stop ();

    if (isOpen_)
    {
      result = inherited::close ();
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Acceptor::close(): \"%m\", returning\n")));
        isOpen_ = false;
        isListening_ = false;
        return;
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
                  ACE_TEXT ("failed to ACE_Acceptor::resume(): \"%m\", returning\n")));
      return;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("resumed listening...\n")));
#endif // _DEBUG

    isSuspended_ = false;
    isListening_ = true;

    return;
  } // end IF

  // not running --> start listening
  // *TODO*: remove type inferences
  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (unlikely (configuration_->socketHandlerConfiguration.socketConfiguration.useLoopBackDevice))
  {
    result =
      configuration_->address.set (configuration_->socketHandlerConfiguration.socketConfiguration.address.get_port_number (), // port
                                   // *PORTABILITY*: disambiguate this under Windows
                                   // *TODO*: bind to specific interface/address ?
                                   ACE_LOCALHOST,                                                                             // hostname
                                   1,                                                                                         // encode ?
                                   AF_INET);                                                                                  // address family
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
      return;
    } // end IF
  } // end IF
  result =
      inherited::open (configuration_->socketHandlerConfiguration.socketConfiguration.address, // local address
                       ACE_Reactor::instance (),                                               // reactor handle
                       ACE_NONBLOCK,                                                           // flags (use non-blocking sockets)
                       //0,                                                                    // flags (*NOTE*: default is blocking sockets)
                       1,                                                                      // accept all pending connections
                       1);                                                                     // try to re-use address
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Acceptor::open(): \"%m\", returning\n")));
    return;
  } // end IF
  isOpen_ = true;
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: started listening: %s\n"),
              inherited::get_handle (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketHandlerConfiguration.socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: started listening: %s\n"),
              inherited::get_handle (),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_->socketHandlerConfiguration.socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

  isListening_ = true;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
                          StreamType,
                          UserDataType>::stop (bool waitForCompletion_in,
                                               bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!isListening_) return; // nothing to do
  ACE_ASSERT (isOpen_);

  int result = inherited::suspend ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Acceptor::suspend(): \"%m\", returning\n")));
    return;
  } // end IF
  isSuspended_ = true;
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("suspended listening...\n")));
#endif // _DEBUG

  isListening_ = false;
}

template <typename HandlerType,
          typename AcceptorType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
bool
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
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
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
void
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
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
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
                          StreamType,
                          UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_SSL_Listener_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  // default behavior
  // *TODO*: remove type inferences
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (configuration_->connectionManager,
                                 configuration_->socketHandlerConfiguration.statisticReportingInterval));
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
          typename ConnectionConfigurationType,
          typename StreamType,
          typename UserDataType>
int
Net_Server_SSL_Listener_T<HandlerType,
                          AcceptorType,
                          AddressType,
                          ConfigurationType,
                          StateType,
                          ConnectionConfigurationType,
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

  return result;
}
