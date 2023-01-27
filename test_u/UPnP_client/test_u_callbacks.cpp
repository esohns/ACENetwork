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
#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_macros.h"

#include "net_client_common_tools.h"

#include "http_defines.h"

#include "ssdp_codes.h"
#include "ssdp_defines.h"

#include "test_u_common.h"
#include "test_u_session_message.h"
#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_connection_stream.h"
#include "test_u_defines.h"
#include "test_u_eventhandler.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

#include "test_u_upnp_client_common.h"
#include "test_u_upnp_client_defines.h"

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
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

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
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
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

  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);

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
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);

  GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ENTRY_SERVER_ADDRESS_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      Net_Common_Tools::IPAddressToString (data_p->gatewayAddress, true, false).c_str ());
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SERVER_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("Out"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.peerAddress.get_port_number ()));

  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_PROACTOR));

  entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ENTRY_EXTERNAL_ADDRESS_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      Net_Common_Tools::IPAddressToString (data_p->externalAddress, true, false).c_str ());
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_EXTERNAL_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (HTTP_DEFAULT_SERVER_PORT));

  entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ENTRY_INTERNAL_ADDRESS_NAME)));
  ACE_ASSERT (entry_p);
  gtk_entry_set_text (entry_p,
                      Net_Common_Tools::IPAddressToString (data_p->interfaceAddress, true, false).c_str ());
  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_INTERNAL_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (HTTP_DEFAULT_SERVER_PORT));

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
#if GTK_CHECK_VERSION(2,30,0)
      GValue value = G_VALUE_INIT;
#else
      GValue value;
      ACE_OS::memset (&value, 0, sizeof (GValue));
      g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      std::string interface_identifier =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
        Common_Tools::GUIDToString (NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier);
#else
        NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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

  // step7: start listening
  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEACTION_LISTEN_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_activate (GTK_ACTION (toggle_action_p));

  return G_SOURCE_REMOVE;
}

gboolean
idle_discovery_complete_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_discovery_complete_cb"));

  // sanity check(s)
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_DISCOVERY_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_service_description_complete_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_service_description_complete_cb"));

  // sanity check(s)
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_EXTERNAL_ADDRESS_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_MAP_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_PRESENTATION_URL_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, TRUE);

  stop_progress_reporting (userData_in);

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_session_success_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_success_cb"));

  // sanity check(s)
  UPnP_Client_UI_CBData* data_p =
      static_cast<UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  ACE_ASSERT (data_p->eventHandler);

  stop_progress_reporting (userData_in);

  switch (data_p->eventHandler->state_)
  {
    case EVENT_HANDLER_STATE_EXTERNAL_ADDRESS_CONTROL:
    {
      GtkStatusbar* statusbar_p =
        GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_STATUSBAR_NAME)));
      ACE_ASSERT (statusbar_p);
      gtk_statusbar_push (statusbar_p, 0,
                          Net_Common_Tools::IPAddressToString (data_p->externalAddress, true, false).c_str ());

      GtkDialog* dialog_p = 
        GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_MAIN_NAME)));
      ACE_ASSERT (dialog_p);
      GtkWidget* widget_p =
        gtk_message_dialog_new (GTK_WINDOW (dialog_p),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_INFO,
                                GTK_BUTTONS_CLOSE,
                                ACE_TEXT_ALWAYS_CHAR ("Success: \"%s\""),
                                Net_Common_Tools::IPAddressToString (data_p->externalAddress, true, false).c_str ());
      gtk_dialog_run (GTK_DIALOG (widget_p));
      gtk_widget_destroy (widget_p); widget_p = NULL;

      break;
    }
    case EVENT_HANDLER_STATE_MAP_CONTROL:
    {
      GtkDialog* dialog_p =
        GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_MAIN_NAME)));
      ACE_ASSERT (dialog_p);
      GtkWidget* widget_p =
        gtk_message_dialog_new (GTK_WINDOW (dialog_p),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_INFO,
                                GTK_BUTTONS_CLOSE,
                                ACE_TEXT_ALWAYS_CHAR ("Success"));
      gtk_dialog_run (GTK_DIALOG (widget_p));
      gtk_widget_destroy (widget_p); widget_p = NULL;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown state (was: %d), continuing\n"),
                  data_p->eventHandler->state_));
      break;
    }
  } // end SWITCH

  return G_SOURCE_REMOVE;
}

