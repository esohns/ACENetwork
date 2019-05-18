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
#include "ace/Synch_Traits.h"

#include "net_ilistener.h"

template <typename HandlerType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Server_AsynchListener_T
 : public ACE_Asynch_Acceptor<HandlerType>
 , public Net_IListener_T<ConfigurationType>
{
  typedef ACE_Asynch_Acceptor<HandlerType> inherited;

  // singleton needs access to the ctor/dtors
  friend class ACE_Singleton<Net_Server_AsynchListener_T<HandlerType,
                                                         AddressType,
                                                         ConfigurationType,
                                                         StateType,
                                                         StreamType,
                                                         UserDataType>,
                             ACE_SYNCH_RECURSIVE_MUTEX>;

 public:
  // convenient types
  typedef ACE_Singleton<Net_Server_AsynchListener_T<HandlerType,
                                                    AddressType,
                                                    ConfigurationType,
                                                    StateType,
                                                    StreamType,
                                                    UserDataType>,
                        ACE_SYNCH_RECURSIVE_MUTEX> SINGLETON_T;

  virtual int accept (size_t = 0,          // bytes to read
                      const void* = NULL); // ACT
  // override default accept strategy
  virtual int validate_connection (const ACE_Asynch_Accept::Result&, // result
                                   const ACE_INET_Addr&,             // remote address
                                   const ACE_INET_Addr&);            // local address
  // default behavior: delegate to baseclass
  inline virtual int should_reissue_accept (void) { return inherited::should_reissue_accept (); }

  // implement Net_IListener_T
  // *WARNING*: this API is NOT re-entrant !
  virtual void start (ACE_thread_t&); // return value: thread handle (if any)
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  inline virtual bool isRunning () const { return isListening_; }

  // *NOTE*: handlers receive the configuration object via
  //         ACE_Service_Handler::act ()
  inline virtual const ConfigurationType& getR_2 () const { ACE_ASSERT (configuration_); return *(configuration_); }
  virtual bool initialize (const ConfigurationType&); // configuration
  inline virtual bool useReactor () const { return false; }

  inline bool isInitialized () const { return isInitialized_; }

  // implement Common_IDumpState
  inline virtual void dump_state () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 protected:
  // override default accept strategy
  virtual void handle_accept (const ACE_Asynch_Accept::Result&); // result
  // override default creation strategy
  virtual HandlerType* make_handler (void); // return value: socket handler handle

 private:
  typedef Net_IListener_T<ConfigurationType> ILISTENER_T;

  Net_Server_AsynchListener_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_Server_AsynchListener_T (const Net_Server_AsynchListener_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Server_AsynchListener_T& operator= (const Net_Server_AsynchListener_T&))
  virtual ~Net_Server_AsynchListener_T ();

  // implement (part of) Net_IListener_T
  inline virtual bool lock (bool = true) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual int unlock (bool = false) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual const ACE_SYNCH_MUTEX& getR () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (ACE_SYNCH_MUTEX ()); ACE_NOTREACHED (return ACE_SYNCH_MUTEX ();) }
  inline virtual void idle () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void finished () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // implement (part of) Common_IControl_T
  inline virtual void initialize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // override default listen strategy
  // *TODO*: currently tailored for TCP only (see implementation)
  virtual int open (const AddressType&,               // listen address
                    size_t = 0,                       // #bytes to read
                    bool = false,                     // pass addresses
                    int = ACE_DEFAULT_ASYNCH_BACKLOG, // backlog
                    int = 1,                          // SO_REUSEADDR ?
                    ACE_Proactor* = 0,                // proactor handle
                    bool = false,                     // validate_connection()s ?
                    int = 1,                          // reissue accept()s ?
                    int = -1);                        // #initial accept()s
  //// override default accept strategy
  //// *NOTE*: ACE doesn't properly handle cancellation (dangling bound port on
  ////         listen socket) --> fix this here... -->
  //// *TODO*: send patch to ACE maintainers
  //virtual void handle_accept(const ACE_Asynch_Accept::Result&); // result

  ConfigurationType* configuration_;
  bool               isInitialized_;
  bool               isListening_;
};

// include template definition
#include "net_server_asynchlistener.inl"

#endif
