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
//#include "ace/Synch.h"

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
#endif

  // sanity check(s)
  struct BitTorrent_Client_SessionThreadData* data_p =
    static_cast<struct BitTorrent_Client_SessionThreadData*> (arg_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->CBData);
  ACE_ASSERT (data_p->configuration);

  guint context_id = 0;
  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
//  gchar* string_p = NULL;
  std::string label_string =
    ACE_TEXT_ALWAYS_CHAR (ACE::basename (data_p->filename.c_str ()));
//  int result_2 = -1;

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, NULL);
    // step1: create new connection handler
    Common_UI_GTK_BuildersIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    // sanity check(s)
    ACE_ASSERT (iterator != state_r.builders.end ());

    // retrieve progress bar handle
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
    progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR)));
    ACE_ASSERT (progress_bar_p);

    // generate context id
    statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR)));
    ACE_ASSERT (statusbar_p);
    //string_p =
    //  Common_UI_GTK_Tools::Locale2UTF8 ((*data_p->phonebookIterator).second.hostName);
    //if (!string_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Common_UI_GTK_Tools::Locale2UTF8(\"%s\"): \"%m\", returning\n"),
    //              ACE_TEXT ((*data_p->phonebookIterator).second.hostName.c_str ())));

    //  // clean up
    //  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (data_p->CBData->GTKState.lock);
    //  data_p->CBData->progressData.completedActions.insert (ACE_Thread::self ());
    //  delete data_p;

    //  return result;
    //} // end IF
    context_id =
      gtk_statusbar_get_context_id (statusbar_p,
                                    label_string.c_str ());
                                    //string_p);
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    //g_free (string_p);
  } // end lock scope

  // *NOTE*: this schedules addition of a new session page
  //         --> make sure to remove it again if things go wrong
  BitTorrent_Client_GUI_Session_t* session_p = NULL;

//  gdk_threads_enter ();
  ACE_NEW_NORETURN (session_p,
                    BitTorrent_Client_GUI_Session_t (*data_p->configuration,
                                                     state_r,
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
//  gdk_threads_leave ();
  // *WARNING*: beyond this point, need to remove the session page !
  //            --> goto remove_page

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, NULL);
    data_p->CBData->sessions.insert (std::make_pair (label_string, session_p));
  } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

clean:
  { // synch access
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, result);
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<void*>::max ());
#endif
    data_p->CBData->progressData.completedActions.insert (ACE_Thread::self ());
  } // end lock scope

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
  ACE_ASSERT (data_p->eventSourceId);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  GtkWindow* window_p = NULL;
  GtkHBox* hbox_p = NULL;
  GtkLabel* label_p = NULL;
  std::string page_tab_label_string;
  GtkVBox* vbox_p = NULL;
  GtkNotebook* notebook_p = NULL;
  gint page_number = -1;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock);

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->label);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("session builder (was: \"%s\") not found, aborting\n"),
                ACE_TEXT (data_p->label.c_str ())));
    goto clean_up;
  } // end IF

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
  // retrieve channel tab
  vbox_p =
    GTK_VBOX (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_VBOX_SESSION)));
  ACE_ASSERT (vbox_p);
  g_object_ref (vbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (vbox_p));

  notebook_p =
      GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_SESSIONS)));
  ACE_ASSERT (notebook_p);
  page_number = gtk_notebook_append_page (notebook_p,
                                          GTK_WIDGET (vbox_p),
                                          GTK_WIDGET (hbox_p));
  if (page_number == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_notebook_append_page(%@), aborting\n"),
                notebook_p));

    // clean up
    g_object_unref (hbox_p);
    g_object_unref (vbox_p);

    goto clean_up;
  } // end IF
  g_object_unref (hbox_p);

  // allow reordering
  gtk_notebook_set_tab_reorderable (notebook_p,
                                    GTK_WIDGET (vbox_p),
                                    TRUE);
  g_object_unref (vbox_p);

  // *IMPORTANT NOTE*: release lock while switching pages
  { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock, G_SOURCE_REMOVE);
    gtk_notebook_set_current_page (notebook_p,
                                   page_number);
  } // end lock scope

