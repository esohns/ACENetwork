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

#ifndef NET_CONNECTION_BASE_H
#define NET_CONNECTION_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_referencecounter_base.h"

#include "stream_stat_statistic_handler.h"

#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

// forward declarations
enum Net_Connection_Status;

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Net_ConnectionBase_T
 : public Common_ReferenceCounterBase
 , virtual public Net_IConnection_T<AddressType,
                                    ConfigurationType,
                                    StateType,
                                    StatisticContainerType>
{
  typedef Common_ReferenceCounterBase inherited;

 public:
  // convenient types
  typedef Net_IConnection_T<AddressType,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  // implement (part of) Net_IConnection_T
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; };
  // *NOTE*: when using a connection manager, the (default) configuration is
  //         retrieved in the ctor
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); return true; };
  inline virtual const StateType& state () const { return state_; };
  inline virtual enum Net_Connection_Status status () const { return state_.status; };

 protected:
  Net_ConnectionBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                        const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Net_ConnectionBase_T ();

  // (de-)register with the connection manager (if any)
//#if defined (__GNUG__)
//  // *WORKAROUND*: the GNU linker (as of g++ 4.9.2) generates broken code ("pure
//  //               virtual method called" for
//  //               Common_IReferenceCount::increase()) when passing 'this' (i.e.
//  //               the default) to the network managers' registerc() method
//  //               --> pass in the 'correct' handle as a workaround
  bool registerc (ICONNECTION_T* = NULL);
//#else
//  bool registerc ();
//#endif
  void deregister ();

  ConfigurationType*     configuration_;
  StateType              state_;

  bool                   isRegistered_;
  ICONNECTION_MANAGER_T* manager_;

 private:
  // convenient types
  typedef ACE_Singleton<TimerManagerType,
                        ACE_SYNCH_MUTEX> TIMER_MANAGER_SINGLETON_T;
  typedef Stream_StatisticHandler_T<StatisticContainerType> STATISTIC_HANDLER_T;

  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T (const Net_ConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T& operator= (const Net_ConnectionBase_T&))

  // convenient types
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               TimerManagerType,
                               UserDataType> OWN_TYPE_T;

  // timer
  STATISTIC_HANDLER_T    statisticHandler_;
  long                   timerId_;
};

// include template definition
#include "net_connection_base.inl"

#endif