gboolean
idle_end_session_error_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_error_cb"));

  // sanity check(s)
  UPnP_Client_UI_CBData* data_p =
      static_cast<UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  ACE_ASSERT (data_p->eventHandler);

  stop_progress_reporting (userData_in);

  GtkDialog* dialog_p = 
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  GtkWidget* widget_p =
    gtk_message_dialog_new (GTK_WINDOW (dialog_p),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE,
                            ACE_TEXT_ALWAYS_CHAR ("action failed"));
  gtk_dialog_run (GTK_DIALOG (widget_p));
  gtk_widget_destroy (widget_p); widget_p = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_ProgressData* data_p =
      static_cast<struct UPnP_Client_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p->state);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->state->builders.end ());
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
  int result = -1;
  float speed = 0.0F;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_CONTINUE);
    speed = data_p->statistic.streamStatistic.bytesPerSecond;
  } // end lock scope
  std::string magnitude_string = ACE_TEXT_ALWAYS_CHAR ("byte(s)/s");
  if (speed)
  {
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("kbyte(s)/s");
    } // end IF
    if (speed >= 1024.0F)
    {
      speed /= 1024.0F;
      magnitude_string = ACE_TEXT_ALWAYS_CHAR ("mbyte(s)/s");
    } // end IF
    result = ACE_OS::sprintf (buffer_a, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (buffer_a));
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
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
    static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->configuration);

  UPnP_Client_SSDP_ConnectionManager_t* connection_manager_p =
    UPNP_CLIENT_SSDP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  UPnP_Client_SSDP_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  if (data_p->configuration->multicastHandle != ACE_INVALID_HANDLE)
  {
    iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->configuration->multicastHandle));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->multicastHandle));
#endif // ACE_WIN32 || ACE_WIN64
    if (iconnection_p)
    {
      iconnection_p->abort ();
      iconnection_p->decrease (); iconnection_p = NULL;
    } // end ELSE
    data_p->configuration->multicastHandle = ACE_INVALID_HANDLE;
  } // end IF
  if (data_p->configuration->handle != ACE_INVALID_HANDLE)
  {
    iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->configuration->handle));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->handle));
#endif // ACE_WIN32 || ACE_WIN64
    if (iconnection_p)
    {
      iconnection_p->abort ();
      iconnection_p->decrease (); iconnection_p = NULL;
    } // end ELSE
    data_p->configuration->handle = ACE_INVALID_HANDLE;
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
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
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
  ACE_ASSERT (userData_in);
  UPnP_Client_UI_CBData* data_p =
      static_cast<UPnP_Client_UI_CBData*> (userData_in);
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
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock, G_SOURCE_REMOVE);
    if (data_p->UIState->logStack.empty ())
      return G_SOURCE_CONTINUE;

    // step1: convert text
    for (Common_MessageStackConstIterator_t iterator_2 = data_p->UIState->logStack.begin ();
         iterator_2 != data_p->UIState->logStack.end ();
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

    data_p->UIState->logStack.clear ();
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
action_discovery_activate_cb (GtkAction* action_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_discovery_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
    static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->streamConfiguration.configuration_->messageAllocator);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("Out"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());

  struct UPnP_Client_MessageData* record_p = NULL;
  ACE_NEW_NORETURN (record_p,
                    struct UPnP_Client_MessageData());
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF
  record_p->method = static_cast<HTTP_Method_t> (SSDP_Codes::SSDP_METHOD_M_SEARCH);
  record_p->URI = ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_REQUEST_URI_STRING);
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  std::string UUID_string =
    ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_S_HEADER_PREFIX_STRING);
  UUID_string += Net_Common_Tools::makeUUID ();
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_S_HEADER_STRING),
                                            UUID_string));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                            Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST((*iterator_2).second)->socketConfiguration.peerAddress, false, false)));
  std::string temp_string = "\"";
  temp_string += ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_MAN_SSDP_DISCOVER_STRING);
  temp_string += "\"";
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_MAN_HEADER_STRING),
                                            temp_string));
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_SERVICE_TYPE_HEADER_STRING),
                                            ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_ST_SSDP_ALL_STRING)));
  std::ostringstream converter;
  converter << SSDP_DISCOVER_MX_DEFAULT_DELAY_S;
  record_p->headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (SSDP_DISCOVER_MX_HEADER_STRING),
                                            converter.str ()));

  UPnP_Client_MessageData_t* message_data_container_p = NULL;
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  ACE_NEW_NORETURN (message_data_container_p,
                    UPnP_Client_MessageData_t (record_p,
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

  UPnP_Client_SSDP_ConnectionManager_t* connection_manager_p =
    UPNP_CLIENT_SSDP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT(connection_manager_p);
  UPnP_Client_SSDP_ConnectionManager_t::ICONNECTION_T* iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->configuration->outboundHandle));