clean_up:
  state_r.eventSourceIds.erase (data_p->eventSourceId);
  data_p->eventSourceId = 0;

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
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: connect signals/slots
  //   gtk_builder_connect_signals((*iterator).second.second,
  //                               &const_cast<main_cb_data&> (userData_in));
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_CONNECT)));
  ACE_ASSERT (button_p);
  gulong result = g_signal_connect (button_p,
                                    ACE_TEXT_ALWAYS_CHAR ("clicked"),
                                    G_CALLBACK (button_connect_clicked_cb),
                                    userData_in);
  ACE_ASSERT (result);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_DISCONNECT)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_disconnect_clicked_cb),
                             userData_in);
  ACE_ASSERT (result);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_ABOUT)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_about_clicked_cb),
                             userData_in);
  ACE_ASSERT (result);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_QUIT)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_quit_clicked_cb),
                             NULL);
  ACE_ASSERT (result);

  // step2: retrieve toplevel handle
  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_WINDOW_MAIN)));
  ACE_ASSERT (window_p);
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"main_dialog\"): \"%m\", returning\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  // connect default signals
  result = g_signal_connect (window_p,
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

  GtkStatusbar* statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR)));
  ACE_ASSERT (statusbar_p);
  data_p->contextId =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR_CONTEXT_DESCRIPTION));

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
idle_remove_session_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_remove_session_cb"));

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionCBData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->eventSourceId); // *NOTE*: seems to be a race condition
  ACE_ASSERT (data_p->session);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  GtkNotebook* notebook_p = NULL;
  GtkVBox* vbox_p = NULL;
  gint page_number = -1;
//  gint number_of_pages = 0;
  Common_UI_GTK_BuildersIterator_t iterator;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock);

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  iterator = state_r.builders.find (data_p->label);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (timestamp was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->label.c_str ())));
    goto clean_up;
  } // end IF

  // remove channel page from connection notebook ?
  notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
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
    // *IMPORTANT NOTE*: release lock while switching pages
    ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock, G_SOURCE_REMOVE);

    gtk_notebook_prev_page (notebook_p);
  } // end IF
  gtk_notebook_remove_page (notebook_p,
                            page_number);

clean_up:
  state_r.eventSourceIds.erase (data_p->eventSourceId);
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

  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  // synch access
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_GTK_CompletedActionsIterator_t iterator_2 = data_p->completedActions.begin ();
         iterator_2 != data_p->completedActions.end ();
         ++iterator_2)
    {
      result = thread_manager_p->join (*iterator_2, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
                    *iterator_2));
      else if (exit_status)
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("thread %d has joined (status was: %d)...\n"),
                    *iterator_2,
                    exit_status));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("thread %u has joined (status was: %@)...\n"),
                    *iterator_2,
                    exit_status));
#endif
      } // end IF

      Common_UI_GTK_PendingActionsIterator_t iterator_3 =
          data_p->pendingActions.find (*iterator_2);
      ACE_ASSERT (iterator_3 != data_p->pendingActions.end ());
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

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_DIALOG_MAIN_ABOUT)));
  ACE_ASSERT (dialog_p);

  // run dialog
  gint response_id = gtk_dialog_run (dialog_p);
  ACE_UNUSED_ARG (response_id);
  gtk_widget_hide (GTK_WIDGET (dialog_p));
}

