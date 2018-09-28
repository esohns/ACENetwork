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

#include "test_u_ui_callbacks.h"

#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "ace/Synch.h"
#include "common_timer_manager.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "net_wlan_configuration.h"

#include "wlan_monitor_common.h"
#include "wlan_monitor_defines.h"

// static variables
static bool toggling_connect_button = false;

void
load_wlan_interfaces (GtkListStore* listStore_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_wlan_interfaces"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter tree_iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Net_WLAN_IInetMonitor_t* iinetwlanmonitor_p =
      NET_WLAN_INETMONITOR_SINGLETON::instance ();
  ACE_ASSERT (iinetwlanmonitor_p);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Identifiers_t interface_identifiers_a =
      Net_WLAN_Tools::getInterfaces (iinetwlanmonitor_p->get ());
  for (Common_IdentifiersIterator_t iterator = interface_identifiers_a.begin ();
#else
  Net_InterfaceIdentifiers_t interface_identifiers_a =
    Net_WLAN_Tools::getInterfaces (AF_UNSPEC, 0);
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
#endif // ACE_WIN32 || ACE_WIN64
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    gtk_list_store_append (listStore_in, &tree_iterator);
    gtk_list_store_set (listStore_in, &tree_iterator,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        0, Net_Common_Tools::interfaceToString (*iterator).c_str (),
                        1, Common_Tools::GUIDToString (*iterator).c_str (),
#else
                        0, (*iterator).c_str (),
                        1, (*iterator).c_str (),
#endif // ACE_WIN32 || ACE_WIN64
                        -1);
  } // end FOR
}

void
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_ssids (HANDLE clientHandle_in,
            REFGUID interfaceIdentifier_in,
#else
load_ssids (const std::string& interfaceIdentifier_in,
#if defined (WEXT_USE)
            ACE_HANDLE handle_in,
#elif defined (NL80211_USE)
            struct nl_sock* socketHandle_in,
            int driverFamilyId_in,
#elif defined (DBUS_USE)
            struct DBusConnection*,
#endif
#endif // ACE_WIN32 || ACE_WIN64
            Net_WLAN_SSIDs_t& SSIDs_out,
            GtkListStore* listStore_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_ssids"));

  // initialize result(s)
  SSIDs_out.clear ();
  gtk_list_store_clear (listStore_in);

  // sanity check(s)
  ACE_ASSERT (listStore_in);

  SSIDs_out =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
      Net_WLAN_Tools::getSSIDs (clientHandle_in,
                                interfaceIdentifier_in);
#else
      Net_WLAN_SSIDs_t ();
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif // WLANAPI_SUPPORT
#else
      Net_WLAN_Tools::getSSIDs (interfaceIdentifier_in,
#if defined (WEXT_USE)
                                handle_in);
#elif defined (NL80211_USE)
                                socketHandle_in,
                                driverFamilyId_in);
#elif defined (DBUS_USE)
                                connection_in);
#else
                                NULL);
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif
#endif // ACE_WIN32 || ACE_WIN64

  gchar* string_p = NULL;
  GtkTreeIter tree_iterator;
  for (Net_WLAN_SSIDsIterator_t iterator_2 = SSIDs_out.begin ();
       iterator_2 != SSIDs_out.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &tree_iterator);
    string_p = Common_UI_GTK_Tools::localeToUTF8 (*iterator_2);
    ACE_ASSERT (string_p);
    gtk_list_store_set (listStore_in, &tree_iterator,
                        0, string_p,
                        -1);

    // clean up
    g_free (string_p);
    string_p = NULL;
  } // end FOR
}

