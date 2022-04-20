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

#include "test_i_gtk_callbacks.h"

#include <limits>
#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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

#include "stream_vis_defines.h"
#include "stream_vis_iresize.h"

#include "net_macros.h"

#include "net_client_common_tools.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_connection_stream.h"
#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_web_tv_common.h"
#include "test_i_web_tv_defines.h"

// initialize statics
static bool un_toggling_play = false;

/////////////////////////////////////////

gboolean
idle_end_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_end_session_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);

  // stop progress reporting
//  GtkSpinner* spinner_p =
//    GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
//                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
//  ACE_ASSERT (spinner_p);
//  gtk_spinner_stop (spinner_p);
//  gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), FALSE);

  //ACE_ASSERT (data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    if (!g_source_remove (data_p->progressData.eventSourceId))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                  data_p->progressData.eventSourceId));
    state_r.eventSourceIds.erase (data_p->progressData.eventSourceId);
    data_p->progressData.eventSourceId = 0;
  } // end lock scope

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progressbar_p, 0.0);
  gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), FALSE);

  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                          GTK_STOCK_MEDIA_PLAY);

    un_toggling_play = true;
    gtk_toggle_button_toggled (toggle_button_p);
  } // end IF

  return G_SOURCE_REMOVE;
}

void
load_resolutions (GtkListStore* listStore_in,
                  const Test_I_WebTV_ChannelResolutions_t& resolutions_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_resolutions"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  std::ostringstream converter;
  GtkTreeIter iterator;
  for (Test_I_WebTV_ChannelResolutionsConstIterator_t iterator_2 = resolutions_in.begin ();
       iterator_2 != resolutions_in.end ();
       ++iterator_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    converter << (*iterator_2).resolution.cx;
#else
    converter << (*iterator_2).resolution.width;
#endif // ACE_WIN32 || ACE_WIN64
    converter << ACE_TEXT_ALWAYS_CHAR (" x ");
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    converter << (*iterator_2).resolution.cy;
#else
    converter << (*iterator_2).resolution.height;
#endif // ACE_WIN32 || ACE_WIN64
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                       0, converter.str ().c_str (),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       1, (*iterator_2).resolution.cx,
                       2, (*iterator_2).resolution.cy,
#else
                       1, (*iterator_2).resolution.width,
                       2, (*iterator_2).resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                       -1);
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  } // end FOR
}

gboolean
idle_load_channel_configuration_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_load_channel_configuration_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
      data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // update configuration
  GtkListStore* liststore_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (liststore_p);
  load_resolutions (liststore_p,
                    (*channel_iterator).second.resolutions);
  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  gtk_combo_box_set_active (combo_box_p, 0);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_BUTTON_LOAD_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            TRUE);
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p),
                            TRUE);

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  state_r.eventSourceIds.clear ();

  gtk_main_quit ();

  // one-shot action
  return G_SOURCE_REMOVE;
}

void
load_channels (GtkListStore* listStore_in,
               const Test_I_WebTV_ChannelConfigurations_t& channels_in)
{
  NETWORK_TRACE (ACE_TEXT ("::load_channels"));

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
  for (Test_I_WebTV_ChannelConfigurationsConstIterator_t iterator_2 = channels_in.begin ();
       iterator_2 != channels_in.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                       0, (*iterator_2).second.name.c_str (),
                       1, (*iterator_2).first,
                       -1);
  } // end FOR
}

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: initialize dialog window(s)
  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT32>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BYTES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             (gdouble)std::numeric_limits<ACE_UINT64>::max ());

  GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_CHANNEL_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  load_channels (list_store_p,
                 *data_p->channels);

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);
  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);

  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME)));
  ACE_ASSERT (combo_box_p);
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p),
                              cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p),
                                  cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT_ALWAYS_CHAR ("text"), 0,
                                  NULL);

  Test_I_WebTV_StreamConfiguration_2_t::ITERATOR_T iterator_3 =
    data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration_2.end ());
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  if (!(*iterator_3).second.second->targetFileName.empty ())
  {
    if (!gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                              (*iterator_3).second.second->targetFileName.c_str ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT ((*iterator_3).second.second->targetFileName.c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF
  } // end IF

//  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
//  default_folder_uri += (*iterator_3).second.second->targetFileName;
//  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
//                                                default_folder_uri.c_str ()))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri(\"%s\"): \"%m\", aborting\n"),
//                ACE_TEXT (default_folder_uri.c_str ())));
//    return G_SOURCE_REMOVE;
//  } // end IF

  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                !(*iterator_3).second.second->targetFileName.empty ());
  toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                !(*iterator_3).second.second->display.device.empty ());

  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progressbar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progressbar_p,
                                   1.0 / static_cast<double> (width));

  GtkStatusbar* statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);
  guint context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_CONTEXT_DATA));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_DATA,
                                             context_id));
  context_id =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_STATUSBAR_CONTEXT_INFORMATION));
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                             context_id));

  // step5: initialize updates
  guint event_source_id = 0;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    //// schedule asynchronous updates of the log view
    //event_source_id = g_timeout_add_seconds (1,
    //                                         idle_update_log_display_cb,
    //                                         data_p);
    //if (event_source_id > 0)
    //  data_p->eventSourceIds.insert (event_source_id);
    //else
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
    //  return G_SOURCE_REMOVE;
    //} // end ELSE

    // schedule asynchronous updates of the info view
    event_source_id =
      g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET_MS,
                     idle_update_info_display_cb,
                     data_p);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?
  toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_PLAY_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), FALSE);

  // step2: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step6a: connect custom signals
  GtkDialog* dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  gulong result = g_signal_connect (G_OBJECT (dialog_p),
                                    ACE_TEXT_ALWAYS_CHAR ("destroy"),
                                    G_CALLBACK (button_quit_clicked_cb),
                                    data_p);