#else
    connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->outboundHandle));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iconnection_p);
  UPnP_Client_IOutbound_SSDP_StreamConnection_t* istream_connection_p =
    dynamic_cast<UPnP_Client_IOutbound_SSDP_StreamConnection_t*> (iconnection_p);
  ACE_ASSERT (istream_connection_p);
  istream_connection_p->send (message_block_p);
  iconnection_p->decrease (); iconnection_p = NULL;
} // action_discovery_activate_cb

void
action_external_address_activate_cb (GtkAction* action_in,
                                     gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_external_address_activate_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  //ACE_ASSERT (data_p->UIState);
  //Common_UI_GTK_BuildersIterator_t iterator =
  //  data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  //ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->eventHandler);

  start_progress_reporting (userData_in);

  data_p->eventHandler->state_ = EVENT_HANDLER_STATE_EXTERNAL_ADDRESS_CONTROL;

  ACE_ASSERT (data_p->control);
  data_p->control->getP ()->externalAddress ();
} // action_presentation_url_activate_cb

void
action_map_activate_cb (GtkAction* action_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_map_activate_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  ACE_ASSERT (data_p->eventHandler);

  start_progress_reporting (userData_in);

  // retrieve data
  GtkSpinButton* spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_EXTERNAL_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ENTRY_EXTERNAL_ADDRESS_NAME)));
  ACE_ASSERT (entry_p);
  std::string address = gtk_entry_get_text (entry_p);
  address += ':';
  std::ostringstream converter;
  converter << gtk_spin_button_get_value_as_int (spin_button_p);
  address += converter.str ();
  ACE_INET_Addr external_address;
  int result = external_address.set (address.c_str ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (address.c_str ())));
    return;
  } // end IF

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_INTERNAL_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ENTRY_INTERNAL_ADDRESS_NAME)));
    ACE_ASSERT (entry_p);
  address = gtk_entry_get_text (entry_p);
  address += ':';
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << gtk_spin_button_get_value_as_int (spin_button_p);
  address += converter.str ();
  ACE_INET_Addr internal_address;
  result = internal_address.set (address.c_str ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (address.c_str ())));
    return;
  } // end IF

  data_p->eventHandler->state_ = EVENT_HANDLER_STATE_MAP_CONTROL;

  ACE_ASSERT (data_p->control);
  data_p->control->getP ()->map (external_address,
                                 internal_address);
} // action_map_activate_cb

void
action_presentation_url_activate_cb (GtkAction* action_in,
                                     gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_presentation_url_activate_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  ACE_ASSERT (data_p->control);

  GtkStatusbar* statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);
  gtk_statusbar_push (statusbar_p, 0,
                      data_p->control->getP ()->presentationURL ().c_str ());

  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  GtkWidget* widget_p =
    gtk_message_dialog_new (GTK_WINDOW (dialog_p),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_INFO,
                            GTK_BUTTONS_CLOSE,
                            ACE_TEXT_ALWAYS_CHAR ("Success: \"%s\""),
                            data_p->control->getP ()->presentationURL ().c_str ());
  gtk_dialog_run (GTK_DIALOG (widget_p));
  gtk_widget_destroy (widget_p); widget_p = NULL;
} // action_presentation_url_activate_cb