gboolean
idle_finalize_ui_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_ui_cb"));

  // leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_log_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TEXTVIEW_LOG_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  GtkTextIter text_iterator;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iterator);

  gchar* converted_text = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);
    // sanity check
    if (data_p->logStack.empty ())
      return G_SOURCE_CONTINUE;

    // step1: convert text
    for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
         iterator_2 != data_p->logStack.end ();
         ++iterator_2)
    {
      converted_text = Common_UI_GTK_Tools::localeToUTF8 (*iterator_2);
      if (!converted_text)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"), aborting\n"),
                    ACE_TEXT ((*iterator_2).c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF

        // step2: display text
      gtk_text_buffer_insert (buffer_p,
                              &text_iterator,
                              converted_text,
                              -1);

      // clean up
      g_free (converted_text);
    } // end FOR

    data_p->logStack.clear ();
  } // end lock scope

    // step3: scroll the view accordingly
    //  // move the iterator to the beginning of line, so it doesn't scroll
    //  // in horizontal direction
    //  gtk_text_iter_set_line_offset (&text_iterator, 0);

    //  // ...and place the mark at iter. The mark will stay there after insertion
    //  // because it has "right" gravity
    //  GtkTextMark* text_mark_p =
    //      gtk_text_buffer_get_mark (buffer_p,
    //                                ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SCROLLMARK_NAME));
    ////  gtk_text_buffer_move_mark (buffer_p,
    ////                             text_mark_p,
    ////                             &text_iterator);

    //  // scroll the mark onscreen
    //  gtk_text_view_scroll_mark_onscreen (view_p,
    //                                      text_mark_p);
  GtkAdjustment* adjustment_p =
    GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_ADJUSTMENT_SCROLLEDWINDOW_V_NAME)));
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p));

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  GtkScale* scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SCALE_SIGNALQUALITY_NAME)));
  ACE_ASSERT (scale_p);
  GtkSpinner* spinner_p =
    GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINNER_NAME)));
  ACE_ASSERT (spinner_p);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);
    for (WLANMMMonitor_EventsIterator_t iterator_2 = data_p->eventStack.begin ();
         iterator_2 != data_p->eventStack.end ();
         iterator_2++)
    {
      switch (*iterator_2)
      {
        case WLAN_MONITOR_EVENT_INTERFACE_HOTPLUG:
        case WLAN_MONITOR_EVENT_INTERFACE_REMOVE:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_HOTPLUGGINGS_NAME)));
          break;
        }
        case WLAN_MONITOR_EVENT_SCAN_COMPLETE:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_SCANS_NAME)));
          break;
        }
        case WLAN_MONITOR_EVENT_ASSOCIATE:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_ASSOCIATIONS_NAME)));

          gtk_widget_set_sensitive (GTK_WIDGET (scale_p),
                                    TRUE);
          gtk_widget_set_visible (GTK_WIDGET (scale_p),
                                  TRUE);

          break;
        }
        case WLAN_MONITOR_EVENT_DISASSOCIATE:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_ASSOCIATIONS_NAME)));

          gtk_widget_set_sensitive (GTK_WIDGET (scale_p),
                                    FALSE);
          gtk_widget_set_visible (GTK_WIDGET (scale_p),
                                  FALSE);

          break;
        }
        case WLAN_MONITOR_EVENT_CONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_ASSOCIATIONS_NAME)));

          gtk_spinner_stop (spinner_p);
          gtk_widget_set_sensitive (GTK_WIDGET (spinner_p),
                                    FALSE);
          gtk_widget_set_visible (GTK_WIDGET (spinner_p),
                                  FALSE);

          break;
        }
        case WLAN_MONITOR_EVENT_DISCONNECT:
        {
          spin_button_p =
                GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                         ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_ASSOCIATIONS_NAME)));

          gtk_widget_set_sensitive (GTK_WIDGET (spinner_p),
                                    TRUE);
          gtk_widget_set_visible (GTK_WIDGET (spinner_p),
                                  TRUE);

          break;
        }
        case WLAN_MONITOR_EVENT_SIGNAL_QUALITY_CHANGED:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *iterator_2));
          break;
        }
      } // end SWITCH
      if (likely (spin_button_p))
        gtk_spin_button_spin (spin_button_p,
                              GTK_SPIN_STEP_FORWARD,
                              1.0);
    } // end FOR
    data_p->eventStack.clear ();
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_ssids_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_ssids_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->monitor);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  Net_WLAN_SSIDs_t ssids = data_p->monitor->SSIDs ();
  GtkTreeIter tree_iterator;
  gchar* string_p = NULL;
  guint num_signal_handlers = 0;

  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_SSID_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_SSID_NAME)));
  ACE_ASSERT (list_store_p);

  num_signal_handlers =
      g_signal_handlers_block_by_func (G_OBJECT (combo_box_p),
                                       (gpointer)combobox_ssid_changed_cb,
                                       userData_in);
  ACE_ASSERT (num_signal_handlers == 1);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);
    gtk_list_store_clear (list_store_p);
    for (Net_WLAN_SSIDsIterator_t iterator_3 = ssids.begin ();
         iterator_3 != ssids.end ();
         ++iterator_3)
    {
      gtk_list_store_append (list_store_p, &tree_iterator);
      string_p = Common_UI_GTK_Tools::localeToUTF8 (*iterator_3);
      ACE_ASSERT (string_p);
      gtk_list_store_set (list_store_p, &tree_iterator,
                          0, string_p,
                          -1);

      // clean up
      g_free (string_p);
      string_p = NULL;
    } // end FOR
  } // end lock scope
  num_signal_handlers =
      g_signal_handlers_unblock_by_func (G_OBJECT (combo_box_p),
                                         (gpointer)combobox_ssid_changed_cb,
                                         userData_in);
  ACE_ASSERT (num_signal_handlers == 1);
  g_signal_emit_by_name (G_OBJECT (combo_box_p),
                         ACE_TEXT_ALWAYS_CHAR ("changed"),
                         userData_in);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_signal_quality_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_signal_quality_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->monitor);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkScale* scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SCALE_SIGNALQUALITY_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_value (GTK_RANGE (scale_p),
                       static_cast<gdouble> (data_p->monitor->get_2 ()));

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_status_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_status_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->monitor);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  bool is_connected_b = !(data_p->monitor->SSID ().empty ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TOGGLEBUTTON_CONNECT_NAME)));
  ACE_ASSERT (toggle_button_p);
  bool is_active_b = gtk_toggle_button_get_active (toggle_button_p);
  bool toggle_b = (is_active_b != is_connected_b);
  if (toggle_b)
  {
    toggling_connect_button = true;
    gtk_toggle_button_set_active (toggle_button_p,
                                  !is_active_b);
  } // end IF

  return G_SOURCE_REMOVE;
}

