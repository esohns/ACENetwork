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
#include "ace/Message_Block.h"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_irefcount.h"
#include "common_istatistic.h"

#include "net_itransportlayer.h"

// forward declarations
enum Net_Connection_Status;

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
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
  // convenience types
  typedef StreamType STREAM_T;

  inline virtual ~Net_IConnection_T () {};

  virtual void info (ACE_HANDLE&,             // return value: I/O handle
                     AddressType&,            // return value: local SAP
                     AddressType&) const = 0; // return value: remote SAP
  virtual unsigned int id () const = 0;

  virtual const StateType& state () const = 0;
  virtual Net_Connection_Status status () const = 0;
  virtual const StreamType& stream () const = 0;

  // *NOTE*: see ACE_Svc_Handler/ACE_Task_Base
  //         (and net_common.h / ACE_Svc_Handler.h for reason codes)
//  virtual int close (u_long = 0) = 0; // reason
  virtual void close () = 0;
  virtual bool send (const ACE_Message_Block&) = 0;
};

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          ///////////////////////////////
          typename SocketConfigurationType,
          ///////////////////////////////
          typename HandlerConfigurationType>
class Net_ISocketConnection_T
 : virtual public Net_IConnection_T<AddressType,
                                    ConfigurationType,
                                    StateType,
                                    StatisticContainerType,
                                    StreamType>
 , virtual public Net_ITransportLayer_T<SocketConfigurationType>
 // *NOTE*: this next line wouldn't compile (with MSVC)
 // *EXPLANATION*: apparently, on function signatures, the standard stipulates
 //                (in 14.5.5.1):
 //                "The types of its parameters and, if the function is a class
 //                member, the cv- qualifiers (if any) on the function itself
 //                and the class in which the member function is declared. The
 //                signature of a function template specialization includes the
 //                types of its template arguments...."
 //                Note that specifically, this does NOT include the return
 //                types.
 //                Here, this probably means that Net_ISocketConnection_T::get
 //                is not considered to be a distinct function, but rather an
 //                overload (here: override) of Net_IConnection_T::get.
 //                [Note that for differing return types, this would be allowed,
 //                if the return types were covariant. This explains the
 //                somewhat misleading error message returned by MSVC.]
 //, public Common_IGet_T<HandlerConfigurationType>
 , public Common_IInitialize_T<HandlerConfigurationType>
{
 public:
  virtual ~Net_ISocketConnection_T () {};

  // *TODO*: see above
  virtual const HandlerConfigurationType& get () = 0;
};

//template <typename AddressType,
//          typename SocketConfigurationType,
//          typename ConfigurationType,
//          typename StateType,
//          typename StatisticContainerType,
//          typename StreamType>
//class Net_ISession_T
// : virtual public Net_ISocketConnection_T<AddressType,
//                                          SocketConfigurationType,
//                                          ConfigurationType,
//                                          StateType,
//                                          StatisticContainerType,
//                                          StreamType>
//{
// public:
//  virtual ~Net_ISession_T () {};
//
//  virtual const StateType& state () const = 0;
//};

#endif
