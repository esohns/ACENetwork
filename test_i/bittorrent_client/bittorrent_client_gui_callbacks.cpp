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

#include "bittorrent_client_gui_callbacks.h"

#include "ace/ACE.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_macros.h"

#include "http_scanner.h"

#include "bittorrent_common.h"
#include "bittorrent_icontrol.h"
#include "bittorrent_tools.h"

#include "stream_misc_messagehandler.h"
#include "bittorrent_client_configuration.h"
#include "bittorrent_client_defines.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"
#include "bittorrent_client_tools.h"

#include "bittorrent_client_gui_common.h"
#include "bittorrent_client_gui_defines.h"
#include "bittorrent_client_gui_session.h"
#include "bittorrent_client_gui_tools.h"

ACE_THR_FUNC_RETURN
session_handler_cb (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::session_handler_cb"));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = -1;
#else
  result = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionThreadData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionThreadData*> (arg_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->CBData);
  ACE_ASSERT (data_p->configuration);

  guint context_id = 0;
  GtkStatusbar* statusbar_p = NULL;
  std::string label_string =
    ACE_TEXT_ALWAYS_CHAR (ACE::basename (data_p->filename.c_str ()));

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
wait_retry:
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, NULL);
    Common_UI_GTK_BuildersIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    // sanity check(s)
    ACE_ASSERT (iterator != state_r.builders.end ());

    if (!data_p->eventSourceId)
      goto wait_retry;

  // retrieve progress bar handle
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

    // generate context id
    statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR)));
    ACE_ASSERT (statusbar_p);
    context_id =
      gtk_statusbar_get_context_id (statusbar_p,
                                    label_string.c_str ());
                                    //string_p);

#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (data_p->eventSourceId);

  // *NOTE*: this schedules addition of a new session page
  //         --> make sure to remove it again if things go wrong
  BitTorrent_Client_GUI_Session_t* session_p = NULL;

  ACE_NEW_NORETURN (session_p,
                    BitTorrent_Client_GUI_Session_t (*data_p->CBData,
                                                     context_id,
                                                     label_string,
                                                     data_p->CBData->UIFileDirectory,
                                                     data_p->controller,
                                                     data_p->filename));
  if (!session_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    goto clean;
  } // end IF
  // *WARNING*: beyond this point, need to remove the session page !
  //            --> goto remove_page

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, NULL);
    data_p->CBData->sessions.insert (std::make_pair (label_string, session_p));
    data_p->CBData->progressData.completedActions.insert (data_p->eventSourceId);
  } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

clean:
  // clean up
  delete data_p; data_p = NULL;

  return result;
}

