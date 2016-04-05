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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <limits>

#include <iphlpapi.h>
#else
#include <netinet/ether.h>
#include <ifaddrs.h>
#endif
#include <sstream>

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "glade/glade.h"

#include "common_file_tools.h"
#include "common_timer_manager.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_tools.h"

#include "net_macros.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_connection_stream.h"
#include "test_u_defines.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

#include "test_u_dhcp_client_common.h"

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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    ACE_FREE_FUNC (ip_adapter_info_p);

    return false;
  } // end IF

  struct _IP_ADAPTER_INFO* ip_adapter_info_2 = ip_adapter_info_p;
  do
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found network interface: \"%s\": \"%s\"...\n"),
    //            ACE_TEXT (ip_adapter_info_2->Description),
    //            ACE_TEXT (Net_Common_Tools::MACAddress2String (ip_adapter_info_2->Address).c_str ())));

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, Common_UI_Tools::Locale2UTF8 (ip_adapter_info_2->Description),
                        1, ip_adapter_info_2->AdapterName,
                        -1);

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

/////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window4096_set_title (,
  //                      caption.c_str ());

//  GtkWidget* about_dialog_p =
//    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p,
                             static_cast<double> (data_p->configuration->socketConfiguration.address.get_port_number ()));

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

  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_BROADCAST_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                data_p->configuration->protocolConfiguration.requestBroadcastReplies);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_REQUEST_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                data_p->configuration->protocolConfiguration.sendRequestOnOffer);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_ASYNCH_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                !data_p->configuration->useReactor);
  check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_LOOPBACK_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                data_p->configuration->socketConfiguration.useLoopBackDevice);

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  gtk_spin_button_set_value (spin_button_p,
                              static_cast<double> (data_p->configuration->streamConfiguration.bufferSize));

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
  gtk_rc_style_unref (rc_style_p);

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
  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    // schedule asynchronous updates of the log view
    event_source_id = g_timeout_add_seconds (1,
                                             idle_update_log_display_cb,
                                             data_p);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
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
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?
  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_REQUEST_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);
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
  //gtk_builder_connect_signals ((*iterator).second.second,
  //                             userData_in);

  GObject* object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEACTION_LISTEN_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (toggleaction_listen_toggled_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_REPORT_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (action_report_activate_cb),
                               userData_in);
  ACE_ASSERT (result_2);

  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_DISCOVER_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (action_discover_activate_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_REQUEST_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (action_request_activate_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_RELEASE_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (action_release_activate_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_INFORM_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("activate"),
                               G_CALLBACK (action_inform_activate_cb),
                               userData_in);
  ACE_ASSERT (result_2);

  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_COMBOBOX_INTERFACE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("changed"),
                        G_CALLBACK (combobox_interface_changed_cb),
                        userData_in);
  ACE_ASSERT (result_2);

  //object_p =
  //    gtk_builder_get_object ((*iterator).second.second,
  //                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_PORT_NAME));
  //ACE_ASSERT (object_p);
  //result_2 = g_signal_connect (object_p,
  //                           ACE_TEXT_ALWAYS_CHAR ("value-changed"),
  //                           G_CALLBACK (spinbutton_port_value_changed_cb),
  //                           cb_data_p);
  //ACE_ASSERT (result_2);

  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_BROADCAST_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("toggled"),
                        G_CALLBACK (checkbutton_broadcast_toggled_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_CHECKBUTTON_REQUEST_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("toggled"),
                        G_CALLBACK (checkbutton_request_toggled_cb),
                        userData_in);
  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR(TEST_U_UI_GTK_TEXTVIEW_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                               G_CALLBACK (textview_size_allocate_cb),
                               userData_in);
  ACE_ASSERT (result_2);

  //-------------------------------------

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_BUTTON_CLEAR_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                      G_CALLBACK (button_clear_clicked_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

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
    if (data_p->configuration->socketConfiguration.networkInterface.empty ())
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
      GValue value = {0,};
      do
      {
        gtk_tree_model_get_value (tree_model_p,
                                  &tree_iterator,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  1, &value);
#else
                                  0, &value);
#endif
        ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
        if (ACE_OS::strcmp (data_p->configuration->socketConfiguration.networkInterface.c_str (),
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
idle_start_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_start_UI_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  //GtkAction* action_p =
  //  GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
  //                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_SEND_NAME)));
  //ACE_ASSERT (action_p);
  //gtk_action_set_sensitive (action_p, TRUE);

  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  return G_SOURCE_REMOVE;
}
gboolean
idle_end_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_UI_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  //GtkAction* action_p =
  //  //GTK_SPIN_BUTTON (glade_xml_get_widget ((*iterator).second.second,
  //  //                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_NUMCONNECTIONS_NAME)));
  //  GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
  //                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_ACTION_SEND_NAME)));
  //ACE_ASSERT (action_p);
  //gtk_action_set_sensitive (action_p, FALSE);

  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_reset_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_reset_UI_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_value (spin_button_p, 0.0);

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_set_text (progress_bar_p, ACE_TEXT_ALWAYS_CHAR (""));

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

    data_p->progressData.transferred = 0;
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  Test_U_GTK_ProgressData* data_p =
      static_cast<Test_U_GTK_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = -1;
  float speed = 0.0F;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->GTKState->lock);

    speed = data_p->statistic.bytesPerSecond;
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
    result = ACE_OS::sprintf (buffer, ACE_TEXT ("%.2f %s"),
                              speed, magnitude_string.c_str ());
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sprintf(): \"%m\", continuing\n")));
  } // end IF
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (buffer));
  gtk_progress_bar_pulse (progress_bar_p);

  // --> reschedule
  return G_SOURCE_CONTINUE;
}