//////////////////////////////////////////

gboolean
idle_initialize_ui_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_ui_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_DIALOG_MAIN_NAME)));
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
  //                                        ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_DIALOG_ABOUT_NAME)));
  //  ACE_ASSERT (about_dialog_p);

  // step2: initialize info view
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_SCANS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_ASSOCIATIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_HOTPLUGGINGS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  // configuration -----------------------
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_INTERFACE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        0, GTK_SORT_ASCENDING);
  load_wlan_interfaces (list_store_p);
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_INTERFACE_NAME)));
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

  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_SSID_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        0, GTK_SORT_ASCENDING);
  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_SSID_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
    //                         GTK_TREE_MODEL (list_store_p));
  cell_renderer_p = gtk_cell_renderer_text_new ();
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

  // step3: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    //GTK_TEXT_VIEW (glade_xml_get_widget ((*iterator).second.second,
    //                                     ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TEXTVIEW_NAME)));
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TEXTVIEW_LOG_NAME)));
  ACE_ASSERT (view_p);
  //  GtkTextBuffer* buffer_p =
  ////      gtk_text_buffer_new (NULL); // text tag table --> create new
  ////      gtk_text_view_set_buffer (view_p, buffer_p);
  //      gtk_text_view_get_buffer (view_p);
  //  ACE_ASSERT (buffer_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (TEST_U_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (WLAN_MONITOR_GTK_PANGO_LOG_FONT_DESCRIPTION)));
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
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags>(GTK_RC_BASE |
                            GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p),
                               &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  GtkScale* scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SCALE_SIGNALQUALITY_NAME)));
  ACE_ASSERT (scale_p);
  gtk_range_set_show_fill_level (GTK_RANGE (scale_p),
                                 TRUE);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  g_value_init (&value, G_TYPE_INT);
#endif // GTK_CHECK_VERSION (2,30,0)
  g_value_set_int (&value, 0);
  g_object_set_property (G_OBJECT (scale_p),
                         ACE_TEXT_ALWAYS_CHAR ("slider-width"),
                         &value);
  g_value_unset (&value);

  // step5: disable some functions ?
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            data_p->allowUserRuntimeStatistic);

  // step6: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6a: connect default signals
  gulong result =
    g_signal_connect (dialog_p,
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      &dialog_p);
  ACE_ASSERT (result);

  //   // step7: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step8: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step9: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);
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
    event_source_id =
        g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
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

  // step10: activate some widgets
  GtkCheckButton* check_button_p =
      GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_CHECKBUTTON_AUTOASSOCIATE_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                data_p->configuration->WLANMonitorConfiguration.autoAssociate);

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_INTERFACE_NAME)));
    ACE_ASSERT (combo_box_p);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_INTERFACE_NAME)));
  ACE_ASSERT (list_store_p);
  gint n_rows = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p),
                                                NULL);
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                            (n_rows > 0));
  if (n_rows > 0)
    gtk_combo_box_set_active (combo_box_p, 0);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gtk_progress_bar_pulse (progress_bar_p);

  return G_SOURCE_CONTINUE;
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
togglebutton_monitor_toggled_cb (GtkToggleButton* toggleButton_in,
                                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_monitor_toggled_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->monitor);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
