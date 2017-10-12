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

#ifndef TEST_U_CLIENT_TIMEOUTHANDLER_H
#define TEST_U_CLIENT_TIMEOUTHANDLER_H

#include <functional>
#include <random>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_itimerhandler.h"
#include "common_timerhandler.h"

#include "test_u_stream.h"

#include "net_client_connector_common.h"

class Test_U_Client_TimeoutHandler
 : public Common_TimerHandler
 , public Common_ITimerHandler
{
  typedef Common_TimerHandler inherited;

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

  Test_U_Client_TimeoutHandler (enum ActionModeType,   // mode
                                unsigned int,          // max #connections
                                Test_U_IConnector_t*); // connector
  inline virtual ~Test_U_Client_TimeoutHandler () {};

  void mode (enum ActionModeType);
  enum ActionModeType mode () const;

  // implement specific behaviour
  virtual void handle (const void*); // asynchronous completion token

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Client_TimeoutHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_Client_TimeoutHandler (const Test_U_Client_TimeoutHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Client_TimeoutHandler& operator= (const Test_U_Client_TimeoutHandler&))

  enum AlternatingModeStateType      alternatingModeState_;
  Test_U_IConnector_t*               connector_;
  mutable ACE_SYNCH_MUTEX            lock_;
  unsigned int                       maximumNumberOfConnections_;
  enum ActionModeType                mode_;
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
