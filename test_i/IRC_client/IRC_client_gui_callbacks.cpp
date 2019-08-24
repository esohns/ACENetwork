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

#include "IRC_client_gui_callbacks.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_tools.h"

#include "net_macros.h"

#include "irc_common.h"
#include "irc_tools.h"

#include "IRC_client_common_modules.h"
#include "IRC_client_configuration.h"
#include "IRC_client_defines.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_network.h"
#include "IRC_client_stream_common.h"
#include "IRC_client_tools.h"

#include "IRC_client_gui_common.h"
#include "IRC_client_gui_connection.h"
#include "IRC_client_gui_defines.h"
#include "IRC_client_gui_messagehandler.h"
#include "IRC_client_gui_tools.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
ACE_THR_FUNC_RETURN
connection_setup_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::connection_setup_function"));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = -1;
#else
  result = arg_in;
#endif

  // sanity check(s)
  struct IRC_Client_ConnectionThreadData* data_p =
    static_cast<struct IRC_Client_ConnectionThreadData*> (arg_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->CBData);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  guint context_id = 0;
  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  gchar* string_p = NULL;
  {// ACE_Guard<ACE_SYNCH_MUTEX> aGuard (data_p->CBData->lock);
    // step1: create new connection handler
    Common_UI_GTK_BuildersIterator_t iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    // sanity check(s)
    ACE_ASSERT (iterator != state_r.builders.end ());

    // retrieve progress bar handle
    gdk_threads_enter ();
    progress_bar_p =
      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_PROGRESSBAR)));
    ACE_ASSERT (progress_bar_p);

    // generate context ID
    statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_STATUSBAR)));
    ACE_ASSERT (statusbar_p);
    //string_p =
    //  Common_UI_GTK_Tools::Locale2UTF8 ((*data_p->phonebookIterator).second.hostName);
    //if (!string_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Common_UI_GTK_Tools::Locale2UTF8(\"%s\"): \"%m\", returning\n"),
    //              ACE_TEXT ((*data_p->phonebookIterator).second.hostName.c_str ())));

    //  // clean up
    //  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (data_p->CBData->lock);
    //  data_p->CBData->completedActions.insert (ACE_Thread::self ());
    //  delete data_p;

    //  return result;
    //} // end IF
    context_id =
      gtk_statusbar_get_context_id (statusbar_p,
                                    data_p->phonebookEntry.hostName.c_str ());
                                    //string_p);
    gdk_threads_leave ();
    //g_free (string_p);
  } // end lock scope

  // *NOTE*: this schedules addition of a new server page
  //         --> make sure to remove it again, if things go wrong
  struct IRC_Client_UI_HandlerCBData* handler_cb_data_p = NULL;
  ACE_NEW_NORETURN (handler_cb_data_p,
                    struct IRC_Client_UI_HandlerCBData ());
  if (!handler_cb_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
//    gdk_threads_leave ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, result);
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<void*>::max ());
#endif
    data_p->CBData->progressData.completedActions.insert (ACE_Thread::self ());
    delete data_p;

    return result;
  } // end IF
  IRC_Client_GUI_Connection* connection_p = NULL;
//  gdk_threads_enter ();
  ACE_NEW_NORETURN (connection_p,
                    IRC_Client_GUI_Connection (&data_p->CBData->connections,
                                               handler_cb_data_p,
                                               context_id,
                                               data_p->phonebookEntry.hostName,
                                               data_p->CBData->UIFileDirectory));
  if (!connection_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
//    gdk_threads_leave ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, result);
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<void*>::max ());
#endif
    data_p->CBData->progressData.completedActions.insert (ACE_Thread::self ());
    delete data_p;

    return result;
  } // end IF
//  gdk_threads_leave ();
  IRC_Client_StreamConfiguration_t::ITERATOR_T iterator_2 =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != data_p->configuration->streamConfiguration.end ());
  (*iterator_2).second.second.subscriber = connection_p;

  // *WARNING*: beyond this point, need to remove the connection page !
  //            --> goto remove_page

  // step2: connect to the server
  std::stringstream converter;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  bool result_2 = false;
  IRC_IControl* icontrol_p = NULL;
  const Stream_IStream_t::STREAM_T* stream_p = NULL;
  const Stream_Module_t* module_p = NULL;
  const Stream_Module_t* current_p = NULL;
  Stream_Module_t* tail_p = NULL;
  IRC_Client_Connection_Manager_t::CONNECTION_T* connection_2 = NULL;
  unsigned short current_port = 0;
  int result_3 = -1;
  bool done = false;
  IRC_Client_IStreamConnection_t* istream_connection_p = NULL;

  // step2a: set up connector
  IRC_Client_Connection_Manager_t* connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  IRC_Client_Connector_t connector (true);
  IRC_Client_AsynchConnector_t asynch_connector (true);
  IRC_Client_IConnector_t* connector_p = &connector;
  if (data_p->configuration->dispatch == COMMON_EVENT_DISPATCH_PROACTOR)
    connector_p = &asynch_connector;

  // step2b: set up configuration passed to processing stream
  IRC_Client_ConnectionConfiguration* configuration_p = NULL;
  struct Net_UserData* user_data_p = NULL;
  // load defaults
  connection_manager_p->get (configuration_p,
                             user_data_p);
  // sanity check(s)
  ACE_ASSERT (configuration_p);

  // step2c: initialize connector
  if (!connector_p->initialize (*configuration_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
    goto remove_page;
  } // end IF

  for (IRC_Client_PortRangesIterator_t iterator = data_p->phonebookEntry.ports.begin ();
       iterator != data_p->phonebookEntry.ports.end ();
       ++iterator)
  {
    // *TODO*: update progress bar units

    if (done)
      break;

    // port range ?
    for (current_port = (*iterator).first;
         current_port <= (*iterator).second;
         ++current_port)
    {
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << ACE_TEXT_ALWAYS_CHAR ("trying port ");
      converter << current_port;
      converter << ACE_TEXT_ALWAYS_CHAR ("...");
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s\n"),
                  ACE_TEXT (converter.str ().c_str ())));
      string_p = Common_UI_GTK_Tools::localeToUTF8 (converter.str ());
      if (!string_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"): \"%m\", returning\n"),
                    ACE_TEXT (converter.str ().c_str ())));
        goto remove_page;
      } // end IF
      gdk_threads_enter ();
      gtk_progress_bar_set_text (progress_bar_p,
                                 string_p);
      gdk_threads_leave ();
      g_free (string_p); string_p = NULL;

      result_3 =
        configuration_p->address.set (current_port,
                                      data_p->phonebookEntry.hostName.c_str (),
                                      1,
                                      ACE_ADDRESS_FAMILY_INET);
      if (result_3 == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
        goto remove_page;
      } // end IF

      // step3: (try to) connect to the server
      handle =
        connector_p->connect (configuration_p->address);
      if (handle == ACE_INVALID_HANDLE)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to connect(%s): \"%m\", continuing\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (configuration_p->address).c_str ())));
      else
      {
        done = true;
        break;
      } // end ELSE
    } // end FOR
  } // end FOR
  if (handle == ACE_INVALID_HANDLE)
  {
connection_failed:
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << ACE_TEXT_ALWAYS_CHAR ("failed to connect to server \"");
    converter << data_p->phonebookEntry.hostName;
    converter << ACE_TEXT_ALWAYS_CHAR ("\"");
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s, returning\n"),
                ACE_TEXT (converter.str ().c_str ())));

    // clean up
    string_p = Common_UI_GTK_Tools::localeToUTF8 (converter.str ());
    if (!string_p)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (converter.str ().c_str ())));
    gdk_threads_enter ();
    gtk_statusbar_push (statusbar_p,
                        data_p->CBData->contextId,
                        string_p);
    gdk_threads_leave ();
    g_free (string_p); string_p = NULL;

    goto remove_page;
  } // end IF
  if (data_p->configuration->dispatch == COMMON_EVENT_DISPATCH_PROACTOR)
  {
    ACE_Time_Value deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (IRC_CLIENT_CONNECTION_ASYNCH_TIMEOUT, 0));
    ACE_Time_Value delay (IRC_CLIENT_CONNECTION_ASYNCH_TIMEOUT_INTERVAL, 0);
    do
    {
      result_3 = ACE_OS::sleep (delay);
      if (result_3 == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &delay));

      // *TODO*: this does not work
      connection_2 = connection_manager_p->get (configuration_p->address);
      if (connection_2)
        break; // done
    } while (COMMON_TIME_NOW < deadline);
  } // end IF
  else
    connection_2 =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle));
#endif
  if (!connection_2)
    goto connection_failed;
  istream_connection_p =
      dynamic_cast<IRC_Client_IStreamConnection_t*> (connection_2);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<IRC_Client_IStreamConnection_t>(%@): \"%m\", aborting\n"),
                connection_2));
    connection_2->close ();
    connection_2->decrease ();

    goto remove_page;
  } // end IF
  stream_p = &istream_connection_p->stream ();
  for (Stream_Iterator_t iterator_2 (*stream_p);
       iterator_2.next (current_p) != 0;
       iterator_2.advance ())
  {
    tail_p =
      const_cast<Stream_IStream_t::STREAM_T*> (stream_p)->tail ();
    if (current_p != tail_p)
      module_p = current_p;
  } // end FOR
  ACE_ASSERT (module_p);
  icontrol_p =
    dynamic_cast<IRC_IControl*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!icontrol_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_IControl>(0x%@) failed, returning\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    connection_2->close ();
    connection_2->decrease ();

    goto remove_page;
  } // end IF

  // step3: initialize new connection handler
  connection_p->initialize (&const_cast<struct IRC_Client_SessionState&> (connection_2->state ()),
                            icontrol_p);
  { // synch access
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, result);
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<void*>::max ());
#endif
    // *TODO*: who deletes the module ? (the stream won't do it !)
    data_p->CBData->connections.insert (std::make_pair (connection_p->getR ().timeStamp,
                                                        connection_p));
  } // end lock scope

  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("registering...\n")));

  // step4: register connection with the server
  try {
    // *NOTE*: this entails a little delay (waiting for the welcome notice...)
    result_2 = icontrol_p->registerc (data_p->loginOptions);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_IControl::registerc(), continuing\n")));
  }
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_IControl::registerc(), returning\n")));
    connection_2->close ();
    connection_2->decrease ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, result);
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<void*>::max ());
#endif
    data_p->CBData->connections.erase (connection_p->getR ().timeStamp);

    goto remove_page;
  } // end IF
  //   ACE_DEBUG((LM_DEBUG,
  //              ACE_TEXT("registering...DONE\n")));

  connection_2->decrease (); connection_2 = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

  goto done;