//  GtkFrame* frame_p =
//      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
//                                         ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_FRAME_CONFIGURATION_NAME)));
//  ACE_ASSERT (frame_p);
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  if (is_active)
  {
    // update configuration
    gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                          GTK_STOCK_MEDIA_STOP);

    GtkSpinButton* spin_button_p =
      GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_SCANS_NAME)));
    ACE_ASSERT (spin_button_p);
    gtk_spin_button_set_value (spin_button_p, 0.0);
    spin_button_p =
        GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_ASSOCIATIONS_NAME)));
    ACE_ASSERT (spin_button_p);
    gtk_spin_button_set_value (spin_button_p, 0.0);spin_button_p =
        GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINBUTTON_HOTPLUGGINGS_NAME)));
    ACE_ASSERT (spin_button_p);
    gtk_spin_button_set_value (spin_button_p, 0.0);

    GtkListStore* list_store_p = NULL;
#if GTK_CHECK_VERSION (3,0,0)
    GValue value_s = G_VALUE_INIT;
#else
    GValue value_s;
#endif
    g_value_init (&value_s, G_TYPE_STRING);
    GtkTreeIter iterator_2;
    GtkComboBox* combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_INTERFACE_NAME)));
    ACE_ASSERT (combo_box_p);
    if (gtk_combo_box_get_active_iter (combo_box_p,
                                       &iterator_2))
    {
      list_store_p =
          GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_INTERFACE_NAME)));
      ACE_ASSERT (list_store_p);
      gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                                &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                1, &value_s);
      ACE_ASSERT (G_VALUE_TYPE (&value_s) == G_TYPE_STRING);
      data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier =
        Common_Tools::StringToGUID (g_value_get_string (&value_s));
#else
                                0, &value_s);
      ACE_ASSERT (G_VALUE_TYPE (&value_s) == G_TYPE_STRING);
      data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier =
          ACE_TEXT_ALWAYS_CHAR (g_value_get_string (&value_s));
#endif
      g_value_unset (&value_s);
    } // end IF

    combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_SSID_NAME)));
    ACE_ASSERT (combo_box_p);
    if (gtk_combo_box_get_active_iter (combo_box_p,
                                       &iterator_2))
    {
      list_store_p =
          GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_SSID_NAME)));
      ACE_ASSERT (list_store_p);
      g_value_init (&value_s, G_TYPE_STRING);
      gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                                &iterator_2,
                                0, &value_s);
      ACE_ASSERT (G_VALUE_TYPE (&value_s) == G_TYPE_STRING);
      data_p->configuration->WLANMonitorConfiguration.SSID =
          ACE_TEXT_ALWAYS_CHAR (g_value_get_string (&value_s));
      g_value_unset (&value_s);
    } // end IF

    GtkCheckButton* check_button_p =
        GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_CHECKBUTTON_AUTOASSOCIATE_NAME)));
    ACE_ASSERT (check_button_p);
    data_p->configuration->WLANMonitorConfiguration.autoAssociate =
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button_p));

    if (!data_p->monitor->initialize (data_p->configuration->WLANMonitorConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_IInitialize_T::initialize(), aborting\n")));
      goto error;
    } // end IF

    try {
      data_p->monitor->start ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_ITask_T::start(): \"%m\", aborting\n")));
      goto error;
    } // end catch

    // start progress reporting
    ACE_ASSERT (!data_p->eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->lock);
      data_p->eventSourceId =
          //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
          //                 idle_update_progress_cb,
          //                 &data_p->progressData,
          //                 NULL);
          g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                              COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
                              idle_update_progress_cb,
                              userData_in,
                              NULL);
      if (data_p->eventSourceId > 0)
        data_p->eventSourceIds.insert (data_p->eventSourceId);
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", continuing\n")));
    } // end lock scope
  } // end IF
  else
  {
    gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                          GTK_STOCK_MEDIA_PLAY);

    try {
      data_p->monitor->stop (true,
                             true);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_ITask_T::stop(): \"%m\", aborting\n")));
      goto error;
    } // end catch

    // stop progress reporting
    ACE_ASSERT (data_p->eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->lock);
      if (!g_source_remove (data_p->eventSourceId))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    data_p->eventSourceId));
      data_p->eventSourceIds.erase (data_p->eventSourceId);
      data_p->eventSourceId = 0;
    } // end lock scope
  } // end ELSE
  //  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
//                            !is_active);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p),
                            is_active);

  return;

