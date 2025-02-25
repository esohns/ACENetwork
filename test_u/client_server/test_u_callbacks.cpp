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

#include "test_u_callbacks.h"

#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "test_u_connection_manager_common.h"
#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_stream.h"

#include "net_client_common.h"
#include "net_client_server_defines.h"
#include "net_client_signalhandler.h"
#include "net_client_timeouthandler.h"

#include "net_server_common.h"

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Test_U_GTK_CBData* data_p =
    static_cast<struct Test_U_GTK_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (data_p->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_CONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_spin (spin_button_p,
                                GTK_SPIN_STEP_FORWARD,
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;

          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_DISCONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_spin (spin_button_p,
                                GTK_SPIN_STEP_BACKWARD,
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;

          break;
        }
        case COMMON_UI_EVENT_SESSION:
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *event_p));
          break;
        }
      } // end SWITCH
      ACE_UNUSED_ARG (is_session_message);
      gtk_spin_button_spin (spin_button_p,
                            GTK_SPIN_STEP_FORWARD,
                            1.0);
      event_p = NULL;
    } // end FOR

    // clean up
    while (!data_p->UIState->eventStack.is_empty ())
    {
      result = data_p->UIState->eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

gboolean
idle_initialize_client_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_client_UI_cb"));

  struct Client_UI_CBData* data_p =
    static_cast<struct Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  struct Client_Configuration* configuration_p =
    static_cast<struct Client_Configuration*> (data_p->configuration);
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->timeoutHandler);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window_set_title (,
  //                      caption.c_str ());

  //  GtkWidget* about_dialog_p =
  //    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
  //                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_DIALOG_ABOUT_NAME)));
  //  ACE_ASSERT (about_dialog_p);

  // step2: initialize info view
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));

  // step3: initialize options
  std::string radio_button_name;
  switch (configuration_p->timeoutHandler->mode ())
  {
    case Client_TimeoutHandler::ACTION_NORMAL:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_RADIOBUTTON_NORMAL_NAME);
      break;
    case Client_TimeoutHandler::ACTION_ALTERNATING:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_RADIOBUTTON_ALTERNATING_NAME);
      break;
    case Client_TimeoutHandler::ACTION_STRESS:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_RADIOBUTTON_STRESS_NAME);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid mode (was: %d), aborting\n"),
                  configuration_p->timeoutHandler->mode ()));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
  GtkRadioButton* radiobutton_p =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              radio_button_name.c_str ()));
  ACE_ASSERT (radiobutton_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_p), TRUE);

#if defined (SSL_SUPPORT)
#if defined (SSL_SUPPORT)
#else
  radiobutton_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_SSL_NAME)));
    ACE_ASSERT (radiobutton_p);
    gtk_widget_set_sensitive (GTK_WIDGET (radiobutton_p), FALSE);
#endif // SSL_SUPPORT
#endif // SSL_SUPPORT

  switch (configuration_p->timeoutHandler->protocol ())
  {
    case NET_TRANSPORTLAYER_TCP:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_TCP_NAME);
      break;
    case NET_TRANSPORTLAYER_UDP:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_UDP_NAME);
      break;
    case NET_TRANSPORTLAYER_SSL:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_SSL_NAME);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid protocol (was: %d), aborting\n"),
                  configuration_p->timeoutHandler->protocol ()));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
  radiobutton_p =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              radio_button_name.c_str ()));
  ACE_ASSERT (radiobutton_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_p), TRUE);

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_PINGINTERVAL_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<ACE_UINT32>::max ());
  unsigned int ping_interval =
    data_p->configuration->protocolConfiguration.pingInterval.msec ();
  gtk_spin_button_set_value (spin_button_p,
    static_cast<gdouble> (ping_interval));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  // step5: initialize updates
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the info view
    guint event_source_id =
      g_timeout_add (NET_UI_GTK_EVENT_RESOLUTION,
                     idle_update_info_display_cb,
                     userData_in);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_CLOSE_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_BUTTON_CLOSEALL_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_PING_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  // step7: (auto-)connect signals/slots
  // *NOTE*: glade_xml_signal_autoconnect does not work reliably
  //glade_xml_signal_autoconnect(userData_out.xml);
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6a: connect default signals
  gulong result =
    g_signal_connect (dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      NULL);
  ACE_ASSERT (result);

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_start_session_cb"));

  struct Test_U_GTK_CBData* data_p =
    static_cast<struct Test_U_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);