remove_page:
  ACE_ASSERT (connection_p);
  connection_p->close ();

done:
  { // synch access
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, result);
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<void*>::max ());
#endif
    data_p->CBData->progressData.completedActions.insert (ACE_Thread::self ());
  } // end lock scope

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

  gboolean sensitive = !gtk_tree_model_iter_has_child (model_in, iter_in);
  // set corresponding property
  g_object_set (renderer_in,
                ACE_TEXT_ALWAYS_CHAR ("sensitive"), sensitive,
                NULL);
}

//////////////////////////////////////////

gboolean
idle_add_channel_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_add_channel_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->eventSourceId);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  GtkWindow* window_p = NULL;
  GtkHBox* hbox_p = NULL;
  GtkLabel* label_p = NULL;
  std::string page_tab_label_string;
  GtkFrame* frame_p = NULL;
  GtkTreeView* tree_view_p = NULL;
  GtkLabel* label_2 = NULL;
  GtkVBox* vbox_p = NULL;
  Common_UI_GTK_BuildersIterator_t iterator_2;
  GtkNotebook* notebook_p = NULL;
  gint page_number = -1;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->builderLabel);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("channel (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->builderLabel.c_str ())));
    goto clean_up;
  } // end IF

  // add new channel page to the connection notebook
  // retrieve (dummy) parent window
  window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_TAB_CHANNEL)));
  ACE_ASSERT (window_p);
  // retrieve channel tab label
  hbox_p =
    GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_CHANNEL_TAB)));
  ACE_ASSERT (hbox_p);
  g_object_ref (hbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (hbox_p));
  // set tab label
  label_p =
    GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_CHANNEL_TAB)));
  ACE_ASSERT (label_p);
  if (!IRC_Tools::isValidChannelName (data_p->id))
  {
    // --> private conversation window, modify label accordingly
    page_tab_label_string = ACE_TEXT_ALWAYS_CHAR ("<b>");
    page_tab_label_string += data_p->id;
    page_tab_label_string += ACE_TEXT_ALWAYS_CHAR ("</b>");

    // hide channel mode tab frame
    frame_p =
      GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_FRAME_CHANNELMODE)));
    ACE_ASSERT (frame_p);
    gtk_widget_hide (GTK_WIDGET (frame_p));
    // hide channel tab treeview
    tree_view_p =
      GTK_TREE_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TREEVIEW_CHANNEL)));
    ACE_ASSERT (tree_view_p);
    gtk_widget_hide (GTK_WIDGET (tree_view_p));

    // erase "topic" label
    label_2 =
      GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_TOPIC)));
    ACE_ASSERT (label_2);
    gtk_label_set_text (label_2, NULL);
  } // end IF
  else
    page_tab_label_string = data_p->id;
  gtk_label_set_text (label_p,
                      page_tab_label_string.c_str ());

  // retrieve (dummy) parent window
  window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_CHANNEL)));
  ACE_ASSERT (window_p);
  // retrieve channel tab
  vbox_p =
    GTK_VBOX (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CHANNEL)));
  ACE_ASSERT (vbox_p);
  g_object_ref (vbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (vbox_p));

  iterator_2 = state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator_2 == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (timestamp was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->timeStamp.c_str ())));
    goto clean_up;
  } // end IF

  notebook_p =
      GTK_NOTEBOOK (gtk_builder_get_object ((*iterator_2).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
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

  // activate new page (iff it's a channel tab !)
  if (IRC_Tools::isValidChannelName (data_p->id))
  {
    // *IMPORTANT NOTE*: release lock while switching pages
    ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock);
    ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock, G_SOURCE_REMOVE);
    gtk_notebook_set_current_page (notebook_p,
                                   page_number);
  } // end IF

clean_up:
  state_r.eventSourceIds.erase (data_p->eventSourceId);
  data_p->eventSourceId = 0;

  return G_SOURCE_REMOVE;
}

gboolean
idle_add_connection_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_add_connection_cb"));

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->eventSourceId); // *NOTE*: seems to be a race condition

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  GtkWindow* window_p = NULL;
  GtkHBox* hbox_p = NULL;
  GtkLabel* label_p = NULL;
  gchar* string_p = NULL;
  GtkButton* button_p = NULL;
  GtkVBox* vbox_p = NULL;
  GtkNotebook* notebook_p = NULL;
  gint page_number = -1;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  Common_UI_GTK_BuildersIterator_t iterator_2 =
    state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator_2 == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->label.c_str ())));
    goto clean_up;
  } // end IF

  // add the new server page to the (parent) notebook
  // retrieve (dummy) parent window
  window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator_2).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_TAB_CONNECTION)));
  ACE_ASSERT (window_p);
  // retrieve server tab label
  hbox_p =
    GTK_HBOX (gtk_builder_get_object ((*iterator_2).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_CONNECTION_TAB)));
  ACE_ASSERT (hbox_p);
  g_object_ref (hbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (hbox_p));
  // set tab label
  label_p =
    GTK_LABEL (gtk_builder_get_object ((*iterator_2).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_CONNECTION_TAB)));
  ACE_ASSERT (label_p);
  string_p = Common_UI_GTK_Tools::localeToUTF8 (data_p->label);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (data_p->label.c_str ())));

    // clean up
    g_object_unref (hbox_p);

    goto clean_up;
  } // end IF
  gtk_label_set_text (label_p, string_p);
  g_free (string_p);
  // disable close button (until connection is fully established)
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator_2).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_DISCONNECT)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);

  // retrieve (dummy) parent window
  window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator_2).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_CONNECTION)));
  ACE_ASSERT (window_p);
  // retrieve server tab
  vbox_p =
    GTK_VBOX (gtk_builder_get_object ((*iterator_2).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CONNECTION)));
  ACE_ASSERT (vbox_p);
  g_object_ref (vbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (vbox_p));

  notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
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

  // activate new page
  gtk_notebook_set_current_page (notebook_p,
                                 page_number);

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
  struct IRC_Client_UI_CBData* data_p =
    static_cast<struct IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // step1: remove any remaining event sources
  // *NOTE*: should be 2: 'this', and idle_update_display_cb...
  unsigned int removed_events = 0;
  while (g_idle_remove_by_data (userData_in))
    removed_events++;
  if (removed_events)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("removed %u queued event(s)...\n"),
                removed_events));
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
  //ACE_ASSERT (removed_events == data_p->eventSourceIds.size ());
  state_r.eventSourceIds.clear ();

  // step2: leave GTK
  gtk_main_quit ();

  return G_SOURCE_REMOVE;
}

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct IRC_Client_UI_CBData* data_p =
    static_cast<struct IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step2: populate phonebook
  GtkTreeStore* tree_store_p =
    GTK_TREE_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TREESTORE_SERVERS)));
  ACE_ASSERT (tree_store_p);
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_SERVERS)));
  ACE_ASSERT (combo_box_p);
  // *NOTE*: the combobox displays (selectable) column headers
  //         --> don't want that
  GList* list_p =
    gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (combo_box_p));
  GtkCellRenderer* cell_renderer_p =
    GTK_CELL_RENDERER (g_list_first (list_p)->data);
  ACE_ASSERT (cell_renderer_p);
  g_list_free (list_p);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                      is_entry_sensitive,
                                      NULL, NULL);
  std::map<std::string, GtkTreeIter> network_map;
  std::map<std::string, GtkTreeIter>::iterator network_map_iterator;
  GtkTreeIter tree_iter, tree_iter_2;
  for (IRC_Client_ServersIterator_t iterator_2 = data_p->phoneBook.servers.begin ();
       iterator_2 != data_p->phoneBook.servers.end ();
       ++iterator_2)
  {
    // known network ?
    network_map_iterator = network_map.find ((*iterator_2).second.netWork);
    if (network_map_iterator == network_map.end ())
    {
      // append new (network) entry
      gtk_tree_store_append (tree_store_p,
                             &tree_iter,
                             NULL);
      std::string network_label =
        ((*iterator_2).second.netWork.empty () ? ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_PHONEBOOK_DEF_NETWORK_LABEL)
                                               : (*iterator_2).second.netWork);
      gtk_tree_store_set (tree_store_p, &tree_iter,
                          0, network_label.c_str (), // column 0
                          -1);

      network_map.insert (std::make_pair ((*iterator_2).second.netWork,
                                          tree_iter));
      network_map_iterator = network_map.find ((*iterator_2).second.netWork);
      ACE_ASSERT (network_map_iterator != network_map.end ());
    } // end IF

    // append new (server) entry
    gtk_tree_store_append (tree_store_p,
                           &tree_iter_2,
                           &(*network_map_iterator).second);
    gtk_tree_store_set (tree_store_p, &tree_iter_2,
                        0, (*iterator_2).first.c_str (), // column 0
                        -1);

    // set active item
    if ((*iterator_2).first ==
        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEFAULT_SERVER_HOSTNAME))
      gtk_combo_box_set_active_iter (combo_box_p,
                                     &tree_iter_2);
  } // end FOR
  if (!data_p->phoneBook.servers.empty ())
  {
    // sort entries (toplevel: ascending)
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (tree_store_p),
                                          0, GTK_SORT_ASCENDING);

    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
  } // end IF

  // step3: connect signals/slots
  //   gtk_builder_connect_signals((*iterator).second.second,
  //                               &const_cast<main_cb_data&> (userData_in));
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_SEND)));
  ACE_ASSERT (entry_p);
  gulong result = g_signal_connect (entry_p,
                                    ACE_TEXT_ALWAYS_CHAR ("focus-in-event"),
                                    G_CALLBACK (entry_send_kb_focused_cb),
                                    userData_in);
  ACE_ASSERT (result);
  result = g_signal_connect (entry_p,
                             ACE_TEXT_ALWAYS_CHAR ("changed"),
                             G_CALLBACK (entry_send_changed_cb),
                             userData_in);
  ACE_ASSERT (result);
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_SEND)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_send_clicked_cb),
                             userData_in);
  ACE_ASSERT (result);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_CONNECT)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_connect_clicked_cb),
                             userData_in);
  ACE_ASSERT (result);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_ABOUT)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_about_clicked_cb),
                             userData_in);
  ACE_ASSERT (result);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_QUIT)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_quit_clicked_cb),
                             NULL);
  ACE_ASSERT (result);
  ACE_UNUSED_ARG (result);

  // step4: retrieve toplevel handle
  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
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
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_STATUSBAR)));
  ACE_ASSERT (statusbar_p);
  data_p->contextId =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_STATUSBAR_CONTEXT_DESCRIPTION));

  //// use correct screen
  //if (parentWidget_in)
  //  gtk_window_set_screen (window,
  //                         gtk_widget_get_screen (const_cast<GtkWidget*> (parentWidget_in)));

  // step5: draw it
  gtk_widget_show_all (GTK_WIDGET (window_p));

  guint event_source_id = 0;
  // *IMPORTANT NOTE*: g_idle_add() is broken in the sense that it hogs one CPU
  //                   (100% on Linux, ~30% on Windows)
  //                   --> use a timer
  guint refresh_interval =
    static_cast<guint> ((1.0F / static_cast<float> (IRC_CLIENT_GUI_GTK_LOG_REFRESH_RATE)) * 1000.0F);
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

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
  state_r.eventSourceIds.clear ();
  state_r.eventSourceIds.insert (event_source_id);

  return G_SOURCE_REMOVE;
}