//                   G_CALLBACK(gtk_widget_destroyed),
//                   &main_dialog_p,
  ACE_ASSERT (result);
//  result = g_signal_connect (G_OBJECT (dialog_p),
//                    ACE_TEXT_ALWAYS_CHAR ("delete-event"),
//                    G_CALLBACK (delete_event_cb),
//                    NULL);
  result = g_signal_connect (dialog_p,
                             ACE_TEXT_ALWAYS_CHAR ("destroy"),
                             G_CALLBACK (gtk_widget_destroyed),
                             NULL);
  ACE_ASSERT (result);

  dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (dialog_p);
  result = g_signal_connect_swapped (G_OBJECT (dialog_p),
                                     ACE_TEXT_ALWAYS_CHAR ("response"),
                                     G_CALLBACK (gtk_widget_hide),
                                     dialog_p);
  ACE_ASSERT (result);

  //-------------------------------------

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  GtkDrawingArea* drawing_area_p =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  (*iterator_3).second.second->window =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT ((*iterator_3).second.second->window);

  // select some widgets
  combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_CHANNEL_NAME)));
  ACE_ASSERT (combo_box_p);
#if GTK_CHECK_VERSION(2,30,0)
  struct _GValue value = G_VALUE_INIT;
#else
  struct _GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  g_value_init (&value, G_TYPE_UINT);
  g_value_set_uint (&value,
                    data_p->currentChannel);
  Common_UI_GTK_Tools::selectValue (combo_box_p,
                                    value,
                                    1);
  g_value_unset (&value);

  return G_SOURCE_REMOVE;
}

gboolean
idle_segment_download_complete_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_segment_download_complete_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_ConnectionManager_2_t::INTERFACE_T* iconnection_manager_2 =
      TEST_I_CONNECTIONMANAGER_SINGLETON_2::instance ();
  ACE_ASSERT (iconnection_manager_2);
  Test_I_ConnectionManager_2_t::ICONNECTION_T* iconnection_p = NULL;
  Test_I_IStreamConnection_2_t* istream_connection_p = NULL;
  HTTP_Form_t HTTP_form;
  HTTP_Headers_t HTTP_headers;
  struct HTTP_Record* HTTP_record_p = NULL;
  Test_I_Message::DATA_T* message_data_p = NULL;
  Test_I_Message* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
    Net_ConnectionConfigurationsIterator_t iterator_2 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("2"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  Test_I_WebTV_StreamConfiguration_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
    data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  if ((data_p->handle == ACE_INVALID_HANDLE) ||
      (*channel_iterator).second.segment.URLs.empty ())
    return G_SOURCE_REMOVE;
  std::string current_URL =
      (*channel_iterator).second.segment.URLs.front ();
  (*channel_iterator).second.segment.URLs.pop_front ();
  bool is_URI_b = HTTP_Tools::URLIsURI (current_URL);
  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string, URI_string_2, URL_string;
  bool use_SSL = false;

  if (is_URI_b)
    URL_string = (*iterator_3).second.second->URL;
  else
    URL_string = current_URL;
  if (!HTTP_Tools::parseURL (URL_string,
                             host_address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT ((*iterator_3).second.second->URL.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  if (is_URI_b)
  {
    size_t position = URI_string.find_last_of ('/', std::string::npos);
    ACE_ASSERT (position != std::string::npos);
    URI_string.erase (position + 1, std::string::npos);
    URI_string_2 = URI_string;
    URI_string_2 += current_URL;
    URI_string = URI_string_2;
  } // end IF

  // send HTTP request
  ACE_ASSERT (data_p->handle != ACE_INVALID_HANDLE);
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_2->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
      iconnection_manager_2->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  istream_connection_p =
      dynamic_cast<Test_I_IStreamConnection_2_t*> (iconnection_p);
  ACE_ASSERT (istream_connection_p);
  const Test_I_IStreamConnection_2_t::STREAM_T& stream_r =
      istream_connection_p->stream ();

  ACE_NEW_NORETURN (HTTP_record_p,
                    struct HTTP_Record ());
  if (!HTTP_record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  HTTP_record_p->form = HTTP_form;
  HTTP_headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                       hostname_string));
  HTTP_record_p->headers = HTTP_headers;
  HTTP_record_p->method =
      (HTTP_form.empty () ? HTTP_Codes::HTTP_METHOD_GET
                          : HTTP_Codes::HTTP_METHOD_POST);
  HTTP_record_p->URI = URI_string;
  HTTP_record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

  ACE_NEW_NORETURN (message_data_p,
                    Test_I_Message::DATA_T ());
  if (!message_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    delete HTTP_record_p; HTTP_record_p = NULL;
    return G_SOURCE_REMOVE;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (HTTP_record_p)
  message_data_p->setPR (HTTP_record_p);

  ACE_ASSERT ((*iterator_2).second->allocatorConfiguration);
  ACE_ASSERT (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator);
allocate:
  message_p =
      static_cast<Test_I_Message*> (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator->malloc ((*iterator_2).second->allocatorConfiguration->defaultBufferSize));
  // keep retrying ?
  if (!message_p &&
      !static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
    delete message_data_p; message_data_p = NULL;
    return G_SOURCE_REMOVE;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  message_p->initialize (message_data_p,
                         stream_r.getR_2 ().getR ().sessionId,
                         NULL);

  message_block_p = message_p;
  istream_connection_p->send (message_block_p);
  message_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("requesting: \"%s\"\n"),
              ACE_TEXT (URI_string.c_str ())));

  // clean up
  iconnection_p->decrease (); iconnection_p = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_video_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_video_display_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)),
                              NULL,
                              false);

  return G_SOURCE_CONTINUE;
}

