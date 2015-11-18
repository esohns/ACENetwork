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

#ifndef IRC_STATEMACHINE_REGISTRATION_H
#define IRC_STATEMACHINE_REGISTRATION_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"
#include "ace/Null_Mutex.h"

#include "common_statemachine_base.h"

#include "irc_exports.h"

enum IRC_RegistrationState
{
  REGISTRATION_STATE_INVALID = -1,
  REGISTRATION_STATE_PASS = 0,
  REGISTRATION_STATE_NICK,
  REGISTRATION_STATE_USER,
  REGISTRATION_STATE_FINISHED,
  /////////////////////////////////////
  REGISTRATION_STATE_MAX
};

class IRC_Export IRC_StateMachine_Registration
 : public Common_StateMachine_Base_T<ACE_SYNCH_NULL_MUTEX,
                                     IRC_RegistrationState>
{
 public:
  IRC_StateMachine_Registration ();
  virtual ~IRC_StateMachine_Registration ();

  // implement (part of) Common_IStateMachine_T
  virtual void initialize ();
  virtual void reset ();
  virtual std::string state2String (IRC_RegistrationState) const;

 protected:
  // implement (part of) Common_IStateMachine_T
  // *NOTE*: only derived classes can change state
  virtual bool change (IRC_RegistrationState); // new state

 private:
  typedef Common_StateMachine_Base_T<ACE_SYNCH_NULL_MUTEX,
                                     IRC_RegistrationState> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_StateMachine_Registration (const IRC_StateMachine_Registration&))
  ACE_UNIMPLEMENTED_FUNC (IRC_StateMachine_Registration& operator= (const IRC_StateMachine_Registration&))

  ACE_SYNCH_NULL_MUTEX lock_;
};

// convenient typedef
typedef Common_IStateMachine_T<IRC_RegistrationState> IRC_IRegistrationStateMachine_t;

#endif
