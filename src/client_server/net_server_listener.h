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

#ifndef NET_SERVER_LISTENER_H
#define NET_SERVER_LISTENER_H

#include "ace/Acceptor.h"
#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_ilistener.h"

template <typename HandlerType,
          typename AcceptorType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          ////////////////////////////////
          typename ConnectionConfigurationType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Server_Listener_T
 : public ACE_Acceptor<HandlerType,
                       AcceptorType>
 , public Net_IListener_T<ConfigurationType, 
                          ConnectionConfigurationType>
{
  typedef ACE_Acceptor<HandlerType,
                       AcceptorType> inherited;

  // singleton needs access to the ctor/dtors
  friend class ACE_Singleton<Net_Server_Listener_T<HandlerType,
                                                   AcceptorType,
                                                   AddressType,
                                                   ConfigurationType,
                                                   StateType,
                                                   ConnectionConfigurationType,
                                                   StreamType,
                                                   UserDataType>,
                             ACE_SYNCH_RECURSIVE_MUTEX>;

 public:
  // convenient types
  typedef ACE_Singleton<Net_Server_Listener_T<HandlerType,
                                              AcceptorType,
                                              AddressType,
                                              ConfigurationType,
                                              StateType,
                                              ConnectionConfigurationType,
                                              StreamType,
                                              UserDataType>,
                        ACE_SYNCH_RECURSIVE_MUTEX> SINGLETON_T;

  inline bool isInitialized () const { return isInitialized_; }

  // override some methods from ACE_Acceptor
  // *NOTE*: "in the event that an accept fails, this method will be called and
  // the return value will be returned from handle_input()."
  virtual int handle_accept_error (void);

  // implement (part of) Net_IListener_T
  // *WARNING*: this API is NOT re-entrant !
  virtual void start (ACE_thread_t&); // return value: thread handle (if any)
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  inline virtual bool isRunning () const { return isListening_; }

  inline virtual const ConnectionConfigurationType& getR_2 () const { ACE_ASSERT (configuration_); ACE_ASSERT (configuration_->connectionConfiguration); return *(configuration_->connectionConfiguration); }
  //virtual bool initialize (const ConnectionConfigurationType&);
  virtual bool initialize (const ConfigurationType&);
  inline virtual bool useReactor () const { return true; }

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);
  // override default accept strategy
  virtual int accept_svc_handler (HandlerType*);
  // override default activation strategy
  virtual int activate_svc_handler (HandlerType*);

 private:
  // convenient types
  typedef Net_IListener_T<ConfigurationType,
                          ConnectionConfigurationType> ILISTENER_T;

  Net_Server_Listener_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_Server_Listener_T (const Net_Server_Listener_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Server_Listener_T& operator= (const Net_Server_Listener_T&))
  virtual ~Net_Server_Listener_T ();

  // implement (part of) Common_IControl_T
  inline virtual void initialize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // implement (part of) Net_IListener_T
  inline virtual bool lock (bool = true) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual int unlock (bool = false) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual const ACE_SYNCH_MUTEX& getR () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (ACE_SYNCH_MUTEX ()); ACE_NOTREACHED (return ACE_SYNCH_MUTEX ();) }
  inline virtual void idle () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void finished () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  ConfigurationType* configuration_;
  bool               hasChanged_;
  bool               isInitialized_;
  bool               isListening_;
  bool               isOpen_;
  bool               isSuspended_;
};

// include template definition
#include "net_server_listener.inl"

#endif