gboolean
idle_remove_channel_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_remove_channel_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
//  ACE_ASSERT (data_p->eventSourceId); // *NOTE*: seems to be a race condition
  ACE_ASSERT (data_p->handler);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  GtkNotebook* notebook_p = NULL;
  Common_UI_GTK_BuildersIterator_t iterator_2;
  GtkVBox* vbox_p = NULL;
  gint page_number = -1;
  gint number_of_pages = 0;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (timestamp was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->timeStamp.c_str ())));
    goto clean_up;
  } // end IF

  // remove channel page from connection notebook ?
  notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);
  if (data_p->handler->isServerLog ())
    goto done; // skip page removal (that page belongs to the connection)

  iterator_2 = state_r.builders.find (data_p->builderLabel);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("handler (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->builderLabel.c_str ())));
    goto clean_up;
  } // end IF

  vbox_p =
      GTK_VBOX (gtk_builder_get_object ((*iterator_2).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CHANNEL)));
  ACE_ASSERT (vbox_p);
  page_number = gtk_notebook_page_num (notebook_p,
                                       GTK_WIDGET (vbox_p));

  if (gtk_notebook_get_current_page (notebook_p) == page_number)
  { // flip away from "this" page ?
    // *IMPORTANT NOTE*: release lock while switching pages
    ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock);
    ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock, G_SOURCE_REMOVE);

    gtk_notebook_prev_page (notebook_p);
  } // end IF
  gtk_notebook_remove_page (notebook_p,
                            page_number);

done:
  number_of_pages = gtk_notebook_get_n_pages (notebook_p);
  if ((number_of_pages == 1) && data_p->connection->closing_)
  {
    struct IRC_Client_UI_ConnectionCBData* cb_data_p =
        &const_cast<struct IRC_Client_UI_ConnectionCBData&> (data_p->connection->getR ());
    cb_data_p->eventSourceId =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_remove_connection_cb,
                       cb_data_p,
                       NULL);
    if (cb_data_p->eventSourceId)
      state_r.eventSourceIds.insert (cb_data_p->eventSourceId);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add_full(idle_remove_connection_cb): \"%m\", continuing\n")));
  } // end IF

clean_up:
  state_r.eventSourceIds.erase (data_p->eventSourceId);
  delete data_p->handler; data_p->handler = NULL;

  return G_SOURCE_REMOVE;
}

gboolean
idle_remove_connection_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_remove_connection_cb"));

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connections);
//  ACE_ASSERT (data_p->eventSourceId); // *NOTE*: seems to be a race condition

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  GtkNotebook* notebook_p = NULL;
  GtkVBox* vbox_p = NULL;
  gint page_number = -1;
  GtkWindow* window_p = NULL;
  Common_UI_GTK_BuildersIterator_t iterator, iterator_2;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  IRC_Client_GUI_ConnectionsIterator_t iterator_3 =
    data_p->connections->end ();

  iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("main builder not found, aborting\n")));
    goto clean_up;
  } // end IF
  iterator_2 = state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator_2 == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->label.c_str ())));
    goto clean_up;
  } // end IF

  // remove server page from parent notebook
  notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  vbox_p =
    GTK_VBOX (gtk_builder_get_object ((*iterator_2).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CONNECTION)));
  ACE_ASSERT (vbox_p);
  page_number = gtk_notebook_page_num (notebook_p,
                                       GTK_WIDGET (vbox_p));
  // flip away from "this" page ?
  if (gtk_notebook_get_current_page (notebook_p) == page_number)
  {
    // *IMPORTANT NOTE*: release lock while switching pages
    ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (state_r.lock);
    ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock, G_SOURCE_REMOVE);

    gtk_notebook_prev_page (notebook_p);
  } // end IF
  gtk_notebook_remove_page (notebook_p,
                            page_number);

//  g_object_unref (G_OBJECT ((*iterator_2).second.second));
//  data_p->builders.erase (iterator_2);

  // if necessary, shrink main window
  window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
  ACE_ASSERT (window_p);
  gtk_window_resize (window_p, 1, 1);

clean_up:
  state_r.eventSourceIds.erase (data_p->eventSourceId);
  iterator_3 = data_p->connections->find (data_p->timeStamp);
  if (iterator_3 != data_p->connections->end ())
  {
    delete (*iterator_3).second;
    data_p->connections->erase (iterator_3);
  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_channel_modes_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->eventSourceId); // *NOTE*: seems to be a race condition

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  GtkToggleButton* toggle_button_p = NULL;
  GtkHBox* hbox_p = NULL;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->builderLabel);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("channel (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->builderLabel.c_str ())));
    goto clean_up;
  } // end IF

  // display (changed) channel modes
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_KEY)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_PASSWORD]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_VOICE)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_VOICE]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BAN)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_BAN]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_USERLIMIT)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_USERLIMIT]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_MODERATED)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_MODERATED]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BLOCKFOREIGN)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_BLOCKFOREIGNMSGS]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_RESTRICTOPIC)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_RESTRICTEDTOPIC]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_INVITEONLY)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_INVITEONLY]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_SECRET)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_SECRET]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_PRIVATE)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_PRIVATE]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_OPERATOR)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                data_p->channelModes[CHANNELMODE_OPERATOR]);

  // enable channel modes ?

  // retrieve channel tab mode hbox handle
  hbox_p =
    GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_CHANNELMODE)));
  ACE_ASSERT (hbox_p);
  gtk_widget_set_sensitive (GTK_WIDGET (hbox_p),
                            data_p->channelModes.test (CHANNELMODE_OPERATOR));

clean_up:
  state_r.eventSourceIds.erase (data_p->eventSourceId);
  data_p->eventSourceId = 0;

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_display_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_display_cb"));

  // sanity check(s)
  struct IRC_Client_UI_CBData* data_p =
    static_cast<IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
  if (data_p->connections.empty ())
    return G_SOURCE_CONTINUE;

  // step0: retrieve active connection
  IRC_Client_GUI_Connection* connection_p =
    IRC_Client_UI_Tools::current (const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR_2 ()),
                                  data_p->connections);
  if (!connection_p) // *NOTE*: most probable cause: no server page/corresponding connection (yet)
    return G_SOURCE_CONTINUE;

  // step1: retrieve active channel
  IRC_Client_GUI_MessageHandler* message_handler_p =
    connection_p->getActiveHandler (false,
                                    false);
  if (!message_handler_p) // *NOTE*: most probable cause: no server page (yet)
    return G_SOURCE_CONTINUE;

  try {
    message_handler_p->update ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_GUI_MessageHandler::update(), continuing\n")));
  }

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  // sanity check(s)
  struct IRC_Client_UI_ProgressData* data_p =
    static_cast<struct IRC_Client_UI_ProgressData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_PROGRESSBAR)));
  ACE_ASSERT (progress_bar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  // synch access
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    Common_UI_GTK_PendingActionsIterator_t iterator_3;
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

      for (iterator_3 = data_p->pendingActions.begin ();
           iterator_3 != data_p->pendingActions.end ();
           ++iterator_3)
        if ((*iterator_3).second.id () == *iterator_2)
          break;
      //iterator_3 = data_p->pendingActions.find (*iterator_2);
      ACE_ASSERT (iterator_3 != data_p->pendingActions.end ());
      data_p->state->eventSourceIds.erase ((*iterator_3).first);
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
      //                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
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

gboolean
idle_update_user_modes_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_update_user_modes_cb"));

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connections);
//  ACE_ASSERT (data_p->eventSourceId); // *NOTE*: seems to be a race condition

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  GtkToggleButton* toggle_button_p = NULL;
  Common_UI_GTK_BuildersIterator_t iterator;
  IRC_Client_GUI_ConnectionsConstIterator_t iterator_2;
  const IRC_Client_SessionState* connection_state_p = NULL;

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  iterator = state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (data_p->label.c_str ())));
    goto clean_up;
  } // end IF

  iterator_2 = data_p->connections->find (data_p->timeStamp);
  // sanity check(s)
  if (iterator_2 == data_p->connections->end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    goto clean_up;
  } // end IF
  ACE_ASSERT ((*iterator_2).second);
  connection_state_p = &(*iterator_2).second->state ();

  // display (changed) user modes
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                connection_state_p->userModes[USERMODE_AWAY]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                connection_state_p->userModes[USERMODE_INVISIBLE]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                connection_state_p->userModes[USERMODE_RECVNOTICES]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_OPERATOR)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                connection_state_p->userModes[USERMODE_OPERATOR]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_RESTRICTED)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                connection_state_p->userModes[USERMODE_RESTRICTEDCONN]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_LOCALOPERATOR)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                connection_state_p->userModes[USERMODE_LOCALOPERATOR]);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                connection_state_p->userModes[USERMODE_RECVWALLOPS]);