error:
//  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
//                            true);
  gtk_widget_set_sensitive (GTK_WIDGET (progress_bar_p),
                            false);
} // togglebutton_monitor_toggled_cb

void
togglebutton_connect_toggled_cb (GtkToggleButton* toggleButton_in,
                                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_connect_toggled_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->monitor);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
//  GtkFrame* frame_p =
//      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
//                                         ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_FRAME_CONFIGURATION_NAME)));
//  ACE_ASSERT (frame_p);
  GtkScale* scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SCALE_SIGNALQUALITY_NAME)));
  ACE_ASSERT (scale_p);
  GtkSpinner* spinner_p =
      GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_SPINNER_NAME)));
  ACE_ASSERT (spinner_p);

  if (is_active)
  {
    gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                          GTK_STOCK_DISCONNECT);

    if (toggling_connect_button)
    {
      toggling_connect_button = false;
      return; // done
    } // end IF

    // --> connect

    // *NOTE*: when monitoring, the signal quality display is handled in the
    //         callbacks
    if (!data_p->monitor->isRunning ())
    {
      gtk_widget_set_visible (GTK_WIDGET (scale_p),
                              TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (scale_p),
                                TRUE);
    } // end IF
    gtk_widget_set_visible (GTK_WIDGET (spinner_p),
                            TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (spinner_p),
                              TRUE);
    gtk_spinner_start (spinner_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    struct ether_addr ap_mac_address;
    ACE_OS::memset (&ap_mac_address, 0, sizeof (struct ether_addr));
#endif
    if (!data_p->monitor->associate (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                                     ap_mac_address,
#endif
                                     data_p->configuration->WLANMonitorConfiguration.SSID))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier).c_str ()),
                  ACE_TEXT (data_p->configuration->WLANMonitorConfiguration.SSID.c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s,%s), returning\n"),
                  ACE_TEXT (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier.c_str ()),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
                  ACE_TEXT (data_p->configuration->WLANMonitorConfiguration.SSID.c_str ())));
#endif
      goto error;
    } // end IF
  } // end IF
  else
  {
    gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                          GTK_STOCK_CONNECT);

    if (toggling_connect_button)
    {
      toggling_connect_button = false;
      return; // done
    } // end IF

    // --> disconnect

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    struct ether_addr ap_mac_address;
    ACE_OS::memset (&ap_mac_address, 0, sizeof (struct ether_addr));
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (!data_p->monitor->associate (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
#else
    if (!data_p->monitor->associate (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
                                     ap_mac_address,
#endif // ACE_WIN32 || ACE_WIN64
                                     ACE_TEXT_ALWAYS_CHAR ("")))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier).c_str ()),
                  ACE_TEXT ("")));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s,%s), returning\n"),
                  ACE_TEXT (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier.c_str ()),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
                  ACE_TEXT ("")));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF

    // *NOTE*: when monitoring, the signal quality display is handled in the
    //         callbacks
    if (!data_p->monitor->isRunning ())
    {
      gtk_widget_set_visible (GTK_WIDGET (scale_p),
                              FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (scale_p),
                                FALSE);
    } // end IF
    gtk_spinner_stop (spinner_p);
    gtk_widget_set_sensitive (GTK_WIDGET (spinner_p),
                              FALSE);
    gtk_widget_set_visible (GTK_WIDGET (spinner_p),
                            FALSE);
  } // end ELSE
//  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
//                            !is_active);

  return;

error:
//  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
  if (is_active)
    gtk_spinner_stop (spinner_p);
  else
    gtk_spinner_start (spinner_p);
  gtk_widget_set_sensitive (GTK_WIDGET (spinner_p),
                            !is_active);
  gtk_widget_set_visible (GTK_WIDGET (spinner_p),
                          !is_active);
  toggling_connect_button = true;
  gtk_toggle_button_set_active (toggleButton_in,
                                !is_active);
} // togglebutton_connect_toggled_cb

void
button_report_clicked_cb (GtkButton* button_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_report_clicked_cb"));

  ACE_UNUSED_ARG (button_in);
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
}

void
combobox_interface_changed_cb (GtkComboBox* comboBox_in,
                               gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_interface_changed_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->monitor);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (comboBox_in,
                                      &iterator_2))
  { // most probable reason: nothing selected
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_INTERFACE_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
#else
                            0, &value);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier =
    Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (g_value_get_string (&value)));
#else
  data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier =
    ACE_TEXT_ALWAYS_CHAR (g_value_get_string (&value));
