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

#include "test_u_gtk_callbacks.h"

#include <sstream>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <limits>

#include "iphlpapi.h"
#else
#include "netinet/ether.h"
#include "ifaddrs.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_file_tools.h"
#include "common_os_tools.h"

#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_macros.h"

#include "net_client_common_tools.h"

#include "http_defines.h"
#include "http_tools.h"

#include "ws_codes.h"
#include "ws_defines.h"

#include "test_u_common.h"
#include "test_u_session_message.h"
#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_connection_stream.h"
#include "test_u_defines.h"
#include "test_u_eventhandler.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

#include "test_u_ws_client_common.h"
#include "test_u_ws_client_defines.h"

// initialize statics
static bool un_toggling_listen = false;

bool
load_network_interfaces (GtkListStore* listStore_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_network_interfaces"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _IP_ADAPTER_INFO* ip_adapter_info_p = NULL;
  ULONG buffer_length = 0;
  ULONG result_2 = GetAdaptersInfo (ip_adapter_info_p, &buffer_length);
  if (result_2 != ERROR_BUFFER_OVERFLOW)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (buffer_length);
  ip_adapter_info_p =
    static_cast<struct _IP_ADAPTER_INFO*> (ACE_MALLOC_FUNC (buffer_length));
  if (!ip_adapter_info_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return false;
  } // end IF

  result_2 = GetAdaptersInfo (ip_adapter_info_p,
                              &buffer_length);
  if (result_2 != NO_ERROR)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetAdaptersInfo(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

    // clean up
    ACE_FREE_FUNC (ip_adapter_info_p);

    return false;
  } // end IF

  struct _IP_ADAPTER_INFO* ip_adapter_info_2 = ip_adapter_info_p;
  gchar* string_p = NULL;
  do
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found network interface: \"%s\": \"%s\"...\n"),
    //            ACE_TEXT (ip_adapter_info_2->Description),
    //            ACE_TEXT (Net_Common_Tools::MACAddress2String (ip_adapter_info_2->Address).c_str ())));
    string_p =
      Common_UI_GTK_Tools::localeToUTF8 (ip_adapter_info_2->Description);
    ACE_ASSERT (string_p);
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, string_p,
                        1, ip_adapter_info_2->AdapterName,
                        -1);
    g_free (string_p);
    string_p = NULL;

    ip_adapter_info_2 = ip_adapter_info_2->Next;
  } while (ip_adapter_info_2);

  // clean up
  ACE_FREE_FUNC (ip_adapter_info_p);

  result = true;
#else
#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  int result_2 = ::getifaddrs (&ifaddrs_p);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
    if (ifaddrs_2->ifa_addr->sa_family != AF_INET)
      continue;

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ifaddrs_2->ifa_name,
                        -1);
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);

  result = true;
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */
#endif

  return result;
}

void
stop_progress_reporting (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::stop_progress_reporting"));

  // sanity check(s)
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  gtk_progress_bar_set_show_text (progress_bar_p, FALSE);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), FALSE);

  ACE_ASSERT (data_p->progressData.eventSourceId > 0);
  if (!g_source_remove (data_p->progressData.eventSourceId))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_source_remove(%u), continuing\n")));
  data_p->progressData.eventSourceId = 0;

}

void
start_progress_reporting (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::start_progress_reporting"));

  // sanity check(s)
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p), TRUE);
  gtk_progress_bar_set_show_text (progress_bar_p, TRUE);

  ACE_ASSERT (!data_p->progressData.eventSourceId);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock);
    data_p->progressData.eventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                          // _LOW doesn't work (on Win32)
                          TEST_U_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                          idle_update_progress_cb,
                          &data_p->progressData,
                          NULL);
    if (data_p->progressData.eventSourceId > 0)
      data_p->UIState->eventSourceIds.insert (data_p->progressData.eventSourceId);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_target_progress_cb): \"%m\", returning\n")));
      return;
    } // end IF
  } // end lock scope
}