void
button_connect_clicked_cb (GtkWidget* widget_in,
                           gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_connect_clicked_cb"));

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
  data_p->configuration->trackerStreamConfiguration.configuration_->filename =
    filename_p;
  g_free (filename_p);
  if (!Common_File_Tools::isReadable (data_p->configuration->trackerStreamConfiguration.configuration_->filename))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT (".torrent file (was: \"%s\") not readable, returning\n"),
                ACE_TEXT (data_p->configuration->trackerStreamConfiguration.configuration_->filename.c_str ())));
    return;
  } // end IF

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
  BitTorrent_Client_TrackerStreamConfiguration_t::ITERATOR_T iterator_2 =
    data_p->configuration->trackerStreamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->trackerStreamConfiguration.end ());
  (*iterator_2).second.second->destination = path_p;
  g_free (path_p);
  if (!Common_File_Tools::isDirectory ((*iterator_2).second.second->destination))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("destination folder (was: \"%s\") does not exist, returning\n"),
                ACE_TEXT ((*iterator_2).second.second->destination.c_str ())));
    return;
  } // end IF

  // start session thread
  struct BitTorrent_Client_SessionThreadData* session_thread_data_p = NULL;
  ACE_NEW_NORETURN (session_thread_data_p,
                    struct BitTorrent_Client_SessionThreadData ());
  if (!session_thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  session_thread_data_p->CBData = data_p;
  session_thread_data_p->configuration = data_p->configuration;
  ACE_thread_t thread_id = -1;
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  //char thread_name[BUFSIZ];
  //ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  char* thread_name_p = NULL;
  ACE_NEW_NORETURN (thread_name_p,
                    char[BUFSIZ]);
  if (!thread_name_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    delete session_thread_data_p;

    return;
  } // end IF
  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  const char* thread_name_2 = thread_name_p;
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
                             &thread_name_2);                  // name
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));

    // clean up
    delete thread_name_p;
    delete session_thread_data_p;

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
#endif

  // setup progress updates
  //if (data_p->progressData.cursorType == GDK_LAST_CURSOR)
  //{
  //  GtkWindow* window_p =
  //    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
  //                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_WINDOW_MAIN)));
  //  ACE_ASSERT (window_p);
  //  GdkWindow* window_2 = gtk_widget_get_window (GTK_WIDGET (window_p));
  //  ACE_ASSERT (window_2);
  //  GdkCursor* cursor_p = gdk_window_get_cursor (window_2);
  //  data_p->progressData.cursorType = gdk_cursor_get_cursor_type (cursor_p);
  //  cursor_p = gdk_cursor_new (BITTORRENT_CLIENT_GUI_GTK_CURSOR_BUSY);
  //  if (!cursor_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to gdk_cursor_new(): \"%m\", returning\n")));

  //    // clean up
  //    delete thread_name_p;
  //    delete connection_thread_data_p;
  //    ACE_THR_FUNC_RETURN exit_status;
  //    result = thread_manager_p->join (thread_id, &exit_status);
  //    if (result == -1)
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%d): \"%m\", continuing\n"),
  //                  thread_id));
  //    data_p->progressData.cursorType = GDK_LAST_CURSOR;

  //    return;
  //  } // end IF
  //  gdk_window_set_cursor (window_2, cursor_p);
  //} // end IF
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    gtk_widget_show (GTK_WIDGET (progress_bar_p));

    guint event_source_id =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                        // _LOW doesn't work (on Win32)
                          BITTORRENT_CLIENT_GUI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                          idle_update_progress_cb,
                          &data_p->progressData,
                          NULL);
    if (!event_source_id)
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
    data_p->progressData.pendingActions[event_source_id] =
      ACE_Thread_ID (thread_id,
                     thread_handle);
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("idle_update_progress_cb: %d\n"),
//                event_source_id));
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
}

void
button_disconnect_clicked_cb (GtkWidget* widget_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_disconnect_clicked_cb"));

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

void
switch_session_cb (GtkNotebook* notebook_in,
                   //GtkNotebookPage* page_in,
                   GtkWidget* page_in,
                   guint pageNum_in,
                   gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::switch_session_cb"));

  ACE_UNUSED_ARG (notebook_in);
  ACE_UNUSED_ARG (page_in);
  ACE_UNUSED_ARG (pageNum_in);

  // sanity check(s)
  struct BitTorrent_Client_UI_SessionCBData* data_p =
    static_cast<struct BitTorrent_Client_UI_SessionCBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());
}

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
  ACE_ASSERT (data_p->session);

  data_p->session->close ();
}

void
button_connection_close_clicked_cb (GtkWidget* widget_in,
                                    gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_connection_close_clicked_cb"));

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
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: retrieve active connection entry
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CONNECTIONS)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter active_iter;
  //   GValue active_value;
  gchar* connection_value = NULL;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &active_iter))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
//   gtk_tree_model_get_value(gtk_combo_box_get_model(serverlist),
//                            &active_iter,
//                            0, &active_value);
  gtk_tree_model_get (gtk_combo_box_get_model (combo_box_p),
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

  ACE_INET_Addr address;
  data_p->session->disconnect (address);

  gtk_widget_set_sensitive (widget_in, FALSE);
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