/////////////////////////////////////////

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  ACE_UNUSED_ARG (userData_in);

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  if (data_p->eventStack.empty ())
    return G_SOURCE_CONTINUE;

  for (Test_U_GTK_EventsIterator_t iterator_2 = data_p->eventStack.begin ();
       iterator_2 != data_p->eventStack.end ();
       iterator_2++)
  {
    switch (*iterator_2)
    {
      case TEST_U_GTKEVENT_START:
      {
        spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
        ACE_ASSERT (spin_button_p);
        gtk_spin_button_set_value (spin_button_p, 0.0);
        spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
        ACE_ASSERT (spin_button_p);
        gtk_spin_button_set_value (spin_button_p, 0.0);
        spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_DATA_NAME)));
        ACE_ASSERT (spin_button_p);
        gtk_spin_button_set_value (spin_button_p, 0.0);

        spin_button_p =
          GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
        ACE_ASSERT (spin_button_p);

        is_session_message = true;
        break;
      }
      case TEST_U_GTKEVENT_DATA:
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
      case TEST_U_GTKEVENT_END:
      {
        spin_button_p =
          GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
        ACE_ASSERT (spin_button_p);

        is_session_message = true;
        break;
      }
      case TEST_U_GTKEVENT_STATISTIC:
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
                    *iterator_2));
        break;
      }
    } // end SWITCH
    ACE_UNUSED_ARG (is_session_message);
    gtk_spin_button_spin (spin_button_p,
                          GTK_SPIN_STEP_FORWARD,
                          1.0);
  } // end FOR

  data_p->eventStack.clear ();

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_log_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

  Common_UI_GTKBuildersIterator_t iterator =
      data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

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
  // sanity check
  if (data_p->logStack.empty ())
    return G_SOURCE_CONTINUE;

  // step1: convert text
  for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
       iterator_2 != data_p->logStack.end ();
       iterator_2++)
  {
    string_p = Common_UI_Tools::Locale2UTF8 (*iterator_2);
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
                  ACE_TEXT ((*iterator_2).c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF

    // step2: display text
    gtk_text_buffer_insert (buffer_p,
                            &text_iterator,
                            string_p,
                            -1);

    // clean up
    g_free (string_p);
  } // end FOR

  data_p->logStack.clear ();

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
action_discover_activate_cb (GtkAction* action_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_discover_activate_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->handle != ACE_INVALID_HANDLE);
  ACE_ASSERT (data_p->configuration->streamConfiguration.messageAllocator);
  ACE_ASSERT (iterator != data_p->builders.end ());

  //gtk_action_set_sensitive (action_in, FALSE);
  //GtkFrame* frame_p =
  //  GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
  //                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_FRAME_CONFIGURATION_NAME)));
  //ACE_ASSERT (frame_p);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

  // retrieve port number
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_PORT_NAME)));
  ACE_ASSERT (spin_button_p);
  unsigned short port_number =
    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (spin_button_p));
  data_p->configuration->socketConfiguration.address.set_port_number (port_number,
                                                                      1);

  // retrieve buffer
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  data_p->configuration->streamConfiguration.bufferSize =
    static_cast<unsigned int> (gtk_spin_button_get_value_as_int (spin_button_p));

  Test_U_Message* message_p = NULL;
allocate:
  message_p =
    static_cast<Test_U_Message*> (data_p->configuration->streamConfiguration.messageAllocator->malloc (data_p->configuration->streamConfiguration.bufferSize));
  // keep retrying ?
  if (!message_p && !data_p->configuration->streamConfiguration.messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));
    return;
  } // end IF
  DHCP_Record DHCP_record;
  DHCP_record.op = DHCP_Codes::DHCP_OP_REQUEST;
  DHCP_record.htype = DHCP_FRAME_HTYPE;
  DHCP_record.hlen = DHCP_FRAME_HLEN;
  DHCP_record.xid = DHCP_Tools::generateXID ();
  if (data_p->configuration->protocolConfiguration.requestBroadcastReplies)
    DHCP_record.flags = DHCP_FLAGS_BROADCAST;
  if (!Net_Common_Tools::interface2MACAddress (data_p->configuration->socketConfiguration.networkInterface,
                                               DHCP_record.chaddr))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2MACAddress(), returning\n")));
    return;
  } // end IF
  // *TODO*: support optional options:
  //         - 'requested IP address'    (50)
  //         - 'IP address lease time'   (51)
  //         - 'overload'                (52)
  char message_type = DHCP_Codes::DHCP_MESSAGE_DISCOVER;
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE,
                                              std::string (1, message_type)));
  //         - 'parameter request list'  (55) [include in all subsequent messages]
  //         - 'message'                 (56)
  //         - 'maximum message size'    (57)
  //         - 'vendor class identifier' (60)
  //         - 'client identifier'       (61)
  message_p->initialize (DHCP_record,
                         NULL);

  Test_U_IConnectionManager_t* iconnection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_U_IConnection_t* iconnection_p =
      iconnection_manager_p->get (data_p->configuration->handle);
  if (!iconnection_p)
  {
    // *NOTE*: most probable reason, the interface IP address has changed
    // *TODO*: restart the listener
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Connection_Manager_T::get(0x%@), returning\n"),
                data_p->configuration->handle));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Connection_Manager_T::get(%d), returning\n"),
                data_p->configuration->handle));