gboolean
idle_start_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_start_session_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_ConnectionManager_2_t::INTERFACE_T* iconnection_manager_2 =
      TEST_I_CONNECTIONMANAGER_SINGLETON_2::instance ();
  ACE_ASSERT (iconnection_manager_2);
  Net_ConnectionConfigurationsIterator_t iterator_2 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("2"));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  Test_I_WebTV_StreamConfiguration_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator =
      data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  ACE_ASSERT (!(*channel_iterator).second.segment.URLs.empty ());

  // close connection
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  ACE_ASSERT (data_p->handle);
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
      iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_p)
  {
    iconnection_p->close();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF
  data_p->handle = ACE_INVALID_HANDLE;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_OS::sleep (2);
#endif // ACE_WIN32 || ACE_WIN64

  Test_I_TCPConnector_2_t connector (true);
#if defined (SSL_SUPPORT)
  Test_I_SSLConnector_2_t ssl_connector (true);
#endif // SSL_SUPPORT
  Test_I_AsynchTCPConnector_2_t asynch_connector (true);
  Test_I_ConnectionManager_2_t::ICONNECTION_T* iconnection_2 = NULL;
  Test_I_IStreamConnection_2_t* istream_connection_p = NULL;
  HTTP_Form_t HTTP_form;
  HTTP_Headers_t HTTP_headers;
  struct HTTP_Record* HTTP_record_p = NULL;
  Test_I_Message::DATA_T* message_data_p = NULL;
  Test_I_Message* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string, URI_string_2, URL_string;
  bool use_SSL = false;
  struct Net_UserData user_data_s;
  ACE_ASSERT (!(*channel_iterator).second.segment.URLs.empty ());
  std::string current_URL =
      (*channel_iterator).second.segment.URLs.front ();
  (*channel_iterator).second.segment.URLs.pop_front ();
  bool is_URI_b = HTTP_Tools::URLIsURI (current_URL);

  if (is_URI_b)
    URL_string = (*iterator_3).second.second->URL;
  else
    URL_string = current_URL;
  if (!HTTP_Tools::parseURL (URL_string,
                             host_address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                ACE_TEXT ((*iterator_3).second.second->URL.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address =
      host_address;
  static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.hostname =
      hostname_string;

  if (is_URI_b)
  {
    size_t position = URI_string.find_last_of ('/', std::string::npos);
    ACE_ASSERT (position != std::string::npos);
    URI_string.erase (position + 1, std::string::npos);
    URI_string_2 = URI_string;
    URI_string_2 += current_URL;
    URI_string = URI_string_2;
  } // end IF

  // connect to peer
  if (data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      data_p->handle =
          Net_Client_Common_Tools::connect (ssl_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    else
#endif // SSL_SUPPORT
      data_p->handle =
          Net_Client_Common_Tools::connect (connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    data_p->handle =
        Net_Client_Common_Tools::connect (asynch_connector,
                                          *static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second),
                                          user_data_s,
                                          static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address,
                                          false, // wait ?
                                          true); // peer address ?
  } // end ELSE
  if (data_p->handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: opened socket to %s\n"),
              data_p->handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: opened socket to %s\n"),
              data_p->handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_2_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  //ACE_ASSERT (!data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    data_p->progressData.eventSourceId =
    //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
    //                 idle_update_progress_cb,
    //                 &data_p->progressData,
    //                 NULL);
    g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                    COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                    idle_update_progress_cb,
                    &data_p->progressData);//,
//                       NULL);
    if (data_p->progressData.eventSourceId > 0)
      state_r.eventSourceIds.insert (data_p->progressData.eventSourceId);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end IF
  } // end lock scope

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    data_p->videoUpdateEventSourceId =
    g_timeout_add (COMMON_UI_REFRESH_DEFAULT_VIDEO_MS, // ms (?)
                   idle_update_video_display_cb,
                   userData_in);
    ACE_ASSERT (data_p->videoUpdateEventSourceId > 0);
    state_r.eventSourceIds.insert (data_p->videoUpdateEventSourceId);
  } // end lock scope

  // send HTTP request
  ACE_ASSERT (data_p->handle != ACE_INVALID_HANDLE);
  iconnection_2 =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_2->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
      iconnection_manager_2->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iconnection_2);
  istream_connection_p =
      dynamic_cast<Test_I_IStreamConnection_2_t*> (iconnection_2);
  ACE_ASSERT (istream_connection_p);
  const Test_I_IStreamConnection_2_t::STREAM_T& stream_r =
      istream_connection_p->stream ();
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (stream_r.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING),
                                                 false,
                                                 false));
  ACE_ASSERT (module_p);
  data_p->dispatch = dynamic_cast<Common_IDispatch*> (module_p->writer ());
  ACE_ASSERT (data_p->dispatch);

  ACE_NEW_NORETURN (HTTP_record_p,
                    struct HTTP_Record ());
  if (!HTTP_record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  HTTP_record_p->form = HTTP_form;
  HTTP_headers.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                       hostname_string));
  HTTP_record_p->headers = HTTP_headers;
  HTTP_record_p->method =
      (HTTP_form.empty () ? HTTP_Codes::HTTP_METHOD_GET
                          : HTTP_Codes::HTTP_METHOD_POST);
  HTTP_record_p->URI = URI_string;
  HTTP_record_p->version = HTTP_Codes::HTTP_VERSION_1_1;

  ACE_NEW_NORETURN (message_data_p,
                    Test_I_Message::DATA_T ());
  if (!message_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    delete HTTP_record_p; HTTP_record_p = NULL;
    return G_SOURCE_REMOVE;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (HTTP_record_p)
  message_data_p->setPR (HTTP_record_p);

  ACE_ASSERT ((*iterator_2).second->allocatorConfiguration);
  ACE_ASSERT (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator);
allocate:
  message_p =
      static_cast<Test_I_Message*> (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator->malloc ((*iterator_2).second->allocatorConfiguration->defaultBufferSize));
  // keep retrying ?
  if (!message_p &&
      !static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->messageAllocator->block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_I_Message: \"%m\", aborting\n")));
    delete message_data_p; message_data_p = NULL;
    return G_SOURCE_REMOVE;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  message_p->initialize (message_data_p,
                         stream_r.getR_2 ().getR ().sessionId,
                         NULL);

  message_block_p = message_p;
  istream_connection_p->send (message_block_p);
  message_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("requesting: \"%s\"\n"),
              ACE_TEXT (URI_string.c_str ())));

  // clean up
  iconnection_2->decrease (); iconnection_2 = NULL;

  return G_SOURCE_REMOVE;
}

