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

#include "ace/OS.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_tools.h"

#include "net_macros.h"

#include "IRC_client_configuration.h"
#include "IRC_client_defines.h"
#include "IRC_client_gui_common.h"
#include "IRC_client_gui_connection.h"
#include "IRC_client_gui_defines.h"
#include "IRC_client_gui_messagehandler.h"
#include "IRC_client_iIRCControl.h"
#include "IRC_client_network.h"
#include "IRC_client_tools.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
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

gboolean
update_display (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::update_display"));

  IRC_Client_GTK_CBData* data_p =
    static_cast<IRC_Client_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->GTKState.lock);
  if (data_p->connections.empty ())
    return TRUE; // G_SOURCE_CONTINUE

  // step0: retrieve active connection
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState.builders.end ());

  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_num = gtk_notebook_get_current_page (notebook_p);
  ACE_ASSERT (tab_num >= 0);
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_num);
  ACE_ASSERT (widget_p);
  // *TODO*: the structure of the tab label is an implementation detail
  // --> should be encapsulated by the connection somehow...
  GtkHBox* hbox_p =
    GTK_HBOX (gtk_notebook_get_tab_label (notebook_p,
                                          widget_p));
  ACE_ASSERT (hbox_p);
  GList* list_p =
    gtk_container_get_children (GTK_CONTAINER (hbox_p));
  ACE_ASSERT (list_p);
  GtkLabel* label_p =
    GTK_LABEL (g_list_first (list_p)->data);
  ACE_ASSERT (label_p);
  std::string connection = gtk_label_get_text (label_p);

  connections_iterator_t connections_iterator =
    data_p->connections.find (connection);
  ACE_ASSERT (connections_iterator != data_p->connections.end ());

  //// step1: retrieve active channel
  //Common_UI_GTKBuildersIterator_t iterator_2 =
  //  data_p->GTKState.builders.find (connection);
  //// sanity check(s)
  //if (iterator_2 == data_p->GTKState.builders.end ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
  //              ACE_TEXT (connection.c_str ())));
  //  return FALSE; // G_SOURCE_REMOVE
  //} // end IF

  //notebook_p =
  //  GTK_NOTEBOOK (gtk_builder_get_object ((*iterator_2).second.second,
  //                                        ACE_TEXT_ALWAYS_CHAR ("server_tab_channel_tabs")));
  //ACE_ASSERT (notebook_p);
  //tab_num =
  //  gtk_notebook_get_current_page (notebook_p);
  //ACE_ASSERT (tab_num >= 0);
  //widget_p =
  //  gtk_notebook_get_nth_page (notebook_p,
  //                             tab_num);
  //ACE_ASSERT (widget_p);
  //// *TODO*: the structure of the tab label is an implementation detail
  //// --> should be encapsulated by the connection somehow...
  //hbox_p =
  //  GTK_HBOX (gtk_notebook_get_tab_label (notebook_p,
  //                                        widget_p));
  //ACE_ASSERT (hbox_p);
  //list_p =
  //  gtk_container_get_children (GTK_CONTAINER (hbox_p));
  //ACE_ASSERT (list_p);
  //label_p =
  //  GTK_LABEL (g_list_first (list_p)->data);
  //ACE_ASSERT (label_p);
  //std::string channel = gtk_label_get_text (label_p);

  ACE_ASSERT ((*connections_iterator).second);
  IRC_Client_GUI_MessageHandler* message_handler_p =
    (*connections_iterator).second->getActiveHandler (false);
  if (!message_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_GUI_Connection::getActiveHandler() (connection was: \"%s\"), returning\n"),
                ACE_TEXT (connection.c_str ())));
    return FALSE; // G_SOURCE_REMOVE
  } // end IF

  try
  {
    message_handler_p->update ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_GUI_MessageHandler::update() (connection was: \"%s\"), returning\n"),
                ACE_TEXT (connection.c_str ())));
    return FALSE; // G_SOURCE_REMOVE
  }

  // --> reschedule
  return TRUE; // G_SOURCE_CONTINUE
}

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  IRC_Client_GTK_CBData* data_p =
    static_cast<IRC_Client_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState.builders.end ());

  // step2: populate phonebook liststore
  GtkTreeStore* treestore_p =
    GTK_TREE_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TREESTORE_SERVERS)));
  ACE_ASSERT (treestore_p);
  GtkComboBox* combobox_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_SERVERS)));
  ACE_ASSERT (combobox_p);
  // *NOTE*: the combobox displays (selectable) column headers
  //         --> don't want that
  GList* list_p =
    gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (combobox_p));
  GtkCellRenderer* renderer_p =
    GTK_CELL_RENDERER (g_list_first (list_p)->data);
  ACE_ASSERT (renderer_p);
  g_list_free (list_p);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combobox_p), renderer_p,
                                      is_entry_sensitive,
                                      NULL, NULL);
  std::map<std::string, GtkTreeIter> network_map;
  std::map<std::string, GtkTreeIter>::iterator network_map_iterator;
  GtkTreeIter tree_iterator, current_row;
  for (IRC_Client_ServersIterator_t iterator = data_p->phoneBook.servers.begin ();
       iterator != data_p->phoneBook.servers.end ();
       iterator++)
  {
    // known network ?
    network_map_iterator = network_map.find ((*iterator).second.network);
    if (network_map_iterator == network_map.end ())
    {
      // new toplevel row
      gtk_tree_store_append (treestore_p,
                             &tree_iterator,
                             NULL);
      std::string network_label =
        ((*iterator).second.network.empty () ? ACE_TEXT_ALWAYS_CHAR ("<none>")
                                             : (*iterator).second.network);
      gtk_tree_store_set (treestore_p, &tree_iterator,
                          0, network_label.c_str (),
                          -1);

      network_map.insert (std::make_pair ((*iterator).second.network,
                                          tree_iterator));

      network_map_iterator = network_map.find ((*iterator).second.network);
      ACE_ASSERT (network_map_iterator != network_map.end ());
    } // end IF

    // append new (text) entry
    gtk_tree_store_append (treestore_p,
                           &current_row,
                           &(*network_map_iterator).second);
    gtk_tree_store_set (treestore_p, &current_row,
                        0, (*iterator).first.c_str (), // column 0
                        -1);

    // set active item
    if ((*iterator).first == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_SERVER_HOSTNAME))
      gtk_combo_box_set_active_iter (combobox_p,
                                     &current_row);
  } // end FOR
  if (!data_p->phoneBook.servers.empty ())
  {
    // sort entries (toplevel: ascending)
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (treestore_p),
                                          0, GTK_SORT_ASCENDING);

    gtk_widget_set_sensitive (GTK_WIDGET (combobox_p), TRUE);
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

  // step4: retrieve toplevel handle
  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
  ACE_ASSERT (window_p);
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"main_dialog\"): \"%m\", returning\n")));
    return FALSE; // G_SOURCE_REMOVE
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

  //// use correct screen
  //if (parentWidget_in)
  //  gtk_window_set_screen (window,
  //                         gtk_widget_get_screen (const_cast<GtkWidget*> (parentWidget_in)));

  // step5: draw it
  gtk_widget_show_all (GTK_WIDGET (window_p));

  guint event_source_id = 0;
  //guint refresh_rate =
  //  static_cast<guint> ((1.0F / static_cast<float> (IRC_CLIENT_GUI_REFRESH_RATE)) * 1000.0F);
  //event_source_id = g_timeout_add (refresh_rate, // ms
  //                                 update_display,
  //                                 userData_in);
  //if (!event_source_id)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
  //  return FALSE; // G_SOURCE_REMOVE
  //} // end IF
  event_source_id = g_idle_add (update_display,
                                userData_in);
  if (!event_source_id)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(): \"%m\", aborting\n")));
    return FALSE; // G_SOURCE_REMOVE
  } // end IF
  data_p->GTKState.eventSourceIds.push_back (event_source_id);

  return FALSE; // G_SOURCE_REMOVE
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  ACE_UNUSED_ARG (userData_in);

  // leave GTK
  gtk_main_quit ();

  return FALSE; // G_SOURCE_REMOVE
}