//  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_CLOSE_NAME)));
  if (button_p) // client only
    gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                              TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_BUTTON_CLOSEALL_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_PING_NAME)));
  if (button_p) // client only
    gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                              TRUE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_cb"));

  // sanity check(s)
  struct ClientServer_UI_CBData* data_p =
    static_cast<struct ClientServer_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  unsigned int number_of_connections = 0;
  switch (data_p->configuration->protocolConfiguration.transportLayer)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    {
      number_of_connections =
        TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ()->count ();
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      number_of_connections =
        TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ()->count ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                  data_p->configuration->protocolConfiguration.transportLayer));
      return FALSE;
    }
  } // end SWITCH
  bool idle_b = !number_of_connections;

  // sanity check(s)
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // step1: idle ?
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_CLOSE_NAME)));
  if (button_p) // client only
    gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                              !idle_b);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_BUTTON_CLOSEALL_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            !idle_b);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_PING_NAME)));
  if (button_p) // client only
    gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                              !idle_b);

  return G_SOURCE_REMOVE;
}

//////////////////////////////////////////

gboolean
idle_initialize_server_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_server_UI_cb"));

  struct Server_UI_CBData* data_p =
    static_cast<struct Server_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon = gtk_image_new_from_file (path.c_str());
  //  ACE_ASSERT (image_icon);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window_set_title(,
  //                     caption.c_str ());

  //  GtkWidget* about_dialog_p =
  //    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
  //                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_DIALOG_ABOUT_NAME)));
  //  ACE_ASSERT (about_dialog_p);

  // step2: initialize info view
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_FRAME_PROTOCOL_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  GtkRadioButton* radiobutton_p = NULL;
#if defined (SSL_SUPPORT)
#if defined (SSL_SUPPORT)
#else
  radiobutton_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_SSL_NAME)));
    ACE_ASSERT (radiobutton_p);
    gtk_widget_set_sensitive (GTK_WIDGET (radiobutton_p), FALSE);
#endif // SSL_SUPPORT
#endif // SSL_SUPPORT

  std::string radio_button_name;
  switch (data_p->configuration->protocolConfiguration.transportLayer)
  {
    case NET_TRANSPORTLAYER_TCP:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_TCP_NAME);
      break;
    case NET_TRANSPORTLAYER_UDP:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_UDP_NAME);
      break;
    case NET_TRANSPORTLAYER_SSL:
      radio_button_name =
        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_SSL_NAME);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid protocol (was: %d), aborting\n"),
                  data_p->configuration->protocolConfiguration.transportLayer));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
  radiobutton_p =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              radio_button_name.c_str ()));
  ACE_ASSERT (radiobutton_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_p), TRUE);

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_PINGINTERVAL_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<ACE_UINT32>::max ());
  unsigned int ping_interval =
    data_p->configuration->protocolConfiguration.pingInterval.msec ();
  gtk_spin_button_set_value (spin_button_p,
    static_cast<gdouble> (ping_interval));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (""));
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p),
                               &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  // step4: initialize updates
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    guint event_source_id =
      g_timeout_add (NET_UI_GTK_EVENT_RESOLUTION,
                     idle_update_info_display_cb,
                     userData_in);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step5: disable some functions ?
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_BUTTON_CLOSEALL_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            data_p->allowUserRuntimeStatistic);

  // step6: (auto-)connect signals/slots
  // *NOTE*: glade_xml_signal_connect_data does not work reliably
  //glade_xml_signal_autoconnect(userData_out.xml);
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6a: connect default signals
  gulong result =
    g_signal_connect (dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      &dialog_p);
  ACE_ASSERT (result);

  // step8: draw main dialog
  gtk_widget_show_all (dialog_p);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  struct ClientServer_ProgressData* data_p =
    static_cast<struct ClientServer_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->state);

  bool done = false;
  Net_IStreamStatisticHandler_t* istatistic_p = NULL;
  unsigned int number_of_connections = 0;
  Net_StreamStatistic_t statistic_s, statistic_2;
  int result = -1;
  float speed_f = 0.0F, factor_f = 0.0F;
  std::string magnitude_string;
  ACE_TCHAR buffer_a[BUFSIZ];
  ACE_Time_Value interval = ACE_Time_Value::zero;
  unsigned int interval_ms = 0;

  // synch access