clean_up:
  state_r.eventSourceIds.erase (data_p->eventSourceId);
  data_p->eventSourceId = 0;

  return G_SOURCE_REMOVE;
}

//////////////////////////////////////////

void
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct IRC_Client_UI_CBData* data_p =
    static_cast<IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_MAIN_ABOUT)));
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
  struct IRC_Client_UI_CBData* data_p =
    static_cast<IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  int result = -1;
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: retrieve active phonebook entry
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_SERVERS)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter tree_iter;
  //   GValue active_value;
  gchar* value_p = NULL;
  std::string network_name_string, server_name_string;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &tree_iter))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(%@), returning\n"),
                combo_box_p));
    return;
  } // end IF
  GtkTreeModel* tree_model_p = gtk_combo_box_get_model (combo_box_p);
  ACE_ASSERT (tree_model_p);
  //   gtk_tree_model_get_value(gtk_combo_box_get_model(serverlist),
  //                            &active_iter,
  //                            0, &active_value);
  gtk_tree_model_get (tree_model_p,
                      &tree_iter,
                      0, &value_p,
                      -1);
  //   ACE_ASSERT(G_VALUE_HOLDS_STRING(&active_value));
  ACE_ASSERT (value_p);
  // *TODO*: convert UTF8 to locale ?
  server_name_string = value_p;
  g_free (value_p);

  GtkTreePath* tree_path_p = gtk_tree_model_get_path (tree_model_p,
                                                      &tree_iter);
  ACE_ASSERT (tree_path_p);
  if (!gtk_tree_path_up (tree_path_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_tree_path_up(%@), returning\n"),
                tree_path_p));

    // clean up
    gtk_tree_path_free (tree_path_p);

    return;
  } // end IF
  if (!gtk_tree_model_get_iter (tree_model_p,
                                &tree_iter,
                                tree_path_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_tree_model_get_iter(%@,%@), returning\n"),
                tree_model_p, tree_path_p));

    // clean up
    gtk_tree_path_free (tree_path_p);

    return;
  } // end IF
  gtk_tree_path_free (tree_path_p);
  //   gtk_tree_model_get_value(gtk_combo_box_get_model(serverlist),
  //                            &active_iter,
  //                            0, &active_value);
  gtk_tree_model_get (tree_model_p,
                      &tree_iter,
                      0, &value_p,
                      -1);
  //   ACE_ASSERT(G_VALUE_HOLDS_STRING(&active_value));
  ACE_ASSERT (value_p);
  // *TODO*: convert UTF8 to locale ?
  network_name_string = value_p;
  //   entry_name = g_value_get_string(&active_value);

  // clean up
  //   g_value_unset(&active_value);
  g_free (value_p);

  if (network_name_string == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_PHONEBOOK_DEF_NETWORK_LABEL))
    network_name_string.clear ();
  IRC_Client_ServersIterator_t phonebook_iterator;
  for (phonebook_iterator = data_p->phoneBook.servers.begin ();
       phonebook_iterator != data_p->phoneBook.servers.end ();
       phonebook_iterator++)
    if (((*phonebook_iterator).second.netWork == network_name_string) &&
        ((*phonebook_iterator).first == server_name_string))
      break;
  if (phonebook_iterator == data_p->phoneBook.servers.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to look up active phonebook entry (was: \"%s\"), returning\n"),
                ACE_TEXT (server_name_string.c_str ())));
    return;
  } // end IF

  // step2: get/set nickname...
  IRC_LoginOptions login_options =
   data_p->configuration->protocolConfiguration.loginOptions;
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_MAIN)));
  ACE_ASSERT (entry_p);
  gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                0, -1);
  // enforce sane values
  // *TODO*: support the NICKLEN=xxx "feature" of the server...
  gtk_entry_set_max_length (entry_p,
                            IRC_PRT_MAXIMUM_NICKNAME_LENGTH);
  //   gtk_entry_set_width_chars(main_entry_entry,
  //                             -1); // reset to default
  gtk_entry_set_text (entry_p,
                      login_options.nickname.c_str ());
  gtk_editable_select_region (GTK_EDITABLE (entry_p),
                              0, -1);
  // retrieve entry dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_MAIN_ENTRY)));
  ACE_ASSERT (dialog_p);
  gtk_window_set_title (GTK_WINDOW (dialog_p),
                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_NICK));
  gint response_id = -1;
  bool nick_name_taken = false;
  do
  {
    response_id = gtk_dialog_run (dialog_p);
    if (response_id)
    { // cancelled
      // clean up
      gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                    0, -1);
      gtk_widget_hide (GTK_WIDGET (dialog_p));

      return;
    } // end IF

    const gchar* string_p = gtk_entry_get_text (entry_p);
    if (!string_p)
      continue; // empty --> try again
    login_options.nickname = string_p;
    //login_options.nickname = Common_UI_GTK_Tools::UTF82Locale (string_p, -1);
    //if (login_options.nickname.empty ())
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF82Locale(\"%s\"): \"%m\", returning\n"),
    //              ACE_TEXT (string_p)));

    //  // clean up
    //  gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
    //                                0, -1);
    //  gtk_widget_hide (GTK_WIDGET (dialog_p));

    //  return;
    //} // end IF

    // sanity check: <= IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH characters ?
    // *TODO*: support the NICKLEN=xxx "feature" of the server...
    if (login_options.nickname.size () > IRC_PRT_MAXIMUM_NICKNAME_LENGTH)
      login_options.nickname.resize (IRC_PRT_MAXIMUM_NICKNAME_LENGTH);

    // sanity check: nickname already in use ?
    nick_name_taken = false;
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      for (IRC_Client_GUI_ConnectionsConstIterator_t iterator_2 = data_p->connections.begin ();
           iterator_2 != data_p->connections.end ();
           ++iterator_2)
      {
        const struct IRC_Client_UI_ConnectionCBData& connection_data_r =
          (*iterator_2).second->getR ();
        const struct IRC_Client_SessionState& connection_state_r =
          (*iterator_2).second->state ();
        // *TODO*: the structure of the tab (label) is an implementation detail
        //         and should be encapsulated by the connection...
        if ((connection_data_r.label == server_name_string) &&
            (connection_state_r.nickName == login_options.nickname))
        {
          nick_name_taken = true;
          break;
        } // end IF
      } // end FOR
    } // end lock scope
    if (nick_name_taken)
    {
      // remind the user
      GtkMessageDialog* message_dialog_p =
          GTK_MESSAGE_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_MAIN_MESSAGE)));
      ACE_ASSERT (message_dialog_p);
      gtk_message_dialog_set_markup (message_dialog_p,
                                     ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_MESSAGEDIALOG_TEXT_NICKNAMETAKEN));
      response_id = gtk_dialog_run (GTK_DIALOG (message_dialog_p));
      gtk_widget_hide (GTK_WIDGET (message_dialog_p));

      continue; // taken --> try again
    } // end IF
    break;
  } while (true); // end WHILE
  gtk_widget_hide (GTK_WIDGET (dialog_p));
  gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                0, -1);

  // start connection thread
  IRC_Client_ConnectionThreadData* connection_thread_data_p = NULL;
  ACE_NEW_NORETURN (connection_thread_data_p,
                    IRC_Client_ConnectionThreadData ());
  if (!connection_thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  connection_thread_data_p->CBData = data_p;
  connection_thread_data_p->configuration = data_p->configuration;
  connection_thread_data_p->phonebookEntry = (*phonebook_iterator).second;
  connection_thread_data_p->loginOptions = login_options;
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
    delete connection_thread_data_p;

    return;
  } // end IF
  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  const char* thread_name_2 = thread_name_p;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  result =
    thread_manager_p->spawn (::connection_setup_function,      // function
                             connection_thread_data_p,         // argument
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
    delete connection_thread_data_p;

    return;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("started connection thread (id was: %d)...\n"),
              thread_id));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("started connection thread (id was: %u)...\n"),
              thread_id));
#endif

  // setup progress updates
  //if (data_p->progressData.cursorType == GDK_LAST_CURSOR)
  //{
  //  GtkWindow* window_p =
  //    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
  //                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
  //  ACE_ASSERT (window_p);
  //  GdkWindow* window_2 = gtk_widget_get_window (GTK_WIDGET (window_p));
  //  ACE_ASSERT (window_2);
  //  GdkCursor* cursor_p = gdk_window_get_cursor (window_2);
  //  data_p->progressData.cursorType = gdk_cursor_get_cursor_type (cursor_p);
  //  cursor_p = gdk_cursor_new (IRC_CLIENT_GUI_GTK_CURSOR_BUSY);
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
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_PROGRESSBAR)));
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
                          IRC_CLIENT_GUI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
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
entry_send_changed_cb (GtkWidget* widget_in,
                       gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::entry_send_changed_cb"));

  // sanity check(s)
  struct IRC_Client_UI_CBData* data_p =
    static_cast<IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkEntry* entry_p = GTK_ENTRY (widget_in);
  ACE_ASSERT (entry_p);
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_SEND)));
  ACE_ASSERT (button_p);

  guint16 text_length = gtk_entry_get_text_length (entry_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            (text_length != 0));
}

gboolean
entry_send_kb_focused_cb (GtkWidget* widget_in,
                          GdkEventFocus* event_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::entry_send_kb_focused_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  struct IRC_Client_UI_CBData* data_p =
    static_cast<IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // make the "change" button the default widget...
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_SEND)));
  ACE_ASSERT (button_p);
  gtk_widget_grab_default (GTK_WIDGET (button_p));

  // propagate the event further...
  return FALSE;
}