void
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  IRC_Client_GTK_CBData* data_p =
    static_cast<IRC_Client_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState.builders.end ());

  // retrieve about dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_ABOUT)));
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
  IRC_Client_GTK_CBData* data_p =
    static_cast<IRC_Client_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState.builders.end ());

  // step1: retrieve active phonebook entry
  // retrieve serverlist handle
  GtkComboBox* combobox_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_SERVERS)));
  ACE_ASSERT (combobox_p);
  GtkTreeIter active_iter;
  //   GValue active_value;
  gchar* active_value = NULL;
  std::string entry_name;
  if (!gtk_combo_box_get_active_iter (combobox_p,
                                      &active_iter))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(%@), returning\n"),
                combobox_p));
    return;
  } // end IF
  //   gtk_tree_model_get_value(gtk_combo_box_get_model(serverlist),
  //                            &active_iter,
  //                            0, &active_value);
  gtk_tree_model_get (gtk_combo_box_get_model (combobox_p),
                      &active_iter,
                      0, &active_value,
                      -1);
  //   ACE_ASSERT(G_VALUE_HOLDS_STRING(&active_value));
  ACE_ASSERT (active_value);
  // *TODO*: convert UTF8 to locale ?
  entry_name = active_value;
  //   entry_name = g_value_get_string(&active_value);

  // clean up
  //   g_value_unset(&active_value);
  g_free (active_value);

  IRC_Client_ServersIterator_t phonebook_iterator =
    data_p->phoneBook.servers.find (entry_name);
  if (phonebook_iterator == data_p->phoneBook.servers.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to lookup active phonebook entry (was: \"%s\"), returning\n"),
                ACE_TEXT (entry_name.c_str ())));
    return;
  } // end IF

  IRC_Client_IRCLoginOptions login_options =
   data_p->configuration->protocolConfiguration.loginOptions;
  // step2: get nickname...
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_MAIN)));
  ACE_ASSERT (entry_p);
  gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                0, -1);

  // enforce sane values
  // *TODO*: support the NICKLEN=xxx "feature" of the server...
  gtk_entry_set_max_length (entry_p,
                            IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH);
//   gtk_entry_set_width_chars(main_entry_entry,
//                             -1); // reset to default
  gtk_entry_set_text (entry_p,
                      login_options.nick.c_str ());
  gtk_editable_select_region (GTK_EDITABLE (entry_p),
                              0, -1);
  // retrieve entry dialog handle
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_ENTRY_MAIN)));
  ACE_ASSERT (dialog_p);
  gtk_window_set_title (GTK_WINDOW (dialog_p),
                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_NICK));
  if (gtk_dialog_run (dialog_p))
  {
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("connection cancelled...\n")));

    // clean up
    gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                  0, -1);

    gtk_widget_hide (GTK_WIDGET (dialog_p));

    return;
  } // end IF
  gtk_widget_hide (GTK_WIDGET (dialog_p));
  login_options.nick =
    Common_UI_Tools::UTF82Locale (gtk_entry_get_text (entry_p),
                                  -1);
  // clean up
  gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                0, -1);
  if (login_options.nick.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to set nickname, returning\n")));
    return;
  } // end IF
  // sanity check: <= IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH characters ?
  // *TODO*: support the NICKLEN=xxx "feature" of the server...
  if (login_options.nick.size () > IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH)
    login_options.nick.resize (IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH);

//  // step3: create/initialize new final module
//  Stream_Module_t* module_p = NULL;
//  if (data_p->configuration->streamConfiguration.streamConfiguration.module)
//  {
//    Stream_IModule_t* imodule_p =
//      dynamic_cast<Stream_IModule_t*> (data_p->configuration->streamConfiguration.streamConfiguration.module);
//    if (!imodule_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: dynamic_cast<Stream_IModule> failed, returning\n"),
//                  ACE_TEXT (data_p->configuration->streamConfiguration.streamConfiguration.module->name ())));
//      return;
//    } // end IF
//    try
//    {
//      module_p = imodule_p->clone ();
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: caught exception in Stream_IModule::clone(), continuing\n"),
//                  ACE_TEXT (data_p->configuration->streamConfiguration.streamConfiguration.module->name ())));
//    } // end IF
//    if (!module_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to Stream_IModule::clone(), returning\n"),
//                  ACE_TEXT (data_p->configuration->streamConfiguration.streamConfiguration.module->name ())));
//      return;
//    } // end IF
//  } // end IF
//  IRC_Client_IIRCControl* IRCControl_p = NULL;
//  if (module_p)
//  {
//    IRCControl_p =
//      dynamic_cast<IRC_Client_IIRCControl*> (module_p->writer ());
//    if (!IRCControl_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("dynamic_cast<IRC_Client_IIRCControl> failed, returning\n")));

//      // clean up
//      delete module_p;