//  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->state->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  // step1: done ?
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
      ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  if (gtk_spin_button_get_value_as_int (spin_button_p) == 0)
  {
    gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), false);
    done = true;
    goto continue_;
  } // end IF

  // step2: update progress bar text
  switch (data_p->configuration->protocolConfiguration.transportLayer)
  {
    case NET_TRANSPORTLAYER_TCP:
      istatistic_p =
        TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
      number_of_connections = TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ()->count ();
      break;
    case NET_TRANSPORTLAYER_UDP:
      istatistic_p =
        TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();
      number_of_connections = TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ()->count ();
      break;
    case NET_TRANSPORTLAYER_SSL:
      istatistic_p =
        TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
      number_of_connections = TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ()->count ();
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid protocol (was: %d), aborting\n"),
                  data_p->configuration->protocolConfiguration.transportLayer));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
  istatistic_p->collect (statistic_s);
  interval = statistic_s.timeStamp - statistic_s.previousTimeStamp;
  interval_ms = interval.msec ();
  factor_f =
    1000.0F / (interval_ms ? static_cast<float> (interval_ms) : std::numeric_limits<float>::max ());
  speed_f =
    (((statistic_s.sentBytes + statistic_s.receivedBytes) - statistic_s.previousBytes) *
    factor_f)/* / static_cast<float> (number_of_connections)*/;
  ACE_UNUSED_ARG (number_of_connections);

  ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
  //speed_f = data_p->statistic.streamStatistic.bytesPerSecond;
  magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  if (speed_f)
  {
    if (speed_f >= 1024.0F)
    {
      speed_f /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("kbyte(s)/s");
    } // end IF
    if (speed_f >= 1024.0F)
    {
      speed_f /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("mbyte(s)/s");
    } // end IF
    result = ACE_OS::sprintf (buffer_a, ACE_TEXT ("%.2f %s"),
                              speed_f, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF

continue_:
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : ACE_TEXT_ALWAYS_CHAR (buffer_a)));
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule ?
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
gint
button_connect_clicked_cb (GtkWidget* widget_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_connect_clicked_cb"));

  int result = -1;

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) and SIGTERM (15) instead...
  int signal = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signal = SIGBREAK;
#else
  signal = SIGUSR1;
#endif // ACE_WIN32 || ACE_WIN64
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  return FALSE;
} // button_connect_clicked_cb

gint
button_close_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_close_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) and SIGTERM (15) instead...
  int signal = 0;
#if !defined(ACE_WIN32) && !defined(ACE_WIN64)
  signal = SIGUSR2;
#else
  signal = SIGTERM;
#endif
  if (ACE_OS::raise (signal) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  return FALSE;
} // button_close_clicked_cb

gint
button_close_all_clicked_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_close_all_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

  TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ()->abort ();
  TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ()->abort ();

  return FALSE;
} // button_close_all_clicked_cb

gint
button_ping_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_ping_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  struct Client_UI_CBData* data_p =
    static_cast<struct Client_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  unsigned int number_of_connections = 0;
  switch (data_p->configuration->protocolConfiguration.transportLayer)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    {
      number_of_connections =
        TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ()->count ();
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      number_of_connections =
        TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ()->count ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                  data_p->configuration->protocolConfiguration.transportLayer));
      return FALSE;
    }
  } // end SWITCH
  // sanity check
  if (!number_of_connections)
    return FALSE;

  // grab a (random) connection handler
  int index = 0;
  // *PORTABILITY*: outside glibc, this is not very portable
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  index = ((::random () % number_of_connections) + 1);
#else
  // *NOTE*: use ACE_OS::rand_r() for improved thread safety !
  //results_out.push_back((ACE_OS::rand() % range_in) + 1);
  unsigned int usecs = static_cast<unsigned int> (COMMON_TIME_NOW.usec ());
  index = ((ACE_OS::rand_r (&usecs) % number_of_connections) + 1);