#endif

    // clean up
    data_p->configuration->handle = ACE_INVALID_HANDLE;

    return;
  } // end IF
  Test_U_IInboundSocketConnection_t* isocket_connection_2 =
    dynamic_cast<Test_U_IInboundSocketConnection_t*> (iconnection_p);
  ACE_ASSERT (isocket_connection_2);
  Test_U_ConnectionState& state_r =
      const_cast<Test_U_ConnectionState&> (isocket_connection_2->state ());
  state_r.timeStamp = COMMON_TIME_NOW;
  state_r.xid = DHCP_record.xid;

  Test_U_InboundConnectionStream& stream_r =
      const_cast<Test_U_InboundConnectionStream&> (isocket_connection_2->stream ());
  const Test_U_StreamSessionData_t* session_data_container_p = stream_r.get ();
  ACE_ASSERT (session_data_container_p);
  Test_U_StreamSessionData& session_data_r =
      const_cast<Test_U_StreamSessionData&> (session_data_container_p->get ());
  session_data_r.timeStamp = state_r.timeStamp;
  session_data_r.xid = DHCP_record.xid;

  // clean up
  iconnection_p->decrease ();
  iconnection_p = NULL;

  // sanity check(s)
  ACE_ASSERT (session_data_r.broadcastConnection);

  Test_U_ISocketConnection_t* isocket_connection_p =
    dynamic_cast<Test_U_ISocketConnection_t*> (session_data_r.broadcastConnection);
  ACE_ASSERT (isocket_connection_p);
  isocket_connection_p->send (message_p);

  return;

//clean:
  //gtk_action_set_sensitive (action_in, TRUE);
  //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
} // action_discover_activate_cb
void
action_inform_activate_cb (GtkAction* action_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_inform_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  Test_U_DHCPClient_GTK_CBData* data_p =
    static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->handle != ACE_INVALID_HANDLE);
  ACE_ASSERT (data_p->configuration->streamConfiguration.messageAllocator);
  ACE_ASSERT (iterator != data_p->builders.end ());

  Test_U_IConnectionManager_t* iconnection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_U_IConnection_t* iconnection_p =
    iconnection_manager_p->get (data_p->configuration->handle);
  ACE_ASSERT (iconnection_p);
  Test_U_IInboundSocketConnection_t* isocket_connection_2 =
    dynamic_cast<Test_U_IInboundSocketConnection_t*> (iconnection_p);
  ACE_ASSERT (isocket_connection_2);
  Test_U_ConnectionState& state_r =
    const_cast<Test_U_ConnectionState&> (isocket_connection_2->state ());

  Test_U_InboundConnectionStream& stream_r =
    const_cast<Test_U_InboundConnectionStream&> (isocket_connection_2->stream ());
  const Test_U_StreamSessionData_t* session_data_container_p = stream_r.get ();
  ACE_ASSERT (session_data_container_p);
  Test_U_StreamSessionData& session_data_r =
    const_cast<Test_U_StreamSessionData&> (session_data_container_p->get ());

  // clean up
  iconnection_p->decrease ();
  iconnection_p = NULL;

  iconnection_p = session_data_r.connection;
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("no (unicast) connection to DHCP server, falling back\n")));

    // sanity check(s)
    ACE_ASSERT (session_data_r.broadcastConnection);

    iconnection_p = session_data_r.broadcastConnection;
  } // end IF