#if GTK_CHECK_VERSION (3,0,0)
gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  ACE_UNUSED_ARG (widget_in);

  ACE_ASSERT (context_in);
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  if (!data_p->dispatch)
    return FALSE; // propagate further

  try {
    data_p->dispatch->dispatch (context_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  gtk_widget_queue_draw (widget_in);

  return FALSE; // propagate further
}
#else
gboolean
drawingarea_expose_event_cb (GtkWidget* widget_in,
                             GdkEvent* event_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_expose_event_cb"));

  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  if (!data_p->dispatch)
    return FALSE;

  // sanity check(s)
  cairo_t* context_p =
    gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget_in)));
  if (unlikely (!context_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
    return FALSE;
  } // end IF

  try {
    data_p->dispatch->dispatch (context_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), continuing\n")));
  }

  cairo_destroy (context_p); context_p = NULL;

  return FALSE; // propagate
} // drawingarea_expose_event_cb
#endif // GTK_CHECK_VERSION(3,0,0)

gboolean
drawing_area_resize_end (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawing_area_resize_end"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkToggleButton* toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME)));
  ACE_ASSERT (toggle_button_p);
  GtkDrawingArea* drawing_area_p =
      GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                                (gtk_toggle_button_get_active (toggle_button_p) ? ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_FULLSCREEN_NAME)
                                                                                                : ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DRAWINGAREA_NAME))));
  ACE_ASSERT (drawing_area_p);
  Test_I_ConnectionManager_2_t::INTERFACE_T* iconnection_manager_2 =
      TEST_I_CONNECTIONMANAGER_SINGLETON_2::instance ();
  ACE_ASSERT (iconnection_manager_2);

  GtkAllocation allocation_s;
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation_s);
  ACE_DEBUG ((LM_DEBUG,
             ACE_TEXT ("window resized to %dx%d\n"),
             allocation_s.width, allocation_s.height));

  Test_I_WebTV_StreamConfiguration_2_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration_2.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  (*iterator_3).second.second->outputFormat.video.resolution.cy =
    allocation_s.height;
  (*iterator_3).second.second->outputFormat.video.resolution.cx =
    allocation_s.width;
