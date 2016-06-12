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

#ifndef NET_SERVER_SSL_LISTENER_H
#define NET_SERVER_SSL_LISTENER_H

#include "ace/Acceptor.h"
#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/ssl/SSL_SOCK_Acceptor.h"

#include "common_idumpstate.h"

#include "net_ilistener.h"

template <typename HandlerType,
          typename AcceptorType,
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_Server_SSL_Listener_T
 : public ACE_SSL_SOCK_Acceptor
 , public Net_IListener_T<ConfigurationType, 
                          HandlerConfigurationType>
 , public Common_IDumpState
{
  // singleton needs access to the ctor/dtors
  friend class ACE_Singleton<Net_Server_SSL_Listener_T<HandlerType,
                                                       AcceptorType,

                                                       AddressType,
                                                       ConfigurationType,
                                                       StateType,
                                                       StreamType,

                                                       HandlerConfigurationType,

                                                       UserDataType>,
                             ACE_SYNCH_RECURSIVE_MUTEX>;

 public:
  //typedef Net_IConnectionManager_T<AddressType,
  //                                 ConfigurationType,
  //                                 StateType,
  //                                 Stream_Statistic,
  //                                 StreamType,
  //                                 //////
  //                                 UserDataType> ICONNECTION_MANAGER_T;

  bool isInitialized () const;

  // override some methods from ACE_Acceptor
  // *NOTE*: "in the event that an accept fails, this method will be called and
  // the return value will be returned from handle_input()."
  virtual int handle_accept_error (void);

  // implement (part of) Net_IListener_T
  // *WARNING*: this API is NOT re-entrant !
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual bool isRunning () const;

  virtual const HandlerConfigurationType& get () const;
  //virtual bool initialize (const HandlerConfigurationType&);
  virtual bool initialize (const ConfigurationType&);
  virtual bool useReactor () const;

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
  typedef ACE_SSL_SOCK_Acceptor inherited;

  typedef Net_IListener_T<ConfigurationType,
                          HandlerConfigurationType> ILISTENER_T;

  Net_Server_SSL_Listener_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_Server_SSL_Listener_T (const Net_Server_SSL_Listener_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Server_SSL_Listener_T& operator= (const Net_Server_SSL_Listener_T&))
  virtual ~Net_Server_SSL_Listener_T ();

  // implement (part of) Common_IControl_T
  virtual void initialize ();

  ConfigurationType*        configuration_;
  HandlerConfigurationType* handlerConfiguration_;
  bool                      isInitialized_;
  bool                      isListening_;
  bool                      isOpen_;
};

// include template implementation
#include "net_server_ssl_listener.inl"

#endif