//      return;
//    } // end IF
//  } // end IF

  // step4: connect to the server
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  for (IRC_Client_PortRangesIterator_t port_range_iter = (*phonebook_iterator).second.listeningPorts.begin ();
       port_range_iter != (*phonebook_iterator).second.listeningPorts.end ();
       port_range_iter++)
  {
    // port range ?
    if ((*port_range_iter).first < (*port_range_iter).second)
    {
      for (unsigned short current_port = (*port_range_iter).first;
           current_port <= (*port_range_iter).second;
           current_port++)
      {
        handle =
          IRC_Client_Tools::connect (data_p->configuration->streamConfiguration.streamConfiguration.messageAllocator,           // message allocator
                                     login_options,                                                                             // login options
                                     data_p->configuration->streamConfiguration.debugScanner,                                   // debug scanner ?
                                     data_p->configuration->streamConfiguration.debugParser,                                    // debug parser ?
                                     data_p->configuration->streamConfiguration.streamConfiguration.statisticReportingInterval, // statistics reporting interval [seconds: 0 --> OFF]
                                     (*phonebook_iterator).second.hostName,                                                     // server hostname
                                     current_port,                                                                              // server listening port
                                     data_p->configuration->streamConfiguration.streamConfiguration.deleteModule,               // delete final module ?
                                     data_p->configuration->streamConfiguration.streamConfiguration.module,                     // final module handle
                                     &data_p->configuration->streamConfiguration.streamModuleConfiguration);                    // module configuration

        if (handle != ACE_INVALID_HANDLE)
          break;
      } // end FOR
    } // end IF
    else
      handle =
        IRC_Client_Tools::connect (data_p->configuration->streamConfiguration.streamConfiguration.messageAllocator,           // message allocator
                                   login_options,                                                                             // login options
                                   data_p->configuration->streamConfiguration.debugScanner,                                   // debug scanner ?
                                   data_p->configuration->streamConfiguration.debugParser,                                    // debug parser ?
                                   data_p->configuration->streamConfiguration.streamConfiguration.statisticReportingInterval, // statistics reporting interval [seconds: 0 --> OFF]
                                   (*phonebook_iterator).second.hostName,                                                     // server hostname
                                   (*port_range_iter).first,                                                                  // server listening port
                                   data_p->configuration->streamConfiguration.streamConfiguration.deleteModule,               // delete final module ?
                                   data_p->configuration->streamConfiguration.streamConfiguration.module,                     // final module handle
                                   &data_p->configuration->streamConfiguration.streamModuleConfiguration);                    // module configuration
    if (handle != ACE_INVALID_HANDLE)
      break;

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to connect to server(\"%s\"), retrying\n"),
                ACE_TEXT ((*phonebook_iterator).second.hostName.c_str ())));
  } // end FOR
  if (handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to server(\"%s\"), returning\n"),
                ACE_TEXT ((*phonebook_iterator).second.hostName.c_str ())));
    return;
  } // end IF

  IRC_Client_Connection_Manager_t::CONNECTION_T* connection_p =
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->get (handle);
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Connection_Manager_t::get(%u), aborting\n"),
                handle));
  } // end IF
  const IRC_Client_Stream& stream = connection_p->stream ();
  Stream_Iterator_t iterator_2 (stream);
  const Stream_Module_t* module_p = NULL;
  for (const Stream_Module_t* current_p = NULL;
       iterator_2.next (current_p) != 0;
       iterator_2.advance ())
    if (current_p != const_cast<IRC_Client_Stream&> (stream).tail ())
      module_p = current_p;
  ACE_ASSERT (module_p);
  IRC_Client_IIRCControl* IRCControl_p =
      dynamic_cast<IRC_Client_IIRCControl*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!IRCControl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_IIRCControl> failed, returning\n")));

    // clean up
    connection_p->close ();
    connection_p->decrease ();

    return;
  } // end IF

  // step4: create/initialize new connection handler
  // retrieve server tabs handle
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  IRC_Client_GUI_Connection* connection_2 = NULL;
  ACE_NEW_NORETURN (connection_2,
                    IRC_Client_GUI_Connection (&data_p->GTKState,
                                               IRCControl_p,
                                               &data_p->connections,
                                               entry_name,
                                               data_p->UIFileDirectory,
                                               notebook_p));
  if (!connection_2)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    connection_p->close ();
    connection_p->decrease ();

    return;
  } // end IF
  if (!connection_2->isInitialized_)
  {
    // most probable reason: connection to this server already exists
    // *TODO*: prevent this upstream
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connection, returning\n")));

    // clean up
    delete connection_2;
    connection_p->close ();
    connection_p->decrease ();

    return;
  } // end IF

  //   ACE_DEBUG((LM_DEBUG,
  //              ACE_TEXT("registering...\n")));

  // step6: register connection with the server
  bool result = false;
  try
  {
    // *NOTE*: this entails a little delay (waiting for the welcome notice...)
    result = IRCControl_p->registerConnection (login_options);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::registerConnection(), continuing\n")));
  }
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_IIRCControl::registerConnection(), returning\n")));

    // clean up
    delete connection_2;
    connection_p->close ();
    connection_p->decrease ();

    return;
  } // end IF
  //   ACE_DEBUG((LM_DEBUG,
  //              ACE_TEXT("registering...DONE\n")));

  connection_p->decrease ();

  // step7: remember this connection
  // synch access
  {
    ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->GTKState.lock);

    // *TODO*: who deletes the module ? (the stream won't do it !)
    data_p->connections.insert (std::make_pair (entry_name, connection_2));
  } // end lock scope
}

