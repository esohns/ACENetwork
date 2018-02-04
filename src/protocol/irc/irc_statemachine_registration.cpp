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
#include "stdafx.h"

#include "ace/Synch.h"
#include "irc_statemachine_registration.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "irc_defines.h"

const char network_irc_statemachine_registration_name_string_[] =
    ACE_TEXT_ALWAYS_CHAR (IRC_STATEMACHINE_REGISTRATION_NAME);

IRC_StateMachine_Registration::IRC_StateMachine_Registration ()
 : inherited (&lock_,                      // lock handle
              IRC_REGISTRATION_STATE_PASS) // (initial) state
 , lock_ ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_StateMachine_Registration::IRC_StateMachine_Registration"));

}

bool
IRC_StateMachine_Registration::change (enum IRC_RegistrationStateType newState_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_StateMachine_Registration::change"));

  // sanity check(s)
  ACE_ASSERT (inherited::stateLock_);

  // synchronize access to state machine
  ACE_Guard<ACE_Null_Mutex> aGuard (*inherited::stateLock_);

  switch (inherited::state_)
  {
    case IRC_REGISTRATION_STATE_PASS:
    {
      switch (newState_in)
      {
        // good case
        case IRC_REGISTRATION_STATE_NICK:
        case IRC_REGISTRATION_STATE_FINISHED:
        {
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: PASS --> %s\n"),
//                       ACE_TEXT (state2String (newState_in).c_str ())));

          inherited::change (newState_in);

          return true;
        }
        // error case
        case IRC_REGISTRATION_STATE_PASS:
        case IRC_REGISTRATION_STATE_USER:
        default:
          break;
      } // end SWITCH

      break;
    }
    case IRC_REGISTRATION_STATE_NICK:
    {
      switch (newState_in)
      {
        // good case
        case IRC_REGISTRATION_STATE_USER:
        {
          //           ACE_DEBUG ((LM_DEBUG,
          //                       ACE_TEXT ("state switch: NICK --> USER\n")));

          inherited::change (newState_in);

          return true;
        }
        // error case
        case IRC_REGISTRATION_STATE_PASS:
        case IRC_REGISTRATION_STATE_NICK:
        case IRC_REGISTRATION_STATE_FINISHED:
        default:
          break;
      } // end SWITCH

      break;
    }
    case IRC_REGISTRATION_STATE_USER:
    {
      switch (newState_in)
      {
        // good case
        case IRC_REGISTRATION_STATE_FINISHED:
        {
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: USER --> FINISHED\n")));

          inherited::change (newState_in);

          return true;
        }
        // error case
        case IRC_REGISTRATION_STATE_PASS:
        case IRC_REGISTRATION_STATE_NICK:
        case IRC_REGISTRATION_STATE_USER:
        default:
          break;
      } // end SWITCH

      break;
    }
    case IRC_REGISTRATION_STATE_FINISHED:
    {
      switch (newState_in)
      {
        // *IMPORTANT NOTE*: the whole stream needs to re-initialize BEFORE this
        //                   happens
        // good case
        case IRC_REGISTRATION_STATE_PASS:
        {
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("state switch: FINISHED --> PASS\n")));

          inherited::change (newState_in);

          return true;
        }
        // error case
        case IRC_REGISTRATION_STATE_NICK:
        case IRC_REGISTRATION_STATE_USER:
        case IRC_REGISTRATION_STATE_FINISHED:
        default:
          break;
      } // end SWITCH

      break;
    }
    default:
      break;
  } // end SWITCH
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("unknown/invalid state switch: \"%s\" --> \"%s\" --> check implementation !, aborting\n"),
              ACE_TEXT (stateToString (inherited::state_).c_str ()),
              ACE_TEXT (stateToString (newState_in).c_str ())));

  return false;
}

std::string
IRC_StateMachine_Registration::stateToString (enum IRC_RegistrationStateType state_in) const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_StateMachine_Registration::stateToString"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID");

  switch (state_in)
  {
    case IRC_REGISTRATION_STATE_NICK:
      result = ACE_TEXT_ALWAYS_CHAR ("NICK"); break;
    case IRC_REGISTRATION_STATE_PASS:
      result = ACE_TEXT_ALWAYS_CHAR ("PASS"); break;
    case IRC_REGISTRATION_STATE_USER:
      result = ACE_TEXT_ALWAYS_CHAR ("USER"); break;
    case IRC_REGISTRATION_STATE_FINISHED:
      result = ACE_TEXT_ALWAYS_CHAR ("FINISHED"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid state (was: %d), aborting\n"),
                  state_in));
      break;
    }
  } // end SWITCH

  return result;
}
