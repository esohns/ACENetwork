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

#ifndef NET_CONNECTION_MANAGER_T_H
#define NET_CONNECTION_MANAGER_T_H

#include "ace/Containers_T.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_istatistic.h"

#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename UserDataType>
class Net_Connection_Manager_T
 : public Net_IConnectionManager_T<ACE_SYNCH_USE,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType>
 , public Common_IStatistic_T<StatisticContainerType>
{
  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_Connection_Manager_T<ACE_SYNCH_USE,
                                                      AddressType,
                                                      ConfigurationType,
                                                      StateType,
                                                      StatisticContainerType,
                                                      UserDataType>,
                             ACE_SYNCH_MUTEX_T>;

 public:
  // convenience types
  typedef ConfigurationType CONFIGURATION_T;
  typedef StateType STATE_T;
  typedef UserDataType USERDATA_T;
  typedef Net_IConnectionManager_T<ACE_SYNCH_USE,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> INTERFACE_T;
  typedef Net_IConnection_T<AddressType,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef ACE_Singleton<Net_Connection_Manager_T<ACE_SYNCH_USE,
                                                 AddressType,
                                                 ConfigurationType,
                                                 StateType,
                                                 StatisticContainerType,
                                                 UserDataType>,
                        ACE_SYNCH_MUTEX_T> SINGLETON_T;

  // configuration / initialization
  void initialize (unsigned int); // maximum number of concurrent connections

  // implement (part of) Net_IConnectionManager_T
  virtual bool lock (bool = true); // block ?
  virtual int unlock (bool = false); // unblock ?
  inline virtual const typename INTERFACE_T::ITASKCONTROL_T::MUTEX_T& getR () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (typename INTERFACE_T::ITASKCONTROL_T::MUTEX_T ()); ACE_NOTREACHED (return typename INTERFACE_T::ITASKCONTROL_T::MUTEX_T ();) }
  inline virtual ACE_thread_t start () { ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, lock_, 0); isActive_ = true; return 0; }
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual bool isRunning () const;
  virtual void wait (bool = true) const; // wait for the message queue ? : worker thread(s) only
  virtual void dump_state () const;
  virtual void abort (enum Net_Connection_AbortStrategy); // strategy
                                                          // *IMPORTANT NOTE*: passing 'true' will hog the CPU --> use wait() instead
  virtual void abort (bool = false); // wait for completion ?
  virtual unsigned int count () const; // return value: # of connections
                                       // *IMPORTANT NOTE*: this API really makes sense only AFTER stop() has been
                                       //                   invoked, i.e. when new connections will be rejected;
                                       //                   otherwise this may block indefinetly
  virtual bool registerc (ICONNECTION_T*); // connection handle
  virtual void deregister (ICONNECTION_T*); // connection handle

  // *WARNING*: these two methods are NOT (!) re-entrant. If you want to set a
  //            specific configuration /user data per connection, use the
  //            locking API (see below)
  virtual void set (const ConfigurationType&, // connection handler (default)
                                              // configuration
                    UserDataType*);           // (stream) user data
  virtual void get (ConfigurationType*&, // return value: (default)
                                         // connection handler configuration
                    UserDataType*&);     // return value: (stream) user data
  virtual ICONNECTION_T* operator[] (unsigned int) const; // index
  virtual ICONNECTION_T* get (Net_ConnectionId_t) const; // id
  virtual ICONNECTION_T* get (const AddressType&, // address
                              bool = true) const; // peer address ? : local address // *TODO*: this parameter is redundant; remove ASAP
  virtual ICONNECTION_T* get (ACE_HANDLE) const; // socket handle

  // implement (part of) Common_IStatistic_T
  virtual void report () const;

 private:
  Net_Connection_Manager_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_Connection_Manager_T (const Net_Connection_Manager_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Connection_Manager_T& operator= (const Net_Connection_Manager_T&))
  virtual ~Net_Connection_Manager_T ();

  // convenient types
  typedef Net_Connection_Manager_T<ACE_SYNCH_USE,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> OWN_TYPE_T;

  typedef ACE_DLList<ICONNECTION_T> CONNECTION_CONTAINER_T;
  typedef ACE_DLList_Iterator<ICONNECTION_T> CONNECTION_CONTAINER_ITERATOR_T;
  typedef ACE_DLList_Reverse_Iterator<ICONNECTION_T> CONNECTION_CONTAINER_REVERSEITERATOR_T;

  // override/hide (part of) Common_ITaskControl
  inline virtual void idle () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void finished () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // implement (part of) Common_IStatistic_T
  // *WARNING*: this assumes lock_ is being held
  virtual bool collect (StatisticContainerType&); // return value: statistic data

  void abortLeastRecent ();
  void abortMostRecent ();

  // implement blocking wait
  mutable ACE_SYNCH_RECURSIVE_CONDITION condition_;
  CONNECTION_CONTAINER_T                connections_;
  bool                                  isActive_;
  bool                                  isInitialized_;
  // *NOTE*: MUST be recursive, otherwise asynchronous abort is not feasible
  mutable ACE_SYNCH_RECURSIVE_MUTEX     lock_;
  unsigned int                          maximumNumberOfConnections_;

  ConfigurationType*                    configuration_; // default-
  UserDataType*                         userData_;
};

// include template definition
#include "net_connection_manager.inl"

#endif