void
entry_send_changed_cb (GtkWidget* widget_in,
                       gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::entry_send_changed_cb"));

  IRC_Client_GTK_CBData* data_p =
    static_cast<IRC_Client_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState.builders.end ());

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
  IRC_Client_GTK_CBData* data_p =
    static_cast<IRC_Client_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState.builders.end ());

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
  IRC_Client_GTK_CBData* data_p =
    static_cast<IRC_Client_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState.builders.end ());

  // step0: retrieve active connection
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_num = gtk_notebook_get_current_page (notebook_p);
  ACE_ASSERT (tab_num >= 0);
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_num);
  ACE_ASSERT (widget_p);
  // *TODO*: the structure of the tab label is an implementation detail
  // --> should be encapsulated by the connection somehow...
  GtkHBox* hbox_p =
    GTK_HBOX (gtk_notebook_get_tab_label (notebook_p,
                                          widget_p));
  ACE_ASSERT (hbox_p);
  GList* list_p =
    gtk_container_get_children (GTK_CONTAINER (hbox_p));
  ACE_ASSERT (list_p);
  GtkLabel* label_p =
    GTK_LABEL (g_list_first (list_p)->data);
  ACE_ASSERT (label_p);
  std::string connection = gtk_label_get_text (label_p);

  ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->GTKState.lock);

  connections_iterator_t connections_iterator =
    data_p->connections.find (connection);
  ACE_ASSERT (connections_iterator != data_p->connections.end ());

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
  std::string message_string =
    Common_UI_Tools::UTF82Locale (gtk_entry_buffer_get_text (buffer_p), // text
                                  text_length);                         // number of bytes
  if (message_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_Tools::UTF82Locale(): \"%m\", returning\n")));

    // clean up
    gtk_entry_buffer_delete_text (buffer_p, // buffer
                                  0,        // start at position 0
                                  -1);      // delete everything

    return;
  } // end IF

  // step2: retrieve active handler(s) (channel/nick)
  // *TODO*: allow multicast to several channels ?
  std::string active_id = (*connections_iterator).second->getActiveID ();
  ACE_ASSERT (!active_id.empty ());

  // step3: pass data to controller
  string_list_t receivers;
  receivers.push_back (active_id);
  IRC_Client_IIRCControl* controller_p =
    (*connections_iterator).second->getController ();
  ACE_ASSERT (controller_p);
  try
  {
    controller_p->send (receivers,
                        message_string);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::send(), continuing\n")));
  }

  // step4: echo data locally...
  message_string.insert (0, ACE_TEXT ("<me> "));
  message_string += ACE_TEXT_ALWAYS_CHAR ("\n");
  IRC_Client_GUI_MessageHandler* message_handler_p =
    (*connections_iterator).second->getActiveHandler (false);
  ACE_ASSERT (message_handler_p);
  try
  {
    message_handler_p->queueForDisplay (message_string);
  }
  catch (...)
  {
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
  //{
  //  ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->GTKState.lock);

  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->GTKState.eventSourceIds.begin ();
  //       iterator != data_p->GTKState.eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->GTKState.eventSourceIds.clear ();
  //} // end lock scope

  // step2: initiate shutdown sequence
  result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  // step3: stop GTK event processing
  COMMON_UI_GTK_MANAGER_SINGLETON::instance()->close (1);
}

void
button_disconnect_clicked_cb (GtkWidget* widget_in,
                              gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::button_disconnect_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  try
  {
    data_p->controller->quit (ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_IRC_LEAVE_REASON));
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::quit(), continuing\n")));
  }

  // *NOTE*: the server should close the connection after this...
}

gboolean
nick_entry_kb_focused_cb (GtkWidget* widget_in,
                          GdkEventFocus* event_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::nick_entry_kb_focused_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  // step0: retrieve active connection
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_num = gtk_notebook_get_current_page (notebook_p);
  ACE_ASSERT (tab_num >= 0);
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_num);
  ACE_ASSERT (widget_p);
  // *TODO*: the structure of the tab label is an implementation detail
  // --> should be encapsulated by the connection somehow...
  GtkHBox* hbox_p =
    GTK_HBOX (gtk_notebook_get_tab_label (notebook_p,
    widget_p));
  ACE_ASSERT (hbox_p);
  GList* list_p =
    gtk_container_get_children (GTK_CONTAINER (hbox_p));
  ACE_ASSERT (list_p);
  GtkLabel* label_p =
    GTK_LABEL (g_list_first (list_p)->data);
  ACE_ASSERT (label_p);
  std::string connection = gtk_label_get_text (label_p);

  Common_UI_GTKBuildersIterator_t iterator_2 =
    data_p->GTKState->builders.find (connection);
  // sanity check(s)
  if (iterator_2 == data_p->GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (connection.c_str ())));
    return TRUE; // propagate
  } // end IF

  // make the "change" button the default widget...
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator_2).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_NICK_ACCEPT)));
  ACE_ASSERT (button_p);
  gtk_widget_grab_default (GTK_WIDGET (button_p));

  return FALSE;
}

void
change_clicked_cb (GtkWidget* widget_in,
                   gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::change_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  // step0: retrieve active connection
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_num = gtk_notebook_get_current_page (notebook_p);
  ACE_ASSERT (tab_num >= 0);
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_num);
  ACE_ASSERT (widget_p);
  // *TODO*: the structure of the tab label is an implementation detail
  // --> should be encapsulated by the connection somehow...
  GtkHBox* hbox_p =
    GTK_HBOX (gtk_notebook_get_tab_label (notebook_p,
    widget_p));
  ACE_ASSERT (hbox_p);
  GList* list_p =
    gtk_container_get_children (GTK_CONTAINER (hbox_p));
  ACE_ASSERT (list_p);
  GtkLabel* label_p =
    GTK_LABEL (g_list_first (list_p)->data);
  ACE_ASSERT (label_p);
  std::string connection = gtk_label_get_text (label_p);

  Common_UI_GTKBuildersIterator_t iterator_2 =
    data_p->GTKState->builders.find (connection);
  // sanity check(s)
  if (iterator_2 == data_p->GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (connection.c_str ())));
    return;
  } // end IF

  // step1: retrieve available data
  // retrieve buffer handle
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator_2).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_NICK)));
  ACE_ASSERT (entry_p);
  GtkEntryBuffer* entrybuffer_p = gtk_entry_get_buffer (entry_p);
  ACE_ASSERT (entrybuffer_p);

  // sanity check
  guint16 text_length = gtk_entry_buffer_get_length (entrybuffer_p);
  if (text_length == 0)
    return; // nothing to do...

  // retrieve textbuffer data
  std::string nick_string =
    Common_UI_Tools::UTF82Locale (gtk_entry_buffer_get_text (entrybuffer_p), // text
                                  text_length);                              // number of bytes
  if (nick_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to convert nickname (was: \"%s\"), returning\n"),
                gtk_entry_buffer_get_text (entrybuffer_p)));

    // clean up
    gtk_entry_buffer_delete_text (entrybuffer_p, // buffer
                                  0,             // start at position 0
                                  -1);           // delete everything

    return;
  } // end IF

  // sanity check: <= IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH characters ?
  // *TODO*: support the NICKLEN=xxx "feature" of the server...
  if (nick_string.size () > IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH)
    nick_string.resize (IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH);

  try
  {
    data_p->controller->nick (nick_string);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::nick(), continuing\n")));
  }

  // clear buffer
  gtk_entry_buffer_delete_text (entrybuffer_p, // buffer
                                0,             // start at position 0
                                -1);           // delete everything
}

