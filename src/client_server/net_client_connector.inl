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

#include "common_defines.h"

#include "net_macros.h"

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::Net_Client_Connector_T (ICONNECTION_MANAGER_T* connectionManager_in,
                                                              unsigned int statisticCollectionInterval_in)
 : inherited (ACE_Reactor::instance (), // default reactor
              ACE_NONBLOCK)             // flags: non-blocking I/O
              //0)                       // flags
 , configuration_ ()
 , connectionManager_ (connectionManager_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::initialize (const HandlerConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::initialize"));

  configuration_ = configuration_in;

  return true;
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
const HandlerConfigurationType&
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::get"));

  return configuration_;
}

//template <typename AddressType,
//          typename SocketConfigurationType,
//          typename ConfigurationType,
//          typename HandlerConfigurationType,
//          typename UserDataType,
//          typename StateType,
//          typename StreamType,
//          typename HandlerType>
//bool
//Net_Client_Connector_T<AddressType,
//                       SocketConfigurationType,
//                       ConfigurationType,
//                       HandlerConfigurationType,
//                       UserDataType,
//                       StateType,
//                       StreamType,
//                       HandlerType>::useReactor () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::useReactor"));
//
//  return true;
//}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

  int result = inherited::close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::close(): \"%m\", continuing\n")));
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
ACE_HANDLE
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::connect (const AddressType& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  HandlerType* handler_p = NULL;
  result =
      inherited::connect (handler_p,                       // service handler
                          address_in,                      // remote SAP
                          ACE_Synch_Options::defaults,     // synch options
                          ACE_sap_any_cast (AddressType&), // local SAP
                          1,                               // re-use address (SO_REUSEADDR) ?
                          O_RDWR,                          // flags
                          0);                              // perms
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = address_in.addr_to_string (buffer, sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to AddressType::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  return handler_p->get_handle ();
}

template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::activate_svc_handler (HandlerType* svc_handler)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::activate_svc_handler"));

  // pre-initialize the connection handler
  // *TODO*: remove type inference
  svc_handler->set (NET_ROLE_CLIENT);

  // *IMPORTANT NOTE*: this bit is mostly copy/pasted from Connector.cpp:251

  // No errors initially
  int error = 0;

  // See if we should enable non-blocking I/O on the <svc_handler>'s
  // peer.
  //if (ACE_BIT_ENABLED (this->flags_, ACE_NONBLOCK) != 0)
  //{
    if (svc_handler->peer ().enable (ACE_NONBLOCK) == -1)
      error = 1;
  //}
  //// Otherwise, make sure it's disabled by default.
  //else if (svc_handler->peer ().disable (ACE_NONBLOCK) == -1)
  //  error = 1;

  // We are connected now, so try to open things up.
  ICONNECTOR_T* iconnector_p = this;
  if (error || svc_handler->open (iconnector_p) == -1)
  {
    // Make sure to close down the <svc_handler> to avoid descriptor
    // leaks.
    // The connection was already made; so this close is a "normal"
    // close operation.
    svc_handler->close (NORMAL_CLOSE_OPERATION);
    return -1;
  }
  else
    return 0;
}
template <typename HandlerType,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Client_Connector_T<HandlerType,
                       AddressType,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (connectionManager_,
                                 statisticCollectionInterval_));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}

/////////////////////////////////////////

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
                                           StateType,
                                           HandlerConfigurationType,
                                           UserDataType>,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::Net_Client_Connector_T (ICONNECTION_MANAGER_T* connectionManager_in,
                                                              unsigned int statisticCollectionInterval_in)
 : configuration_ ()
 , connectionManager_ (connectionManager_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
                                           StateType,
                                           HandlerConfigurationType,
                                           UserDataType>,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
                                           StateType,
                                           HandlerConfigurationType,
                                           UserDataType>,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::initialize (const HandlerConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::initialize"));

  configuration_ = configuration_in;

  return true;
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
const HandlerConfigurationType&
Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
                                           StateType,
                                           HandlerConfigurationType,
                                           UserDataType>,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::get"));

  return configuration_;
}

//template <typename SocketConfigurationType,
//          typename HandlerType,
//          typename ConfigurationType,
//          typename HandlerConfigurationType,
//          typename UserDataType,
//          typename StateType,
//          typename StreamType>
//bool
//Net_Client_Connector_T<ACE_INET_Addr,
//                       SocketConfigurationType,
//                       ConfigurationType,
//                       HandlerConfigurationType,
//                       UserDataType,
//                       StateType,
//                       StreamType,
//                       Net_UDPConnection_T<UserDataType,
//                                           StateType,
//                                           HandlerType> >::useReactor () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::useReactor"));
//
//  return true;
//}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
                                           StateType,
                                           HandlerConfigurationType,
                                           UserDataType>,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
ACE_HANDLE
Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
                                           StateType,
                                           HandlerConfigurationType,
                                           UserDataType>,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  CONNECTION_T* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T<Net_UDPConnection_T>::make_svc_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  ICONNECTOR_T* iconnector_p = this;
  result = handler_p->open (iconnector_p);
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = address_in.addr_to_string (buffer, sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_UDPConnection_T::open(address was: \"%s\"): \"%m\", aborting\n"),
                buffer));
    return ACE_INVALID_HANDLE;
  } // end IF

  return handler_p->get_handle ();
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
                                           StateType,
                                           HandlerConfigurationType,
                                           UserDataType>,
                       ACE_INET_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::make_svc_handler (CONNECTION_T*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    CONNECTION_T (connectionManager_,
                                  statisticCollectionInterval_));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}