/////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT32>::max ()));
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             static_cast<gdouble> (std::numeric_limits<ACE_UINT64>::max ()));

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_LISTSTORE_INTERFACE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        0, GTK_SORT_ASCENDING);
  if (!load_network_interfaces (list_store_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_network_interfaces(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_INTERFACE_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  //"text", 1,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);

  GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ENTRY_URL_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      data_p->URL.c_str ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());

  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_PROACTOR));

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progressbar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progressbar_p,
                                   1.0 / static_cast<double> (width));

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
    return G_SOURCE_REMOVE;
  } // end IF
  // apply font
  GtkRcStyle* rc_style_p = gtk_rc_style_new ();
  if (!rc_style_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_rc_style_new(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  rc_style_p->font_desc = font_description_p;
  GdkColor base_colour, text_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags> (GTK_RC_BASE |
                             GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  // step5: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the log view
    event_source_id = g_timeout_add_seconds (1,
                                             idle_update_log_display_cb,
                                             data_p);
    if (event_source_id > 0)
      data_p->UIState->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id = g_timeout_add (TEST_U_UI_GTKEVENT_RESOLUTION,
                                     idle_update_info_display_cb,
                                     data_p);
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
  //GtkAction* action_p =
  //  GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
  //                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_REQUEST_NAME)));
  //ACE_ASSERT (action_p);
  //gtk_action_set_sensitive (action_p, FALSE);
//  action_p =
//    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_RELEASE_NAME)));
//  ACE_ASSERT (action_p);
//  gtk_action_set_sensitive (action_p, FALSE);

  // step6: (auto-)connect signals/slots
  // *NOTE*: glade_xml_signal_autoconnect does not work reliably
  //glade_xml_signal_autoconnect(userData_out.xml);

  // step6a: connect default signals
  gulong result_2 =
    g_signal_connect (dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      NULL);
  ACE_ASSERT (result_2);

  // step6b: connect custom signals
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step10: select default network interface (if any)
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_LISTSTORE_INTERFACE_NAME)));
  ACE_ASSERT (list_store_p);
  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_INTERFACE_NAME)));
  ACE_ASSERT (combo_box_p);
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (InlineIsEqualGUID (NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier, GUID_NULL))
#else
    if (NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier.empty ())
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    if (NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier.empty ())
#endif // ACE_WIN32 || ACE_WIN64
      gtk_combo_box_set_active (combo_box_p, 0);
    else
    {
      GtkTreeModel* tree_model_p = gtk_combo_box_get_model (combo_box_p);
      ACE_ASSERT (tree_model_p);
      GtkTreeIter tree_iterator;
      if (!gtk_tree_model_get_iter_first (tree_model_p, &tree_iterator))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to gtk_tree_model_get_iter_first(): \"%m\", aborting\n")));
        return G_SOURCE_REMOVE;
      } // end IF
#if GTK_CHECK_VERSION (2,30,0)
      GValue value = G_VALUE_INIT;
#else
      GValue value;
      ACE_OS::memset (&value, 0, sizeof (GValue));
      g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      std::string interface_identifier =
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
        Common_OS_Tools::GUIDToString (NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier);
#else
        NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
#endif // ACE_WIN32 || ACE_WIN64
      do
      {
        gtk_tree_model_get_value (tree_model_p,
                                  &tree_iterator,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  1, &value);
#else
                                  0, &value);
#endif // ACE_WIN32 || ACE_WIN64
        ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if (ACE_OS::strcmp (interface_identifier.c_str (),
#else
        if (ACE_OS::strcmp (NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier.c_str (),
#endif // ACE_WIN32 || ACE_WIN64
                            g_value_get_string (&value)) == 0)
        {
          // clean up
          g_value_unset (&value);

          break;
        } // end IF

        g_value_unset (&value);
      } while (gtk_tree_model_iter_next (tree_model_p, &tree_iterator));
      gtk_combo_box_set_active_iter (combo_box_p, &tree_iterator);
    } // end ELSE
  } // end IF
  else
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_start_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_start_session_cb"));

  // sanity check(s)
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_CONNECT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_PING_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_CLOSE_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);

  start_progress_reporting (userData_in);

  return G_SOURCE_REMOVE;
}