allocate:
  Test_U_Message* message_p =
    static_cast<Test_U_Message*> (data_p->configuration->streamConfiguration.messageAllocator->malloc (data_p->configuration->streamConfiguration.bufferSize));
  // keep retrying ?
  if (!message_p && !data_p->configuration->streamConfiguration.messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));
    return;
  } // end IF
  DHCP_Record DHCP_record;
  DHCP_record.op = DHCP_Codes::DHCP_OP_REQUEST;
  DHCP_record.htype = DHCP_FRAME_HTYPE;
  DHCP_record.hlen = DHCP_FRAME_HLEN;
  DHCP_record.xid = (session_data_r.xid ? session_data_r.xid :
                                          DHCP_Tools::generateXID ());
  if (!session_data_r.xid || !state_r.xid)
  {
    state_r.xid = DHCP_record.xid;
    session_data_r.xid = DHCP_record.xid;
  } // end IF
  if (data_p->configuration->protocolConfiguration.requestBroadcastReplies)
    DHCP_record.flags = DHCP_FLAGS_BROADCAST;
  ACE_INET_Addr IP_address;
  if (!Net_Common_Tools::interface2IPAddress (data_p->configuration->socketConfiguration.networkInterface,
                                              IP_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2IPAddress(), returning\n")));
    return;
  } // end IF
  DHCP_record.ciaddr = IP_address.get_ip_address ();
  if (!Net_Common_Tools::interface2MACAddress (data_p->configuration->socketConfiguration.networkInterface,
                                               DHCP_record.chaddr))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2MACAddress(), returning\n")));
    return;
  } // end IF
  // *TODO*: support optional options:
  //         - 'overload'                (52)
  char message_type = DHCP_Codes::DHCP_MESSAGE_INFORM;
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE,
                                              std::string (1, message_type)));
  //         - 'parameter request list'  (55) [include in all subsequent messages]
  //         - 'message'                 (56)
  //         - 'maximum message size'    (57)
  //         - 'vendor class identifier' (60)
  //         - 'client identifier'       (61)
  message_p->initialize (DHCP_record,
                         NULL);

  Test_U_ISocketConnection_t* isocket_connection_p =
    dynamic_cast<Test_U_ISocketConnection_t*> (iconnection_p);
  ACE_ASSERT (isocket_connection_p);
  isocket_connection_p->send (message_p);

  return;

//clean:
//gtk_action_set_sensitive (action_in, TRUE);
//gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
} // action_inform_activate_cb
void
action_request_activate_cb (GtkAction* action_in,
                            gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_request_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  Test_U_DHCPClient_GTK_CBData* data_p =
    static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->handle != ACE_INVALID_HANDLE);
  ACE_ASSERT (data_p->configuration->streamConfiguration.messageAllocator);
  ACE_ASSERT (iterator != data_p->builders.end ());

  Test_U_IConnectionManager_t* iconnection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_U_IConnection_t* iconnection_p =
    iconnection_manager_p->get (data_p->configuration->handle);
  ACE_ASSERT (iconnection_p);
  Test_U_IInboundSocketConnection_t* isocket_connection_2 =
    dynamic_cast<Test_U_IInboundSocketConnection_t*> (iconnection_p);
  ACE_ASSERT (isocket_connection_2);
  Test_U_ConnectionState& state_r =
    const_cast<Test_U_ConnectionState&> (isocket_connection_2->state ());

  Test_U_InboundConnectionStream& stream_r =
    const_cast<Test_U_InboundConnectionStream&> (isocket_connection_2->stream ());
  const Test_U_StreamSessionData_t* session_data_container_p = stream_r.get ();
  ACE_ASSERT (session_data_container_p);
  Test_U_StreamSessionData& session_data_r =
    const_cast<Test_U_StreamSessionData&> (session_data_container_p->get ());

  // clean up
  iconnection_p->decrease ();
  iconnection_p = NULL;

  // sanity check(s)
  ACE_ASSERT (session_data_r.broadcastConnection);

allocate:
  Test_U_Message* message_p =
    static_cast<Test_U_Message*> (data_p->configuration->streamConfiguration.messageAllocator->malloc (data_p->configuration->streamConfiguration.bufferSize));
  // keep retrying ?
  if (!message_p && !data_p->configuration->streamConfiguration.messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));
    return;
  } // end IF
  DHCP_Record DHCP_record;
  DHCP_record.op = DHCP_Codes::DHCP_OP_REQUEST;
  DHCP_record.htype = DHCP_FRAME_HTYPE;
  DHCP_record.hlen = DHCP_FRAME_HLEN;
  DHCP_record.xid = (session_data_r.xid ? session_data_r.xid :
                                          DHCP_Tools::generateXID ());
  if (!session_data_r.xid || !state_r.xid)
  {
    state_r.xid = DHCP_record.xid;
    session_data_r.xid = DHCP_record.xid;
  } // end IF
  if (data_p->configuration->protocolConfiguration.requestBroadcastReplies)
    DHCP_record.flags = DHCP_FLAGS_BROADCAST;
  if (!Net_Common_Tools::interface2MACAddress (data_p->configuration->socketConfiguration.networkInterface,
                                               DHCP_record.chaddr))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2MACAddress(), returning\n")));
    return;
  } // end IF
  // *TODO*: support optional options:
  //         - 'requested IP address'    (50)
  //         - 'IP address lease time'   (51)
  //         - 'overload'                (52)
  char message_type = DHCP_Codes::DHCP_MESSAGE_REQUEST;
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE,
                              std::string (1, message_type)));
  //         - 'parameter request list'  (55) [include in all subsequent messages]
  //         - 'message'                 (56)
  //         - 'maximum message size'    (57)
  //         - 'vendor class identifier' (60)
  //         - 'client identifier'       (61)
  message_p->initialize (DHCP_record,
                         NULL);

  Test_U_ISocketConnection_t* isocket_connection_p =
    dynamic_cast<Test_U_ISocketConnection_t*> (session_data_r.broadcastConnection);
  ACE_ASSERT (isocket_connection_p);
  isocket_connection_p->send (message_p);

  return;