void
combobox_interface_changed_cb (GtkComboBox* comboBox_in,
                               gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_interface_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->configuration);
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("Out"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());

  GtkTreeIter tree_iterator;
  if (!gtk_combo_box_get_active_iter (comboBox_in, &tree_iterator))
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
//  data_p->configuration->listenerConfiguration.interfaceIdentifier =
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    Common_Tools::StringToGUID (g_value_get_string (&value));
//#else
//      g_value_get_string (&value);
//#endif
  NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.interfaceIdentifier =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    Common_Tools::StringToGUID (g_value_get_string (&value));
#else
    g_value_get_string (&value);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    g_value_get_string (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
}

//void
//checkbutton_broadcast_toggled_cb (GtkCheckButton* checkButton_in,
//                                  gpointer userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::checkbutton_broadcast_toggled_cb"));
//
//  // sanity check(s)
//  ACE_ASSERT (userData_in);
//  struct UPnP_Client_UI_CBData* data_p =
//      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
//  ACE_ASSERT (data_p->configuration);
//
//  data_p->configuration->protocolConfiguration.requestBroadcastReplies =
//    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkButton_in));
//}
//
//void
//checkbutton_request_toggled_cb (GtkCheckButton* checkButton_in,
//                                gpointer userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::checkbutton_request_toggled_cb"));
//
//  // sanity check(s)
//  ACE_ASSERT (userData_in);
//  struct UPnP_Client_UI_CBData* data_p =
//      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
//  ACE_ASSERT (data_p->configuration);
//
//  data_p->configuration->protocolConfiguration.sendRequestOnOffer =
//      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkButton_in));
//}

void
toggleaction_listen_toggled_cb (GtkToggleAction* toggleAction_in,
                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::toggleaction_listen_toggled_cb"));

  if (un_toggling_listen)
  {
    un_toggling_listen = false;
    return;
  } // end IF

  // sanity check(s)
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());

  //GtkToggleButton* toggle_button_p =
  //  GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LISTEN_NAME)));
  //ACE_ASSERT (toggle_button_p);
  bool start_listening = gtk_toggle_action_get_active (toggleAction_in);
  //gtk_button_set_label (GTK_BUTTON (toggle_button_p),
  //                      (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTENING_STRING)
  //                                       : ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTEN_STRING)));

//  GtkImage* image_p =
//    GTK_IMAGE (gtk_builder_get_object ((*iterator).second.second,
//                                       (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_IMAGE_CONNECT_NAME)
//                                                        : ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_IMAGE_DISCONNECT_NAME))));
//  ACE_ASSERT (image_p);
//  gtk_button_set_image (GTK_BUTTON (toggle_button_p), GTK_WIDGET (image_p));
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in),
                           (start_listening ? GTK_STOCK_DISCONNECT
                                            : GTK_STOCK_CONNECT));

  bool failed = true;
  UPnP_Client_SSDP_ConnectionManager_t* connection_manager_p =
    UPNP_CLIENT_SSDP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  if (start_listening)
  {
    UPnP_Client_SSDP_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
    // already listening ? --> stop
    if (data_p->configuration->multicastHandle != ACE_INVALID_HANDLE)
    {
      iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->configuration->multicastHandle));
#else
          connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->multicastHandle));
#endif // ACE_WIN32 || ACE_WIN64
      if (iconnection_p)
      {
        iconnection_p->abort ();
        iconnection_p->decrease (); iconnection_p = NULL;
      } // end ELSE
      data_p->configuration->multicastHandle = ACE_INVALID_HANDLE;
    } // end IF
    if (data_p->configuration->handle != ACE_INVALID_HANDLE)
    {
      iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->configuration->handle));
#else
          connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->handle));
#endif // ACE_WIN32 || ACE_WIN64
      if (iconnection_p)
      {
        iconnection_p->abort ();
        iconnection_p->decrease (); iconnection_p = NULL;
      } // end ELSE
      data_p->configuration->handle = ACE_INVALID_HANDLE;
    } // end IF

    Net_ConnectionConfigurationsIterator_t iterator_3;
