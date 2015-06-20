/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
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

#ifndef NET_ICONNECTION_H
#define NET_ICONNECTION_H

#include "ace/config-macros.h"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_irefcount.h"
#include "common_istatistic.h"

#include "net_itransportlayer.h"

template <typename AddressType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StreamType>
class Net_IConnection_T
 : public Common_IGet_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IStatistic_T<StatisticContainerType>
 , virtual public Common_IRefCount
 , public Common_IDumpState
{
 public:
  inline virtual ~Net_IConnection_T () {};

  virtual void info (ACE_HANDLE&,             // return value: I/O handle
                     AddressType&,            // return value: local SAP
                     AddressType&) const = 0; // return value: remote SAP
  virtual unsigned int id () const = 0;

  virtual Net_Connection_Status status () const = 0;
  virtual const StreamType& stream () const = 0;

  // *NOTE*: see ACE_Svc_Handler/ACE_Task_Base
  //         (and net_common.h / ACE_Svc_Handler.h for reason codes)
  virtual int close (u_long = 0) = 0; // reason
};

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StreamType>
class Net_ISocketConnection_T
 : virtual public Net_IConnection_T<AddressType,
                                    ConfigurationType,
                                    StatisticContainerType,
                                    StreamType>
 , virtual public Net_ITransportLayer_T<SocketConfigurationType>
{
 public:
  virtual ~Net_ISocketConnection_T () {};
};

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename StreamType,
          typename StateType>
class Net_ISession_T
 : virtual public Net_ISocketConnection_T<AddressType,
                                          SocketConfigurationType,
                                          ConfigurationType,
                                          StatisticContainerType,
                                          StreamType>
{
public:
  virtual ~Net_ISession_T () {};

  virtual const StateType& state () const = 0;
};

#endif