void
is_entry_sensitive (GtkCellLayout*   layout_in,
                    GtkCellRenderer* renderer_in,
                    GtkTreeModel*    model_in,
                    GtkTreeIter*     iter_in,
                    gpointer         data_in)
{
  //NETWORK_TRACE (ACE_TEXT ("::is_entry_sensitive"));

  ACE_UNUSED_ARG (layout_in);
  ACE_UNUSED_ARG (data_in);

  gboolean sensitive = !gtk_tree_model_iter_has_child (model_in, iter_in);
  // set corresponding property
  g_object_set (renderer_in,
                ACE_TEXT_ALWAYS_CHAR ("sensitive"), sensitive,
                NULL);
}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
gboolean
idle_add_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_add_session_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionCBData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->label);
  ACE_ASSERT (iterator != state_r.builders.end ());
  Common_UI_GTK_BuildersIterator_t iterator_2 =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator_2 != state_r.builders.end ());

  GtkWindow* window_p = NULL;
  GtkHBox* hbox_p = NULL, *hbox_2 = NULL;
  GtkLabel* label_p = NULL;
  GtkVBox* vbox_p = NULL;
  GtkNotebook* notebook_p = NULL;
  GtkTable* table_p = NULL;
  gint page_number = -1;

  // add new page to the sessions notebook
  // retrieve (dummy) parent window
  window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_WINDOW_TAB_SESSION)));
  ACE_ASSERT (window_p);
  // retrieve session tab label
  hbox_p =
    GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_HBOX_TAB_SESSION)));
  ACE_ASSERT (hbox_p);
  g_object_ref (hbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (hbox_p));
  // set tab label
  label_p =
    GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_LABEL_TAB_SESSION)));
  ACE_ASSERT (label_p);
  gtk_label_set_text (label_p,
                      data_p->label.c_str ());

  // retrieve (dummy) parent window
  window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_WINDOW_SESSION)));
  ACE_ASSERT (window_p);
  // retrieve session tab
  hbox_2 =
    GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_HBOX_PAGE_SESSION)));
  ACE_ASSERT (hbox_2);
  g_object_ref (hbox_2);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (hbox_2));

  notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator_2).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_SESSIONS)));
  ACE_ASSERT (notebook_p);
  page_number = gtk_notebook_append_page (notebook_p,
                                          GTK_WIDGET (hbox_2),
                                          GTK_WIDGET (hbox_p));
  if (page_number == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_notebook_append_page(%@), aborting\n"),
                notebook_p));

    // clean up
    g_object_unref (hbox_p);
    g_object_unref (hbox_2);

    return G_SOURCE_REMOVE;
  } // end IF
  g_object_unref (hbox_p);
  g_object_unref (hbox_2);

  // allow reordering
  gtk_notebook_set_tab_reorderable (notebook_p,
                                    GTK_WIDGET (vbox_p),
                                    TRUE);

  gtk_notebook_set_current_page (notebook_p,
                                 page_number);

  // initialize pieces map
  table_p =
    GTK_TABLE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TABLE_PIECES)));
  ACE_ASSERT (table_p);
  unsigned int number_of_pieces_i = data_p->session->numberOfPieces ();
  unsigned int number_of_colummns_and_rows_i =
    static_cast<unsigned int> (std::ceil (std::sqrt (static_cast<float> (number_of_pieces_i))));
  gtk_table_resize (table_p,
                    number_of_colummns_and_rows_i,
                    number_of_colummns_and_rows_i);
  int x, y;
  GtkButton* button_p = NULL;
#if defined (GTK2_USE)
  GdkColor black = {0, 0x0000, 0x0000, 0x0000};
#elif defined (GTK3_USE)
  GdkRGBA black = { 0.0, 0.0, 0.0, 1.0 };

  GtkCssProvider* css_provider_p = gtk_css_provider_new ();
  ACE_ASSERT (css_provider_p);
  // Load CSS into the object ("-1" says, that the css string is \0-terminated)
  gtk_css_provider_load_from_data (css_provider_p,
                                   ACE_TEXT ("* { background-image:none; background-color:black; }"), -1,
                                   NULL);