gboolean
idle_message_received_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_message_received_cb"));

  // sanity check(s)
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkStatusbar* statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);
  gtk_statusbar_push (statusbar_p,
                      0,
                      data_p->message.c_str ());

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_cb"));

  // sanity check(s)
  WebSocket_Client_UI_CBData* data_p =
    static_cast<WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_CONNECT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_PING_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_CLOSE_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);

  stop_progress_reporting (userData_in);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_ProgressData* data_p =
    static_cast<struct WebSocket_Client_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p->state);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->state->builders.end ());
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  char buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (char[BUFSIZ]));
  int result = -1;
  float speed = 0.0f;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_CONTINUE);
    speed = data_p->statistic.bytesPerSecond;
  } // end lock scope
  std::string magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  if (speed)
  {
    if (speed >= 1024.0f)
    {
      speed /= 1024.0f;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("kbyte(s)/s");
    } // end IF
    if (speed >= 1024.0f)
    {
      speed /= 1024.0f;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("mbyte(s)/s");
    } // end IF
    result = ACE_OS::sprintf (buffer_a,
                              ACE_TEXT_ALWAYS_CHAR ("%.2f %s"),
                              speed,
                              magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             buffer_a);
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  WebSocket_Client_ConnectionManager_t* connection_manager_p =
    WEBSOCKET_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

  WebSocket_Client_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  if (data_p->handle != ACE_INVALID_HANDLE)
  {
    iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
    if (iconnection_p)
    {
      iconnection_p->abort ();
      iconnection_p->decrease (); iconnection_p = NULL;
    } // end ELSE
    data_p->handle = ACE_INVALID_HANDLE;
  } // end IF

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
      static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (data_p->UIState->eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.transferred));

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_STARTED:
        {
          //spin_button_p =
          //    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
          //                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          //ACE_ASSERT (spin_button_p);
          //gtk_spin_button_set_value (spin_button_p, 0.0);
          //spin_button_p =
          //    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
          //                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          //ACE_ASSERT (spin_button_p);
          //gtk_spin_button_set_value (spin_button_p, 0.0);
          //spin_button_p =
          //    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
          //                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATA_NAME)));
          //ACE_ASSERT (spin_button_p);
          //gtk_spin_button_set_value (spin_button_p, 0.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STOPPED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_CONNECT:
        case COMMON_UI_EVENT_DISCONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STEP:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
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

gboolean
idle_update_log_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  // sanity check(s)
  WebSocket_Client_UI_CBData* data_p =
    static_cast<WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  GtkTextIter text_iterator;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iterator);

  gchar* string_p = NULL;

  // sanity check(s)
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->logQueueLock, G_SOURCE_REMOVE);
    if (data_p->UIState->logQueue.empty ())
      return G_SOURCE_CONTINUE;

    // step1: convert text
    for (Common_Log_MessageQueueConstIterator_t iterator_2 = data_p->UIState->logQueue.begin ();
         iterator_2 != data_p->UIState->logQueue.end ();
         iterator_2++)
    {
      string_p = Common_UI_GTK_Tools::localeToUTF8 (*iterator_2);
      if (!string_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"), aborting\n"),
                    ACE_TEXT ((*iterator_2).c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF

      // step2: display text
      gtk_text_buffer_insert (buffer_p,
                              &text_iterator,
                              string_p,
                              -1);

      g_free (string_p); string_p = NULL;
    } // end FOR

    data_p->UIState->logQueue.clear ();
  } // end lock scope

  // step3: scroll the view accordingly
//  // move the iterator to the beginning of line, so it doesn't scroll
//  // in horizontal direction
//  gtk_text_iter_set_line_offset (&text_iterator, 0);

//  // ...and place the mark at iter. The mark will stay there after insertion
//  // because it has "right" gravity
//  GtkTextMark* text_mark_p =
//      gtk_text_buffer_get_mark (buffer_p,
//                                ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SCROLLMARK_NAME));
////  gtk_text_buffer_move_mark (buffer_p,
////                             text_mark_p,
////                             &text_iterator);

//  // scroll the mark onscreen
//  gtk_text_view_scroll_mark_onscreen (view_p,
//                                      text_mark_p);
  //GtkAdjustment* adjustment_p =
  //    GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ADJUSTMENT_NAME)));
  //ACE_ASSERT (adjustment_p);
  //gtk_adjustment_set_value (adjustment_p,
  //                          adjustment_p->upper - adjustment_p->page_size));

  return G_SOURCE_CONTINUE;
}