#else
  (*iterator_3).second.second->outputFormat.video.resolution.height =
      allocation_s.height;
  (*iterator_3).second.second->outputFormat.video.resolution.width =
      allocation_s.width;
#endif // ACE_WIN32 || ACE_WIN64

  if (data_p->handle == ACE_INVALID_HANDLE)
    return FALSE;
  Test_I_ConnectionManager_2_t::ICONNECTION_T* iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      iconnection_manager_2->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
      iconnection_manager_2->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iconnection_p);
  Test_I_IStreamConnection_2_t* istream_connection_p =
      dynamic_cast<Test_I_IStreamConnection_2_t*> (iconnection_p);
  ACE_ASSERT (istream_connection_p);
  Test_I_IStreamConnection_2_t::STREAM_T& stream_r =
      const_cast<Test_I_IStreamConnection_2_t::STREAM_T&> (istream_connection_p->stream ());
  if (!stream_r.isRunning ())
    return FALSE;

  // *NOTE*: two things need doing:
  //         - drop inbound frames until the 'resize' session message is through
  //         - enqueue a 'resize' session message

  // step1:
  const Stream_Module_t* module_p =
      stream_r.find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING));
  ACE_ASSERT (module_p);
  Stream_Visualization_IResize* iresize_p =
      dynamic_cast<Stream_Visualization_IResize*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  ACE_ASSERT (iresize_p);
  try {
    iresize_p->resizing ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("caught exception in Stream_Visualization_IResize::resizing(), aborting\n")));
    return FALSE;
  }

  // step2
  stream_r.control (STREAM_CONTROL_RESIZE, false);

  return FALSE;
} // drawing_area_resize_end

void
drawingarea_size_allocate_cb (GtkWidget* widget_in,
                              GdkRectangle* allocation_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::drawingarea_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (allocation_in);

  static gint timer_id = 0;
  if (timer_id == 0)
  {
    timer_id = g_timeout_add (300, drawing_area_resize_end, userData_in);
    return;
  } // end IF
  g_source_remove (timer_id);
  timer_id = g_timeout_add (300, drawing_area_resize_end, userData_in);
} // drawingarea_size_allocate_cb

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_ProgressData* data_p =
    static_cast<struct Test_I_WebTV_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_TCHAR buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
  int result = -1;
  float speed = 0.0F;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_CONTINUE);
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

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_CONNECT:
        case COMMON_UI_EVENT_DISCONNECT:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_CONNECTIONS_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_spin (spin_button_p,
                                ((*event_p == COMMON_UI_EVENT_CONNECT) ? GTK_SPIN_STEP_FORWARD
                                                                       : GTK_SPIN_STEP_BACKWARD),
                                1.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STARTED:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BYTES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_BYTES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.bytes));

          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);

          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        case COMMON_UI_EVENT_ABORT:
        case COMMON_UI_EVENT_STEP:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
              GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINBUTTON_SESSION_NAME)));
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
    while (!state_r.eventStack.is_empty ())
    {
      result = state_r.eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

//gboolean
//idle_update_log_display_cb (gpointer userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));
//
//  struct Test_I_WebTV_UI_CBData* data_p =
//      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//
//  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);
//
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//      data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != data_p->builders.end ());
//
//  GtkTextView* view_p =
//    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
//                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TEXTVIEW_NAME)));
//  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
//
//  GtkTextIter text_iterator;
//  gtk_text_buffer_get_end_iter (buffer_p,
//                                &text_iterator);
//
//  gchar* string_p = NULL;
//  // sanity check
//  if (data_p->logStack.empty ())
//    return G_SOURCE_CONTINUE;
//
//  // step1: convert text
//  for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
//       iterator_2 != data_p->logStack.end ();
//       iterator_2++)
//  {
//    string_p = Common_UI_GTK_Tools::Locale2UTF8 (*iterator_2);
//    if (!string_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
//                  ACE_TEXT ((*iterator_2).c_str ())));
//      return G_SOURCE_REMOVE;
//    } // end IF
//
//    // step2: display text
//    gtk_text_buffer_insert (buffer_p,
//                            &text_iterator,
//                            string_p,
//                            -1);
//
//    // clean up
//    g_free (string_p);
//  } // end FOR
//
//  data_p->logStack.clear ();
//
//  // step3: scroll the view accordingly
////  // move the iterator to the beginning of line, so it doesn't scroll
////  // in horizontal direction
////  gtk_text_iter_set_line_offset (&text_iterator, 0);
//
////  // ...and place the mark at iter. The mark will stay there after insertion
////  // because it has "right" gravity
////  GtkTextMark* text_mark_p =
////      gtk_text_buffer_get_mark (buffer_p,
////                                ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SCROLLMARK_NAME));
//////  gtk_text_buffer_move_mark (buffer_p,
//////                             text_mark_p,
//////                             &text_iterator);
//
////  // scroll the mark onscreen
////  gtk_text_view_scroll_mark_onscreen (view_p,
////                                      text_mark_p);
//  //GtkAdjustment* adjustment_p =
//  //    GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
//  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_ADJUSTMENT_NAME)));
//  //ACE_ASSERT (adjustment_p);
//  //gtk_adjustment_set_value (adjustment_p,
//  //                          adjustment_p->upper - adjustment_p->page_size));
//
//  return G_SOURCE_CONTINUE;
//}

/////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
togglebutton_play_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::togglebutton_play_toggled_cb"));

  if (un_toggling_play)
  {
    un_toggling_play = false;
    return;
  } // end IF

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);

  Test_I_ConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
  bool success = false;

  if (gtk_toggle_button_get_active (toggleButton_in))
  {
    // --> download segment playlist

    // step1: update widgets
    gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                          GTK_STOCK_MEDIA_STOP);
    GtkFrame* frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CHANNEL_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);
    frame_p =
        GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME)));
    ACE_ASSERT (frame_p);
    gtk_widget_set_sensitive (GTK_WIDGET (frame_p), FALSE);

    // step2: update configuration
    std::string URI_string, URI_string_2;
    GtkToggleButton* toggle_button_p = NULL;
    GtkFileChooserButton* file_chooser_button_p = NULL;
    Net_ConnectionConfigurationsIterator_t iterator_2 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
    Test_I_WebTV_StreamConfiguration_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());
    Test_I_WebTV_StreamConfiguration_t::ITERATOR_T iterator_4 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR ("Marshal"));
    ACE_ASSERT (iterator_4 != data_p->configuration->streamConfiguration.end ());
    (*iterator_3).second.second->parserConfiguration->messageQueue = NULL;
    (*iterator_4).second.second->parserConfiguration->messageQueue = NULL;
    Test_I_TCPConnector_t connector (true);