//clean:
//gtk_action_set_sensitive (action_in, TRUE);
//gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
} // action_request_activate_cb
void
action_release_activate_cb (GtkAction* action_in,
                            gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_release_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  Test_U_DHCPClient_GTK_CBData* data_p =
    static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->handle != ACE_INVALID_HANDLE);
  ACE_ASSERT (data_p->configuration->streamConfiguration.messageAllocator);
  ACE_ASSERT (iterator != data_p->builders.end ());

  Test_U_IConnectionManager_t* iconnection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_U_IConnection_t* iconnection_p =
    iconnection_manager_p->get (data_p->configuration->handle);
  ACE_ASSERT (iconnection_p);
  Test_U_IInboundSocketConnection_t* isocket_connection_2 =
    dynamic_cast<Test_U_IInboundSocketConnection_t*> (iconnection_p);
  ACE_ASSERT (isocket_connection_2);
  Test_U_ConnectionState& state_r =
    const_cast<Test_U_ConnectionState&> (isocket_connection_2->state ());

  Test_U_InboundConnectionStream& stream_r =
    const_cast<Test_U_InboundConnectionStream&> (isocket_connection_2->stream ());
  const Test_U_StreamSessionData_t* session_data_container_p = stream_r.get ();
  ACE_ASSERT (session_data_container_p);
  Test_U_StreamSessionData& session_data_r =
    const_cast<Test_U_StreamSessionData&> (session_data_container_p->get ());

  // clean up
  iconnection_p->decrease ();
  iconnection_p = NULL;

  iconnection_p = session_data_r.connection;
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("no (unicast) connection to DHCP server, falling back\n")));

    // sanity check(s)
    ACE_ASSERT (session_data_r.broadcastConnection);

    // *WARNING*: this isn't standard compliant
    iconnection_p = session_data_r.broadcastConnection;
  } // end IF

allocate:
  Test_U_Message* message_p =
    static_cast<Test_U_Message*> (data_p->configuration->streamConfiguration.messageAllocator->malloc (data_p->configuration->streamConfiguration.bufferSize));
  // keep retrying ?
  if (!message_p && !data_p->configuration->streamConfiguration.messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));
    return;
  } // end IF
  DHCP_Record DHCP_record;
  DHCP_record.op = DHCP_Codes::DHCP_OP_REQUEST;
  DHCP_record.htype = DHCP_FRAME_HTYPE;
  DHCP_record.hlen = DHCP_FRAME_HLEN;
  DHCP_record.xid = (session_data_r.xid ? session_data_r.xid :
                                          DHCP_Tools::generateXID ());
  if (!session_data_r.xid || !state_r.xid)
  {
    state_r.xid = DHCP_record.xid;
    session_data_r.xid = DHCP_record.xid;
  } // end IF
  ACE_INET_Addr IP_address;
  if (!Net_Common_Tools::interface2IPAddress (data_p->configuration->socketConfiguration.networkInterface,
                                              IP_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2IPAddress(), returning\n")));
    return;
  } // end IF
  DHCP_record.ciaddr = IP_address.get_ip_address ();
  if (!Net_Common_Tools::interface2MACAddress (data_p->configuration->socketConfiguration.networkInterface,
                                               DHCP_record.chaddr))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2MACAddress(), returning\n")));
    return;
  } // end IF
  // *TODO*: support optional options:
  //         - 'overload'                (52)
  char message_type = DHCP_Codes::DHCP_MESSAGE_RELEASE;
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE,
                                              std::string (1, message_type)));
  std::string buffer;
  // *NOTE*: the streamer expects all options to be passed in network (i.e. big
  //         endian) byte order
  ACE_UINT32 IP_address_2 = session_data_r.serverAddress.get_ip_address ();
  if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
    IP_address_2 = ACE_SWAP_LONG (IP_address_2);
  buffer.append (reinterpret_cast<char*> (&IP_address_2), 4);
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER,
                                              buffer));
  //         - 'message'                 (56)
  //         - 'client identifier'       (61)
  message_p->initialize (DHCP_record,
                         NULL);

  Test_U_ISocketConnection_t* isocket_connection_p =
    dynamic_cast<Test_U_ISocketConnection_t*> (iconnection_p);
  ACE_ASSERT (isocket_connection_p);
  isocket_connection_p->send (message_p);

  return;

//clean:
//gtk_action_set_sensitive (action_in, TRUE);
//gtk_widget_set_sensitive (GTK_WIDGET (frame_p), TRUE);
} // action_release_activate_cb

void
action_report_activate_cb (GtkAction* action_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_report_activate_cb"));

  ACE_UNUSED_ARG (action_in);
  ACE_UNUSED_ARG (userData_in);

  int result = -1;

// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) instead...
  int signal = 0;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  signal = SIGUSR1;
#else
  signal = SIGBREAK;
#endif
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(\"%S\" (%d)): \"%m\", continuing\n"),
                signal, signal));
} // action_report_activate_cb

void
combobox_interface_changed_cb (GtkComboBox* comboBox_in,
                               gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_interface_changed_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

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
  data_p->configuration->listenerConfiguration.networkInterface =
      g_value_get_string (&value);
  data_p->configuration->socketConfiguration.networkInterface =
    g_value_get_string (&value);
  g_value_unset (&value);
}