#endif

  Net_IPing* iping_p = NULL;
  Test_U_TCPConnectionManager_t::ICONNECTION_T* connection_base_p = NULL;
  Test_U_UDPConnectionManager_t::ICONNECTION_T* connection_base_2 = NULL;
  switch (data_p->configuration->protocolConfiguration.transportLayer)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    {
      connection_base_p =
        TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ()->operator[] (index - 1);
      iping_p = dynamic_cast<Net_IPing*> (connection_base_p);
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      connection_base_2 =
        TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ()->operator[] (index - 1);
      iping_p = dynamic_cast<Net_IPing*> (connection_base_2);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                  data_p->configuration->protocolConfiguration.transportLayer));
      return FALSE;
    }
  } // end SWITCH
  if (iping_p)
    try {
      iping_p->ping ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_IPing::ping(), aborting\n")));
      return FALSE;
    }

  switch (data_p->configuration->protocolConfiguration.transportLayer)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    {
      if (connection_base_p)
          connection_base_p->decrease ();
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    { 
      if (connection_base_2)
        connection_base_2->decrease ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                  data_p->configuration->protocolConfiguration.transportLayer));
      return FALSE;
    }
  } // end SWITCH

  return FALSE;
} // button_ping_clicked_cb

gint
togglebutton_test_toggled_cb (GtkWidget* widget_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_test_toggled_cb"));

  int result = -1;

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Client_UI_CBData* data_p =
    static_cast<struct Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  struct Client_Configuration* configuration_p =
    static_cast<struct Client_Configuration*> (data_p->configuration);
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->timeoutHandler);
//  Common_UI_GTK_BuildersIterator_t iterator =
//    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // schedule action interval timer ?
  if (configuration_p->signalHandlerConfiguration.actionTimerId == -1)
  {
    ACE_Time_Value interval = ACE_Time_Value::max_time;
    switch (configuration_p->timeoutHandler->mode ())
    {
      case Client_TimeoutHandler::ActionModeType::ACTION_ALTERNATING:
      {
        interval.set ((NET_CLIENT_DEFAULT_TEST_ALTERNATING_INTERVAL / 1000),
                      ((NET_CLIENT_DEFAULT_TEST_ALTERNATING_INTERVAL % 1000) * 1000));
        break;
      }
      case Client_TimeoutHandler::ActionModeType::ACTION_NORMAL:
      {
        interval.set ((NET_CLIENT_DEFAULT_TEST_NORMAL_INTERVAL / 1000),
                      ((NET_CLIENT_DEFAULT_TEST_NORMAL_INTERVAL % 1000) * 1000));
        break;
      }
      case Client_TimeoutHandler::ActionModeType::ACTION_STRESS:
      {
        interval.set ((NET_CLIENT_DEFAULT_TEST_STRESS_INTERVAL / 1000),
                      ((NET_CLIENT_DEFAULT_TEST_STRESS_INTERVAL % 1000) * 1000));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown action mode (was: %d), aborting\n"),
                    configuration_p->timeoutHandler->mode ()));
        return FALSE;
      }
    } // end SWITCH
    configuration_p->signalHandlerConfiguration.actionTimerId =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (configuration_p->timeoutHandler, // event handler handle
                                                                  NULL,                                  // asynchronous completion token
                                                                  ACE_Time_Value::zero,                  // first wakeup time
                                                                  interval);                             // interval
    if (configuration_p->signalHandlerConfiguration.actionTimerId == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule action timer: \"%m\", aborting\n")));
      return FALSE;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("scheduled action timer (id: %d, interval: %#T)\n"),
                configuration_p->signalHandlerConfiguration.actionTimerId,
                &interval));
  } // end IF
  else
  {
    const void* act_p = NULL;
    result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (configuration_p->signalHandlerConfiguration.actionTimerId,
                                                                &act_p);
    if (result <= 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel action timer (ID: %d): \"%m\", continuing\n"),
                  configuration_p->signalHandlerConfiguration.actionTimerId));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("cancelled action timer (id: %d)\n"),
                configuration_p->signalHandlerConfiguration.actionTimerId));
    configuration_p->signalHandlerConfiguration.actionTimerId = -1;
  } // end ELSE

  // toggle button image/label
  bool is_active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget_in));
  gtk_button_set_label (GTK_BUTTON (widget_in),
                        (is_active ? GTK_STOCK_MEDIA_STOP
                                   : GTK_STOCK_MEDIA_PLAY));

  return FALSE;
}

