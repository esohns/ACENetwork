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

#ifndef CLIENT_TIMEOUTHANDLER_H
#define CLIENT_TIMEOUTHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

//#include "common_itimerhandler.h"
#include "common_timer_handler.h"

//#include "test_u_stream.h"

#include "net_client_connector_common.h"

class Client_TimeoutHandler
 : public Common_Timer_Handler
{
  typedef Common_Timer_Handler inherited;

 public:
  enum ActionModeType
  {
    ACTION_NORMAL = 0,
    ACTION_ALTERNATING,
    ACTION_STRESS,
    //// ---------------------------------
    //ACTION_GTK, // dispatch UI events
    // -----------------------------------
    ACTION_MAX,
    ACTION_INVALID = -1
  };

  enum AlternatingModeStateType
  {
    ALTERNATING_STATE_CONNECT = 0,
    ALTERNATING_STATE_ABORT,
    // -----------------------------------
    ALTERNATING_STATE_MAX,
    ALTERNATING_STATE_INVALID = -1
  };

  Client_TimeoutHandler (enum ActionModeType,                        // mode
                         unsigned int,                               // max #connections
                         const struct Test_U_ProtocolConfiguration&,
                         const Test_U_TCPConnectionConfiguration&,
                         const Test_U_UDPConnectionConfiguration&,
                         enum Common_EventDispatchType = COMMON_EVENT_DEFAULT_DISPATCH);
  inline virtual ~Client_TimeoutHandler () {}

  inline void mode (enum ActionModeType mode_in) { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_); mode_ = mode_in; }
  inline enum ActionModeType mode () const { enum ActionModeType result = ACTION_INVALID; { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, ACTION_INVALID); result = mode_; } /* end lock scope */ return result; }
  inline enum Net_TransportLayerType protocol () const { ACE_ASSERT (protocolConfiguration_); return protocolConfiguration_->transportLayer; }

  // implement specific behaviour
  virtual void handle (const void*); // asynchronous completion token

 private:
  ACE_UNIMPLEMENTED_FUNC (Client_TimeoutHandler ())
  ACE_UNIMPLEMENTED_FUNC (Client_TimeoutHandler (const Client_TimeoutHandler&))
  ACE_UNIMPLEMENTED_FUNC (Client_TimeoutHandler& operator= (const Client_TimeoutHandler&))

  enum AlternatingModeStateType        alternatingModeState_;
  Test_U_TCPConnectionConfiguration*   connectionConfiguration_;
  Test_U_UDPConnectionConfiguration*   UDPConnectionConfiguration_;
  enum Common_EventDispatchType        eventDispatch_;
  mutable ACE_SYNCH_MUTEX              lock_;
  unsigned int                         maximumNumberOfConnections_;
  enum ActionModeType                  mode_;
  struct Test_U_ProtocolConfiguration* protocolConfiguration_;

  Client_TCP_AsynchConnector_t         AsynchTCPConnector_;
  Client_TCP_Connector_t               TCPConnector_;
  Client_UDP_AsynchConnector_t         AsynchUDPConnector_;
  Client_UDP_Connector_t               UDPConnector_;
#if defined (SSL_SUPPORT)
  Client_SSL_Connector_t               SSLConnector_;
#endif // SSL_SUPPORT
};

#endif
