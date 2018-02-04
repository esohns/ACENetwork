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

#include "common_istatemachine.h"
#include "common_statemachine_base.h"

#include "irc_exports.h"

enum IRC_RegistrationStateType : int
{
  IRC_REGISTRATION_STATE_INVALID = -1,
  IRC_REGISTRATION_STATE_PASS = 0,
  IRC_REGISTRATION_STATE_NICK,
  IRC_REGISTRATION_STATE_USER,
  IRC_REGISTRATION_STATE_FINISHED,
  ////////////////////////////////////////
  IRC_REGISTRATION_STATE_MAX
};

extern const char network_irc_statemachine_registration_name_string_[];

class IRC_Export IRC_StateMachine_Registration
 : public Common_StateMachine_Base_T<network_irc_statemachine_registration_name_string_,
                                     ACE_NULL_SYNCH,
                                     enum IRC_RegistrationStateType,
                                     Common_IStateMachine_T<enum IRC_RegistrationStateType> >
{
  typedef Common_StateMachine_Base_T<network_irc_statemachine_registration_name_string_,
                                     ACE_NULL_SYNCH,
                                     enum IRC_RegistrationStateType,
                                     Common_IStateMachine_T<enum IRC_RegistrationStateType> > inherited;

 public:
  IRC_StateMachine_Registration ();
  inline virtual ~IRC_StateMachine_Registration () {}

  // implement (part of) Common_IStateMachine_T
  virtual std::string stateToString (enum IRC_RegistrationStateType) const;

 protected:
  // implement (part of) Common_IStateMachine_T
  // *NOTE*: only derived classes can change state
  virtual bool change (enum IRC_RegistrationStateType); // new state

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_StateMachine_Registration (const IRC_StateMachine_Registration&))
  ACE_UNIMPLEMENTED_FUNC (IRC_StateMachine_Registration& operator= (const IRC_StateMachine_Registration&))

  ACE_SYNCH_NULL_MUTEX lock_;
};

//////////////////////////////////////////

typedef Common_IStateMachine_T<enum IRC_RegistrationStateType> IRC_IRegistrationStateMachine_t;

#endif
