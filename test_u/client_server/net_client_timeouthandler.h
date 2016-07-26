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

#ifndef NET_CLIENT_TIMEOUTHANDLER_H
#define NET_CLIENT_TIMEOUTHANDLER_H

#include <functional>
#include <random>
#include <string>

#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "test_u_stream.h"

#include "net_client_connector_common.h"

class Net_Client_TimeoutHandler
 : public ACE_Event_Handler
{
 public:
  enum ActionMode_t
  {
    ACTION_NORMAL = 0,
    ACTION_ALTERNATING,
    ACTION_STRESS,
    //// ---------------------
    //ACTION_GTK, // dispatch UI events
    // ---------------------
    ACTION_MAX,
    ACTION_INVALID = -1
  };

  enum AlternatingModeState_t
  {
    ALTERNATING_STATE_CONNECT = 0,
    ALTERNATING_STATE_ABORT,
    // ---------------------
    ALTERNATING_STATE_MAX,
    ALTERNATING_STATE_INVALID = -1
  };

  Net_Client_TimeoutHandler (ActionMode_t,         // mode
                             unsigned int,         // max #connections
                             const ACE_INET_Addr&, // remote SAP
                             Net_IConnector_t*);   // connector
  virtual ~Net_Client_TimeoutHandler ();

  void mode (ActionMode_t);
  ActionMode_t mode () const;

  // implement specific behaviour
  virtual int handle_timeout (const ACE_Time_Value&, // current time
                              const void*);          // asynchronous completion token

 private:
  typedef ACE_Event_Handler inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_TimeoutHandler ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_TimeoutHandler (const Net_Client_TimeoutHandler&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_TimeoutHandler& operator= (const Net_Client_TimeoutHandler&))

  AlternatingModeState_t             alternatingModeState_;
  Net_IConnector_t*                  connector_;
  mutable ACE_SYNCH_MUTEX            lock_;
  unsigned int                       maximumNumberOfConnections_;
  ActionMode_t                       mode_;
  ACE_INET_Addr                      peerAddress_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  char                               randomStateInitializationBuffer_[BUFSIZ];
  struct random_data                 randomState_;
#endif

  // probability
  std::uniform_int_distribution<int> randomDistribution_;
  std::default_random_engine         randomEngine_;
  std::function<int ()>              randomGenerator_;
};

#endif