#endif // GTK2_USE || GTK3_USE
  for (unsigned int i = 0;
       i < number_of_pieces_i;
       ++i)
  {
    button_p = GTK_BUTTON (gtk_button_new ());
    ACE_ASSERT (button_p);
    g_object_set_data (G_OBJECT (button_p),
                       ACE_TEXT ("index"),
                       reinterpret_cast<gpointer> (i));

    gtk_widget_set_visible (GTK_WIDGET (button_p), TRUE); // *TODO*: why ?
#if defined (GTK2_USE)
    gtk_widget_modify_bg (GTK_WIDGET (button_p), GTK_STATE_NORMAL, &black);
    gtk_widget_modify_bg (GTK_WIDGET (button_p), GTK_STATE_PRELIGHT, &black);
    gtk_widget_modify_bg (GTK_WIDGET (button_p), GTK_STATE_ACTIVE, &black);
#elif defined (GTK3_USE)
    //gtk_widget_override_background_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_NORMAL, &black);
    //gtk_widget_override_background_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_PRELIGHT, &black);
    //gtk_widget_override_background_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_ACTIVE, &black);
    //gtk_widget_override_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_NORMAL, &black);
    //gtk_widget_override_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_PRELIGHT, &black);
    //gtk_widget_override_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_ACTIVE, &black);

    gtk_style_context_add_provider (gtk_widget_get_style_context (GTK_WIDGET (button_p)),
                                    GTK_STYLE_PROVIDER (css_provider_p),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif // GTK2_USE || GTK3_USE
    x = i % number_of_colummns_and_rows_i;
    y = i / number_of_colummns_and_rows_i;
    gtk_table_attach_defaults (table_p,
                               GTK_WIDGET (button_p),
                               x, x + 1,
                               y, y + 1);
  } // end FOR
#if defined (GTK3_USE)
  g_object_unref (css_provider_p); css_provider_p = NULL;
#endif // GTK3_USE

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_CBData* data_p =
    static_cast<struct BitTorrent_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // step1: remove any remaining event sources
  // *NOTE*: should be 2: 'this', and idle_update_display_cb...
  unsigned int removed_events = 0;
  while (g_idle_remove_by_data (userData_in))
    ++removed_events;
  if (removed_events)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("removed %u queued event(s)...\n"),
                removed_events));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    //ACE_ASSERT (removed_events == data_p->state.eventSourceIds.size ());
    state_r.eventSourceIds.clear ();
  } // end lock scope

  // step2: leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_CBData* data_p =
    static_cast<struct BitTorrent_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step2: retrieve toplevel handle
  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_WINDOW_MAIN)));
  ACE_ASSERT (window_p);

  // connect default signals
  gulong result = g_signal_connect (window_p,
                                    ACE_TEXT_ALWAYS_CHAR ("delete-event"),
                                    G_CALLBACK (button_quit_clicked_cb),
                                    window_p);
  ACE_ASSERT (result);
  //   g_signal_connect(window_p,
  //                    ACE_TEXT_ALWAYS_CHAR("destroy-event"),
  //                    G_CALLBACK(quit_activated_cb),
  //                    window);
  result = g_signal_connect (window_p,
                             ACE_TEXT_ALWAYS_CHAR ("destroy"),
                             G_CALLBACK (gtk_widget_destroyed),
                             window_p);
  ACE_ASSERT (result);

  GtkFileChooserButton *file_chooser_button_p = NULL;
  if (!data_p->configuration->sessionConfiguration.metaInfoFileName.empty ())
  {
    file_chooser_button_p =
        GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                         ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_FILECHOOSERBUTTON_TORRENT)));
    ACE_ASSERT (file_chooser_button_p);
    GFile* file_p =
      g_file_new_for_path (data_p->configuration->sessionConfiguration.metaInfoFileName.c_str ());
    ACE_ASSERT (file_p);
    GError* error_p = NULL;
    if (!gtk_file_chooser_select_file (GTK_FILE_CHOOSER (file_chooser_button_p),
                                       file_p,
                                       &error_p))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to gtk_file_chooser_select_file(\"%s\"): \"%s\", continuing\n"),
                 ACE_TEXT (g_file_get_path (file_p)),
                 ACE_TEXT (error_p->message)));
      g_error_free (error_p); error_p = NULL;
    } // end IF
    g_object_unref (G_OBJECT (file_p)); file_p = NULL;
  } // end IF
  file_chooser_button_p =
      GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_FILECHOOSERBUTTON_DESTINATION)));
  ACE_ASSERT (file_chooser_button_p);
  std::string download_path =
      Common_File_Tools::getUserDownloadDirectory (ACE_TEXT_ALWAYS_CHAR (""));
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser_button_p),
                                       ACE_TEXT (download_path.c_str ()));

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));

  GtkStatusbar* statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR)));
  ACE_ASSERT (statusbar_p);
  data_p->contextId =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR_CONTEXT_DESCRIPTION));

  GtkFileFilter* file_filter_p =
      GTK_FILE_FILTER (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_FILEFILTER_TORRENT)));
  ACE_ASSERT (file_filter_p);
  gtk_file_filter_add_pattern (file_filter_p,
                               ACE_TEXT ("*.torrent"));

  //// use correct screen
  //if (parentWidget_in)
  //  gtk_window_set_screen (window,
  //                         gtk_widget_get_screen (const_cast<GtkWidget*> (parentWidget_in)));

  // step3: draw it
  gtk_widget_show_all (GTK_WIDGET (window_p));

  guint event_source_id = 0;
  // *IMPORTANT NOTE*: g_idle_add() is broken in the sense that it hogs one CPU
  //                   (100% on Linux, ~30% on Windows)
  //                   --> use a timer
  guint refresh_interval =
    static_cast<guint> ((1.0F / static_cast<float> (BITTORRENT_CLIENT_GUI_GTK_LOG_REFRESH_RATE)) * 1000.0F);
  event_source_id = g_timeout_add_full (G_PRIORITY_DEFAULT,
                                        refresh_interval, // ms
                                        idle_update_display_cb,
                                        userData_in,
                                        NULL);