gint
radiobutton_mode_toggled_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::radiobutton_mode_toggled_cb"));

  int result = -1;

  struct Client_UI_CBData* data_p =
    static_cast<struct Client_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  struct Client_Configuration* configuration_p =
    static_cast<struct Client_Configuration*> (data_p->configuration);
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->timeoutHandler);

  // step0: activated ?
  GtkToggleButton* toggle_button_p = GTK_TOGGLE_BUTTON (widget_in);
  ACE_ASSERT (toggle_button_p);
  if (!gtk_toggle_button_get_active (toggle_button_p))
    return FALSE;

  enum Client_TimeoutHandler::ActionModeType mode =
    Client_TimeoutHandler::ACTION_INVALID;
  GtkButton* button_p = GTK_BUTTON (widget_in);
  ACE_ASSERT (button_p);
  const gchar* label_text_p = gtk_button_get_label (button_p);
  result =
    ACE_OS::strcmp (label_text_p,
                   ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_RADIOBUTTON_NORMAL_LABEL));
  if (result == 0)
    mode = Client_TimeoutHandler::ACTION_NORMAL;
  else
  {
    result =
      ACE_OS::strcmp (label_text_p,
                      ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_RADIOBUTTON_ALTERNATING_LABEL));
    if (result == 0)
      mode = Client_TimeoutHandler::ACTION_ALTERNATING;
    else
    {
      result =
        ACE_OS::strcmp (label_text_p,
                        ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_RADIOBUTTON_STRESS_LABEL));
      if (result == 0)
        mode = Client_TimeoutHandler::ACTION_STRESS;
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown mode (was: \"%s\"), aborting\n"),
                    ACE_TEXT (label_text_p)));
        return TRUE; // propagate
      } // end ELSE
    } // end ELSE
  } // end ELSE

  try {
    configuration_p->timeoutHandler->mode (mode);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_Client_TimeoutHandler::mode(), aborting\n")));
    return TRUE; // propagate
  }

  return FALSE;
}

gint
radiobutton_protocol_toggled_cb (GtkWidget* widget_in,
                                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::radiobutton_protocol_toggled_cb"));

//  int result = -1;

  struct ClientServer_UI_CBData* data_p =
    static_cast<struct ClientServer_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->UIState);
  GtkRadioButton* radio_button_p = GTK_RADIO_BUTTON (widget_in);
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
    return FALSE;

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // step0: activated ?
  enum Net_TransportLayerType protocol_e = NET_TRANSPORTLAYER_INVALID;
  radio_button_p =
    GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_TCP_NAME)));
  ACE_ASSERT (radio_button_p);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
    protocol_e = NET_TRANSPORTLAYER_TCP;
  else
  {
    radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_UDP_NAME)));
    ACE_ASSERT (radio_button_p);
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
      protocol_e = NET_TRANSPORTLAYER_UDP;
    else
    {
      radio_button_p =
        GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_RADIOBUTTON_SSL_NAME)));
      ACE_ASSERT (radio_button_p);
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_button_p)))
        protocol_e = NET_TRANSPORTLAYER_SSL;
    } // end ELSE
  } // end ELSE
  data_p->configuration->protocolConfiguration.transportLayer = protocol_e;

  return FALSE;
}

// -----------------------------------------------------------------------------

