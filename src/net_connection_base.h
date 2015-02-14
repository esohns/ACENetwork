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

#include "common_referencecounter_base.h"

#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

template <typename ConfigurationType,
          typename StatisticsContainerType>
class Net_ConnectionBase_T
 : public Common_ReferenceCounterBase
 , public Net_IConnection_T<StatisticsContainerType>
{
 public:
  // implement (part of) Net_ITransportLayer
  virtual bool init (const ACE_INET_Addr&, // peer address
                     unsigned short);      // port number
  virtual void fini ();

 protected:
  typedef Net_IConnectionManager_T<ConfigurationType,
                                   StatisticsContainerType> Net_IConnectionManager_t;

  Net_ConnectionBase_T (Net_IConnectionManager_t*);
  virtual ~Net_ConnectionBase_T ();

  Net_IConnectionManager_t* manager_;
  ConfigurationType         userData_;
  bool                      isRegistered_;

 private:
  typedef Common_ReferenceCounterBase inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T (const Net_ConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T& operator=(const Net_ConnectionBase_T&));
};

// include template implementation
#include "net_connection_base.inl"

#endif