//  event_source_id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
//                                     idle_update_display_cb,
//                                     userData_in,
//                                     NULL);
  if (!event_source_id)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add_full(idle_update_display_cb): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("idle_update_display_cb: %d\n"),
//              event_source_id));

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    state_r.eventSourceIds.clear ();
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_log_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_log_progress_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionProgressData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionProgressData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->label);
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkTextBuffer* text_buffer_p =
    GTK_TEXT_BUFFER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TEXTBUFFER_SESSION)));
  ACE_ASSERT (text_buffer_p);
  GtkTextIter text_iter;
  gtk_text_buffer_get_end_iter (text_buffer_p,
                                &text_iter);
  std::string message_string = data_p->message;
  message_string += '\n';
  gtk_text_buffer_insert (text_buffer_p,
                          &text_iter,
                          message_string.c_str (), -1);

  // move the iterator to the beginning of line, so the view doesn't scroll
  // in horizontal direction
  gtk_text_iter_set_line_offset (&text_iter, 0);

  GtkTextMark* text_mark_p =
      gtk_text_buffer_get_mark (text_buffer_p,
                                ACE_TEXT_ALWAYS_CHAR ("scroll"));
  ACE_ASSERT (text_mark_p);
  gtk_text_buffer_move_mark (text_buffer_p,
                             text_mark_p,
                             &text_iter);

  // scroll the mark onscreen
  GtkTextView* text_view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TEXTVIEW_SESSION)));
  ACE_ASSERT (text_view_p);
  gtk_text_view_scroll_mark_onscreen (text_view_p,
                                      text_mark_p);

//clean_up:
  delete data_p; data_p = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_piece_complete_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_piece_complete_progress_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionProgressData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionProgressData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->label);
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkTable* table_p =
    GTK_TABLE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TABLE_PIECES)));
  ACE_ASSERT (table_p);
  GList* list_p = gtk_container_get_children (GTK_CONTAINER (table_p));
  if (!list_p)
    return G_SOURCE_CONTINUE; // *NOTE*: wait for the session to establish

  GList* l = list_p;
  GtkButton* button_p = NULL;
  for (;
    l != NULL;
    l = l->next)
  {
    ACE_ASSERT (l && l->data);
    button_p = static_cast<GtkButton*> (l->data);
    if (static_cast<unsigned int> (reinterpret_cast<size_t> (g_object_get_data (G_OBJECT (button_p), ACE_TEXT ("index")))) == static_cast<unsigned int> (data_p->pieceIndex))
      break;
  } // end FOR
  ACE_ASSERT (button_p);

#if defined (GTK2_USE)
  GdkColor green = {0, 0x0000, 0xffff, 0x0000};

//  GtkRcStyle* rc_style_p = gtk_rc_style_new ();
//  rc_style_p->bg[GTK_STATE_NORMAL] = green;
//  rc_style_p->color_flags[GTK_STATE_NORMAL] = GTK_RC_BG;
//  gtk_widget_modify_style (GTK_WIDGET (button_p), rc_style_p);
//  gtk_rc_style_unref (rc_style_p);

  gtk_widget_modify_bg (GTK_WIDGET (button_p), GTK_STATE_NORMAL, &green);
  gtk_widget_modify_bg (GTK_WIDGET (button_p), GTK_STATE_PRELIGHT, &green);
  gtk_widget_modify_bg (GTK_WIDGET (button_p), GTK_STATE_ACTIVE, &green);