void
checkbutton_broadcast_toggled_cb (GtkCheckButton* checkButton_in,
                                  gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::checkbutton_broadcast_toggled_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  data_p->configuration->protocolConfiguration.requestBroadcastReplies =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkButton_in));
}
void
checkbutton_request_toggled_cb (GtkCheckButton* checkButton_in,
                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::checkbutton_request_toggled_cb"));

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  data_p->configuration->protocolConfiguration.sendRequestOnOffer =
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkButton_in));
}

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

  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LISTEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  bool start_listening = gtk_toggle_button_get_active (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTENING_STRING)
                                         : ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTEN_STRING)));

//  GtkImage* image_p =
//    GTK_IMAGE (gtk_builder_get_object ((*iterator).second.second,
//                                       (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_IMAGE_CONNECT_NAME)
//                                                        : ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_IMAGE_DISCONNECT_NAME))));
//  ACE_ASSERT (image_p);
//  gtk_button_set_image (GTK_BUTTON (toggle_button_p), GTK_WIDGET (image_p));
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in),
                           (start_listening ? GTK_STOCK_DISCONNECT
                                            : GTK_STOCK_CONNECT));

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  bool failed = true;
  Test_U_ConnectionManager_t* connection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  if (start_listening)
  {
    // already listening ? --> stop
    if (data_p->configuration->broadcastHandle != ACE_INVALID_HANDLE)
    {
      Test_U_ConnectionManager_t::ICONNECTION_T* iconnection_p =
          connection_manager_p->get (data_p->configuration->broadcastHandle);
      if (iconnection_p)
      {
        iconnection_p->close ();
        iconnection_p->decrease ();
      } // end ELSE
      data_p->configuration->broadcastHandle = ACE_INVALID_HANDLE;
    } // end IF
    if (data_p->configuration->handle != ACE_INVALID_HANDLE)
    {
      Test_U_ConnectionManager_t::ICONNECTION_T* iconnection_p =
          connection_manager_p->get (data_p->configuration->handle);
      if (iconnection_p)
      {
        iconnection_p->close ();
        iconnection_p->decrease ();
      } // end ELSE
      data_p->configuration->handle = ACE_INVALID_HANDLE;
    } // end IF

    // connect (broadcast)
    //    bool handle_connection_manager = false;
    //    ACE_INET_Addr peer_address;
    Test_U_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      connection_manager_p;
    ACE_ASSERT (iconnection_manager_p);
    Test_U_ConnectorBcast_t connector_bcast (iconnection_manager_p,
      data_p->configuration->streamConfiguration.statisticReportingInterval);
    Test_U_AsynchConnectorBcast_t asynch_connector_bcast (iconnection_manager_p,
      data_p->configuration->streamConfiguration.statisticReportingInterval);
    //Test_U_Connector_t connector_bcast (iconnection_manager_p,
    //                                    data_p->configuration->streamConfiguration.statisticReportingInterval);
    //Test_U_AsynchConnector_t asynch_connector_bcast (iconnection_manager_p,
    //                                                 data_p->configuration->streamConfiguration.statisticReportingInterval);
    Test_U_IConnector_t* iconnector_p = NULL;

    bool socket_connect = data_p->configuration->socketConfiguration.connect;
    data_p->configuration->socketConfiguration.connect = false;
    int result =
        data_p->configuration->listenerConfiguration.address.set (static_cast<u_short> (DHCP_DEFAULT_CLIENT_PORT),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *NOTE*: (on Windows(TM) sytems,) this needs to be INADDR_ANY (0.0.0.0) (why ?)
                                                                  static_cast<ACE_UINT32> (INADDR_ANY));
#else
                                                                  static_cast<ACE_UINT32> (INADDR_BROADCAST));
