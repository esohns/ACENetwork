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

#include "net_connection_manager_common.h"
#include "net_macros.h"

#include "net_server_defines.h"

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::Net_Server_Listener_T ()
 : inherited (NULL, // use global (default) reactor
              1)    // always accept ALL pending connections
 , configuration_ (NULL)
 , isInitialized_ (false)
 , isListening_ (false)
 , isOpen_ (false)
 , listeningPort_ (NET_SERVER_DEFAULT_LISTENING_PORT)
 , statisticCollectionInterval_ (0)
 , useLoopback_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::Net_Server_Listener_T"));

}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::~Net_Server_Listener_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::~Net_Server_Listener_T"));

  if (isOpen_)
    inherited::close ();
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
bool
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::initialize (const Net_ListenerConfiguration_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::initialize"));

  configuration_ = configuration_in.socketHandlerConfiguration;
  listeningPort_ = configuration_in.portNumber;
  statisticCollectionInterval_ = configuration_in.statisticCollectionInterval;
  useLoopback_ = configuration_in.useLoopbackDevice;

  isInitialized_ = true;

  return true;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
bool
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::isInitialized () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::isInitialized"));

  return isInitialized_;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
int
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::handle_accept_error (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::handle_accept_error"));

//  ACE_DEBUG((LM_ERROR,
//             ACE_TEXT("failed to accept connection...\n")));

  inherited::dump ();

  // *NOTE*: remain registered with the reactor
  return 0;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
void
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::start ()
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
                  ACE_TEXT ("failed to ACE_Acceptor::resume(): \"%m\", aborting")));
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
                ACE_TEXT ("not initialized, aborting")));
    return;
  } // end IF

  // not running --> start listening
  ACE_INET_Addr local_sap;
  if (useLoopback_)
    local_sap.set (listeningPort_, // local SAP
                   // *PORTABILITY*: disambiguation needed under Windows
                   ACE_LOCALHOST,  // hostname
                   1,              // encode ?
                   AF_INET);       // address family
  else
    local_sap.set (listeningPort_,                       // local SAP
                   // *TODO*: bind to specific interface/address ?
                   static_cast<ACE_UINT32> (INADDR_ANY), // hostname
                   1,                                    // encode ?
                   0);                                   // map IPv6 to IPv4 ?
  result = inherited::open (local_sap,                // local SAP
                            ACE_Reactor::instance (), // corresp. reactor
                            ACE_NONBLOCK,             // flags (use non-blocking sockets !)
                            //0,                        // flags (default is blocking sockets)
                            1,                        // always accept ALL pending connections
                            1);                       // try to re-use address
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Acceptor::open(): \"%m\", returning")));
    return;
  } // end IF
  else
    isOpen_ = true;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("started listening (port: %u)...\n"),
              listeningPort_));

  isListening_ = true;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
void
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::stop (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::stop"));

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
    isListening_ = false;
    isOpen_ = false;

    return;
  } // end IF
  else
    isOpen_ = false;

  isListening_ = false;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("stopped listening...\n")));
//  ACE_DEBUG((LM_DEBUG,
//             ACE_TEXT("suspended listening...\n")));
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
bool
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::isRunning () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::isRunning"));

  return isListening_;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
void
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::dump_state () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::dump_state"));

  int result = -1;

  ACE_TCHAR* buffer_p = NULL;
  result = inherited::info (&buffer_p, BUFSIZ);
  if ((result == -1) ||
      !buffer_p)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Acceptor::info(): \"%m\", aborting\n")));
    return;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              buffer_p));

  // clean up
  delete [] buffer_p;
}

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename ITransportLayerType,
          typename ConnectionType>
int
Net_Server_Listener_T<ConfigurationType,
                      SocketHandlerConfigurationType,
                      UserDataType,
                      ITransportLayerType,
                      ConnectionType>::make_svc_handler (ConnectionType*& handler_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Listener_T::make_svc_handler"));

  // init return value(s)
  handler_inout = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_inout,
                    ConnectionType (NET_CONNECTIONMANAGER_SINGLETON::instance (),
                                    statisticCollectionInterval_));
  if (!handler_inout)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_inout ? -1 : 0);
}