void
usersbox_changed_cb (GtkWidget* widget_in,
                     gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::usersbox_changed_cb"));

  ACE_UNUSED_ARG (widget_in);
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);

  // step1: retrieve active users entry
  // retrieve server tab users combobox handle
  GtkComboBox* server_tab_users_combobox = GTK_COMBO_BOX (widget_in);
  ACE_ASSERT (server_tab_users_combobox);
  GtkTreeIter active_iter;
  //   GValue active_value;
  gchar* user_value = NULL;
  if (!gtk_combo_box_get_active_iter (server_tab_users_combobox,
                                      &active_iter))
  {
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("failed to gtk_combo_box_get_active_iter(%@), returning\n"),
//                server_tab_users_combobox));

    return;
  } // end IF
//   gtk_tree_model_get_value(gtk_combo_box_get_model(serverlist),
//                            &active_iter,
//                            0, &active_value);
  gtk_tree_model_get (gtk_combo_box_get_model (server_tab_users_combobox),
                      &active_iter,
                      0, &user_value, // just retrieve the first column...
                      -1);
//   ACE_ASSERT(G_VALUE_HOLDS_STRING(&active_value));
  ACE_ASSERT (user_value);

  // convert UTF8 to locale
//   user_string = g_value_get_string(&active_value);
  std::string user_string =
    Common_UI_Tools::UTF82Locale (user_value,
                                      g_utf8_strlen (user_value, -1));
  if (user_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to convert user name, returning\n")));

    // clean up
    g_free (user_value);

    return;
  } // end IF

  // clean up
  g_free (user_value);

  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH characters ?
  // *TODO*: support the NICKLEN=xxx "feature" of the server...
  if (user_string.size () > IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH)
    user_string.resize (IRC_CLIENT_CNF_IRC_MAX_NICK_LENGTH);

  // *TODO*: if a conversation exists, simply activate the corresponding page
  data_p->connection->createMessageHandler (user_string);
}

void
refresh_users_clicked_cb (GtkWidget* widget_in,
                          gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::refresh_users_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
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
  try
  {
    data_p->controller->who (name, false);
  }
  catch (...)
  {
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
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  // step0: retrieve active connection
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_num = gtk_notebook_get_current_page (notebook_p);
  ACE_ASSERT (tab_num >= 0);
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_num);
  ACE_ASSERT (widget_p);
  // *TODO*: the structure of the tab label is an implementation detail
  // --> should be encapsulated by the connection somehow...
  GtkHBox* hbox_p =
    GTK_HBOX (gtk_notebook_get_tab_label (notebook_p,
                                          widget_p));
  ACE_ASSERT (hbox_p);
  GList* list_p =
    gtk_container_get_children (GTK_CONTAINER (hbox_p));
  ACE_ASSERT (list_p);
  GtkLabel* label_p =
    GTK_LABEL (g_list_first (list_p)->data);
  ACE_ASSERT (label_p);
  std::string connection = gtk_label_get_text (label_p);

  Common_UI_GTKBuildersIterator_t iterator_2 =
    data_p->GTKState->builders.find (connection);
  // sanity check(s)
  if (iterator_2 == data_p->GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (connection.c_str ())));
    return TRUE; // propagate
  } // end IF

  // make the "change" button the default widget...
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator_2).second.second,
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
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (data_p->GTKState);

  // step0: retrieve active connection
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CONNECTIONS)));
  ACE_ASSERT (notebook_p);
  gint tab_num = gtk_notebook_get_current_page (notebook_p);
  ACE_ASSERT (tab_num >= 0);
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   tab_num);
  ACE_ASSERT (widget_p);
  // *TODO*: the structure of the tab label is an implementation detail
  // --> should be encapsulated by the connection somehow...
  GtkHBox* hbox_p =
    GTK_HBOX (gtk_notebook_get_tab_label (notebook_p,
    widget_p));
  ACE_ASSERT (hbox_p);
  GList* list_p =
    gtk_container_get_children (GTK_CONTAINER (hbox_p));
  ACE_ASSERT (list_p);
  GtkLabel* label_p =
    GTK_LABEL (g_list_first (list_p)->data);
  ACE_ASSERT (label_p);
  std::string connection = gtk_label_get_text (label_p);

  Common_UI_GTKBuildersIterator_t iterator_2 =
    data_p->GTKState->builders.find (connection);
  // sanity check(s)
  if (iterator_2 == data_p->GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (connection.c_str ())));
    return;
  } // end IF

  // step1: retrieve available data
  // retrieve buffer handle
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object ((*iterator_2).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_CONNECTION_CHANNEL)));
  ACE_ASSERT (entry_p);
  GtkEntryBuffer* entrybuffer_p = gtk_entry_get_buffer (entry_p);
  ACE_ASSERT (entrybuffer_p);

  // sanity check
  guint16 text_length = gtk_entry_buffer_get_length (entrybuffer_p);
  if (text_length == 0)
    return; // nothing to do...

  // retrieve textbuffer data
  std::string channel_string =
    Common_UI_Tools::UTF82Locale (gtk_entry_buffer_get_text (entrybuffer_p), // text
                                  text_length);                              // number of bytes
  if (channel_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to convert channel name (was: \"%s\"), returning\n"),
                gtk_entry_buffer_get_text (entrybuffer_p)));

    // clean up
    gtk_entry_buffer_delete_text (entrybuffer_p, // buffer
                                  0,             // start at position 0
                                  -1);           // delete everything

    return;
  } // end IF

  // sanity check(s): has '#' prefix ?
  if (channel_string.find ('#', 0) != 0)
    channel_string.insert (channel_string.begin (), '#');
  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
  // *TODO*: support the CHANNELLEN=xxx "feature" of the server...
  if (channel_string.size () > IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH)
    channel_string.resize (IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH);

  // *TODO*: support channel keys/multi-join ?
  string_list_t channels, keys;
  channels.push_back (channel_string);
  try
  {
    data_p->controller->join (channels, keys);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::join(), continuing\n")));
  }

  // clear buffer
  gtk_entry_buffer_delete_text (entrybuffer_p, // buffer
                                0,             // start at position 0
                                -1);           // delete everything
}

void
channelbox_changed_cb (GtkWidget* widget_in,
                       gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::channelbox_changed_cb"));

  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
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
    Common_UI_Tools::UTF82Locale (channel_value,
                                  g_utf8_strlen (channel_value, -1));
  if (channel_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to convert channel name (was: \"%s\", returning\n"),
                channel_value));

    // clean up
    g_free (channel_value);

    return;
  } // end IF

  // clean up
  g_free(channel_value);

  // sanity check(s): has '#' prefix ?
  if (channel_string.find ('#', 0) != 0)
    channel_string.insert (channel_string.begin (), '#');
  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
  // *TODO*: support the CHANNELLEN=xxx "feature" of the server...
  if (channel_string.size () > IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH)
    channel_string.resize (IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH);

  // *TODO*: support channel key ?
  string_list_t channels;
  channels.push_back (channel_string);
  string_list_t keys;
  try
  {
    data_p->controller->join (channels, keys);
  }
  catch (...)
  {
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
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  // *NOTE*: empty list --> list them all !
  string_list_t channel_list;
  try
  {
    data_p->controller->list (channel_list);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::list(), continuing\n")));
  }
}