#endif
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to set listening address: \"%m\", returning\n")));
      goto continue_;
    } // end IF

    if (data_p->configuration->useReactor)
      iconnector_p = &connector_bcast;
    else
      iconnector_p = &asynch_connector_bcast;
    if (!iconnector_p->initialize (data_p->configuration->socketHandlerConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
      goto continue_;
    } // end IF

    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result =
        data_p->configuration->listenerConfiguration.address.addr_to_string (buffer,
                                                                             sizeof (buffer),
                                                                             1);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

//    // step1: initialize connection manager
//    peer_address =
//        data_p->configuration->socketConfiguration.address;
//    data_p->configuration->socketConfiguration.address =
//        data_p->configuration->listenerConfiguration.address;
//    connection_manager_p->set (*data_p->configuration,
//                               &data_p->configuration->userData);
//    handle_connection_manager = true;

    // step2: connect
    data_p->configuration->broadcastHandle =
        iconnector_p->connect (data_p->configuration->listenerConfiguration.address);
    // *TODO*: support one-thread operation by scheduling a signal and manually
    //         running the dispatch loop for a limited time...
    if (!data_p->configuration->useReactor)
    {
      data_p->configuration->broadcastHandle = ACE_INVALID_HANDLE;

      // *TODO*: avoid tight loop here
      ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
      //result = ACE_OS::sleep (timeout);
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
      //              &timeout));
      ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
      Test_U_AsynchConnector_t::ICONNECTION_T* iconnection_p = NULL;
      do
      {
        iconnection_p =
            connection_manager_p->get (data_p->configuration->listenerConfiguration.address,
                                       false);
        if (iconnection_p)
        {
          data_p->configuration->broadcastHandle =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              reinterpret_cast<ACE_HANDLE> (iconnection_p->id ());
#else
              static_cast<ACE_HANDLE> (iconnection_p->id ());
#endif
          iconnection_p->decrease ();
          break;
        } // end IF
      } while (COMMON_TIME_NOW < deadline);
      if (iconnection_p)
        iconnection_p->decrease ();
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to connect to \"%s\" (timed out at: %#T), continuing\n"),
                    ACE_TEXT (buffer),
                    &timeout));
    } // end IF
    if (data_p->configuration->broadcastHandle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                  ACE_TEXT (buffer)));

      // clean up
      iconnector_p->abort ();

      goto continue_;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: started listening (UDP) (\"%s\")...\n"),
                data_p->configuration->broadcastHandle,
                buffer));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: started listening (UDP) (\"%s\")...\n"),
                data_p->configuration->broadcastHandle,
                buffer));
#endif

    // connect (unicast)
    //Test_U_Connector_t connector (iconnection_manager_p,
    //                              data_p->configuration->streamConfiguration.statisticReportingInterval);
    //Test_U_AsynchConnector_t asynch_connector (iconnection_manager_p,
    //                                           data_p->configuration->streamConfiguration.statisticReportingInterval);
    //if (data_p->configuration->useReactor)
    //  iconnector_p = &connector;
    //else
    //  iconnector_p = &asynch_connector;
    //if (!iconnector_p->initialize (data_p->configuration->socketHandlerConfiguration))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
    //  return;
    //} // end IF

    if (data_p->configuration->listenerConfiguration.useLoopBackDevice)
      result =
        data_p->configuration->listenerConfiguration.address.set (static_cast<u_short> (DHCP_DEFAULT_CLIENT_PORT),
                                                                  static_cast<ACE_UINT32> (INADDR_LOOPBACK));
    else if (!data_p->configuration->listenerConfiguration.networkInterface.empty ())
    {
      if (!Net_Common_Tools::interface2IPAddress (data_p->configuration->listenerConfiguration.networkInterface,
                                                  data_p->configuration->listenerConfiguration.address))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_Common_Tools::interface2IPAddress(), continuing\n")));
        result = -1;
      } // end IF
      data_p->configuration->listenerConfiguration.address.set_port_number (DHCP_DEFAULT_CLIENT_PORT,
                                                                            1);
      result = 0;
    } // end ELSE IF
    else
      result =
        data_p->configuration->listenerConfiguration.address.set (static_cast<u_short> (DHCP_DEFAULT_CLIENT_PORT),
                                                                  static_cast<ACE_UINT32> (INADDR_ANY));
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to set listening address: \"%m\", returning\n")));
      goto continue_;
    } // end IF

    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result =
        data_p->configuration->listenerConfiguration.address.addr_to_string (buffer,
                                                                             sizeof (buffer),
                                                                             1);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

//    // step1: initialize connection manager
//    peer_address =
//        data_p->configuration->socketConfiguration.address;
//    data_p->configuration->socketConfiguration.address =
//        data_p->configuration->listenerConfiguration.address;
//    connection_manager_p->set (*data_p->configuration,
//                               &data_p->configuration->userData);
//    handle_connection_manager = true;

    // step2: connect
    data_p->configuration->handle =
        iconnector_p->connect (data_p->configuration->listenerConfiguration.address);
    // *TODO*: support one-thread operation by scheduling a signal and manually
    //         running the dispatch loop for a limited time...
    if (!data_p->configuration->useReactor)
    {
      data_p->configuration->handle = ACE_INVALID_HANDLE;

      // *TODO*: avoid tight loop here
      ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
      //result = ACE_OS::sleep (timeout);
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
      //              &timeout));
      ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
      Test_U_AsynchConnector_t::ICONNECTION_T* iconnection_p = NULL;
      do
      {
        iconnection_p =
            connection_manager_p->get (data_p->configuration->listenerConfiguration.address,
                                       false);
        if (iconnection_p)
        {
          data_p->configuration->handle =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              reinterpret_cast<ACE_HANDLE> (iconnection_p->id ());
#else
              static_cast<ACE_HANDLE> (iconnection_p->id ());
#endif
          iconnection_p->decrease ();
          break;
        } // end IF
      } while (COMMON_TIME_NOW < deadline);
      if (iconnection_p)
        iconnection_p->decrease ();
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to connect to \"%s\" (timed out at: %#T), continuing\n"),
                    ACE_TEXT (buffer),
                    &timeout));
    } // end IF
    if (data_p->configuration->handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                  ACE_TEXT (buffer)));

      // clean up
      iconnector_p->abort ();

      goto continue_;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: started listening (UDP) (\"%s\")...\n"),
                data_p->configuration->handle,
                buffer));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: started listening (UDP) (\"%s\")...\n"),
                data_p->configuration->handle,
                buffer));
