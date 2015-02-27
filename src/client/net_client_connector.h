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
#include "ace/SOCK_Connector.h"

#include "net_client_iconnector.h"
#include "net_connection_manager_common.h"

template <typename ConfigurationType,
          typename SessionDataType,
          typename TransportLayerType,
          typename ConnectionType>
class Net_Client_Connector
 : public ACE_Connector<ConnectionType,
                        ACE_SOCK_CONNECTOR>
 , public Net_Client_IConnector
{
 public:
  typedef Net_IConnectionManager_T<ConfigurationType,
                                   SessionDataType,
                                   TransportLayerType,
                                   Stream_Statistic_t> ICONNECTION_MANAGER_T;

  Net_Client_Connector (ICONNECTION_MANAGER_T*);
  virtual ~Net_Client_Connector ();

  // override default creation strategy
  virtual int make_svc_handler (ConnectionType*&);

  // implement Net_Client_IConnector
  virtual void abort ();
  virtual bool connect (const ACE_INET_Addr&);

 private:
  typedef ACE_Connector<ConnectionType,
                        ACE_SOCK_CONNECTOR> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector ());
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector (const Net_Client_Connector&));
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector& operator= (const Net_Client_Connector&));

  ICONNECTION_MANAGER_T* interfaceHandle_;
};

// include template implementation
#include "net_client_connector.inl"

#endif