void
user_mode_toggled_cb (GtkToggleButton* widget_in,
                      gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::user_mode_toggled_cb"));

  int result = -1;

  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->IRCSessionState.nickname.empty ());

  IRC_Client_UserMode mode = USERMODE_INVALID;
  // find out which button toggled...
  const gchar* name_p = gtk_buildable_get_name (GTK_BUILDABLE (widget_in));
  //const gchar* name_p = gtk_widget_get_name (GTK_WIDGET (widget_in));
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
                                 ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY));
        if (result == 0)
          mode = USERMODE_AWAY;
        else
        {
          result = ACE_OS::strcmp (name_p,
                                   ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS));
          if (result == 0)
            mode = USERMODE_RECVWALLOPS;
          else
          {
            result = ACE_OS::strcmp (name_p,
                                     ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES));
            if (result == 0)
              mode = USERMODE_RECVNOTICES;
            else
            {
              result = ACE_OS::strcmp (name_p,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE));
              if (result == 0)
                mode = USERMODE_INVISIBLE;
              else
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("unknown/invalid user mode toggled (was: 0x%@/\"%s\"), returning\n"),
                            widget_in, ACE_TEXT (name_p)));
                return;
              } // end ELSE
            } // end ELSE
          } // end ELSE
        } // end ELSE
      } // end ELSE
    } // end ELSE
  } // end ELSE

  // check if the state is inconsistent --> submit change request, else do nothing
  // i.e. state is off and widget is "on" (or vice-versa)
  // *NOTE*: avoid recursion
  if (data_p->IRCSessionState.userModes.test (mode) == widget_in->active)
    return;
  // re-toggle button for now...
  // *NOTE*: will be auto-toggled according to the outcome of the change request
  gtk_toggle_button_set_active (widget_in, data_p->IRCSessionState.userModes.test (mode));

  string_list_t parameters;
  try
  {
    data_p->controller->mode (data_p->IRCSessionState.nickname,               // user mode
                              IRC_Client_Tools::IRCUserMode2Char (mode),      // corresponding mode char
                              !data_p->IRCSessionState.userModes.test (mode), // enable ?
                              parameters);                                    // parameters
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::mode(\"%s\"), continuing\n"),
                ACE_TEXT (IRC_Client_Tools::IRCUserMode2String (mode).c_str ())));
  }
}