#elif defined (GTK3_USE)
//GdkRGBA green = { 0.0, 1.0, 0.0, 1.0 };
//gtk_widget_override_background_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_NORMAL, &green);
//gtk_widget_override_background_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_PRELIGHT, &green);
//gtk_widget_override_background_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_ACTIVE, &green);
//gtk_widget_override_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_NORMAL, &green);
//gtk_widget_override_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_PRELIGHT, &green);
//gtk_widget_override_color (GTK_WIDGET (button_p), GTK_STATE_FLAG_ACTIVE, &green);

  GtkCssProvider* css_provider_p = gtk_css_provider_new ();
  ACE_ASSERT (css_provider_p);
  // Load CSS into the object ("-1" says, that the css string is \0-terminated)
  gtk_css_provider_load_from_data (css_provider_p,
                                   ACE_TEXT ("* { background-image:none; background-color:green; }"), -1,
                                   NULL);

  gtk_style_context_remove_class (gtk_widget_get_style_context (GTK_WIDGET (button_p)),
                                  ACE_TEXT ("*"));
  gtk_style_context_add_provider (gtk_widget_get_style_context (GTK_WIDGET (button_p)),
                                  GTK_STYLE_PROVIDER (css_provider_p),
                                  GTK_STYLE_PROVIDER_PRIORITY_USER);
  g_object_unref (css_provider_p); css_provider_p = NULL;
#endif // GTK2_USE || GTK3_USE

//clean_up:
  g_list_free (list_p); list_p = NULL;
  delete data_p; data_p = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_complete_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_complete_progress_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionProgressData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionProgressData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_WINDOW_MAIN)));
  ACE_ASSERT (window_p);
  GtkWidget* widget_p =
    gtk_message_dialog_new_with_markup (window_p,
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        (data_p->cancelled ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO),
                                        GTK_BUTTONS_CLOSE,
                                        ACE_TEXT ("%s completed"),
                                        data_p->label.c_str ());
  gtk_dialog_run (GTK_DIALOG (widget_p));
  gtk_widget_destroy (widget_p); widget_p = NULL;

//clean_up:
  delete data_p; data_p = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_remove_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_remove_session_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionCBData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->eventSourceId);
  ACE_ASSERT (data_p->session);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->label);
  ACE_ASSERT (iterator != state_r.builders.end ());
  Common_UI_GTK_BuildersIterator_t iterator_2 =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator_2 != state_r.builders.end ());

  GtkNotebook* notebook_p = NULL;
  GtkVBox* vbox_p = NULL;
  gint page_number = -1;

  // remove session page from sessions notebook ?
  notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator_2).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_SESSIONS)));
  ACE_ASSERT (notebook_p);

  vbox_p =
      GTK_VBOX (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_VBOX_SESSION)));
  ACE_ASSERT (vbox_p);
  page_number = gtk_notebook_page_num (notebook_p,
                                       GTK_WIDGET (vbox_p));

  if (gtk_notebook_get_current_page (notebook_p) == page_number)
  { // flip away from "this" page ?
    gtk_notebook_prev_page (notebook_p);
  } // end IF && lock scope
  gtk_notebook_remove_page (notebook_p,
                            page_number);

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    data_p->CBData->progressData.completedActions.insert (data_p->eventSourceId);
  } // end lock scope

  data_p->session->wait ();
  delete data_p->session; data_p->session = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_display_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_CBData* data_p =
    static_cast<struct BitTorrent_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  if (data_p->sessions.empty ())
    return G_SOURCE_CONTINUE;
  // step0: retrieve active session
  BitTorrent_Client_GUI_Session_t* session_p =
      BitTorrent_Client_UI_Tools::current (state_r,
                                           data_p->sessions);
  if (!session_p) // *NOTE*: most probable cause: no session page (yet)
    return G_SOURCE_CONTINUE;

  // step1: update progress display
  try {
//    session_p->update ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_Client_GUI_Session::update(), continuing\n")));
  }

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_ProgressData* data_p =
    static_cast<struct BitTorrent_Client_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);

  int result = -1;
  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  // synch access
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_GTK_CompletedActionsIterator_t iterator_2 = data_p->completedActions.begin ();
         iterator_2 != data_p->completedActions.end ();
         ++iterator_2)
    {
      Common_UI_GTK_PendingActionsIterator_t iterator_3 =
          data_p->pendingActions.find (*iterator_2);
      ACE_ASSERT (iterator_3 != data_p->pendingActions.end ());

      ACE_thread_t thread_id = (*iterator_3).second.id ();
      if (!thread_id)
        goto continue_;

      result = thread_manager_p->join (thread_id,
                                       &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    thread_id));
      else if (exit_status)
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("thread %d has joined (status was: %d)...\n"),
                    thread_id,
                    exit_status));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("thread %u has joined (status was: %@)...\n"),
                    thread_id,
                    exit_status));
