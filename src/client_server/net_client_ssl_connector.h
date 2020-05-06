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

#ifndef NET_CLIENT_SSL_CONNECTOR_H
#define NET_CLIENT_SSL_CONNECTOR_H

#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "stream_statemachine_control.h"

#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"

template <typename HandlerType,
          typename ConnectorType, // ACE_SSL_SOCK_Connector
          ////////////////////////////////
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Client_SSL_Connector_T
 : public ACE_Connector<HandlerType,
                        ConnectorType>
 , public Net_IConnector_T<ACE_INET_Addr,
                           ConfigurationType>
{
  typedef ACE_Connector<HandlerType,
                        ConnectorType> inherited;

 public:
  typedef ACE_INET_Addr ADDRESS_T;
  typedef ConfigurationType CONFIGURATION_T;
  typedef StreamType STREAM_T;

  typedef Net_IConnection_T<ACE_INET_Addr,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  Net_TCPSocketConfiguration_t,
                                  StreamType,
                                  enum Stream_StateMachine_ControlState> ISTREAM_CONNECTION_T;

  typedef Net_IConnector_T<ACE_INET_Addr,
                           ConfigurationType> ICONNECTOR_T;

  Net_Client_SSL_Connector_T (bool = true); // managed ?
  inline virtual ~Net_Client_SSL_Connector_T () {}

  // implement Net_Client_IConnector_T
  virtual enum Net_TransportLayerType transportLayer () const;
  inline virtual bool useReactor () const { return true; }

  // *NOTE*: handlers retrieve the configuration object with get ()
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; }
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); return true; }

  virtual void abort ();
  virtual ACE_HANDLE connect (const ACE_INET_Addr&);

 protected:
  // override default activation strategy
  virtual int activate_svc_handler (HandlerType*);
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_SSL_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_SSL_Connector_T (const Net_Client_SSL_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_SSL_Connector_T& operator= (const Net_Client_SSL_Connector_T&))

  // convenient types
  typedef Net_Client_SSL_Connector_T<HandlerType,
                                     ConnectorType,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     StreamType,
                                     UserDataType> OWN_TYPE_T;
  typedef Net_ITransportLayer_T<Net_TCPSocketConfiguration_t> ITRANSPORTLAYER_T;

  ConfigurationType* configuration_; // connection-
  bool               managed_;
};

// include template definition
#include "net_client_ssl_connector.inl"

#endif
