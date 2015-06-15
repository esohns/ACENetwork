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

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType>
Net_Client_Connector_T<AddressType,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::Net_Client_Connector_T (const HandlerConfigurationType* configuration_in,
                                                             ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                             unsigned int statisticCollectionInterval_in)
 : inherited (ACE_Reactor::instance (), // default reactor
              ACE_NONBLOCK)             // flags: non-blocking I/O
              //0)                       // flags
 , configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType>
Net_Client_Connector_T<AddressType,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType>
bool
Net_Client_Connector_T<AddressType,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::useReactor () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::useReactor"));

  return true;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType>
void
Net_Client_Connector_T<AddressType,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

  int result = inherited::close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::close(): \"%m\", continuing\n")));
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType>
ACE_HANDLE
Net_Client_Connector_T<AddressType,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::connect (const AddressType& address_in)
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
    // *TODO*: find a replacement for ACE_Netlink_Addr::addr_to_string
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

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType>
const HandlerConfigurationType*
Net_Client_Connector_T<AddressType,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  return configuration_;
}

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType>
int
Net_Client_Connector_T<AddressType,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (interfaceHandle_,
                                 statisticCollectionInterval_));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}

/////////////////////////////////////////

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
Net_Client_Connector_T<ACE_INET_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::Net_Client_Connector_T (const HandlerConfigurationType* configuration_in,
                                                                                   ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                                                   unsigned int statisticCollectionInterval_in)
 : configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
Net_Client_Connector_T<ACE_INET_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
bool
Net_Client_Connector_T<ACE_INET_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::useReactor () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::useReactor"));

  return true;
}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
void
Net_Client_Connector_T<ACE_INET_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
ACE_HANDLE
Net_Client_Connector_T<ACE_INET_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

  int result = -1;

  CONNECTION_T* handler_p = NULL;
  result = make_svc_handler (handler_p);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_Connector_T<Net_UDPConnection>::make_svc_handler(): \"%m\", aborting\n")));
    return ACE_INVALID_HANDLE;
  } // end IF
  ACE_ASSERT (handler_p);

  // *NOTE*: the handler registers with the reactor and will be freed (or
  //         recycled) automatically
  result =
      handler_p->open (configuration_);
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = address_in.addr_to_string (buffer, sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));

//    // clean up
//    delete handler_p;

    return ACE_INVALID_HANDLE;
  } // end IF

  return handler_p->get_handle ();
}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
const HandlerConfigurationType*
Net_Client_Connector_T<ACE_INET_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  return configuration_;
}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
int
Net_Client_Connector_T<ACE_INET_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       Net_UDPConnection_T<UserDataType,
                                           SessionDataType,
                                           HandlerType> >::make_svc_handler (CONNECTION_T*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    CONNECTION_T (interfaceHandle_,
                                  statisticCollectionInterval_));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
Net_Client_Connector_T<ACE_Netlink_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::Net_Client_Connector_T (const HandlerConfigurationType* configuration_in,
                                                             ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                             unsigned int statisticCollectionInterval_in)
 : configuration_ (configuration_in)
 , interfaceHandle_ (interfaceHandle_in)
 , statisticCollectionInterval_ (statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::Net_Client_Connector_T"));

}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
Net_Client_Connector_T<ACE_Netlink_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::~Net_Client_Connector_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::~Net_Client_Connector_T"));

}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
bool
Net_Client_Connector_T<ACE_Netlink_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::useReactor () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::useReactor"));

  return true;
}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
void
Net_Client_Connector_T<ACE_Netlink_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::abort"));

}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
ACE_HANDLE
Net_Client_Connector_T<ACE_Netlink_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::connect (const ACE_Netlink_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::connect"));

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

  // *NOTE*: the handler registers with the reactor and will be freed (or
  //         recycled) automatically
  result =
      handler_p->open (configuration_);
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

//    // clean up
//    delete handler_p;

    return ACE_INVALID_HANDLE;
  } // end IF

  return handler_p->get_handle ();
}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
const HandlerConfigurationType*
Net_Client_Connector_T<ACE_Netlink_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::getConfiguration () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::getConfiguration"));

  return configuration_;
}

template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename HandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType>
int
Net_Client_Connector_T<ACE_Netlink_Addr,
                       SocketConfigurationType,
                       ConfigurationType,
                       HandlerConfigurationType,
                       UserDataType,
                       SessionDataType,
                       StreamType,
                       HandlerType>::make_svc_handler (HandlerType*& handler_out)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector_T::make_svc_handler"));

  // initialize return value(s)
  handler_out = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_out,
                    HandlerType (interfaceHandle_,
                                 statisticCollectionInterval_));
  if (!handler_out)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return (handler_out ? 0 : -1);
}
#endif