#if defined (SSL_SUPPORT)
    Test_I_SSLConnector_t ssl_connector (true);
#endif // SSL_SUPPORT
    Test_I_AsynchTCPConnector_t asynch_connector (true);
    HTTP_Form_t HTTP_form;
    HTTP_Headers_t HTTP_headers;
//    GtkSpinner* spinner_p = NULL;
    GtkProgressBar* progressbar_p = NULL;
    GtkTreeIter iterator_5;
#if GTK_CHECK_VERSION (2,30,0)
    struct _GValue value = G_VALUE_INIT;
#else
    struct _GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
    Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator;
    ACE_INET_Addr host_address;
    std::string hostname_string;
    bool use_SSL = false;
    struct Net_UserData user_data_s;

    // retrieve stream URL
    GtkComboBox* combo_box_p =
        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
    ACE_ASSERT (combo_box_p);
    if (!gtk_combo_box_get_active_iter (combo_box_p,
                                        &iterator_5))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
      return;
    } // end IF
    GtkListStore* list_store_p =
        GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
    ACE_ASSERT (list_store_p);
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_5,
                              1,
                              &value);
    ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.cx =
        g_value_get_uint (&value);
#else
    data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.width =
        g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
    g_value_unset (&value);
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_5,
                              2, &value);
    ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.cy =
        g_value_get_uint (&value);
#else
    data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.height =
        g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
    g_value_unset (&value);
    ACE_ASSERT (data_p->channels);
    ACE_ASSERT (data_p->currentChannel);
    channel_iterator = data_p->channels->find (data_p->currentChannel);
    ACE_ASSERT (channel_iterator != data_p->channels->end ());
    for (Test_I_WebTV_ChannelResolutionsConstIterator_t iterator_5 = (*channel_iterator).second.resolutions.begin ();
         iterator_5 != (*channel_iterator).second.resolutions.end ();
         ++iterator_5)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (((*iterator_5).resolution.cx == data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.cx) &&
          ((*iterator_5).resolution.cy == data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.cy))
#else
      if (((*iterator_5).resolution.width == data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.width) &&
          ((*iterator_5).resolution.height == data_p->configuration->streamConfiguration_2.configuration_->mediaType.video.resolution.height))