void
button_send_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_send_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct IRC_Client_UI_CBData* data_p =
    static_cast<IRC_Client_UI_CBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step0: retrieve active connection
  IRC_Client_GUI_Connection* connection_p =
    IRC_Client_UI_Tools::current (state_r,
                                  data_p->connections);
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_GUI_Tools::current(), returning\n")));
    return;
  } // end IF

  // step1: retrieve available data

  // step1a: retrieve buffer handle
  GtkEntryBuffer* buffer_p = NULL;
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_SEND)));
  ACE_ASSERT (entry_p);
  buffer_p = gtk_entry_get_buffer (entry_p);
  ACE_ASSERT (buffer_p);

  // sanity check
  guint16 text_length = gtk_entry_buffer_get_length (buffer_p);
  if (text_length == 0)
    return; // nothing to do...

  // step1b: retrieve textbuffer data
  const gchar* string_p = gtk_entry_buffer_get_text (buffer_p);
  ACE_ASSERT (string_p);
  std::string message_string;
  switch (data_p->configuration->encoding)
  {
    case IRC_CHARACTERENCODING_ASCII:
    {
      // *TODO*:
      ACE_ASSERT (false);
      break;
    }
    case IRC_CHARACTERENCODING_LOCALE:
    {
      message_string = Common_UI_GTK_Tools::UTF8ToLocale (string_p, -1);
      if (message_string.empty ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"): \"%m\", returning\n"),
                    string_p));

        // clean up
        gtk_entry_buffer_delete_text (buffer_p, // buffer
                                      0,        // start at position 0
                                      -1);      // delete everything

        return;
      } // end IF

      break;
    }
    case IRC_CHARACTERENCODING_UTF8:
    {
      message_string = string_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid character encoding (was: %d), returning\n"),
                  data_p->configuration->encoding));

      // clean up
      gtk_entry_buffer_delete_text (buffer_p, // buffer
                                    0,        // start at position 0
                                    -1);      // delete everything

      return;
    }
  } // end SWITCH

  // step2: retrieve active handler(s) (channel/nick)
  // *TODO*: allow multicast to several channels ?
  //std::string active_id = (*connections_iterator).second->getActiveID ();
  IRC_Client_GUI_MessageHandler* message_handler_p =
    connection_p->getActiveHandler (false,
                                    false);
  if (!message_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_GUI_Connection::getActiveHandler(), returning\n")));
    return;
  } // end IF
  const struct IRC_Client_UI_HandlerCBData& cb_data_r =
      message_handler_p->getR ();
  std::string active_id = cb_data_r.id;

  // step3: pass data to controller
  string_list_t receivers;
  receivers.push_back (active_id);
  //IRC_Client_IIRCControl* controller_p =
  //  (*connections_iterator).second->getController ();
  const struct IRC_Client_UI_ConnectionCBData& connection_data_r =
    connection_p->getR ();
  ACE_ASSERT (connection_data_r.controller);
  try {
    connection_data_r.controller->send (receivers,
                                        message_string);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::send(), continuing\n")));
  }

  // step4: echo data locally...
  try {
    message_handler_p->queueForDisplay (string_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_GUI_MessageHandler::queueForDisplay(), continuing\n")));
  }

  // step5: clear buffer
  gtk_entry_buffer_delete_text (buffer_p, // buffer
                                0,        // start at position 0
                                -1);      // delete everything
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
  //{ ACE_Guard<ACE_SYNCH_MUTEX> aGuard (data_p->lock);
  //  for (Common_UI_GTKeventSourceIdsIterator_t iterator = data_p->eventSourceIds.begin ();
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
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, true);
}

void
button_disconnect_clicked_cb (GtkWidget* widget_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_disconnect_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  try {
    data_p->controller->quit (ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_LEAVE_REASON));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::quit(), continuing\n")));
  }

  // update widgets
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  // *NOTE*: the server should close the connection after this...
  //         --> the connection notebook page cleans itself (see end ())
  IRC_Client_GUI_ConnectionsConstIterator_t iterator =
      data_p->connections->find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == data_p->connections->end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    return;
  } // end IF
  ACE_ASSERT ((*iterator).second);
  (*iterator).second->finalize (false); // don't lock
  gtk_widget_set_sensitive (widget_in, FALSE);
}

gboolean
nickname_entry_kb_focused_cb (GtkWidget* widget_in,
                              GdkEventFocus* event_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::nickname_entry_kb_focused_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // step0: retrieve active connection
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    return TRUE; // propagate
  } // end IF

  // make the "change" button the default widget...
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_NICK_ACCEPT)));
  ACE_ASSERT (button_p);
  gtk_widget_grab_default (GTK_WIDGET (button_p));

  return FALSE;
}

void
nickname_clicked_cb (GtkWidget* widget_in,
                     gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::nickname_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // step0: retrieve active connection
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    return;
  } // end IF

  // step1: retrieve available data
  // retrieve buffer handle
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_NICK)));
  ACE_ASSERT (entry_p);
  GtkEntryBuffer* entry_buffer_p = gtk_entry_get_buffer (entry_p);
  ACE_ASSERT (entry_buffer_p);

  // sanity check
  guint16 text_length = gtk_entry_buffer_get_length (entry_buffer_p);
  if (text_length == 0)
    return; // nothing to do...

  // retrieve textbuffer data
  std::string nickname_string =
    gtk_entry_buffer_get_text (entry_buffer_p);
  ACE_ASSERT (!nickname_string.empty ());
  //  Common_UI_GTK_Tools::UTF82Locale (gtk_entry_buffer_get_text (entry_buffer_p), // text
  //                                text_length);                               // number of bytes
  //if (nickname_string.empty ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to convert nickname (was: \"%s\"), returning\n"),
  //              gtk_entry_buffer_get_text (entrybuffer_p)));

  //  // clean up
  //  gtk_entry_buffer_delete_text (entry_buffer_p, // buffer
  //                                0,              // start at position 0
  //                                -1);            // delete everything

  //  return;
  //} // end IF

  // sanity check: <= IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH characters ?
  // *TODO*: support the NICKLEN=xxx "feature" of the server...
  if (nickname_string.size () > IRC_PRT_MAXIMUM_NICKNAME_LENGTH)
    nickname_string.resize (IRC_PRT_MAXIMUM_NICKNAME_LENGTH);

  try {
    data_p->controller->nick (nickname_string);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::nick(), continuing\n")));
  }

  // clear buffer
  gtk_entry_buffer_delete_text (entry_buffer_p, // buffer
                                0,              // start at position 0
                                -1);            // delete everything
}

void
usersbox_changed_cb (GtkWidget* widget_in,
                     gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::usersbox_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connections);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // step1: retrieve active users entry
  // retrieve server tab users combobox handle
  GtkComboBox* combo_box_p = GTK_COMBO_BOX (widget_in);
  ACE_ASSERT (combo_box_p);
  GtkTreeIter tree_iter;
  //   GValue active_value;
  gchar* string_p = NULL;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &tree_iter))
    return; // done

  gtk_tree_model_get (gtk_combo_box_get_model (combo_box_p),
                      &tree_iter,
                      0, &string_p, // just retrieve the first column...
                      -1);
//   ACE_ASSERT(G_VALUE_HOLDS_STRING(&string_p));
  ACE_ASSERT (string_p);

  // convert UTF8 to locale
//   user_string = g_value_get_string(&active_value);
  std::string username =
    Common_UI_GTK_Tools::UTF8ToLocale (string_p,
                                       g_utf8_strlen (string_p, -1));
  if (username.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"), returning\n"),
                ACE_TEXT (string_p)));

    // clean up
    g_free (string_p);

    return;
  } // end IF
  g_free (string_p);
  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH characters ?
  // *TODO*: support the NICKLEN=xxx "feature" of the server...
  if (username.size () > IRC_PRT_MAXIMUM_NICKNAME_LENGTH)
    username.resize (IRC_PRT_MAXIMUM_NICKNAME_LENGTH);

  // *TODO*: if a conversation exists, simply activate the corresponding page
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    IRC_Client_GUI_ConnectionsConstIterator_t iterator =
      data_p->connections->find (data_p->timeStamp);
    if (iterator == data_p->connections->end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection (timestamp was: \"%s\") not found, returning\n"),
                  ACE_TEXT (data_p->timeStamp.c_str ())));
      return;
    } // end IF
    ACE_ASSERT ((*iterator).second);
    (*iterator).second->createMessageHandler (username,
                                              false,
                                              false);
  } // end lock scope
}

void
refresh_users_clicked_cb (GtkWidget* widget_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::refresh_users_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

//   // *NOTE*: empty parameter --> current server
//   std::string servername;
//   try
//   {
//     data->controller->users(servername);
//   }
//   catch (...)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("caught exception in RPG_Net_Protocol_IIRCControl::users(), continuing\n")));
//   }

  // *NOTE*: empty parameter (or "0") --> ALL users
  // (see RFC1459 section 4.5.1)
  std::string name (ACE_TEXT_ALWAYS_CHAR ("0"));
  try {
    data_p->controller->who (name, false);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::who(), continuing\n")));
  }
}

gboolean
channel_entry_kb_focused_cb (GtkWidget* widget_in,
                             GdkEventFocus* event_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::channel_entry_kb_focused_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // step0: retrieve active connection
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    return TRUE; // propagate
  } // end IF

  // make the "change" button the default widget...
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_JOIN)));
  ACE_ASSERT (button_p);
  gtk_widget_grab_default (GTK_WIDGET (button_p));

  return FALSE;
}

void
join_clicked_cb (GtkWidget* widget_in,
                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::join_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // step0: retrieve active connection
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    return;
  } // end IF

  // step1: retrieve available data
  // retrieve buffer handle
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_CONNECTION_CHANNEL)));
  ACE_ASSERT (entry_p);
  GtkEntryBuffer* entry_buffer_p = gtk_entry_get_buffer (entry_p);
  ACE_ASSERT (entry_buffer_p);

  // sanity check
  guint16 text_length = gtk_entry_buffer_get_length (entry_buffer_p);
  if (text_length == 0)
    return; // nothing to do...

  // retrieve textbuffer data
  std::string channel_string =
    gtk_entry_buffer_get_text (entry_buffer_p);
  ACE_ASSERT (!channel_string.empty ());
  //  Common_UI_GTK_Tools::UTF82Locale (gtk_entry_buffer_get_text (entrybuffer_p), // text
  //                                text_length);                              // number of bytes
  //if (channel_string.empty ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to convert channel name (was: \"%s\"), returning\n"),
  //              gtk_entry_buffer_get_text (entrybuffer_p)));

  //  // clean up
  //  gtk_entry_buffer_delete_text (entrybuffer_p, // buffer
  //                                0,             // start at position 0
  //                                -1);           // delete everything

  //  return;
  //} // end IF

  // sanity check(s): has '#' prefix ?
  if (channel_string.find ('#', 0) != 0)
    channel_string.insert (channel_string.begin (), '#');
  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
  // *TODO*: support the CHANNELLEN=xxx "feature" of the server...
  if (channel_string.size () > IRC_PRT_MAXIMUM_CHANNEL_LENGTH)
    channel_string.resize (IRC_PRT_MAXIMUM_CHANNEL_LENGTH);

  // *TODO*: support channel keys/multi-join ?
  string_list_t channels, keys;
  channels.push_back (channel_string);
  try {
    data_p->controller->join (channels, keys);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::join(), continuing\n")));
  }

  // clear buffer
  gtk_entry_buffer_delete_text (entry_buffer_p, // buffer
                                0,              // start at position 0
                                -1);            // delete everything
}

