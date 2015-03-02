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

#ifndef Net_CONNECTION_BASE_H
#define Net_CONNECTION_BASE_H

#include "ace/INET_Addr.h"

#include "common_iinitialize.h"
#include "common_referencecounter_base.h"

#include "net_configuration.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

template <typename ConfigurationType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticsContainerType>
class Net_ConnectionBase_T
 : public Common_ReferenceCounterBase
 , public Net_IConnection_T<ITransportLayerType,
                            StatisticsContainerType>
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
//  Net_ConnectionBase_T ();

  // implement (part of) Net_ITransportLayer_T
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // socket configuration
  virtual void finalize ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&); // handler configuration

 protected:
  typedef Net_IConnectionManager_T<ConfigurationType,
                                   SessionDataType,
                                   ITransportLayerType,
                                   StatisticsContainerType> ICONNECTION_MANAGER_T;

  Net_ConnectionBase_T (ICONNECTION_MANAGER_T*);
  virtual ~Net_ConnectionBase_T ();

  ConfigurationType      configuration_;
  bool                   isRegistered_;
  ICONNECTION_MANAGER_T* manager_;
  SessionDataType        sessionData_;

 private:
  typedef Common_ReferenceCounterBase inherited;
  typedef Net_IConnection_T<ITransportLayerType,
                            StatisticsContainerType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T (const Net_ConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T& operator= (const Net_ConnectionBase_T&));
};

// include template implementation
#include "net_connection_base.inl"

#endif