/////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
action_connect_activate_cb (GtkAction* action_in,
                            gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_connect_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->streamConfiguration.configuration_->messageAllocator);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  WebSocket_Client_StreamConfiguration_t::ITERATOR_T iterator_3 =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ENTRY_URL_NAME)));
  ACE_ASSERT (entry_p);
  const gchar* text_p = gtk_entry_get_text (entry_p);
  ACE_ASSERT (text_p);
  data_p->URL = text_p;

  (*iterator_3).second.second->parserConfiguration->messageQueue = NULL;
  (*iterator_3).second.second->URL = data_p->URL;

  WebSocket_Client_Connector_t connector;
#if defined (SSL_SUPPORT)
  WebSocket_Client_SSLConnector_t ssl_connector;
#endif // SSL_SUPPORT
  WebSocket_Client_AsynchConnector_t asynch_connector;
  struct Net_UserData user_data_s;
  ACE_INET_Addr address;
  std::string hostname_string, URI_string, key_string;
  bool use_SSL = false;
  if (!HTTP_Tools::parseURL (data_p->URL,
                             address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT (data_p->URL.c_str ())));
    return;
  } // end IF

  std::string::size_type position =
    hostname_string.find_last_of (':', std::string::npos);
  if (position == std::string::npos)
    address.set_port_number (use_SSL ? HTTPS_DEFAULT_SERVER_PORT : HTTP_DEFAULT_SERVER_PORT,
                             1);

  static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.address =
    address;
  static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.hostname =
    hostname_string;

  if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      data_p->handle = Net_Client_Common_Tools::connect (ssl_connector,
                                                         *static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second),
                                                         user_data_s,
                                                         static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.address,
                                                         false, // wait ?
                                                         true,  // peer address ?
                                                         0);    // #retries
    else
#endif // SSL_SUPPORT
      data_p->handle = Net_Client_Common_Tools::connect (connector,
                                                         *static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second),
                                                         user_data_s,
                                                         static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.address,
                                                         false, // wait ?
                                                         true,  // peer address ?
                                                         0);    // #retries
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    data_p->handle =
      Net_Client_Common_Tools::connect (asynch_connector,
                                        *static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second),
                                        user_data_s,
                                        static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.address,
                                        false, // wait ?
                                        true,  // peer address ?
                                        0);    // #retries
  } // end ELSE
  if (data_p->handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
    return;
  } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("0x%@: opened socket to %s\n"),
//              data_p->handle,
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%d: opened socket to %s\n"),
//              data_p->handle,
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<WebSocket_Client_ConnectionConfiguration*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
//#endif // ACE_WIN32 || ACE_WIN64

  struct WebSocket_Client_MessageData* record_p = NULL;
  ACE_NEW_NORETURN (record_p,
                    struct WebSocket_Client_MessageData ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF
  record_p->method = static_cast<HTTP_Method_t> (HTTP_Codes::HTTP_METHOD_GET);
  record_p->URI = URI_string;
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                            hostname_string));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_UPGRADE_STRING),
                                            ACE_TEXT_ALWAYS_CHAR ("websocket")));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONNECTION_STRING),
                                            ACE_TEXT_ALWAYS_CHAR ("Upgrade")));
  key_string = WebSocket_Tools::generateSecHeaderKey ();
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (WEBSOCKET_PRT_HEADER_SEC_WEBSOCKET_KEY_STRING),
                                            key_string));
  //record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_ORIGIN_STRING),
  //                                          ACE_TEXT_ALWAYS_CHAR ("")));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (WEBSOCKET_PRT_HEADER_SEC_WEBSOCKET_VERSION_STRING),
                                            ACE_TEXT_ALWAYS_CHAR ("13")));

  WebSocket_Client_MessageData_t* message_data_container_p = NULL;
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  ACE_NEW_NORETURN (message_data_container_p,
                    WebSocket_Client_MessageData_t (record_p,
                                                    true)); // delete record in dtor ?
  if (!message_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    delete record_p;
    return;
  } // end IF

  ACE_ASSERT ((*iterator_2).second->allocatorConfiguration);
  ACE_UINT32 pdu_size_i =
    (*iterator_2).second->allocatorConfiguration->defaultBufferSize;// +