#endif // ACE_WIN32 || ACE_WIN64
      } // end IF

continue_:
      state_r.eventSourceIds.erase ((*iterator_3).first);
      data_p->pendingActions.erase (iterator_3);
    } // end FOR
    data_p->completedActions.clear ();

    if (data_p->pendingActions.empty ())
    {
      //if (data_p->cursorType != GDK_LAST_CURSOR)
      //{
      //  GdkCursor* cursor_p = gdk_cursor_new (data_p->cursorType);
      //  if (!cursor_p)
      //  {
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("failed to gdk_cursor_new(%d): \"%m\", continuing\n"),
      //                data_p->cursorType));
      //    return FALSE; // G_SOURCE_REMOVE
      //  } // end IF
      //  GtkWindow* window_p =
      //    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
      //                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_WINDOW_MAIN)));
      //  ACE_ASSERT (window_p);
      //  GdkWindow* window_2 = gtk_widget_get_window (GTK_WIDGET (window_p));
      //  ACE_ASSERT (window_2);
      //  gdk_window_set_cursor (window_2, cursor_p);
      //  data_p->cursorType = GDK_LAST_CURSOR;
      //} // end IF
      gtk_widget_hide (GTK_WIDGET (progress_bar_p));
      return G_SOURCE_REMOVE;
    } // end IF
  } // end lock scope

  gtk_progress_bar_pulse (progress_bar_p);

  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

void
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

  // sanity check(s)
//  struct BitTorrent_Client_UI_CBData* data_p =
//    static_cast<struct BitTorrent_Client_UI_CBData*> (userData_in);
//  ACE_ASSERT (data_p);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_DIALOG_ABOUT)));
  ACE_ASSERT (dialog_p);

  // run dialog
  gint response_id = gtk_dialog_run (dialog_p);
  ACE_UNUSED_ARG (response_id);
  gtk_widget_hide (GTK_WIDGET (dialog_p));
}