//template <typename HandlerType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StreamType,
//          typename HandlerConfigurationType,
//          typename UserDataType>
//int
//Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
//                                           StateType,
//                                           HandlerConfigurationType,
//                                           UserDataType>,
//                       ACE_INET_Addr,
//                       ConfigurationType,
//                       StateType,
//                       StreamType,
//                       HandlerConfigurationType,
//                       UserDataType>::connect_svc_handler (CONNECTION_T*& handler_inout,
//                                                           const ACE_SOCK_Connector::PEER_ADDR& remoteAddress_in,
//                                                           ACE_Time_Value* timeout_in,
//                                                           const ACE_SOCK_Connector::PEER_ADDR& localAddress_in,
//                                                           int reuseAddress_in,
//                                                           int flags_in,
//                                                           int permissions_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect_svc_handler"));
//
//  //ACE_UNUSED_ARG (remoteAddress_in);
//  ACE_UNUSED_ARG (timeout_in);
//  ACE_UNUSED_ARG (localAddress_in);
//  ACE_UNUSED_ARG (reuseAddress_in);
//  ACE_UNUSED_ARG (flags_in);
//  ACE_UNUSED_ARG (permissions_in);
//
//  // sanity check(s)
//  ACE_ASSERT (handler_inout);
//
//  Net_IConnector_t* iconnector_p = this;
//  result = handler_inout->open (iconnector_p);
//  if (result == -1)
//  {
//    ACE_TCHAR buffer[BUFSIZ];
//    ACE_OS::memset (buffer, 0, sizeof (buffer));
//    result = remoteAddress_in.addr_to_string (buffer, sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_UDPConnection_T::open(address was: \"%s\"): \"%m\", aborting\n"),
//                buffer));
//    return ACE_INVALID_HANDLE;
//  } // end IF
//
//  return handler_inout->get_handle ();
//}
//template <typename HandlerType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StreamType,
//          typename HandlerConfigurationType,
//          typename UserDataType>
//int
//Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,
//                                           StateType,
//                                           HandlerConfigurationType,
//                                           UserDataType>,
//                       ACE_INET_Addr,
//                       ConfigurationType,
//                       StateType,
//                       StreamType,
//                       HandlerConfigurationType,
//                       UserDataType>::connect_svc_handler (CONNECTION_T*& handler_inout,
//                                                           CONNECTION_T*& handlerCopy_inout,
//                                                           const ACE_SOCK_Connector::PEER_ADDR& remoteAddress_in,
//                                                           ACE_Time_Value* timeout_in,
//                                                           const ACE_SOCK_Connector::PEER_ADDR& localAddress_in,
//                                                           int reuseAddress_in,
//                                                           int flags_in,
//                                                           int permissions_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect_svc_handler"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (-1);
//
//  ACE_NOTREACHED (return -1);
//}

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Client_Connector_T<HandlerType,
                       ACE_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::Net_Client_Connector_T (ICONNECTION_MANAGER_T* connectionManager_in,
                                                              unsigned int statisticCollectionInterval_in)
 : configuration_ ()
 , connectionManager_ (connectionManager_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
Net_Client_Connector_T<HandlerType,
                       ACE_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
bool
Net_Client_Connector_T<HandlerType,
                       ACE_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::initialize (const HandlerConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::initialize"));

  configuration_ = configuration_in;

  return true;
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
const HandlerConfigurationType&
Net_Client_Connector_T<HandlerType,
                       ACE_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::get"));

  return configuration_;
}

//template <typename SocketConfigurationType,
//          typename HandlerType,
//          typename ConfigurationType,
//          typename HandlerConfigurationType,
//          typename UserDataType,
//          typename StateType,
//          typename StreamType>
//bool
//Net_Client_Connector_T<ACE_Netlink_Addr,
//                       SocketConfigurationType,
//                       ConfigurationType,
//                       HandlerConfigurationType,
//                       UserDataType,
//                       StateType,
//                       StreamType,
//                       HandlerType>::useReactor () const
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::useReactor"));
//
//  return true;
//}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
void
Net_Client_Connector_T<HandlerType,
                       ACE_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
ACE_HANDLE
Net_Client_Connector_T<HandlerType,
                       ACE_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::connect (const ACE_Netlink_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  // *TODO*: implement TCP version
  int result = -1;

  HandlerType* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T::make_svc_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  ICONNECTOR_T* iconnector_p = this;
  result = handler_p->open (iconnector_p);
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    // *TODO*: find a replacement for ACE_INET_Addr::addr_to_string
//    result = address_in.addr_to_string (buffer, sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Netlink_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HandlerType::open(\"%s\"): \"%m\", aborting\n"),
                buffer));
    return ACE_INVALID_HANDLE;
  } // end IF

  return handler_p->get_handle ();
}

template <typename HandlerType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          typename HandlerConfigurationType,
          typename UserDataType>
int
Net_Client_Connector_T<HandlerType,
                       ACE_Netlink_Addr,
                       ConfigurationType,
                       StateType,
                       StreamType,
                       HandlerConfigurationType,
                       UserDataType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (connectionManager_,
                                 statisticCollectionInterval_));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}

#endif