gint
togglebutton_listen_toggled_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_listen_toggled_cb"));

  ACE_UNUSED_ARG (widget_in);
  // sanity check(s)
  struct Server_UI_CBData* data_p =
    static_cast<struct Server_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  struct Server_Configuration* configuration_p =
    static_cast<struct Server_Configuration*> (data_p->configuration);
  ACE_ASSERT (configuration_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  Test_U_UDPConnectionConfiguration* connection_configuration_p = NULL;
  Net_ConnectionConfigurationsIterator_t iterator_2;
  bool is_active_b =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget_in));
  if (is_active_b)
  {
    ACE_thread_t thread_id = 0;
    switch (data_p->configuration->protocolConfiguration.transportLayer)
    {
      case NET_TRANSPORTLAYER_TCP:
      { ACE_ASSERT (configuration_p->TCPListener);
        configuration_p->TCPListener->start (NULL);
        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      { ACE_ASSERT (configuration_p->UDPConnector);
        iterator_2 =
          data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("UDP"));
        ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
        connection_configuration_p =
          static_cast<Test_U_UDPConnectionConfiguration*> ((*iterator_2).second);
        ACE_ASSERT (connection_configuration_p);
        ACE_HANDLE handle_h =
          configuration_p->UDPConnector->connect (connection_configuration_p->socketConfiguration.listenAddress);
        ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("connected to %s\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (connection_configuration_p->socketConfiguration.listenAddress).c_str ())));
        break;
      }
      case NET_TRANSPORTLAYER_SSL:
      {
#if defined (SSL_SUPPORT)
        ACE_ASSERT (configuration_p->SSLListener);
        configuration_p->SSLListener->start (NULL);
#endif // SSL_SUPPORT
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    data_p->configuration->protocolConfiguration.transportLayer));
        return FALSE;
      }
    } // end SWITCH
    ACE_UNUSED_ARG (thread_id);
  } // end IF
  else
  {
    switch (data_p->configuration->protocolConfiguration.transportLayer)
    {
      case NET_TRANSPORTLAYER_TCP:
      { ACE_ASSERT (configuration_p->TCPListener);
        configuration_p->TCPListener->stop ();
        break;
      }
      case NET_TRANSPORTLAYER_UDP:
      {
        iterator_2 =
          data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("UDP"));
        ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
        connection_configuration_p =
          static_cast<Test_U_UDPConnectionConfiguration*> ((*iterator_2).second);
        ACE_ASSERT (connection_configuration_p);
        typename Test_U_UDPConnectionManager_t::INTERFACE_T* connection_manager_p =
          TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();
        ACE_ASSERT (connection_manager_p);
        typename Test_U_UDPConnectionManager_t::ICONNECTION_T* connection_p =
          connection_manager_p->get (connection_configuration_p->socketConfiguration.listenAddress,
                                     false);
        if (connection_p)
        {
          connection_p->abort ();
          connection_p->decrease (); connection_p = NULL;
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("disconnected from %s\n"),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (connection_configuration_p->socketConfiguration.listenAddress).c_str ())));
        } // end IF
        break;
      }
      case NET_TRANSPORTLAYER_SSL:
      {
#if defined (SSL_SUPPORT)
        ACE_ASSERT (configuration_p->SSLListener);
        configuration_p->SSLListener->stop ();
#endif // SSL_SUPPORT
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown transport layer (was: %d), returning\n"),
                    data_p->configuration->protocolConfiguration.transportLayer));
        return FALSE;
      }
    } // end SWITCH
  } // end IF

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_FRAME_PROTOCOL_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
                            !is_active_b);

  return FALSE;
} // togglebutton_listen_toggled_cb

gint
button_report_clicked_cb (GtkWidget* widget_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_report_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
  int signal = 0;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  signal = SIGUSR1;
#else
  signal = SIGBREAK;
#endif
  if (ACE_OS::raise (signal) == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  return FALSE;
}

// -----------------------------------------------------------------------------

void
spinbutton_connections_value_changed_client_cb (GtkSpinButton* spinButton_in,
                                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::spinbutton_connections_value_changed_client_cb"));

  struct Client_UI_CBData* data_p =
    static_cast<struct Client_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (spinButton_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  gint value = gtk_spin_button_get_value_as_int (spinButton_in);
  if (value == 0)
    data_p->progressData.eventSourceId = 0;

  // step1: update buttons
  GtkWidget* widget_p = NULL;
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  if (data_p->configuration)
  {
    widget_p =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_CLOSE_NAME)));
    ACE_ASSERT (widget_p);
    gtk_widget_set_sensitive (widget_p, (value > 0));
    widget_p =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_GTK_BUTTON_PING_NAME)));
    ACE_ASSERT (widget_p);
    gtk_widget_set_sensitive (widget_p, (value > 0));
  } // end IF

  // step2: start progress reporting ?
  if (!value || data_p->progressData.eventSourceId)
    goto continue_;

  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), true);

//  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock);
    data_p->progressData.eventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                // _LOW doesn't work (on Win32)
                          NET_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                          idle_update_progress_cb,
                          &data_p->progressData,
                          NULL);
    if (data_p->progressData.eventSourceId > 0)
      data_p->UIState->eventSourceIds.insert (data_p->progressData.eventSourceId);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", continuing\n")));
//  } // end lock scope

