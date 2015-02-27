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

#ifndef Net_SOCKETCONNECTION_BASE_H
#define Net_SOCKETCONNECTION_BASE_H

#include "ace/config-macros.h"
#include "ace/Event_Handler.h"

template <typename SocketHandlerType,
          typename TransportLayerType,
          typename ConfigurationType,
          typename SessionDataType,
          typename StatisticsContainerType>
class Net_SocketConnectionBase_T
 : public SocketHandlerType
 , public TransportLayerType
 //, public Net_ConnectionBase_T<ConfigurationType,
 //                              SessionDataType,
 //                              StatisticsContainerType>
{
 public:
  //typedef Net_IConnectionManager_T<ConfigurationType,
  //                                 SessionDataType,
  //                                 StatisticsContainerType> Net_IConnectionManager_t;

  virtual ~Net_SocketConnectionBase_T ();

  // implement (part of) Net_IInetTransportLayer
  virtual void ping (); // ping the peer !

  // implement Common_IStatistic
  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticsContainerType&) const; // return value: statistic data
  virtual void report () const;

  //// override some task-based members
  //virtual int open (void* = NULL); // args
  //virtual int close (u_long = 0); // args

//  // *NOTE*: enqueue any received data onto our stream for further processing
//   virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 protected:
  Net_SocketConnectionBase_T ();

 private:
  typedef SocketHandlerType inherited;
  typedef TransportLayerType inherited2;
  //typedef Net_ConnectionBase_T<ConfigurationType,
  //                             SessionDataType,
  //                             StatisticsContainerType> inherited3;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  ACE_UNIMPLEMENTED_FUNC (Net_SocketConnectionBase_T (const Net_SocketConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_SocketConnectionBase_T& operator= (const Net_SocketConnectionBase_T&));
};

#include "net_socketconnection_base.inl"

#endif