#endif // ACE_WIN32 || ACE_WIN64
  g_value_reset (&value);
  gint n_rows = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p),
                                                NULL);

  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_SSID_NAME)));
  ACE_ASSERT (combo_box_p);
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_SSID_NAME)));
  ACE_ASSERT (list_store_p);
  Net_WLAN_SSIDs_t ssids;
  guint num_signal_handlers =
      g_signal_handlers_block_by_func (G_OBJECT (combo_box_p),
                                       (gpointer)combobox_ssid_changed_cb,
                                       userData_in);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->lock);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_USE)
    load_ssids (data_p->monitor->get (),
                data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
#endif // WLANAPI_USE
#else
    load_ssids (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
#if defined (WEXT_USE)
                data_p->monitor->get (),
#elif defined (NL80211_USE)
                const_cast<struct nl_sock*> (data_p->monitor->getP ()),
                data_p->monitor->get (),
#elif defined (DBUS_USE)
                data_p->monitor->getP (),
#endif
#endif // ACE_WIN32 || ACE_WIN64
                ssids,
                list_store_p);
  } // end lock scope
  num_signal_handlers =
      g_signal_handlers_unblock_by_func (G_OBJECT (combo_box_p),
                                         (gpointer)combobox_ssid_changed_cb,
                                         userData_in);
  ACE_ASSERT (num_signal_handlers == 1);
  gint n_rows_2 = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p),
                                                  NULL);
  gchar* string_p = NULL;
  guint index_i = std::numeric_limits<unsigned int>::max ();
  std::string current_essid_string =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_USE)
      Net_WLAN_Tools::associatedSSID (data_p->monitor->get (),
                                      data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier);
#else
      ACE_TEXT_ALWAYS_CHAR ("");
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif // WLANAPI_USE
#else
      Net_WLAN_Tools::associatedSSID (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
#if defined (WEXT_USE)
                                      data_p->monitor->get ());
#elif defined (NL80211_USE)
                                      const_cast<struct nl_sock*> (data_p->monitor->getP ()),
                                      data_p->monitor->get ());
#elif defined (DBUS_USE)
                                      data_p->monitor->getP ());
#else
                                      NULL);
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif
#endif // ACE_WIN32 || ACE_WIN64
  bool select_ssid = false;
  if (!data_p->configuration->WLANMonitorConfiguration.SSID.empty () || // configured
      (data_p->configuration->WLANMonitorConfiguration.SSID.empty () &&
       !current_essid_string.empty ()))                                 // not configured and associated
  {
    string_p =
        Common_UI_GTK_Tools::localeToUTF8 ((!data_p->configuration->WLANMonitorConfiguration.SSID.empty () ? data_p->configuration->WLANMonitorConfiguration.SSID
                                                                                                           : current_essid_string.c_str ()));
    ACE_ASSERT (string_p);
    g_value_set_string (&value,
                        string_p);
    g_free (string_p);
    index_i = Common_UI_GTK_Tools::valueToIndex (GTK_TREE_MODEL (list_store_p),
                                                 value,
                                                 0);
    g_value_reset (&value);
    if (index_i != std::numeric_limits<unsigned int>::max ())
      select_ssid = true;
  } // end IF

  // update widget(s)
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TOGGLEBUTTON_MONITOR_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p),
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//                                !InlineIsEqualGUID (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier, GUID_NULL) &&
//#else
//                                !data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier.empty () &&
//#endif
                                (n_rows > 0));
  gtk_widget_set_sensitive (GTK_WIDGET (comboBox_in),
                            (n_rows > 0));
  gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                            (n_rows_2 > 0));

  if (select_ssid)
    gtk_combo_box_set_active (combo_box_p,
                              index_i);
}

void
combobox_ssid_changed_cb (GtkComboBox* comboBox_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_ssid_changed_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->monitor);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_LISTSTORE_SSID_NAME)));
  ACE_ASSERT (list_store_p);
  gint n_rows = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p),
                                                NULL);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)

  GtkTreeIter iterator_2;
  std::string current_essid_string =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_USE)
    Net_WLAN_Tools::associatedSSID (data_p->monitor->get (),
                                    data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier);
#else
  ACE_TEXT_ALWAYS_CHAR ("");
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif // WLANAPI_USE
#else
  Net_WLAN_Tools::associatedSSID (data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
#if defined (WEXT_USE)
                                  data_p->monitor->get ());
#elif defined (NL80211_USE)
                                  const_cast<struct nl_sock*> (data_p->monitor->getP ()),
                                  data_p->monitor->get ());
