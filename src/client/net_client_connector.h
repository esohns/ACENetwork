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

#ifndef NET_CLIENT_CONNECTOR_H
#define NET_CLIENT_CONNECTOR_H

#include "ace/Global_Macros.h"
#include "ace/Connector.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"

#include "net_udpconnection.h"

#include "net_client_iconnector.h"
#include "net_connection_manager_common.h"

template <typename AddressType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename ConnectionType>
class Net_Client_Connector_T
 : public ACE_Connector<ConnectionType,
                        ACE_SOCK_CONNECTOR>
 , public Net_Client_IConnector_T<AddressType,
                                  SocketHandlerConfigurationType>
{
 public:
  typedef Net_IConnectionManager_T<ConfigurationType,
                                   UserDataType,
                                   Stream_Statistic_t,
                                   ITransportLayerType> ICONNECTION_MANAGER_T;

  // *TODO*: pass SocketHandlerConfigurationType instead
  Net_Client_Connector_T (const ConfigurationType*, // configuration handle
                          ICONNECTION_MANAGER_T*,   // connection manager handle
                          unsigned int = 0);        // statistics collecting interval (second(s))
                                                    // 0 --> DON'T collect statistics
  virtual ~Net_Client_Connector_T ();

  // override default instantiation strategy
  virtual int make_svc_handler (ConnectionType*&);

  // implement Net_Client_IConnector_T
  virtual const SocketHandlerConfigurationType* getConfiguration () const;
  virtual void abort ();
  virtual bool connect (const AddressType&);

 private:
  typedef ACE_Connector<ConnectionType,
                        ACE_SOCK_CONNECTOR> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&));

  // *TODO*: retain SocketHandlerConfigurationType instead
  const ConfigurationType* configuration_;
  ICONNECTION_MANAGER_T*   interfaceHandle_;
  unsigned int             statisticCollectionInterval_; // seconds
};

// partial specialization (for UDP)
template <typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType>
class Net_Client_Connector_T<ACE_INET_Addr,
                             ConfigurationType,
                             SocketHandlerConfigurationType,
                             UserDataType,
                             SessionDataType,
                             ITransportLayerType,
                             Net_UDPConnection_T<UserDataType,
                                                 SessionDataType,
                                                 HandlerType> >
 : public Net_Client_IConnector_T<ACE_INET_Addr,
                                  SocketHandlerConfigurationType>
{
 public:
  typedef Net_IConnectionManager_T<ConfigurationType,
                                   UserDataType,
                                   Stream_Statistic_t,
                                   ITransportLayerType> ICONNECTION_MANAGER_T;
  typedef Net_UDPConnection_T<UserDataType,
                              SessionDataType,
                              HandlerType> CONNECTION_T;

  // *TODO*: pass SocketHandlerConfigurationType instead
  Net_Client_Connector_T (const ConfigurationType*, // configuration handle
                          ICONNECTION_MANAGER_T*,   // connection manager handle
                          unsigned int = 0);        // statistics collecting interval (second(s))
                                                    // 0 --> DON'T collect statistics
  virtual ~Net_Client_Connector_T ();

  // override default instantiation strategy
  virtual int make_svc_handler (CONNECTION_T*&);

  // implement Net_Client_IConnector_T
  // *TODO*: why is it necessary to provide an implementation when there is (a
  //         more generic) one available ? (gcc complains about abort() and
  //         getConfiguration())
  virtual const SocketHandlerConfigurationType* getConfiguration () const;
  // *NOTE*: this is just a stub
  virtual void abort ();
  // specialize (part of) Net_Client_IConnector_T
  virtual bool connect (const ACE_INET_Addr&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&));

  // *TODO*: retain SocketHandlerConfigurationType instead
  const ConfigurationType* configuration_;
  ICONNECTION_MANAGER_T*   interfaceHandle_;
  unsigned int             statisticCollectionInterval_; // seconds
};

// include template implementation
#include "net_client_connector.inl"

#endif