//    UPnP_Client_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
//      connection_manager_p;
//    ACE_ASSERT (iconnection_manager_p);
    UPnP_Client_OutboundConnector_t connector_out (true);
    UPnP_Client_OutboundAsynchConnector_t asynch_connector_out (true);
    UPnP_Client_InboundConnector_t connector (true);
    UPnP_Client_InboundAsynchConnector_t asynch_connector (true);
    UPnP_Client_InboundConnectorMcast_t connector_mcast (true);
    UPnP_Client_InboundAsynchConnectorMcast_t asynch_connector_mcast (true);
    //UPnP_Client_IConnection_t* iconnection_p = NULL;

    // *IMPORTANT NOTE*: bind()ing is weird. On Windows systems, the FIRST bound
    //                   socket will receive the inbound data. On Linux, it is
    //                   the LAST...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (data_p->configuration->outboundHandle != ACE_INVALID_HANDLE)
      goto continue_;

    // connect (multicast out)
    iterator_3 =
        data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("Out"));
    ACE_ASSERT (iterator_3 != data_p->configuration->connectionConfigurations.end ());
    if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
      data_p->configuration->outboundHandle =
        Net_Client_Common_Tools::connect<UPnP_Client_OutboundConnector_t> (connector_out,
                                                                           *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                           data_p->configuration->userData,
                                                                           NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress,
                                                                           true, true);
    else
      data_p->configuration->outboundHandle =
        Net_Client_Common_Tools::connect<UPnP_Client_OutboundAsynchConnector_t> (asynch_connector_out,
                                                                                 *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                                 data_p->configuration->userData,
                                                                                 NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress,
                                                                                 true, true);
    if (unlikely (data_p->configuration->outboundHandle == ACE_INVALID_HANDLE))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress).c_str ())));
      goto continue_2;
    } // end IF
    iconnection_p =
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->outboundHandle));
    ACE_ASSERT (iconnection_p);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%u: connected to %s\n"),
                iconnection_p->id (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress).c_str ())));
    iconnection_p->decrease (); iconnection_p = NULL;
continue_:
#endif // ACE_WIN32 || ACE_WIN64

    // connect (unicast in)
    iterator_3 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("In"));
    ACE_ASSERT (iterator_3 != data_p->configuration->connectionConfigurations.end ());
    if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
      data_p->configuration->handle =
        Net_Client_Common_Tools::connect<UPnP_Client_InboundConnector_t> (connector,
                                                                          *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                          data_p->configuration->userData,
                                                                          NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress,
                                                                          true,
                                                                          false);
    else
      data_p->configuration->handle =
        Net_Client_Common_Tools::connect<UPnP_Client_InboundAsynchConnector_t> (asynch_connector,
                                                                                *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                                data_p->configuration->userData,
                                                                                NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress,
                                                                                true,
                                                                                false);
    if (unlikely (data_p->configuration->handle == ACE_INVALID_HANDLE))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
      goto continue_2;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: opened UDP socket: %s\n"),
                data_p->configuration->handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: opened UDP socket: %s\n"),
                data_p->configuration->handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

    // connect (multicast in)
    iterator_3 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("In_2"));
    ACE_ASSERT (iterator_3 != data_p->configuration->connectionConfigurations.end ());
    if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
      data_p->configuration->multicastHandle =
        Net_Client_Common_Tools::connect<UPnP_Client_InboundConnectorMcast_t> (connector_mcast,
                                                                               *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                               data_p->configuration->userData,
                                                                               NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress,
                                                                               true, false);
    else
      data_p->configuration->multicastHandle =
        Net_Client_Common_Tools::connect<UPnP_Client_InboundAsynchConnectorMcast_t> (asynch_connector_mcast,
                                                                                     *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                                     data_p->configuration->userData,
                                                                                     NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress,
                                                                                     true, false);
    if (unlikely (data_p->configuration->multicastHandle == ACE_INVALID_HANDLE))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
      goto continue_2;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: opened UDP socket: %s\n"),
                data_p->configuration->multicastHandle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: opened UDP socket: %s\n"),
                data_p->configuration->multicastHandle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.listenAddress).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (data_p->configuration->outboundHandle != ACE_INVALID_HANDLE)
      goto continue_3;

    // connect (multicast out)
    iterator_3 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("Out"));
    ACE_ASSERT (iterator_3 != data_p->configuration->connectionConfigurations.end ());
    if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
      data_p->configuration->outboundHandle =
        Net_Client_Common_Tools::connect<UPnP_Client_OutboundConnector_t> (connector_out,
                                                                           *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                           data_p->configuration->userData,
                                                                           NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress,
                                                                           true, true);
    else
      data_p->configuration->outboundHandle =
        Net_Client_Common_Tools::connect<UPnP_Client_OutboundAsynchConnector_t> (asynch_connector_out,
                                                                                 *static_cast<UPnP_Client_SSDP_ConnectionConfiguration*> ((*iterator_3).second),
                                                                                 data_p->configuration->userData,
                                                                                 NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress,
                                                                                 true, true);
    if (unlikely (data_p->configuration->outboundHandle == ACE_INVALID_HANDLE))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress).c_str ())));
      goto continue_2;
    } // end IF
    iconnection_p =
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->outboundHandle));
    ACE_ASSERT (iconnection_p);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%u: connected to %s\n"),
                iconnection_p->id (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator_3).second)->socketConfiguration.peerAddress).c_str ())));
    iconnection_p->decrease ();