void
switch_channel_cb (GtkNotebook* notebook_in,
                   GtkNotebookPage* page_in,
                   guint pageNum_in,
                   gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::switch_channel_cb"));

  ACE_UNUSED_ARG (notebook_in);
  ACE_UNUSED_ARG (page_in);
  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

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

  IRC_Client_GTK_ConnectionCBData* data_p =
    static_cast<IRC_Client_GTK_ConnectionCBData*>(userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (data_p->GTKState);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkToggleButton* togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_TOOLS_1)));
  ACE_ASSERT (togglebutton_p);

  bool activating = gtk_toggle_button_get_active (togglebutton_p);

  // check if the state is inconsistent --> submit change request, else do nothing
  // i.e. state is off and widget is "on" or vice-versa
  // *NOTE*: avoid recursion
  if (data_p->IRCSessionState.away == activating)
    return;
  // re-toggle button for now...
  // *NOTE*: will be auto-toggled according to the outcome of the change request
  gtk_toggle_button_set_active (togglebutton_p, data_p->IRCSessionState.away);

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
                        IRC_CLIENT_DEF_IRC_AWAY_MESSAGE);
    gtk_editable_select_region (GTK_EDITABLE (entry_p),
                                0, -1);

    GtkDialog* dialog_p =
      GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_ENTRY_CONNECTION)));
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
      Common_UI_Tools::UTF82Locale (gtk_entry_get_text (entry_p),
                                        -1);
    // clean up
    gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                  0, -1);

    if (away_message.empty ())
    {
      // *NOTE*: need to set SOME value, as an AWAY-message with no parameter will
      // actually "un-away" the user (see RFC1459 Section 5.1)...
      away_message = IRC_CLIENT_DEF_IRC_AWAY_MESSAGE;
    } // end IF
  } // end IF

  try
  {
    data_p->controller->away (away_message);
  }
  catch (...)
  {
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

  int result = -1;

  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (data_p->GTKState);
  ACE_ASSERT (!data_p->id.empty ());

  IRC_Client_ChannelMode mode           = CHANNELMODE_INVALID;
  bool                   need_parameter = false;
  std::string            entry_label;
  // find out which button toggled...
  const gchar* name_p = gtk_widget_get_name (GTK_WIDGET (toggleButton_in));
  result = ACE_OS::strcmp (name_p,
                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_KEY));
  if (result == 0)
  {
    mode = CHANNELMODE_PASSWORD;
    need_parameter = true;
    entry_label = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_PASSWORD);
  } // end IF
  else
  {
    result = ACE_OS::strcmp (name_p,
                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_VOICE));
    if (result == 0)
    {
      mode = CHANNELMODE_VOICE;
      need_parameter = true;
      entry_label = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_VOICE);
    } // end IF
    else
    {
      result = ACE_OS::strcmp (name_p,
                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BAN));
      if (result == 0)
      {
        mode = CHANNELMODE_BAN;
        need_parameter = true;
        entry_label = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_BAN);
      } // end IF
      else
      {
        result = ACE_OS::strcmp (name_p,
                                 ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_USERLIMIT));
        if (result == 0)
        {
          mode = CHANNELMODE_USERLIMIT;
          need_parameter = true;
          entry_label = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DIALOG_ENTRY_TITLE_MODE_USERLIMIT);
        }
        else
        {
          result = ACE_OS::strcmp (name_p,
                                   ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_MODERATED));
          if (result == 0)
            mode = CHANNELMODE_MODERATED;
          else
          {
            result = ACE_OS::strcmp (name_p,
                                     ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BLOCKFOREIGN));
            if (result == 0)
              mode = CHANNELMODE_BLOCKFOREIGNMSGS;
            else
            {
              result = ACE_OS::strcmp (name_p,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_RESTRICTOPIC));
              if (result == 0)
                mode = CHANNELMODE_RESTRICTEDTOPIC;
              else
              {
                result = ACE_OS::strcmp (name_p,
                                         ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_INVITEONLY));
                if (result == 0)
                  mode = CHANNELMODE_INVITEONLY;
                else
                {
                  result = ACE_OS::strcmp (name_p,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_SECRET));
                  if (result == 0)
                    mode = CHANNELMODE_SECRET;
                  else
                  {
                    result = ACE_OS::strcmp (name_p,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_PRIVATE));
                    if (result == 0)
                      mode = CHANNELMODE_PRIVATE;
                    else
                    {
                      result = ACE_OS::strcmp (name_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_OPERATOR));
                      if (result == 0)
                        mode = CHANNELMODE_OPERATOR;
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

  // check if the state is inconsistent --> submit change request, else do nothing
  // i.e. state is off and widget is "on" or vice-versa
  // *NOTE*: avoid recursion
  if (data_p->channelModes.test (mode) == toggleButton_in->active)
    return;
  // re-toggle button for now...
  // *NOTE*: will be auto-toggled according to the outcome of the change request
  gtk_toggle_button_set_active (toggleButton_in,
                                data_p->channelModes.test (mode));

  // *NOTE*: see also: IRC_Client_MessageHandler ctor
  std::string page_tab_label_string;
  if (!IRC_Client_Tools::isValidIRCChannelName (data_p->id))
  {
    // --> private conversation window, modify label accordingly
    page_tab_label_string = ACE_TEXT_ALWAYS_CHAR ("[");
    page_tab_label_string += data_p->id;
    page_tab_label_string += ACE_TEXT_ALWAYS_CHAR ("]");
  } // end IF
  else
    page_tab_label_string = data_p->id;
  std::string builder_label = data_p->connection->getLabel ();
  builder_label += ACE_TEXT_ALWAYS_CHAR ("::");
  builder_label += page_tab_label_string;

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (builder_label);
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

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
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_ENTRY_CHANNEL)));
    ACE_ASSERT (dialog_p);
    gtk_window_set_title (GTK_WINDOW (dialog_p),
                          entry_label.c_str ());
    if (gtk_dialog_run (dialog_p))
    {
//       ACE_DEBUG((LM_DEBUG,
//                  ACE_TEXT("channel mode cancelled...\n")));

      // clean up
      gtk_entry_buffer_delete_text (gtk_entry_get_buffer (entry_p),
                                    0, -1);

      //gtk_widget_hide (GTK_WIDGET (dialog_p));

      return;
    } // end IF
    //gtk_widget_hide (GTK_WIDGET (dialog_p));

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
  try
  {
    data_p->controller->mode (data_p->id,                                   // channel name
                              IRC_Client_Tools::IRCChannelMode2Char (mode), // corresponding mode char
                              !data_p->channelModes.test (mode),            // enable ?
                              parameters);                                  // parameters
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::mode(\"%s\"), continuing\n"),
                ACE_TEXT (IRC_Client_Tools::IRCChannelMode2String (mode).c_str ())));
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
  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (data_p->GTKState);
  ACE_ASSERT (!data_p->id.empty ());

  // *NOTE*: see also: IRC_Client_MessageHandler ctor
  std::string page_tab_label_string;
  if (!IRC_Client_Tools::isValidIRCChannelName (data_p->id))
  {
    // --> private conversation window, modify label accordingly
    page_tab_label_string = ACE_TEXT_ALWAYS_CHAR ("[");
    page_tab_label_string += data_p->id;
    page_tab_label_string += ACE_TEXT_ALWAYS_CHAR ("]");
  } // end IF
  else
    page_tab_label_string = data_p->id;
  std::string builder_label = data_p->connection->getLabel ();
  builder_label += ACE_TEXT_ALWAYS_CHAR ("::");
  builder_label += page_tab_label_string;

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (builder_label);
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

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
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_DIALOG_ENTRY_CHANNEL)));
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
  try
  {
    data_p->controller->topic (data_p->id,
                               topic_string);
  }
  catch (...)
  {
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

  ACE_UNUSED_ARG (widget_in);
  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->id.empty ());

  // *NOTE*: if 'this' is not a channel handler, just close then page tab
  if (!IRC_Client_Tools::isValidIRCChannelName (data_p->id))
  {
    // *WARNING*: this will delete 'this' !
    data_p->connection->terminateMessageHandler (data_p->id);

    return; // done
  } // end IF

  string_list_t channels;
  channels.push_back (data_p->id);
  try
  {
    data_p->controller->part (channels);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::part(), continuing\n")));
  }
}

gboolean
members_clicked_cb (GtkWidget* widget_in,
                    GdkEventButton* event_in,
                    gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::members_clicked_cb"));

  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // supposed to be a context menu -> right-clicked ?
  if (event_in->button != 3)
    return FALSE; // --> propagate event

  // sanity check(s)
  ACE_ASSERT (GTK_TREE_VIEW (widget_in));
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (data_p->GTKState);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  // find out which row was actually clicked
  GtkTreePath* path = NULL;
  if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget_in),
                                      static_cast<gint> (event_in->x), static_cast<gint> (event_in->y),
                                      &path, NULL,
                                      NULL, NULL))
    return FALSE; // no row at this position --> propagate event
  ACE_ASSERT (path);

  // retrieve current selection
  GtkTreeSelection* selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (widget_in));
  ACE_ASSERT (selection);
  // nothing selected ? --> nothing to do
  if (gtk_tree_selection_count_selected_rows (selection) == 0)
  {
    // clean up
    gtk_tree_path_free (path);

    return TRUE; // done
  } // end IF

  // path part of the selection ? --> keep selection : new selection
  if (!gtk_tree_selection_path_is_selected (selection, path))
  {
    gtk_tree_selection_unselect_all (selection);
    gtk_tree_selection_select_path (selection, path);
  } // end IF

  // clean up
  gtk_tree_path_free (path);

  GList* selected_rows = NULL;
  GtkTreeModel* model = NULL;
  selected_rows = gtk_tree_selection_get_selected_rows (selection,
                                                        &model);
  ACE_ASSERT (selected_rows);
  ACE_ASSERT (model);
  data_p->parameters.clear ();
  GtkTreePath* current_path = NULL;
  GtkTreeIter current_iter;
//   GValue current_value;
  gchar* current_value = NULL;
  for (GList* iterator_2 = g_list_first (selected_rows);
       iterator_2 != NULL;
       iterator_2 = g_list_next (iterator_2))
  {
    current_path = static_cast<GtkTreePath*> (iterator_2->data);
    ACE_ASSERT (current_path);

    // path --> iter
    if (!gtk_tree_model_get_iter (model,
                                  &current_iter,
                                  current_path))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_tree_model_get_iter, continuing\n")));
      continue;
    } // end IF
    // iter --> value