#elif defined (DBUS_USE)
                                  data_p->monitor->getP ());
#else
                                  NULL);
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
#endif
#endif // ACE_WIN32 || ACE_WIN64
  std::string selected_essid_string;
  gchar* string_p = NULL;
  guint index_i = std::numeric_limits<unsigned int>::max ();
  bool select_ssid = false;
  bool can_connect = true;
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TOGGLEBUTTON_CONNECT_NAME)));
  ACE_ASSERT (toggle_button_p);
  // retrieve the active entry
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->lock);
    if (!gtk_combo_box_get_active_iter (comboBox_in,
                                        &iterator_2))
    { // most probable reason: nothing selected
      can_connect = false;

      // --> select configured entry (if any) and available
      if (data_p->configuration->WLANMonitorConfiguration.SSID.empty ())
      { // not configured
        // --> select current entry (if any) and available
        if (current_essid_string.empty ())
          goto continue_;

        string_p = Common_UI_GTK_Tools::localeToUTF8 (current_essid_string);
        ACE_ASSERT (string_p);
        g_value_set_string (&value,
                            string_p);
        g_free (string_p);
      } // end IF
      else
      { // configured
        string_p =
            Common_UI_GTK_Tools::localeToUTF8 (data_p->configuration->WLANMonitorConfiguration.SSID);
        ACE_ASSERT (string_p);
        g_value_set_string (&value,
                            string_p);
        g_free (string_p);
      } // end ELSE
      index_i =
          Common_UI_GTK_Tools::valueToIndex (GTK_TREE_MODEL (list_store_p),
                                             value,
                                             0);
      g_value_reset (&value);
      if (index_i != std::numeric_limits<unsigned int>::max ())
        select_ssid = true;
      goto continue_;
    } // end IF
    g_value_unset (&value);
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_2,
                              0, &value);
  } // end lock scope
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  selected_essid_string =
      Common_UI_GTK_Tools::UTF8ToLocale (g_value_get_string (&value),
                                         -1);
  g_value_reset (&value);
  if (!ACE_OS::strcmp (selected_essid_string.c_str (),
                       data_p->configuration->WLANMonitorConfiguration.SSID.c_str ()))
  {
    // the selected SSID is the configured SSID

    if (data_p->configuration->WLANMonitorConfiguration.SSID.empty () ||                 // not configured
        !ACE_OS::strcmp (current_essid_string.c_str (),
                         data_p->configuration->WLANMonitorConfiguration.SSID.c_str ())) // already associated
      goto continue_;

    // the current SSID is not the configured SSID
    // --> auto-associate ?
    if (data_p->configuration->WLANMonitorConfiguration.autoAssociate)
    {
      GtkCheckButton* check_button_p =
          GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_CHECKBUTTON_AUTOASSOCIATE_NAME)));
      ACE_ASSERT (check_button_p);
      g_signal_emit_by_name (G_OBJECT (check_button_p),
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             userData_in);
    } // end IF
  } // end IF
  else
  {
    // the selected SSID is not the configured SSID

    if (!data_p->monitor->isRunning () ||
        data_p->configuration->WLANMonitorConfiguration.SSID.empty ())
    { // not active or configured
      // --> update configuration
      data_p->configuration->WLANMonitorConfiguration.SSID =
          selected_essid_string;

      // already associated with selected SSID ?
      // --> activate disconnect function
      if (!ACE_OS::strcmp (current_essid_string.c_str (),
                           data_p->configuration->WLANMonitorConfiguration.SSID.c_str ()))
      { // associated with selected SSID
        if (!gtk_toggle_button_get_active (toggle_button_p))
        {
//          guint num_signal_handlers =
//              g_signal_handlers_block_by_func (G_OBJECT (combo_box_p),
//                                               (gpointer)combobox_ssid_changed_cb,
//                                               userData_in);
//          ACE_ASSERT (num_signal_handlers == 1);
          toggling_connect_button = true;
          ACE_ASSERT (toggle_button_p);
          gtk_toggle_button_set_active (toggle_button_p,
                                        TRUE);
//          num_signal_handlers =
//              g_signal_handlers_unblock_by_func (G_OBJECT (combo_box_p),
//                                                 (gpointer)combobox_ssid_changed_cb,
//                                                 userData_in);
//          ACE_ASSERT (num_signal_handlers == 1);
        } // end IF

        goto continue_;
      } // end IF

      // not associated with selected SSID
      // --> auto-associate ?
      if (data_p->configuration->WLANMonitorConfiguration.autoAssociate)
      {
        // already associated ? --> disconnect first
        if (!current_essid_string.empty ())
        { ACE_ASSERT (toggle_button_p);
          gtk_toggle_button_set_active (toggle_button_p,
                                        FALSE);
        } // end IF

        GtkCheckButton* check_button_p =
            GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_CHECKBUTTON_AUTOASSOCIATE_NAME)));
        ACE_ASSERT (check_button_p);
        g_signal_emit_by_name (G_OBJECT (check_button_p),
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               userData_in);
      } // end IF
      goto continue_;
    } // end IF

    // active and configured
    // --> re-select configured SSID, if available
    string_p =
        Common_UI_GTK_Tools::localeToUTF8 (data_p->configuration->WLANMonitorConfiguration.SSID);
    ACE_ASSERT (string_p);
    g_value_set_string (&value,
                        string_p);
    g_free (string_p);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->lock);
      index_i =
          Common_UI_GTK_Tools::valueToIndex (GTK_TREE_MODEL (list_store_p),
                                              value,
                                              0);
    } // end lock scope
    g_value_reset (&value);
    if (index_i != std::numeric_limits<unsigned int>::max ())
      select_ssid = true;
  } // end ELSE

