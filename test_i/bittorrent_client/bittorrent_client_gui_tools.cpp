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

//#include "ace/Synch.h"

//#include "http_scanner.h"

//#include "stream_misc_messagehandler.h"
#include "bittorrent_client_gui_tools.h"

#include "ace/Log_Msg.h"

#include "common_ui_defines.h"

#include "net_macros.h"

#include "bittorrent_client_network.h"

#include "bittorrent_client_gui_session.h"
#include "bittorrent_client_gui_defines.h"

//bool
//BitTorrent_Client_UI_Tools::current (const std::string& timeStamp_in,
//                              const BitTorrent_Client_GUI_Connections_t& connections_in,
//                              std::string& nickName_out,
//                              std::string& channel_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_UI_Tools::current"));

//  // initialize return value(s)
//  nickName_out.clear ();
//  channel_out.clear ();

//  // step1: retrieve connection handle
//  BitTorrent_Client_GUI_Connection* connection_p = NULL;
//  for (BitTorrent_Client_GUI_ConnectionsConstIterator_t iterator = connections_in.begin ();
//       iterator != connections_in.end ();
//       ++iterator)
//  {
//    const BitTorrent_Client_GTK_ConnectionCBData& connection_data_r =
//      (*iterator).second->get ();
//    if (connection_data_r.timeStamp == timeStamp_in)
//    {
//      connection_p = (*iterator).second;
//      ACE_ASSERT (connection_p);
//      //connection_p->current (nickName_out,
//      //                       channel_out);
//      const BitTorrent_Client_SessionState& connection_state_r =
//        connection_p->state ();
//      nickName_out = connection_state_r.nickName;
//      channel_out = connection_state_r.channel;
//      break;
//    } // end IF
//  } // end FOR
//  if (!connection_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("could not find connection (timestamp was: \"%s\"), aborting\n")));
//    return false;
//  } // end IF

//  return true;
//}

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
BitTorrent_Client_GUI_Session_t*
BitTorrent_Client_UI_Tools::current (const Common_UI_GTK_State_t& UIState_in,
                                     const BitTorrent_Client_GUI_Sessions_t& sessions_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_UI_Tools::current"));

  // initialize return value(s)
  BitTorrent_Client_GUI_Session_t* result_p = NULL;

  // sanity check(s)
  Common_UI_GTK_BuildersConstIterator_t iterator =
    UIState_in.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != UIState_in.builders.end ());

  // step1: retrieve connections notebook
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_SESSIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_number = gtk_notebook_get_current_page (notebook_p);
  if (tab_number == -1)
    return NULL; // no connection page (yet ?)
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_number);
  ACE_ASSERT (widget_p);

  // step2: find session whose main window corresponds with the currently
  //        active notebook page
  for (BitTorrent_Client_GUI_SessionsConstIterator_t iterator_2 = sessions_in.begin ();
       iterator_2 != sessions_in.end ();
       ++iterator_2)
  {
    const struct BitTorrent_Client_UI_SessionCBData& data_r =
      (*iterator_2).second->getR ();
    Common_UI_GTK_BuildersConstIterator_t iterator_3 =
      UIState_in.builders.find (data_r.label);
    if (iterator_3 == UIState_in.builders.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("could not find builder (connection was: \"%s\"), aborting\n"),
                  ACE_TEXT (data_r.label.c_str ())));
      break;
    } // end IF

    GtkWidget* widget_2 =
      GTK_WIDGET (gtk_builder_get_object ((*iterator_3).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_VBOX_SESSION)));
    ACE_ASSERT (widget_2);
    if (widget_p == widget_2)
    {
      result_p = (*iterator_2).second;
      break;
    } // end IF
  } // end FOR

  // *NOTE*: there is a delay between session establishment and registration.
  //         In this state, a session page is already added, even though
  //         CBData_in.sessions has not been updated (yet)
  //         --> no session will be found here

  return result_p;
}
#endif // GTK_USE
#endif // GUI_SUPPORT