continue_:
  widget_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_BUTTON_CLOSEALL_NAME)));
  ACE_ASSERT (widget_p);
  gtk_widget_set_sensitive (widget_p, (value > 0));

//  // step3: update information display
//  if (value == 0)
//  {
//    GtkSpinButton* spin_button_p =
//      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                               ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
//    ACE_ASSERT (spin_button_p);
//    gtk_spin_button_set_value (spin_button_p, 0.0);
//    spin_button_p =
//      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                               ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
//    ACE_ASSERT (spin_button_p);
//    gtk_spin_button_set_value (spin_button_p, 0.0);
//  } // end IF
} // spinbutton_connections_value_changed_client_cb
void
spinbutton_connections_value_changed_server_cb (GtkSpinButton* spinButton_in,
                                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::spinbutton_connections_value_changed_server_cb"));

  struct Server_UI_CBData* data_p =
    static_cast<struct Server_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (spinButton_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  gint value = gtk_spin_button_get_value_as_int (spinButton_in);
  if (value == 0)
    data_p->progressData.eventSourceId = 0;

  // step1: update buttons
  GtkWidget* widget_p = NULL;
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  // step2: start progress reporting ?
  if ((value != 1) || data_p->progressData.eventSourceId)
    goto continue_;

  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), true);

//  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock);
    data_p->progressData.eventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                // _LOW doesn't work (on Win32)
                          NET_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                          idle_update_progress_cb,
                          &data_p->progressData,
                          NULL);
    if (data_p->progressData.eventSourceId > 0)
      data_p->UIState->eventSourceIds.insert (data_p->progressData.eventSourceId);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", continuing\n")));
//  } // end lock scope

continue_:
  widget_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_BUTTON_CLOSEALL_NAME)));
  ACE_ASSERT (widget_p);
  gtk_widget_set_sensitive (widget_p, (value > 0));

  // step3: update information display
  if (value == 0)
  {
    GtkSpinButton* spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMSESSIONMESSAGES_NAME)));
    ACE_ASSERT (spin_button_p);
    gtk_spin_button_set_value (spin_button_p, 0.0);
    spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME)));
    ACE_ASSERT (spin_button_p);
    gtk_spin_button_set_value (spin_button_p, 0.0);
  } // end IF
} // spinbutton_connections_value_changed_server_cb

void
spinbutton_ping_interval_value_changed_client_cb (GtkSpinButton* spinButton_in,
                                                  gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::spinbutton_ping_interval_value_changed_client_cb"));

  struct Client_UI_CBData* data_p =
    static_cast<struct Client_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (spinButton_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  gint value = gtk_spin_button_get_value_as_int (spinButton_in);
  data_p->configuration->protocolConfiguration.pingInterval.set_msec (value);
} // spinbutton_ping_interval_value_changed_client_cb
void
spinbutton_ping_interval_value_changed_server_cb (GtkSpinButton* spinButton_in,
                                                  gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::spinbutton_ping_interval_value_changed_server_cb"));

  struct Server_UI_CBData* data_p =
    static_cast<struct Server_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (spinButton_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  gint value = gtk_spin_button_get_value_as_int (spinButton_in);
  data_p->configuration->protocolConfiguration.pingInterval.set_msec (value);
} // spinbutton_ping_interval_value_changed_server_cb

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_U_GTK_CBData* data_p =
    static_cast<struct Test_U_GTK_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (NET_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (NET_UI_GTK_DIALOG_ABOUT_NAME)));
    return TRUE; // propagate
  } // end IF

  // run dialog
  gint result = gtk_dialog_run (about_dialog);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  gtk_widget_hide (GTK_WIDGET (about_dialog));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_U_GTK_CBData* data_p =
      static_cast<struct Test_U_GTK_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  int result = -1;

  // step1: remove event sources
  { ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->UIState->lock);
    for (Common_UI_GTK_EventSourceIdsIterator_t iterator = data_p->UIState->eventSourceIds.begin ();
         iterator != data_p->UIState->eventSourceIds.end ();
         iterator++)
      if (!g_source_remove (*iterator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    *iterator));
    data_p->UIState->eventSourceIds.clear ();
  } // end lock scope

  // step2: initiate shutdown sequence
  result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true);

  return FALSE;
} // button_quit_clicked_cb

#ifdef __cplusplus
}
#endif /* __cplusplus */