continue_:
  // update widget(s)
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p),
                            can_connect);
  gtk_widget_set_sensitive (GTK_WIDGET (comboBox_in),
                            (n_rows > 0));

  if (select_ssid)
    gtk_combo_box_set_active (comboBox_in,
                              index_i);
}

void
togglebutton_autoassociate_toggled_cb (GtkToggleButton* toggleButton_in,
                                       gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_autoassociate_toggled_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // update configuration
  data_p->configuration->WLANMonitorConfiguration.autoAssociate =
    gtk_toggle_button_get_active (toggleButton_in);

  // sanity check(s)
  if (!data_p->configuration->WLANMonitorConfiguration.autoAssociate)
    return; // nothing to do
  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_COMBOBOX_SSID_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_2;
  // sanity check(s)
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
    return; // nothing to do

  // update widget(s)
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TOGGLEBUTTON_CONNECT_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                TRUE);
}

void
togglebutton_backgroundscan_toggled_cb (GtkToggleButton* toggleButton_in,
                                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_backgroundscan_toggled_cb"));

  struct WLANMonitor_GTK_CBData* data_p =
    static_cast<struct WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // update configuration
  data_p->configuration->WLANMonitorConfiguration.enableBackgroundScans =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    (gtk_toggle_button_get_active (toggleButton_in) ? TRUE : FALSE);
#else
      gtk_toggle_button_get_active (toggleButton_in);
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Net_WLAN_IInetMonitor_t* iinetwlanmonitor_p =
    NET_WLAN_INETMONITOR_SINGLETON::instance ();
  // sanity check(s)
  ACE_ASSERT (iinetwlanmonitor_p);
  //if (!iinetwlanmonitor_p->isRunning ())
  //  return; // nothing to do

  Net_WLAN_Tools::setDeviceSettingBool (iinetwlanmonitor_p->get (),
                                        data_p->configuration->WLANMonitorConfiguration.interfaceIdentifier,
                                        wlan_intf_opcode_background_scan_enabled,
                                        data_p->configuration->WLANMonitorConfiguration.enableBackgroundScans);
#endif
}

void
button_clear_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  WLANMonitor_GTK_CBData* data_p =
    static_cast<WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_TEXTVIEW_LOG_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p =
//    gtk_text_buffer_new (NULL); // text tag table --> create new
    gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_set_text (buffer_p,
                            ACE_TEXT_ALWAYS_CHAR (""), 0);
}

void
button_about_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  WLANMonitor_GTK_CBData* data_p =
    static_cast<WLANMonitor_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_GTK_ABOUTDIALOG_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (WLAN_MONITOR_GTK_ABOUTDIALOG_NAME)));
    return;
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
} // button_about_clicked_cb

void
button_quit_clicked_cb (GtkButton* button_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (button_in);
  ACE_UNUSED_ARG (userData_in);

  int result = -1;

  //WLANMonitor_GTK_CBData* data_p = static_cast<WLANMonitor_GTK_CBData*> (userData_in);
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
  result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  // step3: stop GTK event processing
  // *NOTE*: triggering UI shutdown here is more consistent, compared to doing
  //         it from the signal handler
  COMMON_UI_GTK_MANAGER_SINGLETON::instance()->stop (false,  // wait ?
                                                     false); // N/A
} // button_quit_clicked_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