//     gtk_tree_model_get_value(model,
//                              current_iter,
//                              0, &current_value);
    gtk_tree_model_get (model,
                        &current_iter,
                        0, &current_value,
                        -1);
    // *TODO*: check if these
    data_p->parameters.push_back (Common_UI_Tools::UTF82Locale (current_value,
                                                                g_utf8_strlen (current_value, -1)));

    // clean up
    g_free (current_value);
    gtk_tree_path_free (current_path);
  } // end FOR

  // clean up
  g_list_free (selected_rows);

  // remove ourselves from any selection...
  string_list_iterator_t iterator_2 = data_p->parameters.begin ();
  string_list_iterator_t self = data_p->parameters.end ();
  std::string nickname = data_p->connection->getNickname ();
  for (;
       iterator_2 != data_p->parameters.end ();
       iterator_2++)
  {
//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("selected: \"%s\"\n"),
//                (*iterator).c_str()));

    if (*iterator_2 == nickname)
    {
      self = iterator_2;
      continue;
    } // end IF
    // *NOTE*: ignore leading '@'
    if ((*iterator_2).find ('@', 0) == 0)
      if (((*iterator_2).find (nickname, 1) == 1) &&
          ((*iterator_2).size () == (nickname.size () + 1)))
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
  GtkMenuItem* menuitem_p = NULL;
  // --> retrieve list of active channels
  // *TODO*: for invite-only channels, only operators are allowed to invite
  // strangers --> remove those channels where this condition doesn't apply
  string_list_t active_channels;
  data_p->connection->channels (active_channels);
  if (active_channels.size () > 1)
  {
    // clear popup menu
    GtkMenu* menu_2_p =
      GTK_MENU (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE)));
    ACE_ASSERT (menu_2_p);
    GList* list_p =
      gtk_container_get_children (GTK_CONTAINER (menu_2_p));
    if (list_p)
    {
      for (GList* list_2_p = g_list_first (list_p);
           list_2_p != NULL;
           list_2_p = g_list_next (list_2_p))
        gtk_container_remove (GTK_CONTAINER (menu_2_p),
                              GTK_WIDGET (list_2_p->data));
      // clean up
      g_list_free (list_p);
    } // end IF

    // populate popup menu
    GtkAction* action_p =
      GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_INVITE)));
    ACE_ASSERT (action_p);
    menuitem_p = NULL;
    for (string_list_const_iterator_t iterator_2 = active_channels.begin ();
         iterator_2 != active_channels.end ();
         iterator_2++)
    {
      // skip current channel
      if (*iterator_2 == data_p->id)
        continue;

      menuitem_p = GTK_MENU_ITEM (gtk_action_create_menu_item (action_p));
      ACE_ASSERT (menuitem_p);
      gtk_menu_item_set_label (menuitem_p,
                               (*iterator_2).c_str ());
      gtk_menu_shell_append (GTK_MENU_SHELL (menu_2_p),
                             GTK_WIDGET (menuitem_p));
    } // end FOR
    gtk_widget_show_all (GTK_WIDGET (menu_2_p));

    menuitem_p =
      GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_INVITE)));
    ACE_ASSERT (menuitem_p);
    gtk_widget_set_sensitive (GTK_WIDGET (menuitem_p), TRUE);
  } // end IF
  else
  {
    menuitem_p =
      GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_INVITE)));
    ACE_ASSERT (menuitem_p);
    gtk_widget_set_sensitive (GTK_WIDGET (menuitem_p), FALSE);
  } // end ELSE

  // en-/disable some entries...
  menuitem_p =
    GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_KICK)));
  ACE_ASSERT (menuitem_p);
  gtk_widget_set_sensitive (GTK_WIDGET (menuitem_p),
                            data_p->channelModes.test (CHANNELMODE_OPERATOR));
  menuitem_p =
    GTK_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_BAN)));
  ACE_ASSERT (menuitem_p);
  gtk_widget_set_sensitive (GTK_WIDGET (menuitem_p),
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

  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->connection);
  ACE_ASSERT (!data_p->parameters.empty ());

  gint page_num = -1;
  for (string_list_const_iterator_t iterator = data_p->parameters.begin ();
       iterator != data_p->parameters.end ();
       iterator++)
  {
    page_num = data_p->connection->exists (*iterator);
    if (page_num == -1)
      data_p->connection->createMessageHandler (*iterator);
  } // end FOR
}

void
action_invite_cb (GtkAction* action_in,
                  gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_invite_cb"));

  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (data_p->GTKState);
  ACE_ASSERT (!data_p->parameters.empty ());

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  // retrieve the channel
  // --> currently active menuitem of the invite_channel_members_menu
  GtkMenu* menu_p =
    GTK_MENU (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE)));
  ACE_ASSERT (menu_p);
  GtkMenuItem* menuitem_p =
    GTK_MENU_ITEM (gtk_menu_get_active (menu_p));
  ACE_ASSERT (menuitem_p);
  std::string channel_string =
    Common_UI_Tools::UTF82Locale (gtk_menu_item_get_label (menuitem_p),
                                  -1);
  ACE_ASSERT (!channel_string.empty ());

  for (string_list_const_iterator_t iterator_2 = data_p->parameters.begin ();
       iterator_2 != data_p->parameters.end ();
       iterator_2++)
  {
    try
    {
      data_p->controller->invite (*iterator_2,
                                  channel_string);
    }
    catch (...)
    {
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

  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->parameters.empty ());

  try
  {
    data_p->controller->userhost (data_p->parameters);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::userhost(), continuing\n")));
  }
}

void
action_kick_cb (GtkAction* action_in,
                gpointer userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::action_kick_cb"));

  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (!data_p->id.empty ());
  ACE_ASSERT (!data_p->parameters.empty ());

  for (string_list_const_iterator_t iterator = data_p->parameters.begin();
       iterator != data_p->parameters.end ();
       iterator++)
  {
    try
    {
      data_p->controller->kick (data_p->id,
                                *iterator,
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_IRC_KICK_REASON));
    }
    catch (...)
    {
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

  IRC_Client_GTK_HandlerCBData* data_p =
    static_cast<IRC_Client_GTK_HandlerCBData*> (userData_in);

  // sanity check(s)
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

    try
    {
      data_p->controller->mode (data_p->id,
                                IRC_Client_Tools::IRCChannelMode2Char (CHANNELMODE_BAN),
                                true,
                                parameters);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in IRC_Client_IIRCControl::mode(), continuing\n")));
    }
  } // end FOR
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