void
channelbox_changed_cb (GtkWidget* widget_in,
                       gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::channelbox_changed_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  // step1: retrieve active channel entry
  // retrieve server tab channels combobox handle
  GtkComboBox* combobox_p = GTK_COMBO_BOX (widget_in);
  ACE_ASSERT (combobox_p);
  GtkTreeIter active_iter;
  //   GValue active_value;
  gchar* channel_value = NULL;
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
                      0, &channel_value, // just retrieve the first column...
                      -1);
  //   ACE_ASSERT(G_VALUE_HOLDS_STRING(&active_value));
  ACE_ASSERT (channel_value);

  // convert UTF8 to locale
//   channel_string = g_value_get_string(&active_value);
  std::string channel_string =
    Common_UI_GTK_Tools::UTF8ToLocale (channel_value,
                                       g_utf8_strlen (channel_value, -1));
  if (channel_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF8ToLocale(\"%s\"), returning\n"),
                channel_value));

    // clean up
    g_free (channel_value);

    return;
  } // end IF

  // clean up
  g_free (channel_value);

  // sanity check(s): has '#' prefix ?
  if (channel_string.find ('#', 0) != 0)
    channel_string.insert (channel_string.begin (), '#');
  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
  // *TODO*: support the CHANNELLEN=xxx "feature" of the server...
  if (channel_string.size () > IRC_PRT_MAXIMUM_CHANNEL_LENGTH)
    channel_string.resize (IRC_PRT_MAXIMUM_CHANNEL_LENGTH);

  // *TODO*: support channel key ?
  string_list_t channels;
  channels.push_back (channel_string);
  string_list_t keys;
  try {
    data_p->controller->join (channels, keys);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::join(), continuing\n")));
  }
}

void
refresh_channels_clicked_cb (GtkWidget* widget_in,
                             gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::refresh_channels_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  // *NOTE*: empty list --> list them all !
  string_list_t channel_list;
  try {
    data_p->controller->list (channel_list);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::list(), continuing\n")));
  }
}

void
user_mode_toggled_cb (GtkToggleButton* toggleButton_in,
                      gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::user_mode_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connections);
  ACE_ASSERT (data_p->controller);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  int result = -1;
  IRC_UserMode mode = USERMODE_INVALID;
  // find out which button toggled...
  const gchar* name_p =
    gtk_buildable_get_name (GTK_BUILDABLE (toggleButton_in));
  result = ACE_OS::strcmp (name_p,
                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY));
  if (result == 0)
    mode = USERMODE_AWAY;
  else
  {
    result = ACE_OS::strcmp (name_p,
                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE));
    if (result == 0)
      mode = USERMODE_INVISIBLE;
    else
    {
      result = ACE_OS::strcmp (name_p,
                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_OPERATOR));
      if (result == 0)
        mode = USERMODE_OPERATOR;
      else
      {
        result = ACE_OS::strcmp (name_p,
                                 ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_LOCALOPERATOR));
        if (result == 0)
          mode = USERMODE_LOCALOPERATOR;
        else
        {
          result = ACE_OS::strcmp (name_p,
                                   ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_RESTRICTED));
          if (result == 0)
            mode = USERMODE_RESTRICTEDCONN;
          else
          {
            result = ACE_OS::strcmp (name_p,
                                     ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES));
            if (result == 0)
              mode = USERMODE_RECVNOTICES;
            else
            {
              result = ACE_OS::strcmp (name_p,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS));
              if (result == 0)
                mode = USERMODE_RECVWALLOPS;
              else
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("unknown/invalid user mode toggled (was: 0x%@/\"%s\"), returning\n"),
                            toggleButton_in, ACE_TEXT (name_p)));
                return;
              } // end ELSE
            } // end ELSE
          } // end ELSE
        } // end ELSE
      } // end ELSE
    } // end ELSE
  } // end ELSE

  // check if this change is actually an acknowledgement (avoids recursion too)
  if (data_p->acknowledgements || data_p->pending)
  {
    // --> server has acknowledged a mode change ?
    if (data_p->acknowledgements)
    {
      gtk_toggle_button_set_inconsistent (toggleButton_in, FALSE);
      --data_p->acknowledgements;
    } // end IF
    else
      data_p->pending = false;
    return; // done
  } // end IF

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  IRC_Client_GUI_ConnectionsConstIterator_t iterator =
    data_p->connections->find (data_p->timeStamp);
  // sanity check(s)
  if (iterator == data_p->connections->end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    return;
  } // end IF
  ACE_ASSERT ((*iterator).second);
  const IRC_Client_SessionState& connection_state_r =
    (*iterator).second->state ();

  // re-toggle button (until acknowledgement from the server arrives...)
  data_p->pending = true;
  gtk_toggle_button_set_active (toggleButton_in,
                                connection_state_r.userModes.test (mode));
  gtk_toggle_button_set_inconsistent (toggleButton_in, TRUE);

  string_list_t parameters;
  try {
    data_p->controller->mode (connection_state_r.nickName,               // user mode
                              IRC_Tools::UserModeToChar (mode),          // corresponding mode char
                              !connection_state_r.userModes.test (mode), // enable ?
                              parameters);                               // parameters
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::mode(\"%s\"), continuing\n"),
                ACE_TEXT (IRC_Tools::UserModeToString (mode).c_str ())));
  }
}

void
switch_channel_cb (GtkNotebook* notebook_in,
                   //GtkNotebookPage* page_in,
                   GtkWidget* page_in,
                   guint pageNum_in,
                   gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::switch_channel_cb"));

  ACE_UNUSED_ARG (notebook_in);
  ACE_UNUSED_ARG (page_in);

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*> (userData_in);
  ACE_ASSERT (data_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // check whether the switch was to the server log tab
  // --> disable corresponding widget(s) in the main UI
  GtkHBox* hbox_p =
      GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_SEND)));
  ACE_ASSERT (hbox_p);
  gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), (pageNum_in != 0));
}

void
action_away_cb (GtkAction* action_in,
                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_away_cb"));

  // sanity check(s)
  struct IRC_Client_UI_ConnectionCBData* data_p =
    static_cast<IRC_Client_UI_ConnectionCBData*>(userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkToggleButton* togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_TOOLS_1)));
  ACE_ASSERT (togglebutton_p);

  bool activating = gtk_toggle_button_get_active (togglebutton_p);

  // check if the state is inconsistent --> submit change request, else do nothing
  // i.e. state is off and widget is "on" or vice-versa
  IRC_Client_GUI_ConnectionsConstIterator_t iterator_2 =
    data_p->connections->find (data_p->timeStamp);
  // sanity check(s)
  if (iterator_2 == data_p->connections->end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") not found, returning\n"),
                ACE_TEXT (data_p->label.c_str ())));
    return;
  } // end IF
  ACE_ASSERT ((*iterator_2).second);
  const IRC_Client_SessionState& connection_state_r =
    (*iterator_2).second->state ();
  // *NOTE*: avoid recursion
  if (connection_state_r.away == activating)
    return;
  // re-toggle button for now...
  // *NOTE*: will be auto-toggled according to the outcome of the change request
  gtk_toggle_button_set_active (togglebutton_p,
                                connection_state_r.away);

  // activating ? --> retrieve away message
  std::string away_message;
  if (activating)
  {
    GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_CONNECTION)));
    ACE_ASSERT (entry_p);
    // clean up
    gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                  0, -1);
  //   gtk_entry_set_max_length(entry_p,
  //                            0); // no limit
  //   gtk_entry_set_width_chars(entry_p,
  //                             -1); // reset to default
    gtk_entry_set_text (entry_p,
                        ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_AWAY_MESSAGE));
    gtk_editable_select_region (GTK_EDITABLE (entry_p),
                                0, -1);

    GtkDialog* dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_CONNECTION_ENTRY)));
    ACE_ASSERT (dialog_p);
    gtk_window_set_title (GTK_WINDOW (dialog_p),
                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_AWAY));
    if (gtk_dialog_run (dialog_p))
    {
  //     ACE_DEBUG((LM_DEBUG,
  //                ACE_TEXT("away cancelled...\n")));

      // clean up
      gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                    0, -1);

      gtk_widget_hide (GTK_WIDGET (dialog_p));

      return;
    } // end IF
    //gtk_widget_hide (GTK_WIDGET (dialog_p));

    away_message =
      Common_UI_GTK_Tools::UTF8ToLocale (gtk_entry_get_text (entry_p),
                                     -1);
    // clean up
    gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                  0, -1);

    if (away_message.empty ())
    {
      // *NOTE*: need to set SOME value, as an AWAY-message with no parameter will
      // actually "un-away" the user (see RFC1459 Section 5.1)...
      away_message = ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_AWAY_MESSAGE);
    } // end IF
  } // end IF

  try {
    data_p->controller->away (away_message);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::away(\"%s\"), continuing\n"),
                ACE_TEXT (away_message.c_str ())));
  }
}

