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
#include "test_u_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "net_macros.h"

#include "test_u_ui_callbacks.h"

Test_U_EventHandler::Test_U_EventHandler (struct WLANMonitor_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onAssociate (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onAssociate (const std::string& interfaceIdentifier_in,
#endif
                                  const std::string& SSID_in,
                                  bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onAssociate"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (SSID_in);
  ACE_UNUSED_ARG (success_in);
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onConnect (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onConnect (const std::string& interfaceIdentifier_in,
#endif
                                const std::string& SSID_in,
                                bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onConnect"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (SSID_in);
  ACE_UNUSED_ARG (success_in);
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onHotPlug (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onHotPlug (const std::string& interfaceIdentifier_in,
#endif
                                bool enabled_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onHotPlug"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);
  ACE_UNUSED_ARG (enabled_in);
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_EventHandler::onScanComplete (REFGUID interfaceIdentifier_in,
#else
Test_U_EventHandler::onScanComplete (const std::string& interfaceIdentifier_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_EventHandler::onScanComplete"));

  ACE_UNUSED_ARG (interfaceIdentifier_in);

  // sanity check(s)
  if (!CBData_)
    return;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    guint event_source_id = g_idle_add (idle_update_ssids_cb,
                                        CBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_ssids_cb): \"%m\", returning\n")));
      return;
    } // end IF
    CBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope
}
