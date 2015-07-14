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

#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Netlink_Addr.h"
#include "ace/SOCK_Connector.h"

#include "net_netlinkconnection.h"
#include "net_udpconnection.h"

#include "net_client_iconnector.h"
#include "net_connection_manager_common.h"

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename ConnectorConfigurationType, // connector | socket handler configuration type
          typename UserDataType,
          typename StateType,
          typename StreamType,
          typename HandlerType>
class Net_Client_Connector_T
 : public ACE_Connector<HandlerType,
                        ACE_SOCK_CONNECTOR>
 , public Net_Client_IConnector_T<AddressType,
                                  ConnectorConfigurationType>
{
 public:
  //typedef Net_IConnectionManager_T<AddressType,
  //                                 SocketConfigurationType,
  //                                 ConfigurationType,
  //                                 UserDataType,
  //                                 Stream_Statistic,
  //                                 StreamType> ICONNECTION_MANAGER_T;

  Net_Client_Connector_T ();
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  //virtual bool useReactor () const; // ? : uses proactor

  // *IMPORTANT NOTE*: fire-and-forget API: the first (!) generated handler
  //                   assumes responsibility for the socket handler
  //                   configuration (member). The handler retrieves the
  //                   configuration object via get () (see below)
  //                   --> instances must be (re)initialize()d for every new
  //                       connection
  virtual bool initialize (const ConnectorConfigurationType&);

  // *IMPORTANT NOTE*: works only once (!) for every initialize() call
  virtual const ConnectorConfigurationType& get () const; // never mind the 'const' here

  virtual void abort ();
  virtual ACE_HANDLE connect (const AddressType&);

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  typedef ACE_Connector<HandlerType,
                        ACE_SOCK_CONNECTOR> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  mutable ConnectorConfigurationType configuration_;
};

/////////////////////////////////////////

// partial specialization (for UDP)
template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename ConnectorConfigurationType,
          typename UserDataType,
          typename StateType,
          typename StreamType>
class Net_Client_Connector_T<ACE_INET_Addr,
                             SocketConfigurationType,
                             ConfigurationType,
                             ConnectorConfigurationType,
                             UserDataType,
                             StateType,
                             StreamType,
                             Net_UDPConnection_T<UserDataType,
                                                 StateType,
                                                 HandlerType,
                                                 ConnectorConfigurationType> >
 : public Net_Client_IConnector_T<ACE_INET_Addr,
                                  ConnectorConfigurationType>
{
 public:
  //typedef Net_IConnectionManager_T<ACE_INET_Addr,
  //                                 SocketConfigurationType,
  //                                 ConfigurationType,
  //                                 UserDataType,
  //                                 Stream_Statistic,
  //                                 StreamType> ICONNECTION_MANAGER_T;
  typedef Net_UDPConnection_T<UserDataType,
                              StateType,
                              HandlerType,
                              ConnectorConfigurationType> CONNECTION_T;

  Net_Client_Connector_T ();
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  //virtual bool useReactor () const; // ? : uses proactor

  // *IMPORTANT NOTE*: fire-and-forget API: the first (!) generated handler
  //                   assumes responsibility for the socket handler
  //                   configuration (member). The configuration object is
  //                   passed to the handler in the open() method invocation
  //                   (see connect () code)
  //                   --> instances must be (re)initialize()d for every new
  //                       connection
  virtual bool initialize (const ConnectorConfigurationType&);

  // *TODO*: why is it necessary to provide an implementation when there is (a
  //         more generic) one available ? (gcc complains about abort() and
  //         getConfiguration())
  // *NOTE*: this is just a stub
  virtual const ConnectorConfigurationType& get () const;
  // *NOTE*: this is just a stub
  virtual void abort ();
  // specialize (part of) Net_Client_IConnector_T
  virtual ACE_HANDLE connect (const ACE_INET_Addr&);

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (CONNECTION_T*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  ConnectorConfigurationType configuration_;
};

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
// partial specialization (for Netlink)
template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename ConnectorConfigurationType,
          typename UserDataType,
          typename StateType,
          typename StreamType>
class Net_Client_Connector_T<ACE_Netlink_Addr,
                             SocketConfigurationType,
                             ConfigurationType,
                             ConnectorConfigurationType,
                             UserDataType,
                             StateType,
                             StreamType,
                             HandlerType>
 : public Net_Client_IConnector_T<ACE_Netlink_Addr,
                                  ConnectorConfigurationType>
{
 public:
  //typedef Net_IConnectionManager_T<ACE_Netlink_Addr,
  //                                 SocketConfigurationType,
  //                                 ConfigurationType,
  //                                 UserDataType,
  //                                 Stream_Statistic,
  //                                 StreamType> ICONNECTION_MANAGER_T;

  Net_Client_Connector_T ();
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  //virtual bool useReactor () const; // ? : uses proactor

  // *IMPORTANT NOTE*: fire-and-forget API: the first (!) generated handler
  //                   assumes responsibility for the socket handler
  //                   configuration (member). The configuration object is
  //                   passed to the handler in the open() method invocation
  //                   (see connect () code)
  //                   --> instances must be (re)initialize()d for every new
  //                       connection
  virtual bool initialize (const ConnectorConfigurationType&);

  // *NOTE*: this is just a stub
  virtual const ConnectorConfigurationType& get () const;
  // *NOTE*: this is just a stub
  virtual void abort ();
  virtual ACE_HANDLE connect (const ACE_Netlink_Addr&);

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  ConnectorConfigurationType configuration_;
};
#endif

// include template implementation
#include "net_client_connector.inl"

#endif