void
button_start_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_start_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct BitTorrent_Client_UI_CBData* data_p =
    static_cast<struct BitTorrent_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: retrieve active .torrent file
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_FILECHOOSERBUTTON_TORRENT)));
  ACE_ASSERT (file_chooser_button_p);
  gchar* filename_p =
      gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_button_p));
  if (!filename_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no .torrent file selected, returning\n")));
    return;
  } // end IF
  if (!Common_File_Tools::isReadable (ACE_TEXT_ALWAYS_CHAR (filename_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT (".torrent file (was: \"%s\") not readable, returning\n"),
                ACE_TEXT (filename_p)));
    g_free (filename_p);
    return;
  } // end IF
  data_p->configuration->sessionConfiguration.metaInfoFileName =
    filename_p;
  g_free (filename_p);

  // step2: retrieve target directory
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_FILECHOOSERBUTTON_DESTINATION)));
  ACE_ASSERT (file_chooser_button_p);
  gchar* path_p =
      gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_button_p));
  if (!path_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no destination folder selected, returning\n")));
    return;
  } // end IF
  if (!Common_File_Tools::isDirectory (ACE_TEXT_ALWAYS_CHAR (path_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("destination folder (was: \"%s\") does not exist, returning\n"),
                ACE_TEXT (path_p)));
    g_free (path_p);
    return;
  } // end IF
  BitTorrent_Client_PeerStreamConfiguration_t::ITERATOR_T iterator_2 =
      data_p->configuration->peerStreamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->peerStreamConfiguration.end ());
  (*iterator_2).second.second->destination = path_p;
  g_free (path_p);

  // start session thread
  // *IMPORTANT NOTE*: every session should have its' own dedicated event source
  //                   id --> *TODO*: use g_idle_add instead of native threads
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    struct BitTorrent_Client_UI_SessionThreadData* session_thread_data_p = NULL;
    ACE_NEW_NORETURN (session_thread_data_p,
                      struct BitTorrent_Client_UI_SessionThreadData ());
    if (!session_thread_data_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
      return;
    } // end IF
    session_thread_data_p->CBData = data_p;
    session_thread_data_p->configuration = data_p->configuration;
    session_thread_data_p->controller = data_p->controller;
    session_thread_data_p->filename =
        data_p->configuration->sessionConfiguration.metaInfoFileName;
    ACE_thread_t thread_id = -1;
    ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
    char thread_name_a[BUFSIZ];
    ACE_OS::memset (thread_name_a, 0, sizeof (char[BUFSIZ]));
    const char* thread_name_p = thread_name_a;
    ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
    ACE_ASSERT (thread_manager_p);
    result =
      thread_manager_p->spawn (::session_handler_cb,             // function
                               session_thread_data_p,            // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),              // flags
                               &thread_id,                       // thread id
                               &thread_handle,                   // thread handle
                               ACE_DEFAULT_THREAD_PRIORITY,      // priority
                               COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1, // *TODO*: group id
                               NULL,                             // stack
                               0,                                // stack size
                               &thread_name_p);                  // name
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));
      delete session_thread_data_p; session_thread_data_p = NULL;
      return;
    } // end IF
  #if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("started session thread (id was: %d)...\n"),
                thread_id));
  #else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("started session thread (id was: %u)...\n"),
                thread_id));
  #endif // ACE_WIN32 || ACE_WIN64

    // setup progress updates ?
    GtkProgressBar* progress_bar_p = NULL;
    if (!data_p->progressData.pendingActions.empty ())
      goto continue_;

    progress_bar_p =
        GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR)));
    ACE_ASSERT (progress_bar_p);
    gtk_widget_show (GTK_WIDGET (progress_bar_p));

    session_thread_data_p->eventSourceId =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                        // _LOW doesn't work (on Win32)
                          BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                          idle_update_progress_cb,
                          &data_p->progressData,
                          NULL);
    if (!session_thread_data_p->eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", returning\n")));

      // clean up
      gtk_widget_hide (GTK_WIDGET (progress_bar_p));
      ACE_THR_FUNC_RETURN exit_status;
      result = thread_manager_p->join (thread_id, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    thread_id));

      return;
    } // end IF
    data_p->progressData.pendingActions[session_thread_data_p->eventSourceId] =
      ACE_Thread_ID (thread_id,
                     thread_handle);
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("idle_update_progress_cb: %d\n"),
//                event_source_id));
    state_r.eventSourceIds.insert (session_thread_data_p->eventSourceId);
  } // end lock scope

continue_:
  ;
}

void
button_stop_all_clicked_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_stop_all_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct BitTorrent_Client_UI_CBData* data_p =
    static_cast<struct BitTorrent_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());
}

void
button_quit_clicked_cb (GtkWidget* widget_in,
                        GdkEvent* event_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  int result = -1;

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);
  ACE_UNUSED_ARG (userData_in);
  //Net_GTK_CBData_t* data_p = static_cast<Net_GTK_CBData_t*> (userData_in);
  //// sanity check(s)
  //ACE_ASSERT (data_p);

  //// step1: remove event sources
  //{
  //  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (data_p->state.lock);

  //  for (Common_UI_GTKeventSourceIdsIterator_t iterator = data_p->state.eventSourceIds.begin ();
  //       iterator != data_p->state.eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->state.eventSourceIds.clear ();
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
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?
}