void
channel_mode_toggled_cb (GtkToggleButton* toggleButton_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::channel_mode_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->id.empty ());

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  int result = -1;
  IRC_ChannelMode mode           = CHANNELMODE_INVALID;
  bool            need_parameter = false;
  std::string     entry_label;
  // find out which button toggled...
  const gchar* name_p =
    gtk_buildable_get_name (GTK_BUILDABLE (toggleButton_in));
  result =
    ACE_OS::strcmp (name_p,
                    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_ANONYMOUS));
  if (result == 0)
    mode = CHANNELMODE_ANONYMOUS;
  else
  {
    result =
      ACE_OS::strcmp (name_p,
                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BAN));
    if (result == 0)
    {
      mode = CHANNELMODE_BAN;
      need_parameter = true;
      entry_label =
        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_BAN);
    } // end IF
    else
    {
      result =
        ACE_OS::strcmp (name_p,
                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_INVITEONLY));
      if (result == 0)
        mode = CHANNELMODE_INVITEONLY;
      else
      {
        result =
          ACE_OS::strcmp (name_p,
                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_KEY));
        if (result == 0)
        {
          mode = CHANNELMODE_PASSWORD;
          need_parameter = true;
          entry_label =
            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_PASSWORD);
        } // end IF
        else
        {
          result =
            ACE_OS::strcmp (name_p,
                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_USERLIMIT));
          if (result == 0)
          {
            mode = CHANNELMODE_USERLIMIT;
            need_parameter = true;
            entry_label =
              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_USERLIMIT);
          }
          else
          {
            result =
              ACE_OS::strcmp (name_p,
                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_MODERATED));
            if (result == 0)
              mode = CHANNELMODE_MODERATED;
            else
            {
              result =
                ACE_OS::strcmp (name_p,
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BLOCKFOREIGN));
              if (result == 0)
                mode = CHANNELMODE_BLOCKFOREIGNMSGS;
              else
              {
                result =
                  ACE_OS::strcmp (name_p,
                                  ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_OPERATOR));
                if (result == 0)
                  mode = CHANNELMODE_OPERATOR;
                else
                {
                  result =
                    ACE_OS::strcmp (name_p,
                                    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_PRIVATE));
                  if (result == 0)
                    mode = CHANNELMODE_PRIVATE;
                  else
                  {
                    result =
                      ACE_OS::strcmp (name_p,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_QUIET));
                    if (result == 0)
                      mode = CHANNELMODE_QUIET;
                    else
                    {
                      result =
                        ACE_OS::strcmp (name_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_REOP));
                      if (result == 0)
                        mode = CHANNELMODE_REOP;
                      else
                      {
                        result =
                          ACE_OS::strcmp (name_p,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_SECRET));
                        if (result == 0)
                          mode = CHANNELMODE_SECRET;
                        else
                        {
                          result =
                            ACE_OS::strcmp (name_p,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_RESTRICTOPIC));
                          if (result == 0)
                            mode = CHANNELMODE_RESTRICTEDTOPIC;
                          else
                          {
                            result =
                              ACE_OS::strcmp (name_p,
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_VOICE));
                            if (result == 0)
                            {
                              mode = CHANNELMODE_VOICE;
                              need_parameter = true;
                              entry_label =
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_VOICE);
                            } // end IF
                            else
                            {
                              ACE_DEBUG ((LM_ERROR,
                                          ACE_TEXT ("unknown/invalid channel mode toggled (was: %@), returning\n"),
                                          toggleButton_in));
                              return;
                            } // end ELSE
                          } // end ELSE
                        } // end ELSE
                      } // end ELSE
                    } // end ELSE
                  } // end ELSE
                } // end ELSE
              } // end ELSE
            } // end ELSE
          } // end ELSE
        } // end ELSE
      } // end ELSE
    } // end ELSE
  } // end ELSE

  // check if this change is actually an acknowledgement (avoids recursion too)
  if (data_p->acknowledgements || data_p->pending)
  {
    // --> server has acknowledged a mode change ?
    if (data_p->acknowledgements)
    {
      gtk_toggle_button_set_inconsistent (toggleButton_in, FALSE);
      --data_p->acknowledgements;
    } // end IF
    else
      data_p->pending = false;
    return; // done
  } // end IF

  // re-toggle button (until acknowledgement from the server arrives...)
  data_p->pending = true;
  gtk_toggle_button_set_active (toggleButton_in,
                                data_p->channelModes.test (mode));
  gtk_toggle_button_set_inconsistent (toggleButton_in, TRUE);

  // *NOTE*: see also: IRC_Client_MessageHandler ctor
  std::string page_tab_label_string;
  if (!IRC_Tools::isValidChannelName (data_p->id))
  {
    // --> private conversation window, modify label accordingly
    page_tab_label_string = ACE_TEXT_ALWAYS_CHAR ("[");
    page_tab_label_string += data_p->id;
    page_tab_label_string += ACE_TEXT_ALWAYS_CHAR ("]");
  } // end IF
  else
    page_tab_label_string = data_p->id;

  // *TODO*: there must be a better way to do this
  //         (see: IRC_client_messagehandler.cpp:480)
  const struct IRC_Client_UI_ConnectionCBData& connection_data_r =
    data_p->connection->getR ();
  std::string builder_label = connection_data_r.timeStamp;
  builder_label += ACE_TEXT_ALWAYS_CHAR ("::");
  builder_label += page_tab_label_string;

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (builder_label);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  std::string value;
  string_list_t parameters;
  if (need_parameter &&
      !gtk_toggle_button_get_active (toggleButton_in)) // no need when switching mode off...
  {
    GtkEntry* entry_p =
      GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_CHANNEL)));
    ACE_ASSERT (entry_p);
    // clean up
    gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                  0, -1);
//   gtk_entry_set_max_length(entry_p,
//                            0); // no limit
//   gtk_entry_set_width_chars(entry_p,
//                             -1); // reset to default

    // retrieve entry dialog handle
    GtkDialog* dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_CHANNEL_ENTRY)));
    ACE_ASSERT (dialog_p);
    gtk_window_set_title (GTK_WINDOW (dialog_p),
                          entry_label.c_str ());
    gint response_id = gtk_dialog_run (dialog_p);
    if (response_id)
    {
//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("channel mode cancelled...\n")));

      // clean up
      gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                    0, -1);
      gtk_widget_hide (GTK_WIDGET (dialog_p));

      return;
    } // end IF
    gtk_widget_hide (GTK_WIDGET (dialog_p));

    value = gtk_entry_get_text (entry_p);
    // clean up
    gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                  0, -1);

    if (value.empty ())
    {
    //     ACE_DEBUG((LM_DEBUG,
    //                ACE_TEXT("channel mode cancelled...\n")));
      return;
    } // end IF

    parameters.push_back (value);
  } // end IF

  try {
    data_p->controller->mode (data_p->id,                          // channel name
                              IRC_Tools::ChannelModeToChar (mode), // corresponding mode char
                              !data_p->channelModes.test (mode),   // enable ?
                              parameters);                         // parameters
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::mode(\"%s\"), continuing\n"),
                ACE_TEXT (IRC_Tools::ChannelModeToString (mode).c_str ())));
  }
}

void
topic_clicked_cb (GtkWidget* widget_in,
                  GdkEventButton* event_in,
                  gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::topic_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->id.empty ());

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  // *NOTE*: see also: IRC_Client_MessageHandler ctor
  std::string page_tab_label_string;
  if (!IRC_Tools::isValidChannelName (data_p->id))
  {
    // --> private conversation window, modify label accordingly
    page_tab_label_string = ACE_TEXT_ALWAYS_CHAR ("[");
    page_tab_label_string += data_p->id;
    page_tab_label_string += ACE_TEXT_ALWAYS_CHAR ("]");
  } // end IF
  else
    page_tab_label_string = data_p->id;

  // *TODO*: there must be a better way to do this
  //         (see: IRC_client_messagehandler.cpp:480)
  const struct IRC_Client_UI_ConnectionCBData& connection_data_r =
    data_p->connection->getR ();
  std::string builder_label = connection_data_r.timeStamp;
  builder_label += ACE_TEXT_ALWAYS_CHAR ("::");
  builder_label += page_tab_label_string;

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (builder_label);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve entry handle
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_CHANNEL)));
  ACE_ASSERT (entry_p);
//   gtk_entry_set_max_length(entry_p,
//                            0); // no limit
//   gtk_entry_set_width_chars(entry_p,
//                             -1); // reset to default
  // retrieve label handle
  GtkLabel* label_p =
    GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_TOPIC)));
  ACE_ASSERT (label_p);
  gtk_entry_buffer_set_text (gtk_entry_get_buffer (entry_p),
                             gtk_label_get_text (label_p),
                             g_utf8_strlen (gtk_label_get_text (label_p), -1));
  gtk_editable_select_region (GTK_EDITABLE (entry_p),
                              0, -1);

  // retrieve entry dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_CHANNEL_ENTRY)));
  ACE_ASSERT (dialog_p);
  gtk_window_set_title (GTK_WINDOW (dialog_p),
                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_TOPIC));
  if (gtk_dialog_run (dialog_p))
  {
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("channel topic cancelled...\n")));

    // clean up
    gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                  0, -1);

    gtk_widget_hide (GTK_WIDGET (dialog_p));

    return;
  } // end IF
  gtk_widget_hide (GTK_WIDGET (dialog_p));

  std::string topic_string = gtk_entry_get_text (entry_p);
  try {
    data_p->controller->topic (data_p->id,
                               topic_string);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::topic(), continuing\n")));
  }

  // clean up
  gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                0, -1);
}

void
part_clicked_cb (GtkWidget* widget_in,
                 gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::part_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->id.empty ());

  // *NOTE*: if 'this' is not a channel handler (i.e. private dialogue), just
  //         close the page tab
  if (!IRC_Tools::isValidChannelName (data_p->id))
    data_p->connection->terminateMessageHandler (data_p->id);
  else
  { // --> inform the server
    string_list_t channels;
    channels.push_back (data_p->id);
    try {
      data_p->controller->part (channels);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in IRC_Client_IIRCControl::part(), continuing\n")));
    }
  } // end IF

  gtk_widget_set_sensitive (widget_in, FALSE);
}

