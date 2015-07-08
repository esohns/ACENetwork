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

#include "IRC_client_gui_tools.h"

#include "ace/Log_Msg.h"
#include "ace/Synch.h"

#include "common_ui_defines.h"

#include "net_macros.h"

#include "IRC_client_gui_connection.h"
#include "IRC_client_gui_defines.h"

bool
IRC_Client_UI_Tools::current (const std::string& timestamp_in,
                              const IRC_Client_GTK_CBData& CBData_in,
                              std::string& nickname_out,
                              std::string& channel_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_UI_Tools::current"));

  // initialize return value(s)
  nickname_out.clear ();
  channel_out.clear ();

  // step1: retrieve connection handle
  IRC_Client_GUI_Connection* connection_p = NULL;
  for (IRC_Client_GUI_ConnectionsConstIterator_t iterator = CBData_in.connections.begin ();
       iterator != CBData_in.connections.end ();
       ++iterator)
  {
    const IRC_Client_GTK_ConnectionCBData& connection_data_r =
      (*iterator).second->get ();
    if (connection_data_r.timestamp == timestamp_in)
    {
      nickname_out = connection_data_r.IRCSessionState.nickname;
      channel_out = (*iterator).second->getActiveID ();
      connection_p = (*iterator).second;
      break;
    } // end IF
  } // end FOR
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("could not find connection (timestamp was: \"%s\"), aborting\n")));
    return false;
  } // end IF

  return true;
}

IRC_Client_GUI_Connection*
IRC_Client_UI_Tools::current (const IRC_Client_GTK_CBData& CBData_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_UI_Tools::current"));

  // initialize return value(s)
  IRC_Client_GUI_Connection* result_p = NULL;

  // sanity check(s)
  Common_UI_GTKBuildersConstIterator_t iterator =
    CBData_in.GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != CBData_in.GTKState.builders.end ());

  // step1: retrieve connections notebook
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_number = gtk_notebook_get_current_page (notebook_p);
  if (tab_number == -1)
    return NULL; // no connection page (yet ?)
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_number);
  ACE_ASSERT (widget_p);

  // step2: find connection whose main window corresponds with the currently
  //        active notebook page
  for (IRC_Client_GUI_ConnectionsConstIterator_t iterator_2 = CBData_in.connections.begin ();
       iterator_2 != CBData_in.connections.end ();
       ++iterator_2)
  {
    const IRC_Client_GTK_ConnectionCBData& connection_data_r =
      (*iterator_2).second->get ();
    Common_UI_GTKBuildersConstIterator_t iterator_3 =
      CBData_in.GTKState.builders.find (connection_data_r.timestamp);
    if (iterator_3 == CBData_in.GTKState.builders.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("could not find builder (connection was: \"%s\"), aborting\n"),
                  ACE_TEXT (connection_data_r.label.c_str ())));
      break;
    } // end IF

    GtkWidget* widget_2 =
      GTK_WIDGET (gtk_builder_get_object ((*iterator_3).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CONNECTION)));
    ACE_ASSERT (widget_2);
    if (widget_p == widget_2)
    {
      result_p = (*iterator_2).second;
      break;
    } // end IF
  } // end FOR

  // *NOTE*: there is a delay between connection establishment and registration.
  //         In this state, a server page is already added, even though
  //         CBData_in.connections has not been updated (yet)
  //         --> no connection will be found here

  return result_p;
}
