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

#include "test_i_module_eventhandler.h"

#include "net_macros.h"

Test_I_Module_EventHandler::Test_I_Module_EventHandler (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_EventHandler::Test_I_Module_EventHandler"));

}

ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
Test_I_Module_EventHandler::clone ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_EventHandler::clone"));

  // initialize return value(s)
  ACE_Task<ACE_MT_SYNCH,
           Common_TimePolicy_t>* task_p = NULL;

  ACE_NEW_NORETURN (task_p,
                    Test_I_Module_EventHandler (NULL));
  if (!task_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::mod_->name ()));
  else
  {
    inherited* inherited_p = dynamic_cast<inherited*> (task_p);
    ACE_ASSERT (inherited_p);
    ACE_ASSERT (inherited::configuration_);
    inherited_p->initialize (*inherited::configuration_,
                             inherited::allocator_);
  } // end ELSE

  return task_p;
}

//////////////////////////////////////////

Test_I_Module_EventHandler_2::Test_I_Module_EventHandler_2 (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_EventHandler_2::Test_I_Module_EventHandler_2"));

}

ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
Test_I_Module_EventHandler_2::clone ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_EventHandler_2::clone"));

  // initialize return value(s)
  ACE_Task<ACE_MT_SYNCH,
           Common_TimePolicy_t>* task_p = NULL;

  ACE_NEW_NORETURN (task_p,
                    Test_I_Module_EventHandler_2 (NULL));
  if (!task_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                inherited::mod_->name ()));
  else
  {
    inherited* inherited_p = dynamic_cast<inherited*> (task_p);
    ACE_ASSERT (inherited_p);
    ACE_ASSERT (inherited::configuration_);
    inherited_p->initialize (*inherited::configuration_,
                             inherited::allocator_);
  } // end ELSE

  return task_p;
}
