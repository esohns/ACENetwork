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

#include "net_macros.h"

#include "net_server_defines.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::Net_Server_Listener_T ()
 : inherited (NULL, // use global (default) reactor
              1)    // always accept ALL pending connections
 , configuration_ ()
 , handlerConfiguration_ ()
 , isInitialized_ (false)
 , isListening_ (false)
 , isOpen_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::Net_Server_Listener_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::~Net_Server_Listener_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::~Net_Server_Listener_T"));

  if (isOpen_)
    inherited::close ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
const HandlerConfigurationType&
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::get"));

  return handlerConfiguration_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::initialize (const HandlerConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::initialize"));

  handlerConfiguration_ = configuration_in;

  return true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::useReactor () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::useReactor"));

  return true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::isInitialized () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::isInitialized"));

  return isInitialized_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::handle_accept_error (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::handle_accept_error"));

//  ACE_DEBUG((LM_ERROR,
//             ACE_TEXT("failed to accept connection...\n")));

  inherited::dump ();

  // *NOTE*: remain registered with the reactor
  return 0;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::start"));

  int result = -1;

  // sanity check(s)
  if (isListening_)
    return; // nothing to do...

  if (isOpen_)
  {
    // already open (maybe suspended ?) --> resume listening...
    result = inherited::resume ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Acceptor::resume(): \"%m\", returning\n")));
    else
    {
      isListening_ = true;

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("resumed listening...\n")));
    } // end ELSE

    return;
  } // end IF

  // sanity check: configured ?
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF

  // not running --> start listening
  ACE_INET_Addr local_address;
  // *TODO*: remove type inferences
  if (configuration_.useLoopbackDevice)
    result = local_address.set (configuration_.portNumber, // port number
                                // *PORTABILITY*: disambiguation needed under Win32
                                ACE_LOCALHOST,             // hostname
                                1,                         // encode ?
                                ACE_ADDRESS_FAMILY_INET);  // address family
  else
    result = local_address.set (configuration_.portNumber,            // port number
                                // *TODO*: bind to specific interface/address ?
                                static_cast<ACE_UINT32> (INADDR_ANY), // hostname
                                1,                                    // encode ?
                                0);                                   // map IPv6 to IPv4 ?
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
    return;
  } // end IF
  result = inherited::open (local_address,            // local address
                            ACE_Reactor::instance (), // corresp. reactor
                            ACE_NONBLOCK,             // flags (use non-blocking sockets !)
                            //0,                        // flags (default is blocking sockets)
                            1,                        // always accept ALL pending connections
                            1);                       // try to re-use address
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Acceptor::open(): \"%m\", returning\n")));
    return;
  } // end IF
  else
    isOpen_ = true;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: started listening (port: %u)...\n"),
              inherited::get_handle (),
              configuration_.portNumber));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: started listening (port: %u)...\n"),
              inherited::get_handle (),
              configuration_.portNumber));
#endif

  isListening_ = true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::stop (bool waitForCompletion_in,
                                           bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!isListening_)
    return; // nothing to do...

  int result = -1;
//  if (inherited::suspend() == -1)
//  {
//    ACE_DEBUG((LM_ERROR,
//               ACE_TEXT("failed to ACE_Acceptor::suspend(): \"%m\", returning\n")));
//    return;
//  } // end IF
  result = inherited::close ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Acceptor::close(): \"%m\", returning\n")));

    // clean up
    isOpen_ = false;
    isListening_ = false;

    return;
  } // end IF
  isOpen_ = false;
  isListening_ = false;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stopped listening...\n")));
//  ACE_DEBUG((LM_DEBUG,
//             ACE_TEXT("suspended listening...\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::isRunning"));

  return isListening_;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::initialize"));

  configuration_ = configuration_in;
  isInitialized_ = true;

  return true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::dump_state"));

  int result = -1;

  ACE_TCHAR* buffer_p = NULL;
  result = inherited::info (&buffer_p, BUFSIZ);
  if ((result == -1) || !buffer_p)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Acceptor::info(): \"%m\", returning\n")));
    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              buffer_p));

  // clean up
  delete [] buffer_p;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Server_Listener_T<HandlerType,
                      AddressType,
                      ConfigurationType,
                      StateType,
                      StreamType,
                      HandlerConfigurationType,
                      UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_.socketHandlerConfiguration);

  // default behavior
  // *TODO*: remove type inference
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (configuration_.connectionManager,
                                 configuration_.socketHandlerConfiguration->statisticReportingInterval));
                                 //configuration_.statisticReportingInterval));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}
