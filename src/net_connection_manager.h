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

#ifndef Net_CONNECTION_MANAGER_H
#define Net_CONNECTION_MANAGER_H

#include "ace/Singleton.h"
#include "ace/Synch.h"
#include "ace/Condition_T.h"
#include "ace/Containers_T.h"

#include "common_istatistic.h"
#include "common_idumpstate.h"

#include "net_exports.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"

template <typename ConfigurationType,
          typename StatisticsContainerType>
class Net_Connection_Manager_T
 : public Net_IConnectionManager_T<ConfigurationType,
                                   StatisticsContainerType>
 , public Common_IStatistic_T<StatisticsContainerType>
 , public Common_IDumpState
{
  // singleton needs access to the ctor/dtors
  friend class ACE_Singleton<Net_Connection_Manager_T<ConfigurationType,
                                                      StatisticsContainerType>,
                             ACE_Recursive_Thread_Mutex>;

  //// needs access to (de-)register itself with the singleton
  //friend class Net_SocketHandlerBase_T<ConfigurationType,
  //                                     StatisticsContainerType>;

 public:
  typedef Net_IConnection_T<StatisticsContainerType> Connection_t;

  // configuration / initialization
  void init (unsigned int); // maximum number of concurrent connections
  // *NOTE*: argument is passed in init() to EVERY new connection during registration
  void set (const ConfigurationType&); // (user) data

  // implement RPG_Common_IControl
  virtual void start ();
  virtual void stop (bool = true); // locked access ?
  virtual bool isRunning () const;

  void abortConnections ();
  void waitConnections () const;
  unsigned int numConnections () const;

  // *TODO*: used for unit testing purposes ONLY !
  //void lock();
  //void unlock();
  const Connection_t* operator[] (unsigned int) const;
  // --------------------------------------------------------------------------
  void abortOldestConnection ();
  void abortNewestConnection ();

  // implement Common_IStatistic
  virtual void report () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  typedef ACE_DLList<Connection_t> Connections_t;
  typedef ACE_DLList_Iterator<Connection_t> ConnectionsIterator_t;
  typedef ACE_DLList_Reverse_Iterator<Connection_t> ConnectionsReverseIterator_t;

  // implement Net_IConnectionManager
  virtual void getConfiguration (ConfigurationType&); // return value: configuration
  virtual bool registerConnection (Connection_t*); // connection
  virtual void deregisterConnection (const Connection_t*); // connection

  // implement Common_IStatistic
  // *WARNING*: this assumes myLock is being held !
  virtual bool collect (StatisticsContainerType&) const; // return value: statistic data

  Net_Connection_Manager_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_Connection_Manager_T (const Net_Connection_Manager_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_Connection_Manager_T& operator= (const Net_Connection_Manager_T&));
  virtual ~Net_Connection_Manager_T ();

  // *NOTE*: MUST be recursive, otherwise asynch abort is not feasible
  mutable ACE_Recursive_Thread_Mutex                lock_;
  // implement blocking wait...
  mutable ACE_Condition<ACE_Recursive_Thread_Mutex> condition_;

  unsigned int                                      maxNumConnections_;
  Connections_t                                     connections_;
  ConfigurationType                                 userData_; // handler data
  bool                                              isInitialized_;
  bool                                              isActive_;
};

// include template implementation
#include "net_connection_manager.inl"

#endif