continue_3:
#endif // ACE_WIN32 || ACE_WIN64

    failed = false;

continue_2:
    if (unlikely (failed))
      goto error;

    // step3: start progress reporting
    start_progress_reporting (userData_in);
  } // end IF
  else
  {
    UPnP_Client_SSDP_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
    if (data_p->configuration->multicastHandle != ACE_INVALID_HANDLE)
    {
      iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->configuration->multicastHandle));
#else
        connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->multicastHandle));
#endif
      if (iconnection_p)
      {
        iconnection_p->abort ();
        iconnection_p->decrease (); iconnection_p = NULL;
      } // end ELSE
      data_p->configuration->multicastHandle = ACE_INVALID_HANDLE;
    } // end IF
    if (data_p->configuration->handle != ACE_INVALID_HANDLE)
    {
      iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->configuration->handle));
#else
        connection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->configuration->handle));
#endif
      if (iconnection_p)
      {
        iconnection_p->abort ();
        iconnection_p->decrease (); iconnection_p = NULL;
      } // end ELSE
      data_p->configuration->handle = ACE_INVALID_HANDLE;
    } // end IF

    // stop progress reporting
    ACE_ASSERT (data_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->UIState->lock);
      if (!g_source_remove (data_p->progressData.eventSourceId))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    data_p->progressData.eventSourceId));
      data_p->UIState->eventSourceIds.erase (data_p->progressData.eventSourceId);
      data_p->progressData.eventSourceId = 0;
    } // end lock scope
    GtkProgressBar* progressbar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    // *NOTE*: this disables "activity mode" (in Gtk2)
    gtk_progress_bar_set_fraction (progressbar_p, 0.0);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), false);
  } // end ELSE

  return;

error:
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_CONNECT);
  un_toggling_listen = true;
  gtk_toggle_action_set_active (toggleAction_in, FALSE);
} // toggle_action_listen_toggled_cb

void
spinbutton_server_port_value_changed_cb (GtkSpinButton* spinButton_in,
                                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::spinbutton_server_port_value_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
    static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  unsigned short port_number =
    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (spinButton_in));
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("Out"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  NET_CONFIGURATION_UDP_CAST ((*iterator_2).second)->socketConfiguration.peerAddress.set_port_number (port_number,
                                                                                                      1);
} // spinbutton_server_port_value_changed_cb

// -----------------------------------------------------------------------------

void
button_clear_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
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
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->UIState->builders.end ());
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog);

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
  struct UPnP_Client_UI_CBData* data_p =
    static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
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
  struct UPnP_Client_UI_CBData* data_p =
      static_cast<struct UPnP_Client_UI_CBData*> (userData_in);
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