//    (*iterator_2).second->allocatorConfiguration->paddingBytes;
  Test_U_Message* message_p = NULL;
allocate:
  message_p =
    static_cast<Test_U_Message*> ((*iterator_2).second->messageAllocator->malloc (pdu_size_i));
  // keep retrying ?
  if (!message_p && !(*iterator_2).second->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));
    message_data_container_p->decrease (); message_data_container_p = NULL;
    return;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  message_p->initialize (message_data_container_p,
                         1,//message_p->sessionId (),
                         NULL);
  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  ACE_Message_Block* message_block_p = message_p;

  WebSocket_Client_ConnectionManager_t* connection_manager_p =
    WEBSOCKET_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  WebSocket_Client_ConnectionManager_t::ICONNECTION_T* iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iconnection_p);
  WebSocket_Client_IStreamConnection_t* istream_connection_p =
    dynamic_cast<WebSocket_Client_IStreamConnection_t*> (iconnection_p);
  ACE_ASSERT (istream_connection_p);
  istream_connection_p->send (message_block_p);
  iconnection_p->decrease (); iconnection_p = NULL;
} // action_connect_activate_cb

void
action_ping_activate_cb (GtkAction* action_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_ping_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->streamConfiguration.configuration_->messageAllocator);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());

  struct WebSocket_Client_MessageData* record_p = NULL;
  ACE_NEW_NORETURN (record_p,
                    struct WebSocket_Client_MessageData ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF
  record_p->opcode = WebSocket_Codes::OPCODE_PING;
  ACE_NEW_NORETURN (record_p->payload.string,
                    char[BUFSIZ]);
  ACE_OS::strcpy (record_p->payload.string,
                  ACE_TEXT_ALWAYS_CHAR ("PingPong"));
  record_p->payloadSize = ACE_OS::strlen (record_p->payload.string);

  WebSocket_Client_MessageData_t* message_data_container_p = NULL;
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  ACE_NEW_NORETURN (message_data_container_p,
                    WebSocket_Client_MessageData_t (record_p,
                                                    true)); // delete record in dtor ?
  if (!message_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    delete record_p;
    return;
  } // end IF

  ACE_ASSERT ((*iterator_2).second->allocatorConfiguration);
  ACE_UINT32 pdu_size_i =
    (*iterator_2).second->allocatorConfiguration->defaultBufferSize;// +
//    (*iterator_2).second->allocatorConfiguration->paddingBytes;
  Test_U_Message* message_p = NULL;
allocate:
  message_p =
    static_cast<Test_U_Message*> ((*iterator_2).second->messageAllocator->malloc (pdu_size_i));
  // keep retrying ?
  if (!message_p && !(*iterator_2).second->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));
    message_data_container_p->decrease (); message_data_container_p = NULL;
    return;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  message_p->initialize (message_data_container_p,
                         1,//message_p->sessionId (),
                         NULL);
  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  ACE_Message_Block* message_block_p = message_p;

  WebSocket_Client_ConnectionManager_t* connection_manager_p =
    WEBSOCKET_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  WebSocket_Client_ConnectionManager_t::ICONNECTION_T* iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iconnection_p);
  WebSocket_Client_IStreamConnection_t* istream_connection_p =
    dynamic_cast<WebSocket_Client_IStreamConnection_t*> (iconnection_p);
  ACE_ASSERT (istream_connection_p);
  istream_connection_p->send (message_block_p);
  iconnection_p->decrease (); iconnection_p = NULL;
} // action_ping_activate_cb

