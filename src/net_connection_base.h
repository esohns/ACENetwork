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
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_referencecounter_base.h"

#include "net_connection_manager.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

// forward declarations
class ACE_Message_Block;
enum Net_Connection_Status;

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename UserDataType>
class Net_ConnectionBase_T
 : public Common_ReferenceCounterBase
 , virtual public Net_IConnection_T<AddressType,
                                    ConfigurationType,
                                    StateType,
                                    StatisticContainerType>
 , public Common_IRegister
{
  typedef Common_ReferenceCounterBase inherited;

 public:
  // convenient types
  typedef Common_ReferenceCounterBase REFERENCECOUNTER_T;
  typedef Net_Connection_Manager_T<ACE_SYNCH_USE,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> CONNECTION_MANAGER_T;

  inline virtual ~Net_ConnectionBase_T () { ACE_ASSERT (!isRegistered_); }

  // implement (part of) Net_IConnection_T
  using REFERENCECOUNTER_T::decrease;
  using REFERENCECOUNTER_T::increase;
  inline virtual bool collect (StatisticContainerType& statistic_out) { statistic_out = state_.statistic; return true; }
  virtual void update (const ACE_Time_Value&);
  virtual void report () const;
  // *NOTE*: iff using a connection manager ('managed', see ctor argument), the
  //         configuration is retrieved in the ctor already
  inline virtual bool initialize (const ConfigurationType& configuration_in) { /*ACE_ASSERT (!configuration_);*/ configuration_ = &const_cast<ConfigurationType&> (configuration_in); return true; }
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; }
  // missing: Common_IDumpState
  // missing: info
  // missing: id
  // missing: notification
  inline virtual const StateType& state () const { return state_; }
  inline virtual enum Net_Connection_Status status () const { return state_.status; }
  // missing: close
  // missing: waitForCompletion

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_ConnectionBase_T () : inherited (1, true) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  Net_ConnectionBase_T (bool); // managed ?

  // implement Common_IRegister
  virtual bool register_ ();
  virtual void deregister ();

  // helper methods
  ACE_Message_Block* allocateMessage (unsigned int); // requested size

  ConfigurationType* configuration_;
  StateType          state_;

  bool               isManaged_;
  bool               isRegistered_;

 private:
  // convenient types
  typedef Net_ConnectionBase_T<ACE_SYNCH_USE,
                               AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T (const Net_ConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_ConnectionBase_T& operator= (const Net_ConnectionBase_T&))
};

// include template definition
#include "net_connection_base.inl"

#endif
