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

#ifndef NET_SERVER_ASYNCHLISTENER_H
#define NET_SERVER_ASYNCHLISTENER_H

#include "ace/Asynch_Acceptor.h"
#include "ace/Asynch_IO.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch.h"

#include "common_idumpstate.h"

#include "net_server_common.h"
#include "net_server_ilistener.h"

template <typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename HandlerType>
class Net_Server_AsynchListener_T
 : public ACE_Asynch_Acceptor<HandlerType>
 , public Net_Server_IListener_T<Net_ListenerConfiguration_t>
 , public Common_IDumpState
{
  // singleton needs access to the ctor/dtors
  friend class ACE_Singleton<Net_Server_AsynchListener_T<ConfigurationType,
                                                         SocketHandlerConfigurationType,
                                                         UserDataType,
                                                         HandlerType>,
                             ACE_Recursive_Thread_Mutex>;

 public:
  virtual int accept (size_t = 0,          // bytes to read
                      const void* = NULL); // ACT
  // override default accept strategy
  virtual int validate_connection (const ACE_Asynch_Accept::Result&, // result
                                   const ACE_INET_Addr&,             // remote address
                                   const ACE_INET_Addr&);            // local address
  virtual int should_reissue_accept (void);

  // implement Net_Server_IListener_T
  virtual bool initialize (const Net_ListenerConfiguration_t&);
  virtual bool useReactor () const;

  bool isInitialized () const;

  // implement Common_IControl
  // *WARNING*: this API is NOT re-entrant !
  virtual void start ();
  virtual void stop (bool = true); // locked access ?
  virtual bool isRunning () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // override default creation strategy
  virtual HandlerType* make_handler (void);

 private:
  typedef ACE_Asynch_Acceptor<HandlerType> inherited;

  Net_Server_AsynchListener_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_Server_AsynchListener_T (const Net_Server_AsynchListener_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_Server_AsynchListener_T& operator= (const Net_Server_AsynchListener_T&));
  virtual ~Net_Server_AsynchListener_T ();

//  // override default accept strategy
//  // *NOTE*: ACE doesn't properly handle cancellation (dangling bound port on listen socket) -->
//  // fix this here... --> *TODO*: send patch to ACE people
//  virtual void handle_accept(const ACE_Asynch_Accept::Result&); // result

  int                                   addressFamily_;
  Stream_IAllocator*                    allocator_;
  const SocketHandlerConfigurationType* configuration_;
  Net_IInetConnectionManager_t*         interfaceHandle_;
  bool                                  isInitialized_;
  bool                                  isListening_;
  unsigned short                        listeningPort_;
  unsigned int                          statisticCollectionInterval_;
  bool                                  useLoopback_;
};

// include template implementation
#include "net_server_asynchlistener.inl"

#endif