#endif

    failed = false;

continue_:
    data_p->configuration->socketConfiguration.connect = socket_connect;
//    // reset connection manager
//    if (handle_connection_manager)
//    {
//      data_p->configuration->socketConfiguration.address = peer_address;
//      connection_manager_p->set (*data_p->configuration,
//                                 &data_p->configuration->userData);
//    } // end IF

    if (failed)
      goto error;

    // step3: start progress reporting
    GtkProgressBar* progressbar_p =
        GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), TRUE);

    ACE_ASSERT (!data_p->progressEventSourceID);
    {
      ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

      data_p->progressEventSourceID =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &data_p->progressData,
        //                 NULL);
          g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                          // _LOW doesn't work (on Win32)
                              TEST_U_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                              idle_update_progress_cb,
                              &data_p->progressData,
                              NULL);
      if (data_p->progressEventSourceID > 0)
        data_p->eventSourceIds.insert (data_p->progressEventSourceID);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_target_progress_cb): \"%m\", returning\n")));
        return;
      } // end IF
    } // end lock scope
  } // end IF
  else
  {
    if (data_p->configuration->broadcastHandle != ACE_INVALID_HANDLE)
    {
      Test_U_ConnectionManager_t::ICONNECTION_T* iconnection_p =
        connection_manager_p->get (data_p->configuration->broadcastHandle);
      if (iconnection_p)
      {
        iconnection_p->close ();
        iconnection_p->decrease ();
      } // end ELSE
      data_p->configuration->broadcastHandle = ACE_INVALID_HANDLE;
    } // end IF
    if (data_p->configuration->handle != ACE_INVALID_HANDLE)
    {
      Test_U_ConnectionManager_t::ICONNECTION_T* iconnection_p =
        connection_manager_p->get (data_p->configuration->handle);
      if (iconnection_p)
      {
        iconnection_p->close ();
        iconnection_p->decrease ();
      } // end ELSE
      data_p->configuration->handle = ACE_INVALID_HANDLE;
    } // end IF

    // stop progress reporting
    ACE_ASSERT (data_p->progressEventSourceID);
    {
      ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (data_p->lock);

      if (!g_source_remove (data_p->progressEventSourceID))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    data_p->progressEventSourceID));
      data_p->eventSourceIds.erase (data_p->progressEventSourceID);
      data_p->progressEventSourceID = 0;
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
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTEN_STRING)
                                         : ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_TOGGLEBUTTON_LABEL_LISTENING_STRING)));

//  GtkImage* image_p =
//    GTK_IMAGE (gtk_builder_get_object ((*iterator).second.second,
//                                       (start_listening ? ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_IMAGE_CONNECT_NAME)
//                                                        : ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_IMAGE_DISCONNECT_NAME))));
//  ACE_ASSERT (image_p);
//  gtk_button_set_image (GTK_BUTTON (toggle_button_p), GTK_WIDGET (image_p));
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_CONNECT);
  un_toggling_listen = true;
  gtk_toggle_action_set_active (toggleAction_in, FALSE);
} // toggle_action_listen_toggled_cb

//void
//spinbutton_port_value_changed_cb (GtkWidget* widget_in,
//                                  gpointer userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::spinbutton_port_value_changed_cb"));
//
//  Stream_GTK_CBData* data_p =
//    static_cast<Stream_GTK_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->configuration);
//
//  unsigned short port_number =
//    static_cast<unsigned short> (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget_in)));
//  data_p->configuration->socketConfiguration.peerAddress.set_port_number (port_number,
//                                                                          1);
//} // spinbutton_port_value_changed_cb

// -----------------------------------------------------------------------------

gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (iterator != data_p->builders.end ());

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

  return FALSE;
}

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_UI_GTK_DIALOG_ABOUT_NAME)));
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

  int result = -1;

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);
  //Stream_GTK_CBData* data_p = static_cast<Stream_GTK_CBData*> (userData_in);
  //// sanity check(s)
  //ACE_ASSERT (data_p);

  //// step1: remove event sources
  //{
  //  ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->lock);

  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->eventSourceIds.begin ();
  //       iterator != data_p->eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->eventSourceIds.clear ();
  //} // end lock scope

  // step2: initiate shutdown sequence
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int signal = SIGINT;
#else
  int signal = SIGQUIT;
#endif
  result = ACE_OS::raise (signal);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                signal));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, true);

  return FALSE;
} // button_quit_clicked_cb

void
textview_size_allocate_cb (GtkWidget* widget_in,
                           GdkRectangle* rectangle_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::textview_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (rectangle_in);
  Test_U_DHCPClient_GTK_CBData* data_p =
      static_cast<Test_U_DHCPClient_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  ACE_ASSERT(iterator != data_p->builders.end ());

  GtkScrolledWindow* scrolled_window_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_UI_GTK_SCROLLEDWINDOW_NAME)));
  ACE_ASSERT (scrolled_window_p);
  GtkAdjustment* adjustment_p =
    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            adjustment_p->upper - adjustment_p->page_size);
} // textview_size_allocate_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