#endif // ACE_WIN32 || ACE_WIN64
        URI_string = (*iterator_5).URI;
    ACE_ASSERT (!URI_string.empty ());
    (*iterator_3).second.second->URL.clear ();
    if (!HTTP_Tools::parseURL ((*channel_iterator).second.mainURL,
                               host_address,
                               hostname_string,
                               URI_string_2,
                               use_SSL))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                 ACE_TEXT ((*iterator_3).second.second->URL.c_str ())));
      return;
    } // end IF
    (*iterator_3).second.second->URL = ACE_TEXT_ALWAYS_CHAR ("http");
    (*iterator_3).second.second->URL +=
        (use_SSL ? ACE_TEXT_ALWAYS_CHAR ("s") : ACE_TEXT_ALWAYS_CHAR (""));
    (*iterator_3).second.second->URL += ACE_TEXT_ALWAYS_CHAR ("://");
    (*iterator_3).second.second->URL += hostname_string;
    size_t position = URI_string_2.find_last_of ('/', std::string::npos);
    ACE_ASSERT (position != std::string::npos);
    URI_string_2.erase (position + 1, std::string::npos);
    (*iterator_3).second.second->URL += URI_string_2;
    (*iterator_3).second.second->URL += URI_string;
    static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address =
        host_address;
    static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.hostname =
        hostname_string;

    // save to file ?
    toggle_button_p =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME)));
    ACE_ASSERT (toggle_button_p);
    if (!gtk_toggle_button_get_active (toggle_button_p))
    {
      (*iterator_3).second.second->targetFileName.clear ();
      goto continue_;
    } // end IF
    file_chooser_button_p =
        GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                         ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
    ACE_ASSERT (file_chooser_button_p);
    (*iterator_3).second.second->targetFileName =
      gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p));
//    ACE_ASSERT (Common_File_Tools::isDirectory ((*iterator_3).second.second->targetFileName));

    // step3: connect to peer
    if (data_p->configuration->dispatchConfiguration.numberOfReactorThreads > 0)
    {
#if defined (SSL_SUPPORT)
      if (use_SSL)
        data_p->handle =
            Net_Client_Common_Tools::connect (ssl_connector,
                                              *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                              user_data_s,
                                              static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                              false, // wait ?
                                              true); // peer address ?
      else
#endif // SSL_SUPPORT
        data_p->handle =
            Net_Client_Common_Tools::connect (connector,
                                              *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                              user_data_s,
                                              static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                              false, // wait ?
                                              true); // peer address ?
    } // end IF
    else
    {
#if defined (SSL_SUPPORT)
      // *TODO*: add SSL support to the proactor framework
      ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
      data_p->handle =
          Net_Client_Common_Tools::connect (asynch_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    } // end ELSE
    if (data_p->handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
      goto error;
    } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("0x%@: opened socket to %s\n"),
                data_p->handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: opened socket to %s\n"),
                data_p->handle,
                ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

    success = true;

continue_:
    if (!success)
      goto error;

    // step3: start progress reporting
//    spinner_p =
//      GTK_SPINNER (gtk_builder_get_object ((*iterator).second.second,
//                                           ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_SPINNER_NAME)));
//    ACE_ASSERT (spinner_p);
//    gtk_widget_set_sensitive (GTK_WIDGET (spinner_p), TRUE);
//    gtk_spinner_start (spinner_p);
    progressbar_p =
        GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                  ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_PROGRESSBAR_NAME)));
    ACE_ASSERT (progressbar_p);
    gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), TRUE);

    //ACE_ASSERT (!data_p->progressData.eventSourceId);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      data_p->progressData.eventSourceId =
        //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
        //                 idle_update_progress_cb,
        //                 &data_p->progressData,
        //                 NULL);
        g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                       COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                       idle_update_progress_cb,
                       &data_p->progressData);//,
//                       NULL);
      if (data_p->progressData.eventSourceId > 0)
        state_r.eventSourceIds.insert (data_p->progressData.eventSourceId);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", aborting\n")));
        goto error;
      } // end IF
    } // end lock scope

    return;
  } // end IF

  // --> disconnect

  ACE_ASSERT (data_p->handle != ACE_INVALID_HANDLE);
  iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (data_p->handle));
#else
    iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (data_p->handle));
#endif // ACE_WIN32 || ACE_WIN64
  if (iconnection_p)
  {
    iconnection_p->close ();
    iconnection_p->decrease (); iconnection_p = NULL;
  } // end IF
  data_p->handle = ACE_INVALID_HANDLE;
  iconnection_manager_p->abort ();

  return;

error:
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_MEDIA_PLAY);
//  box_p =
//    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
//                                     ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_VBOX_CONFIGURATION_NAME)));
//  ACE_ASSERT (box_p);
//  gtk_widget_set_sensitive (GTK_WIDGET (box_p), TRUE);

  un_toggling_play = true;
  gtk_toggle_button_set_active (toggleButton_in, FALSE);
} // toggle_button_connect_toggled_cb

gint
button_load_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_load_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (iconnection_manager_p);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator;
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  channel_iterator = data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());
  Test_I_WebTV_StreamConfiguration_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration.end ());

  gtk_widget_set_sensitive (widget_in, FALSE);

  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string, URL_string;
  bool use_SSL = false;
  if (!HTTP_Tools::parseURL ((*channel_iterator).second.mainURL,
                             host_address,
                             hostname_string,
                             URI_string,
                             use_SSL))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
               ACE_TEXT ((*channel_iterator).second.mainURL.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
  (*iterator_3).second.second->URL = (*channel_iterator).second.mainURL;
  Net_ConnectionConfigurationsIterator_t iterator_2 =
      data_p->configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->connectionConfigurations.end ());
  static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address =
      host_address;
  static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.hostname =
      hostname_string;

  // select connector
  Test_I_TCPConnector_t connector (true);