gboolean
members_clicked_cb (GtkWidget* widget_in,
                    GdkEventButton* event_in,
                    gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::members_clicked_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (GTK_TREE_VIEW (widget_in));
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  // supposed to be a context menu -> right-clicked ?
  if (event_in->button != 3)
    return FALSE; // --> propagate event

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, TRUE);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // find out which row was actually clicked
  GtkTreePath* tree_path_p = NULL;
  if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget_in),
                                      static_cast<gint> (event_in->x), static_cast<gint> (event_in->y),
                                      &tree_path_p, NULL,
                                      NULL, NULL))
    return FALSE; // no row at this position --> propagate event
  ACE_ASSERT (tree_path_p);

  // retrieve current selection
  GtkTreeSelection* tree_selection_p =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (widget_in));
  ACE_ASSERT (tree_selection_p);
  // nothing selected ? --> nothing to do
  if (gtk_tree_selection_count_selected_rows (tree_selection_p) == 0)
  {
    // clean up
    gtk_tree_path_free (tree_path_p);

    return TRUE; // done
  } // end IF

  // path part of the selection ? --> keep selection : new selection
  if (!gtk_tree_selection_path_is_selected (tree_selection_p, tree_path_p))
  {
    gtk_tree_selection_unselect_all (tree_selection_p);
    gtk_tree_selection_select_path (tree_selection_p, tree_path_p);
  } // end IF

  // clean up
  gtk_tree_path_free (tree_path_p);

  GList* list_p = NULL;
  GtkTreeModel* tree_model_p = NULL;
  list_p = gtk_tree_selection_get_selected_rows (tree_selection_p,
                                                 &tree_model_p);
  ACE_ASSERT (list_p);
  ACE_ASSERT (tree_model_p);
  data_p->parameters.clear ();
  GtkTreePath* tree_path_2 = NULL;
  GtkTreeIter tree_iter;
//   GValue current_value;
  gchar* string_p = NULL;
  std::string nickname_string;
  for (GList* iterator_2 = g_list_first (list_p);
       iterator_2 != NULL;
       iterator_2 = g_list_next (iterator_2))
  {
    tree_path_2 = static_cast<GtkTreePath*> (iterator_2->data);
    ACE_ASSERT (tree_path_2);

    // path --> iter
    if (!gtk_tree_model_get_iter (tree_model_p,
                                  &tree_iter,
                                  tree_path_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_tree_model_get_iter, continuing\n")));
      continue;
    } // end IF
    // iter --> value
//     gtk_tree_model_get_value(model,
//                              current_iter,
//                              0, &current_value);
    gtk_tree_model_get (tree_model_p,
                        &tree_iter,
                        0, &string_p,
                        -1);

    nickname_string = string_p;
    //  Common_UI_GTK_Tools::UTF82Locale (string_p,
    //                                g_utf8_strlen (string_p, -1));
    //if (nickname_string.empty ())
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Common_UI_GTK_Tools::UTF82Locale(\"%s\"): \"%m\", continuing\n"),
    //              ACE_TEXT (string_p)));
    //  continue;
    //} // end IF
    if (nickname_string[0] == '@')
      nickname_string.erase (nickname_string.begin ());
    data_p->parameters.push_back (nickname_string);

    // clean up
    g_free (string_p);
    gtk_tree_path_free (tree_path_2);
  } // end FOR

  // clean up
  g_list_free (list_p);

  // remove current nickname from any selection...
  string_list_iterator_t iterator_2 = data_p->parameters.begin ();
  string_list_iterator_t self = data_p->parameters.end ();
  const IRC_Client_SessionState& connection_state_r =
    data_p->connection->state ();
  //const IRC_Client_UI_ConnectionCBData& connection_data_r =
  //  data_p->connection->get ();
  for (;
       iterator_2 != data_p->parameters.end ();
       iterator_2++)
  {
    if (*iterator_2 == connection_state_r.nickName)
    {
      self = iterator_2;
      continue;
    } // end IF
    // *NOTE*: ignore leading '@'
    if ((*iterator_2).find ('@', 0) == 0)
      if (((*iterator_2).find (connection_state_r.nickName, 1) == 1) &&
          ((*iterator_2).size () == (connection_state_r.nickName.size () + 1)))
      {
        self = iterator_2;
        continue;
      } // end IF
  } // end FOR
  if (self != data_p->parameters.end ())
    data_p->parameters.erase (self);
  // no selection ? --> nothing to do
  if (data_p->parameters.empty ())
    return TRUE; // done

  // initialize popup menu
  GtkMenu* menu_p =
    GTK_MENU (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENU_CHANNEL)));
  ACE_ASSERT (menu_p);

  // initialize active_channels submenu
  GtkMenuItem* menu_item_p = NULL;
  // --> retrieve list of active channels
  // *TODO*: for invite-only channels, only operators are allowed to invite
  //         strangers
  //         --> remove those channels where this condition doesn't apply
  string_list_t active_channels;
  data_p->connection->channels (active_channels);
  if (active_channels.size () > 1)
  {
    // clear popup menu
    GtkMenu* menu_2 =
      GTK_MENU (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE)));
    ACE_ASSERT (menu_2);
    list_p =
      gtk_container_get_children (GTK_CONTAINER (menu_2));
    if (list_p)
    {
      for (GList* list_2 = g_list_first (list_p);
           list_2 != NULL;
           list_2 = g_list_next (list_2))
        gtk_container_remove (GTK_CONTAINER (menu_2),
                              GTK_WIDGET (list_2->data));
      // clean up
      g_list_free (list_p);
    } // end IF

    // populate popup menu
    GtkAction* action_p =
      GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_INVITE)));
    ACE_ASSERT (action_p);
    menu_item_p = NULL;
    for (string_list_const_iterator_t iterator_2 = active_channels.begin ();
         iterator_2 != active_channels.end ();
         iterator_2++)
    {
      // skip current channel
      if (*iterator_2 == data_p->id)
        continue;

      menu_item_p = GTK_MENU_ITEM (gtk_action_create_menu_item (action_p));
      ACE_ASSERT (menu_item_p);
      gtk_menu_item_set_label (menu_item_p,
                               (*iterator_2).c_str ());
      gtk_menu_shell_append (GTK_MENU_SHELL (menu_2),
                             GTK_WIDGET (menu_item_p));
    } // end FOR
    gtk_widget_show_all (GTK_WIDGET (menu_2));

    menu_item_p =
      GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_INVITE)));
    ACE_ASSERT (menu_item_p);
    gtk_widget_set_sensitive (GTK_WIDGET (menu_item_p), TRUE);
  } // end IF
  else
  {
    menu_item_p =
      GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_INVITE)));
    ACE_ASSERT (menu_item_p);
    gtk_widget_set_sensitive (GTK_WIDGET (menu_item_p), FALSE);
  } // end ELSE

  // en-/disable some entries...
  menu_item_p =
    GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_KICK)));
  ACE_ASSERT (menu_item_p);
  gtk_widget_set_sensitive (GTK_WIDGET (menu_item_p),
                            data_p->channelModes.test (CHANNELMODE_OPERATOR));
  menu_item_p =
    GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_BAN)));
  ACE_ASSERT (menu_item_p);
  gtk_widget_set_sensitive (GTK_WIDGET (menu_item_p),
                            data_p->channelModes.test (CHANNELMODE_OPERATOR));

  gtk_menu_popup (menu_p,
                  NULL, NULL,
                  NULL, NULL,
                  event_in->button,
                  event_in->time);

  return TRUE;
}

void
action_msg_cb (GtkAction* action_in,
               gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_msg_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (!data_p->parameters.empty ());

  gint page_number = -1;
  for (string_list_const_iterator_t iterator = data_p->parameters.begin ();
       iterator != data_p->parameters.end ();
       iterator++)
  {
    page_number = data_p->connection->exists (*iterator,
                                              false);
    if (page_number == -1)
      data_p->connection->createMessageHandler (*iterator,
                                                true,
                                                false);
  } // end FOR
}

void
action_invite_cb (GtkAction* action_in,
                  gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_invite_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->parameters.empty ());

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (data_p->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve the channel
  // --> currently active menuitem of the invite_channel_members_menu
  GtkMenu* menu_p =
    GTK_MENU (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE)));
  ACE_ASSERT (menu_p);
  GtkMenuItem* menu_item_p =
    GTK_MENU_ITEM (gtk_menu_get_active (menu_p));
  ACE_ASSERT (menu_item_p);
  std::string channel_string = gtk_menu_item_get_label (menu_item_p);
    //Common_UI_GTK_Tools::UTF82Locale (gtk_menu_item_get_label (menu_item_p),
    //                              -1);
  ACE_ASSERT (!channel_string.empty ());

  for (string_list_const_iterator_t iterator_2 = data_p->parameters.begin ();
       iterator_2 != data_p->parameters.end ();
       iterator_2++)
  {
    try {
      data_p->controller->invite (*iterator_2,
                                  channel_string);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in IRC_Client_IIRCControl::invite(), continuing\n")));
    }
  } // end FOR
}

void
action_info_cb (GtkAction* action_in,
                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_info_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->parameters.empty ());

  try {
    data_p->controller->userhost (data_p->parameters);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::userhost(), continuing\n")));
  }
}

void
action_kick_cb (GtkAction* action_in,
                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_kick_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->id.empty ());
  ACE_ASSERT (!data_p->parameters.empty ());

  for (string_list_const_iterator_t iterator = data_p->parameters.begin();
       iterator != data_p->parameters.end ();
       iterator++)
  {
    try {
      data_p->controller->kick (data_p->id,
                                *iterator,
                                ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_KICK_REASON));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in IRC_Client_IIRCControl::kick(), continuing\n")));
    }
  } // end FOR
}

void
action_ban_cb (GtkAction* action_in,
               gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_ban_cb"));

  // sanity check(s)
  struct IRC_Client_UI_HandlerCBData* data_p =
    static_cast<IRC_Client_UI_HandlerCBData*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->id.empty ());
  ACE_ASSERT (!data_p->parameters.empty ());

  string_list_t parameters;
  std::string ban_mask_string;
  for (string_list_const_iterator_t iterator = data_p->parameters.begin();
       iterator != data_p->parameters.end ();
       iterator++)
  {
    parameters.clear ();
    // *TODO*: this probably needs some refinement --> users can just change
    // nicks and rejoin... should cover at least the user/hostnames as well ?
    ban_mask_string = *iterator;
    ban_mask_string += ACE_TEXT_ALWAYS_CHAR ("!*@*");
    parameters.push_back (ban_mask_string);

    try {
      data_p->controller->mode (data_p->id,
                                IRC_Tools::ChannelModeToChar (CHANNELMODE_BAN),
                                true,
                                parameters);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in IRC_IControl_T::mode(), continuing\n")));
    }
  } // end FOR
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