void
action_close_activate_cb (GtkAction* action_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_close_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->streamConfiguration.configuration_->messageAllocator);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());

  struct WebSocket_Client_MessageData* record_p = NULL;
  ACE_NEW_NORETURN (record_p,
                    struct WebSocket_Client_MessageData ());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF
  record_p->opcode = WebSocket_Codes::OPCODE_CLOSE;
  ACE_NEW_NORETURN (record_p->payload.blob,
                    uint8_t[BUFSIZ]);
  ACE_ASSERT (record_p->payload.blob);
  uint16_t value_i = WebSocket_Codes::STATUS_GOING_AWAY;
  if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
    value_i = ACE_SWAP_WORD (value_i);
  *reinterpret_cast<uint16_t*> (record_p->payload.blob) = value_i;
  ACE_OS::strcpy (reinterpret_cast<char*> (record_p->payload.blob + 2),
                  ACE_TEXT_ALWAYS_CHAR ("going away"));
  record_p->payloadSize =
    2 + ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR ("going away"));

  WebSocket_Client_MessageData_t* message_data_container_p = NULL;
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  ACE_NEW_NORETURN (message_data_container_p,
                    WebSocket_Client_MessageData_t (record_p,
                                                    true)); // delete record in dtor ?
  if (!message_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    delete record_p;
    return;
  } // end IF

  ACE_ASSERT ((*iterator_2).second->allocatorConfiguration);
  ACE_UINT32 pdu_size_i =
    (*iterator_2).second->allocatorConfiguration->defaultBufferSize;// +
//    (*iterator_2).second->allocatorConfiguration->paddingBytes;
  Test_U_Message* message_p = NULL;
allocate:
  message_p =
    static_cast<Test_U_Message*> ((*iterator_2).second->messageAllocator->malloc (pdu_size_i));
  // keep retrying ?
  if (!message_p && !(*iterator_2).second->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));
    message_data_container_p->decrease (); message_data_container_p = NULL;
    return;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  message_p->initialize (message_data_container_p,
                         1,//message_p->sessionId (),
                         NULL);
  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  ACE_Message_Block* message_block_p = message_p;

  WebSocket_Client_ConnectionManager_t* connection_manager_p =
    WEBSOCKET_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  WebSocket_Client_ConnectionManager_t::ICONNECTION_T* iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iconnection_p);
  WebSocket_Client_IStreamConnection_t* istream_connection_p =
    dynamic_cast<WebSocket_Client_IStreamConnection_t*> (iconnection_p);
  ACE_ASSERT (istream_connection_p);
  istream_connection_p->send (message_block_p);
  iconnection_p->decrease (); iconnection_p = NULL;
} // action_close_activate_cb

void
combobox_interface_changed_cb (GtkComboBox* comboBox_in,
                               gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_interface_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->configuration);
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());

  GtkTreeIter tree_iterator;
  if (!gtk_combo_box_get_active_iter (comboBox_in,
                                      &tree_iterator))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(): \"%m\", returning\n")));
    return;
  } // end IF
  GtkTreeModel* tree_model_p = gtk_combo_box_get_model (comboBox_in);
  ACE_ASSERT (tree_model_p);
  GValue value = {0,};
  gtk_tree_model_get_value (tree_model_p,
                            &tree_iterator,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
#else
                            0, &value);
#endif
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  NET_CONFIGURATION_TCP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    Common_OS_Tools::StringToGUID (g_value_get_string (&value));
#else
    g_value_get_string (&value);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
#else
    g_value_get_string (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
}

// -----------------------------------------------------------------------------

void
button_clear_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
      static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p =
//    gtk_text_buffer_new (NULL); // text tag table --> create new
    gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  gtk_text_buffer_set_text (buffer_p,
                            ACE_TEXT_ALWAYS_CHAR (""), 0);
} // button_clear_clicked_cb

void
button_about_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog);
  //gtk_widget_show (GTK_WIDGET (about_dialog));

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
} // button_about_clicked_cb

void
button_quit_clicked_cb (GtkButton* button_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  int result = -1;

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct WebSocket_Client_UI_CBData* data_p =
    static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->UIState);

  // step1: remove event sources
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, data_p->UIState->lock);
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int signal = SIGINT;
#else
  int signal = SIGQUIT;
#endif // ACE_WIN32 || ACE_WIN64
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?
} // button_quit_clicked_cb

void
textview_size_allocate_cb (GtkWidget* widget_in,
                           GdkRectangle* rectangle_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::textview_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (rectangle_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct WebSocket_Client_UI_CBData* data_p =
      static_cast<struct WebSocket_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT(iterator != data_p->UIState->builders.end ());

  GtkScrolledWindow* scrolled_window_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SCROLLEDWINDOW_NAME)));
  ACE_ASSERT (scrolled_window_p);
  GtkAdjustment* adjustment_p =
    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p) - gtk_adjustment_get_page_size (adjustment_p));
} // textview_size_allocate_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