#if defined (SSL_SUPPORT)
  Test_I_SSLConnector_t ssl_connector (true);
#endif // SSL_SUPPORT
  Test_I_AsynchTCPConnector_t asynch_connector (true);
  struct Net_UserData user_data_s;
//  ACE_ASSERT (!data_p->handle)
  if (data_p->configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
  {
#if defined (SSL_SUPPORT)
    if (use_SSL)
      data_p->handle =
          Net_Client_Common_Tools::connect (ssl_connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
    else
#endif // SSL_SUPPORT
      data_p->handle =
          Net_Client_Common_Tools::connect (connector,
                                            *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                            user_data_s,
                                            static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                            false, // wait ?
                                            true); // peer address ?
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (!use_SSL);
#endif // SSL_SUPPORT
    data_p->handle =
        Net_Client_Common_Tools::connect (asynch_connector,
                                          *static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second),
                                          user_data_s,
                                          static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
                                          false, // wait ?
                                          true); // peer address ?
  } // end ELSE
//    iconnection_p =
//        iconnection_manager_p->get (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address,
//                                    true);
  if (data_p->handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to connect to %s, aborting\n"),
               ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("0x%@: opened TCP socket to %s\n"),
              data_p->handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: opened TCP socket to %s\n"),
              data_p->handle,
              ACE_TEXT (Net_Common_Tools::IPAddressToString (static_cast<Test_I_WebTV_ConnectionConfiguration_t*> ((*iterator_2).second)->socketConfiguration.address).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (!data_p->progressData.eventSourceId);
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, FALSE);
    data_p->progressData.eventSourceId =
                                         //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                                         //                 idle_update_progress_cb,
                                         //                 &data_p->progressData,
                                         //                 NULL);
        g_timeout_add (//G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                       COMMON_UI_REFRESH_DEFAULT_PROGRESS_MS, // ms (?)
                       idle_update_progress_cb,
                       &data_p->progressData);//,
               //                       NULL);
    ACE_ASSERT (data_p->progressData.eventSourceId > 0);
    state_r.eventSourceIds.insert (data_p->progressData.eventSourceId);
  } // end lock scope

  return FALSE;
} // button_load_clicked_cb

void
combobox_resolution_changed_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  Test_I_WebTV_StreamConfiguration_2_t::ITERATOR_T iterator_3 =
      data_p->configuration->streamConfiguration_2.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_3 != data_p->configuration->streamConfiguration_2.end ());
  ACE_ASSERT ((*iterator_3).second.second->delayConfiguration);
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator;
  ACE_ASSERT (data_p->channels);
  ACE_ASSERT (data_p->currentChannel);
  channel_iterator = data_p->channels->find (data_p->currentChannel);
  ACE_ASSERT (channel_iterator != data_p->channels->end ());

  GtkTreeIter iterator_2;
  gboolean result = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                                   &iterator_2);
  ACE_ASSERT (result);
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION (2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
  Common_Image_Resolution_t resolution_s;
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  resolution_s.cx = g_value_get_uint (&value);
#else
  resolution_s.width = g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  resolution_s.cy = g_value_get_uint (&value);
#else
  resolution_s.height = g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  data_p->currentStream = resolution_s.cx;
#else
  data_p->currentStream = resolution_s.width;
#endif // ACE_WIN32 || ACE_WIN64

  // set corresponding video framerate
  for (Test_I_WebTV_ChannelResolutionsConstIterator_t iterator_2 = (*channel_iterator).second.resolutions.begin ();
       iterator_2 != (*channel_iterator).second.resolutions.end ();
       ++iterator_2)
    if ((*iterator_2).resolution.width == resolution_s.width)
    {
      (*iterator_3).second.second->delayConfiguration->averageTokensPerInterval =
          (*iterator_2).frameRate;
      break;
    } // end IF

} // combobox_resolution_changed_cb

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Test_I_WebTV_UI_CBData* data_p =
      static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_I_UI_GTK_DIALOG_ABOUT_NAME)));
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
  struct Test_I_WebTV_UI_CBData* data_p =
    static_cast<struct Test_I_WebTV_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // step1: remove all event sources
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, state_r.lock, FALSE);
    for (Common_UI_GTK_EventSourceIdsIterator_t iterator = state_r.eventSourceIds.begin ();
         iterator != state_r.eventSourceIds.end ();
         iterator++)
      if (!g_source_remove (*iterator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    *iterator));
    state_r.eventSourceIds.clear ();
  } // end lock scope

  // step2: initiate shutdown sequence
  int result = -1;
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
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  return FALSE;
} // button_quit_clicked_cb
#ifdef __cplusplus
}
#endif /* __cplusplus */