//void
//switch_session_cb (GtkNotebook* notebook_in,
//                   //GtkNotebookPage* page_in,
//                   GtkWidget* page_in,
//                   guint pageNum_in,
//                   gpointer userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::switch_session_cb"));

//  ACE_UNUSED_ARG (notebook_in);
//  ACE_UNUSED_ARG (page_in);
//  ACE_UNUSED_ARG (pageNum_in);

//  // sanity check(s)
//  struct BitTorrent_Client_UI_SessionCBData* data_p =
//    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
//  ACE_ASSERT (data_p);

//  Common_UI_GTK_Manager_t* gtk_manager_p =
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//  ACE_ASSERT (gtk_manager_p);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

//  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

//  Common_UI_GTK_BuildersIterator_t iterator =
//    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != state_r.builders.end ());
//}

void
button_session_close_clicked_cb (GtkWidget* widget_in,
                                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_session_close_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionCBData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->handler);

  data_p->handler->close ();
}

void
button_session_connection_close_clicked_cb (GtkWidget* widget_in,
                                            gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_session_connection_close_clicked_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionCBData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->session);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->label);
  ACE_ASSERT (iterator != state_r.builders.end ());
  Common_UI_GTK_BuildersIterator_t iterator_2 =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator_2 != state_r.builders.end ());

  // step1: retrieve active connection entry
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CONNECTION)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter active_iter;
  //   GValue active_value;
  gchar* connection_value_p = NULL;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &active_iter))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
//   gtk_tree_model_get_value (gtk_combo_box_get_model(serverlist),
//                             &active_iter,
//                             0, &active_value);
  gtk_tree_model_get (gtk_combo_box_get_model (combo_box_p),
                      &active_iter,
                      0, &connection_value_p, // just retrieve the first column...
                      -1);
  //   ACE_ASSERT(G_VALUE_HOLDS_STRING(&active_value));
  ACE_ASSERT (connection_value_p);

  // convert UTF8 to locale
//   connection_string = g_value_get_string(&active_value);
  std::string connection_string =
    Common_UI_GTK_Tools::UTF8ToLocale (connection_value_p,
                                       g_utf8_strlen (connection_value_p, -1));
  if (connection_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"), returning\n"),
                connection_value_p));
    g_free (connection_value_p);
    return;
  } // end IF
  g_free (connection_value_p); connection_value_p = NULL;

  ACE_INET_Addr address;
  data_p->session->disconnect (address);
}

void
combobox_connections_changed_cb (GtkWidget* widget_in,
                                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::combobox_connections_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct BitTorrent_Client_UI_SessionCBData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
//  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

  // step1: retrieve active connection entry
  // retrieve session tab connections combobox handle
  GtkComboBox* combobox_p = GTK_COMBO_BOX (widget_in);
  ACE_ASSERT (combobox_p);
  GtkTreeIter active_iter;
  //   GValue active_value;
  gchar* connection_value = NULL;
  if (!gtk_combo_box_get_active_iter (combobox_p,
                                      &active_iter))
  {
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("failed to gtk_combo_box_get_active_iter(%@), returning\n"),
//                server_tab_channels_combobox));

    return;
  } // end IF
//   gtk_tree_model_get_value(gtk_combo_box_get_model(serverlist),
//                            &active_iter,
//                            0, &active_value);
  gtk_tree_model_get (gtk_combo_box_get_model (combobox_p),
                      &active_iter,
                      0, &connection_value, // just retrieve the first column...
                      -1);
  //   ACE_ASSERT(G_VALUE_HOLDS_STRING(&active_value));
  ACE_ASSERT (connection_value);

  // convert UTF8 to locale
//   channel_string = g_value_get_string(&active_value);
  std::string connection_string =
    Common_UI_GTK_Tools::UTF8ToLocale (connection_value,
                                       g_utf8_strlen (connection_value, -1));
  g_free (connection_value);
  if (connection_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"), returning\n"),
                connection_value));
    return;
  } // end IF
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
