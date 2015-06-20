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

#include "IRC_client_gui_connection.h"

#include <sstream>

#include "common_file_tools.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_tools.h"

#include "net_macros.h"

#include "IRC_client_gui_callbacks.h"
#include "IRC_client_gui_defines.h"
#include "IRC_client_gui_messagehandler.h"
#include "IRC_client_iIRCControl.h"
#include "IRC_client_tools.h"

IRC_Client_GUI_Connection::IRC_Client_GUI_Connection (Common_UI_GTKState* state_in,
                                                      IRC_Client_IIRCControl* controller_in,
                                                      connections_t* connections_in,
                                                      const std::string& label_in,
                                                      const std::string& UIFileDirectory_in,
                                                      GtkNotebook* parent_in)
 : isInitialized_ (false)
 , CBData_ ()
 , isFirstUsersMsg_ (true)
 , label_ (label_in)
 , UIFileDirectory_ (UIFileDirectory_in)
 , lock_ ()
 , messageHandlers_ ()
 , contextID_ (0)
 , parent_ (parent_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::IRC_Client_GUI_Connection"));

  // sanity check(s)
  ACE_ASSERT (state_in);
  ACE_ASSERT (parent_in);
  ACE_ASSERT (controller_in);
  ACE_ASSERT (connections_in);
  if (!Common_File_Tools::isDirectory (UIFileDirectory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: \"%s\"): not a directory, returning\n"),
                ACE_TEXT (UIFileDirectory_in.c_str ())));
    return;
  } // end IF

  // initialize cb data
  CBData_.connection = this;
  CBData_.connections = connections_in;
  CBData_.controller = controller_in;
  CBData_.GTKState = state_in;
  //   CBData_.nick.clear(); // cannot set this now...
  CBData_.IRCSessionState.userModes.reset ();

  // create new GtkBuilder
  GtkBuilder* builder_p = gtk_builder_new ();
  if (!builder_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  Common_UI_GTKBuildersIterator_t iterator =
    CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator != CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection \"%s\" exists, returning\n"),
                ACE_TEXT (label_.c_str ())));

    // clean up
    g_object_unref (G_OBJECT (builder_p));

    return;
  } // end IF
  std::string ui_definition_filename = UIFileDirectory_;
  ui_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_filename += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_CONNECTION_FILE);
  if (!Common_File_Tools::isReadable (ui_definition_filename))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid UI file (was: \"%s\"): not readable, returning\n"),
                ACE_TEXT (ui_definition_filename.c_str ())));

    // clean up
    g_object_unref (G_OBJECT (builder_p));

    return;
  } // end IF

  // load widget tree
  GError* error = NULL;
  gtk_builder_add_from_file (builder_p,
                             ui_definition_filename.c_str (),
                             &error);
  if (error)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_add_from_file(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (ui_definition_filename.c_str ()),
                ACE_TEXT (error->message)));

    // clean up
    g_error_free (error);
    g_object_unref (G_OBJECT (builder_p));

    return;
  } // end IF

  // generate context ID
  iterator =
      CBData_.GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != CBData_.GTKState->builders.end ());

  // retrieve status bar
  GtkStatusbar* statusbar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_STATUSBAR)));
  ACE_ASSERT (statusbar_p);

  contextID_ =
    gtk_statusbar_get_context_id (statusbar_p,
                                  ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_STATUSBAR_CONTEXT_DESCRIPTION));

  // retrieve server tab channels store
  GtkListStore* liststore_p =
    GTK_LIST_STORE (gtk_builder_get_object (builder_p,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNELS)));
  ACE_ASSERT (liststore_p);
  // make it sort the channels by #members...
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (liststore_p),
                                        1, GTK_SORT_DESCENDING);

  // connect signal(s)
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_DISCONNECT)));
  ACE_ASSERT (button_p);
  gulong result_2 = g_signal_connect (button_p,
                                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                                      G_CALLBACK (button_disconnect_clicked_cb),
                                      &CBData_);
  ACE_ASSERT (result_2);
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object (builder_p,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_NICK)));
  ACE_ASSERT (entry_p);
  result_2 = g_signal_connect (entry_p,
                               ACE_TEXT_ALWAYS_CHAR ("focus-in-event"),
                               G_CALLBACK (nick_entry_kb_focused_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_NICK_ACCEPT)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (change_clicked_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  GtkComboBox* combobox_p =
    GTK_COMBO_BOX (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_USERS)));
  ACE_ASSERT (combobox_p);
  result_2 = g_signal_connect (combobox_p,
                             ACE_TEXT_ALWAYS_CHAR ("changed"),
                             G_CALLBACK (usersbox_changed_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_USERS_REFRESH)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (refresh_users_clicked_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  entry_p =
    GTK_ENTRY (gtk_builder_get_object (builder_p,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ENTRY_CONNECTION_CHANNEL)));
  ACE_ASSERT (entry_p);
  result_2 = g_signal_connect (entry_p,
                             ACE_TEXT_ALWAYS_CHAR ("focus-in-event"),
                             G_CALLBACK (channel_entry_kb_focused_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_JOIN)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (join_clicked_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  combobox_p =
    GTK_COMBO_BOX (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_CHANNELS)));
  ACE_ASSERT (combobox_p);
  result_2 = g_signal_connect (combobox_p,
                             ACE_TEXT_ALWAYS_CHAR ("changed"),
                             G_CALLBACK (channelbox_changed_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_CHANNELS_REFRESH)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (refresh_channels_clicked_cb),
                             &CBData_);
  ACE_ASSERT (result_2);

  // togglebuttons
  GtkToggleButton* togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_OPERATOR)));
  ACE_ASSERT (togglebutton_p);
  result_2 = g_signal_connect (togglebutton_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (user_mode_toggled_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_LOCALOPERATOR)));
  ACE_ASSERT (togglebutton_p);
  result_2 = g_signal_connect (togglebutton_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (user_mode_toggled_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_RESTRICTED)));
  ACE_ASSERT (togglebutton_p);
  result_2 = g_signal_connect (togglebutton_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (user_mode_toggled_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
  ACE_ASSERT (togglebutton_p);
  result_2 = g_signal_connect (togglebutton_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (user_mode_toggled_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS)));
  ACE_ASSERT (togglebutton_p);
  result_2 = g_signal_connect (togglebutton_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (user_mode_toggled_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES)));
  ACE_ASSERT (togglebutton_p);
  result_2 = g_signal_connect (togglebutton_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (user_mode_toggled_cb),
                             &CBData_);
  ACE_ASSERT (result_2);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE)));
  ACE_ASSERT (togglebutton_p);
  result_2 = g_signal_connect (togglebutton_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (user_mode_toggled_cb),
                             &CBData_);
  ACE_ASSERT (result_2);

  // actions
  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_AWAY)));
  ACE_ASSERT (action_p);
  result_2 = g_signal_connect (action_p,
                             ACE_TEXT_ALWAYS_CHAR ("activate"),
                             G_CALLBACK (action_away_cb),
                             &CBData_);
  ACE_ASSERT (result_2);

  // retrieve channel tabs
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object (builder_p,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);
  result_2 = g_signal_connect (notebook_p,
                             ACE_TEXT_ALWAYS_CHAR ("switch-page"),
                             G_CALLBACK (switch_channel_cb),
                             &CBData_);
  ACE_ASSERT (result_2);

  // retrieve server log tab child
  GtkScrolledWindow* scrolledwindow_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder_p,
                                                 ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_SCROLLEDWINDOW_CONNECTION)));
  ACE_ASSERT (scrolledwindow_p);
  // disallow reordering the server log tab
  gtk_notebook_set_tab_reorderable (notebook_p,
                                    GTK_WIDGET (scrolledwindow_p),
                                    FALSE);

  // create default IRC_Client_GUI_MessageHandler (== server log)
  // retrieve server log textview
  GtkTextView* textview_p =
    GTK_TEXT_VIEW (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TEXTVIEW_CONNECTION)));
  ACE_ASSERT (textview_p);
  IRC_Client_GUI_MessageHandler* messagehandler_p = NULL;
  ACE_NEW_NORETURN (messagehandler_p,
                    IRC_Client_GUI_MessageHandler (CBData_.GTKState,
                                                   textview_p));
  if (!messagehandler_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    g_object_unref (G_OBJECT (builder_p));

    return;
  } // end IF

  // *NOTE*: the empty channel name string denotes the log handler !
  // *NOTE*: no updates yet --> no need for locking
  // *NOTE*: in theory, there is a race condition as the user may start
  // interacting with the new UI elements by now - as GTK will draw the new elements
  // only after we return, this is not really a problem...
  std::pair <MESSAGE_HANDLERSITERATOR_T, bool> result =
      messageHandlers_.insert (std::make_pair (std::string (),
                                               messagehandler_p));
  if (!result.second)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to insert message handler: \"%m\", returning\n")));

    // clean up
    delete messagehandler_p;
    g_object_unref (G_OBJECT (builder_p));

    return;
  } // end IF

  // subscribe to updates from the controller
  try
  {
    CBData_.controller->subscribe (this);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::subscribe(%@), returning\n"),
                this));

    // clean up
    messageHandlers_.erase (result.first);
    delete messagehandler_p;
    g_object_unref (G_OBJECT (builder_p));

    return;
  }

  // add the new server page to the (parent) notebook
  // retrieve (dummy) parent window
  GtkWindow* window_p =
    GTK_WINDOW (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_TAB_CONNECTION)));
  ACE_ASSERT (window_p);
  // retrieve server tab label
  GtkHBox* hbox_p =
    GTK_HBOX (gtk_builder_get_object (builder_p,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_CONNECTION_TAB)));
  ACE_ASSERT (hbox_p);
  g_object_ref (hbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (hbox_p));
  // set tab label
  GtkLabel* label_p =
    GTK_LABEL (gtk_builder_get_object (builder_p,
                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_CONNECTION_TAB)));
  ACE_ASSERT (label_p);
  // *TODO*: convert to UTF8 ?
  gtk_label_set_text (label_p,
                      label_in.c_str ());

  // retrieve (dummy) parent window
  window_p =
    GTK_WINDOW (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_CONNECTION)));
  ACE_ASSERT (window_p);
  // retrieve server tab
  GtkVBox* vbox_p =
    GTK_VBOX (gtk_builder_get_object (builder_p,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CONNECTION)));
  ACE_ASSERT (vbox_p);
  g_object_ref (vbox_p);
  gtk_container_remove (GTK_CONTAINER (window_p),
                        GTK_WIDGET (vbox_p));
  gint page_num =
    gtk_notebook_append_page (parent_,
                              GTK_WIDGET (vbox_p),
                              GTK_WIDGET (hbox_p));
  if (page_num == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_notebook_append_page(%@), returning\n"),
                parent_));

    // clean up
    g_object_unref (hbox_p);
    g_object_unref (vbox_p);
    try
    {
      CBData_.controller->unsubscribe (this);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in IRC_Client_IIRCControl::unsubscribe(%@), continuing\n"),
                  this));
    }
    messageHandlers_.erase (result.first);
    delete messagehandler_p;
    g_object_unref (G_OBJECT (builder_p));

    return;
  } // end IF
  g_object_unref (hbox_p);

  // allow reordering
  gtk_notebook_set_tab_reorderable (parent_,
                                    GTK_WIDGET (vbox_p),
                                    TRUE);
  g_object_unref (vbox_p);

  // activate new page
  gtk_notebook_set_current_page (parent_,
                                 page_num);

  // synch access
  {
    ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

    CBData_.GTKState->builders[label_] =
        std::make_pair (ui_definition_filename, builder_p);
  } // end lock scope

  isInitialized_ = true;
}

IRC_Client_GUI_Connection::~IRC_Client_GUI_Connection ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::~IRC_Client_GUI_Connection"));

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  // unsubscribe to updates from the controller
  try
  {
    CBData_.controller->unsubscribe (this);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::unsubscribe(%@), continuing\n"),
                this));
  }

  // clean up message handlers
  for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
       iterator != messageHandlers_.end ();
       iterator++)
    delete (*iterator).second;

  ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  // remove this from the connection list
  connections_iterator_t iterator =
    CBData_.connections->find (label_);
  if (iterator != CBData_.connections->end ())
    CBData_.connections->erase (iterator);

  // remove server page from parent notebook
  Common_UI_GTKBuildersIterator_t iterator_2 =
      CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator_2 == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (label_.c_str ())));

    return;
  } // end IF
  GtkVBox* vbox_p =
    GTK_VBOX (gtk_builder_get_object ((*iterator_2).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CONNECTION)));
  ACE_ASSERT (vbox_p);
  guint page_num = gtk_notebook_page_num (parent_,
                                          GTK_WIDGET (vbox_p));
  // flip away from "this" page ?
  if (gtk_notebook_get_current_page (parent_) == static_cast<gint> (page_num))
    gtk_notebook_prev_page (parent_);
  gtk_notebook_remove_page (parent_,
                            page_num);

  g_object_unref (G_OBJECT ((*iterator_2).second.second));
  CBData_.GTKState->builders.erase (iterator_2);
}

void
IRC_Client_GUI_Connection::start (const IRC_Client_StreamModuleConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::start"));

  ACE_UNUSED_ARG (configuration_in);

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("connected...\n")));
}

void
IRC_Client_GUI_Connection::notify (const IRC_Client_IRCMessage& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  Common_UI_GTKBuildersIterator_t iterator =
      CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (label_.c_str ())));
    return;
  } // end IF

  switch (message_in.command.discriminator)
  {
    case IRC_Client_IRCMessage::Command::NUMERIC:
    {
      switch (message_in.command.numeric)
      {
        case IRC_Client_IRC_Codes::RPL_WELCOME:          //   1
        {
          // *NOTE*: this is the first message in any connection !

          // remember nickname
          CBData_.IRCSessionState.nickname = message_in.params.front ();

          gdk_threads_enter ();

          // --> display (starting) nickname
          // set server tab nickname label
          GtkLabel* label_p =
            GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_NICK)));
          ACE_ASSERT (label_p);
          // --> see Pango Text Attribute Markup Language...
          std::string nickname_string = ACE_TEXT_ALWAYS_CHAR ("<b><i>nickname</i></b> ");
          nickname_string += message_in.params.front ();
          gtk_label_set_markup (label_p,
                                nickname_string.c_str ());

          // retrieve button handle
          GtkHBox* hbox_p =
            GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_NICK_CHANNEL)));
          ACE_ASSERT (hbox_p);
          gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), TRUE);
          hbox_p =
            GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_USERMODE)));
          ACE_ASSERT (hbox_p);
          gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), TRUE);

          gdk_threads_leave ();

          // *WARNING*: falls through !
        }
        case IRC_Client_IRC_Codes::RPL_YOURHOST:         //   2
        case IRC_Client_IRC_Codes::RPL_CREATED:          //   3
        case IRC_Client_IRC_Codes::RPL_MYINFO:           //   4
        case IRC_Client_IRC_Codes::RPL_PROTOCTL:         //   5
        case IRC_Client_IRC_Codes::RPL_YOURID:           //  42
        case IRC_Client_IRC_Codes::RPL_STATSDLINE:       // 250
        case IRC_Client_IRC_Codes::RPL_LUSERCLIENT:      // 251
        case IRC_Client_IRC_Codes::RPL_LUSEROP:          // 252
        case IRC_Client_IRC_Codes::RPL_LUSERUNKNOWN:     // 253
        case IRC_Client_IRC_Codes::RPL_LUSERCHANNELS:    // 254
        case IRC_Client_IRC_Codes::RPL_LUSERME:          // 255
        case IRC_Client_IRC_Codes::RPL_TRYAGAIN:         // 263
        case IRC_Client_IRC_Codes::RPL_LOCALUSERS:       // 265
        case IRC_Client_IRC_Codes::RPL_GLOBALUSERS:      // 266
        case IRC_Client_IRC_Codes::RPL_INVITING:         // 341
        {
          log (message_in);
          break;
        }
        case IRC_Client_IRC_Codes::RPL_USERHOST:         // 302
        {
          // bisect (WS-separated) userhost records from the final parameter

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting records: \"%s\"...\n"),
          //            ACE_TEXT (message_in.params.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string record;
          string_list_t list;
          do
          {
            current_position = message_in.params.back ().find (' ', last_position);

            record =
              message_in.params.back ().substr (last_position,
                                                (((current_position == std::string::npos) ? message_in.params.back ().size ()
                                                                                          : current_position) - last_position));

            // check whether the record is empty
            if (!record.empty ())
              list.push_back (record);

            // advance
            last_position = current_position + 1;
          } while (current_position != std::string::npos);

          std::string message_string;
          for (string_list_const_iterator_t iterator = list.begin ();
               iterator != list.end ();
               iterator++)
          {
            current_position = (*iterator).find ('=', 0);
            if (current_position == std::string::npos)
            {
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("invalid USERHOST record: \"%s\", continuing\n"),
                          ACE_TEXT ((*iterator).c_str ())));
              continue;
            } // end IF

            message_string = (*iterator).substr (0, current_position);
            message_string += ACE_TEXT_ALWAYS_CHAR (" is \"");

            // advance
            last_position = current_position;

            current_position = (*iterator).find ('@', current_position);
            if (current_position == std::string::npos)
            {
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("invalid USERHOST record: \"%s\", continuing\n"),
                          ACE_TEXT ((*iterator).c_str ())));
              continue;
            } // end IF
            message_string += (*iterator).substr (last_position + 1,
                                                  (current_position - last_position - 1));
            message_string += ACE_TEXT_ALWAYS_CHAR ("\" on \"");
            message_string += (*iterator).substr (current_position + 1);
            message_string += ACE_TEXT_ALWAYS_CHAR ("\"\n");

            log (message_string);
          } // end FOR

          break;
        }
        case IRC_Client_IRC_Codes::RPL_UNAWAY:           // 305
        {
          gdk_threads_enter ();

          // *WARNING*: needs the lock protection, otherwise there is a race...
          CBData_.IRCSessionState.away = false;

          // retrieve togglebutton
          GtkToggleButton* togglebutton_p =
            GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
          ACE_ASSERT (togglebutton_p);
          gtk_toggle_button_set_active (togglebutton_p, FALSE);

          gdk_threads_leave ();

          log (message_in);

          break;
        }
        case IRC_Client_IRC_Codes::RPL_NOWAWAY:          // 306
        {
          gdk_threads_enter ();

          // *WARNING*: needs the lock protection, otherwise there is a race...
          CBData_.IRCSessionState.away = true;

          // retrieve togglebutton
          GtkToggleButton* togglebutton_p =
            GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
          ACE_ASSERT (togglebutton_p);
          gtk_toggle_button_set_active (togglebutton_p, TRUE);

          gdk_threads_leave ();

          log (message_in);

          break;
        }
        case IRC_Client_IRC_Codes::RPL_ENDOFWHO:         // 315
        {
          gdk_threads_enter ();

          // retrieve server tab users store
          GtkComboBox* combobox_p =
            GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_USERS)));
          ACE_ASSERT (combobox_p);
          gtk_widget_set_sensitive (GTK_WIDGET (combobox_p), TRUE);

          gdk_threads_leave ();

          log (message_in);

          isFirstUsersMsg_ = true;

          break;
        }
        case IRC_Client_IRC_Codes::RPL_LISTSTART:        // 321
        {
          gdk_threads_enter ();

          // retrieve server tab channels store
          GtkListStore* liststore_p =
            GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNELS)));
          ACE_ASSERT (liststore_p);

          // clear the store
          gtk_list_store_clear (liststore_p);

          gdk_threads_leave ();

          // *WARNING*: falls through !
        }
        case IRC_Client_IRC_Codes::RPL_LISTEND:          // 323
        {
          gdk_threads_enter();

          // retrieve server tab channels store
          GtkComboBox* combobox_p =
            GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_COMBOBOX_CHANNELS)));
          ACE_ASSERT (combobox_p);
          gtk_widget_set_sensitive (GTK_WIDGET (combobox_p), TRUE);

          gdk_threads_leave ();

          log (message_in);

          break;
        }
        case IRC_Client_IRC_Codes::RPL_LIST:             // 322
        {
          gdk_threads_enter ();

          // retrieve server tab channels store
          GtkListStore* liststore_p =
            GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNELS)));
          ACE_ASSERT (liststore_p);

          // convert <# visible>
          IRC_Client_ParametersIterator_t param_iterator = message_in.params.begin ();
          ACE_ASSERT (message_in.params.size () >= 3);
          std::advance (param_iterator, 2);
          std::stringstream converter;
          guint num_members = 0;
          converter << *param_iterator;
          converter >> num_members;
          param_iterator--;

          // convert text
          gchar* converted_text =
            Common_UI_Tools::Locale2UTF8 (*param_iterator);
          if (!converted_text)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
                        ACE_TEXT ((*param_iterator).c_str ())));

            // clean up
            gdk_threads_leave ();

            break;
          } // end IF

          GtkTreeIter list_iterator;
          gtk_list_store_append (liststore_p,
                                 &list_iterator);
          gtk_list_store_set (liststore_p, &list_iterator,
                              0, converted_text,                     // channel name
                              1, num_members,                        // # visible members
                              2, message_in.params.back ().c_str (), // topic
                              -1);

          // clean up
          g_free (converted_text);
          gdk_threads_leave ();

          break;
        }
        case IRC_Client_IRC_Codes::RPL_NOTOPIC:          // 331
        case IRC_Client_IRC_Codes::RPL_TOPIC:            // 332
        case IRC_Client_IRC_Codes::RPL_TOPICWHOTIME:     // 333
        {
          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (message_in.params.front ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (message_in.params.front ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->setTopic (((message_in.command.numeric == IRC_Client_IRC_Codes::RPL_NOTOPIC) ? ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_DEF_TOPIC_TEXT)
                                                                                                                   : message_in.params.back ()));

          break;
        }
        case IRC_Client_IRC_Codes::RPL_WHOREPLY:         // 352
        {
          // bisect user information from parameter strings
          IRC_Client_ParametersIterator_t iterator_2 =
            message_in.params.begin ();
          ACE_ASSERT (message_in.params.size () >= 8);
          std::advance (iterator_2, 5); // nick position
          std::string nick = *iterator_2;
          iterator_2++;
          bool away = ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("G"), 0) == 0);
          bool is_IRCoperator =
            ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("*"), 1) == 1);
          bool is_operator =
            ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("@"), 2) != std::string::npos);
          bool is_voiced =
            ((*iterator_2).find (ACE_TEXT_ALWAYS_CHAR ("+"), 2) != std::string::npos);
          unsigned int hop_count = 0;
          std::string real_name;
          std::stringstream converter;
          std::string::size_type ws_position = 0;
          ws_position = message_in.params.back ().find (' ', 0);
          converter << message_in.params.back ().substr (0, ws_position);
          converter >> hop_count;
          real_name = message_in.params.back ().substr (ws_position + 1);

          gdk_threads_enter ();

          // retrieve server tab users store
          Common_UI_GTKBuildersIterator_t iterator_3 =
            CBData_.GTKState->builders.find (label_);
          // sanity check(s)
          if (iterator_3 == CBData_.GTKState->builders.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                        ACE_TEXT (label_.c_str ())));

            // clean up
            gdk_threads_leave ();

            break;
          } // end IF

          GtkListStore* liststore_p =
            GTK_LIST_STORE (gtk_builder_get_object ((*iterator_3).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_USERS)));
          ACE_ASSERT (liststore_p);

          if (isFirstUsersMsg_)
          {
            gtk_list_store_clear (liststore_p);

            isFirstUsersMsg_ = false;
          } // end IF

          // ignore own record
          if (nick == CBData_.IRCSessionState.nickname)
          {
            // clean up
            gdk_threads_leave ();

            break;
          } // end IF

          // step1: convert text
          gchar* string_p = Common_UI_Tools::Locale2UTF8 (nick);
          if (!string_p)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to convert nickname: \"%s\", aborting\n"),
                        ACE_TEXT (nick.c_str ())));

            // clean up
            gdk_threads_leave ();

            break;
          } // end IF
          gchar* string_2 = Common_UI_Tools::Locale2UTF8 (real_name);
          if (!string_2)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to convert name: \"%s\", aborting\n"),
                        ACE_TEXT (real_name.c_str ())));

            // clean up
            g_free (string_p);
            gdk_threads_leave ();

            break;
          } // end IF

          // step2: append new (text) entry
          GtkTreeIter iterator;
          gtk_list_store_append (liststore_p, &iterator);
          gtk_list_store_set (liststore_p, &iterator,
                              0, string_p, // column 0
                              1, away,
                              2, is_IRCoperator,
                              3, is_operator,
                              4, is_voiced,
                              5, hop_count,
                              6, string_2,
                              -1);

          // clean up
          g_free (string_p);
          g_free (string_2);
          gdk_threads_leave ();

          break;
        }
        case IRC_Client_IRC_Codes::RPL_NAMREPLY:         // 353
        {
          // bisect (WS-separated) nicknames from the final parameter string

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting nicknames: \"%s\"...\n"),
          //            ACE_TEXT (message_in.params.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string nick;
          string_list_t list;
          bool is_operator = false;
          do
          {
            current_position =
              message_in.params.back ().find (' ', last_position);
            nick =
              message_in.params.back ().substr (last_position,
                                                (((current_position == std::string::npos) ? message_in.params.back ().size ()
                                                                                          : current_position) - last_position));

            // check whether user is a channel operator
            if (nick.find (CBData_.IRCSessionState.nickname) != std::string::npos)
              is_operator = ((nick[0] == '@') &&
                             (nick.size () == (CBData_.IRCSessionState.nickname.size () + 1)));

            list.push_back (nick);

            // advance
            last_position = current_position + 1;
          } while (current_position != std::string::npos);

          // retrieve channel name
          IRC_Client_ParametersIterator_t param_iterator =
            message_in.params.begin ();
          ACE_ASSERT (message_in.params.size () >= 3);
          std::advance (param_iterator, 2);

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (*param_iterator);
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT ((*param_iterator).c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->members (list);

          if (is_operator)
          {
            // *NOTE*: ops always have a voice...
            std::string op_mode = ACE_TEXT_ALWAYS_CHAR ("+ov");
            (*handler_iterator).second->setModes (op_mode,
                                                  std::string ()); // none
          } // end IF

          break;
        }
        case IRC_Client_IRC_Codes::RPL_ENDOFNAMES:       // 366
        {
          // retrieve channel name
          IRC_Client_ParametersIterator_t param_iterator =
            message_in.params.begin ();
          param_iterator++;

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (*param_iterator);
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT ((*param_iterator).c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->endMembers ();

          break;
        }
        case IRC_Client_IRC_Codes::RPL_ENDOFBANLIST:     // 368
        case IRC_Client_IRC_Codes::RPL_MOTD:             // 372
        case IRC_Client_IRC_Codes::RPL_MOTDSTART:        // 375
        case IRC_Client_IRC_Codes::RPL_ENDOFMOTD:        // 376
        case IRC_Client_IRC_Codes::ERR_NOSUCHNICK:       // 401
        case IRC_Client_IRC_Codes::ERR_NOMOTD:           // 422
        case IRC_Client_IRC_Codes::ERR_NICKNAMEINUSE:    // 433
        case IRC_Client_IRC_Codes::ERR_NOTREGISTERED:    // 451
        case IRC_Client_IRC_Codes::ERR_NEEDMOREPARAMS:   // 461
        case IRC_Client_IRC_Codes::ERR_YOUREBANNEDCREEP: // 465
        case IRC_Client_IRC_Codes::ERR_BADCHANNAME:      // 479
        case IRC_Client_IRC_Codes::ERR_CHANOPRIVSNEEDED: // 482
        case IRC_Client_IRC_Codes::ERR_UMODEUNKNOWNFLAG: // 501
        {
          log (message_in);

          if ((message_in.command.numeric == IRC_Client_IRC_Codes::ERR_NOSUCHNICK)       ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_NICKNAMEINUSE)    ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_NOTREGISTERED)    ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_YOUREBANNEDCREEP) ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_BADCHANNAME)      ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_CHANOPRIVSNEEDED) ||
              (message_in.command.numeric == IRC_Client_IRC_Codes::ERR_UMODEUNKNOWNFLAG))
            error (message_in); // show in statusbar as well...

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown (numeric) command/reply (was: \"%s\" (%u)), continuing\n"),
                      ACE_TEXT (IRC_Client_Tools::IRCCode2String (message_in.command.numeric).c_str ()),
                      message_in.command.numeric));

          message_in.dump_state ();

          break;
        }
      } // end SWITCH

      break;
    }
    case IRC_Client_IRCMessage::Command::STRING:
    {
      IRC_Client_IRCMessage::CommandType command =
        IRC_Client_Tools::IRCCommandString2Type (*message_in.command.string);
      switch (command)
      {
        case IRC_Client_IRCMessage::NICK:
        {
          // remember changed nickname...
          std::string old_nick = CBData_.IRCSessionState.nickname;
          CBData_.IRCSessionState.nickname = message_in.params.front ();

          // --> display (changed) nickname
          // step1: set server tab nickname label
          gdk_threads_enter ();

          GtkLabel* label_p =
            GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_NICK)));
          ACE_ASSERT (label_p);
          // --> see Pango Text Attribute Markup Language...
          std::string nickname_string = ACE_TEXT_ALWAYS_CHAR ("<b><i>nickname</i></b> ");
          nickname_string += CBData_.IRCSessionState.nickname;
          gtk_label_set_markup (label_p,
                                nickname_string.c_str ());

          gdk_threads_leave ();

          // step2: update channel tab nickname label(s)
          for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
               iterator != messageHandlers_.end ();
               iterator++)
          {
            if ((*iterator).second->isServerLog ())
              continue;

            (*iterator).second->updateNick (old_nick);
          } // end FOR

          // *WARNING*: falls through !
        }
        case IRC_Client_IRCMessage::USER:
        case IRC_Client_IRCMessage::QUIT:
        {
          log (message_in);

          if ((message_in.prefix.origin == CBData_.IRCSessionState.nickname) &&
              (command == IRC_Client_IRCMessage::QUIT))
            error (message_in); // --> show on statusbar as well...

          break;
        }
        case IRC_Client_IRCMessage::JOIN:
        {
          // there are two possibilities:
          // - reply from a successful join request
          // - stranger entering the channel

          // reply from a successful join request ?
          if (message_in.prefix.origin == CBData_.IRCSessionState.nickname)
          {
            createMessageHandler (message_in.params.front ());
            break;
          } // end IF

          // someone joined a common channel...

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (message_in.params.back ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (message_in.params.back ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->add (message_in.prefix.origin);

          break;
        }
        case IRC_Client_IRCMessage::PART:
        {
          // there are two possibilities:
          // - reply from a (successful) part request
          // - someone left a common channel

          // reply from a successful part request ?
          if (message_in.prefix.origin == CBData_.IRCSessionState.nickname)
          {
            terminateMessageHandler (message_in.params.back ());
            break;
          } // end IF

          // someone left a common channel...

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (message_in.params.back ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (message_in.params.back ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->remove (message_in.prefix.origin);

          break;
        }
        case IRC_Client_IRCMessage::MODE:
        {
          // there are two possibilities:
          // - user mode message
          // - channel mode message

          // retrieve mode string
          IRC_Client_ParametersIterator_t param_iterator =
            message_in.params.begin ();
          param_iterator++;

          if (message_in.params.front () == CBData_.IRCSessionState.nickname)
          {
            // --> user mode
            // *WARNING*: needs the lock protection, otherwise there is a race...
            IRC_Client_Tools::merge (message_in.params.back (),
                                     CBData_.IRCSessionState.userModes);

            // *WARNING*: needs the lock protection, otherwise there is a race...
            updateModeButtons ();
          } // end IF
          else
          {
            // --> channel mode

            // retrieve message handler
            MESSAGE_HANDLERSITERATOR_T handler_iterator =
              messageHandlers_.find (message_in.params.front ());
            if (handler_iterator == messageHandlers_.end ())
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                          ACE_TEXT (message_in.params.front ().c_str ())));
              break;
            } // end IF

            // *WARNING*: needs the lock protection, otherwise there is a race...
            (*handler_iterator).second->setModes (*param_iterator,
                                                  ((*param_iterator == message_in.params.back ()) ? std::string ()
                                                                                                  : message_in.params.back ()));
          } // end ELSE

          // log this event
          log (message_in);

          break;
        }
        case IRC_Client_IRCMessage::TOPIC:
        {
          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (message_in.params.front ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (message_in.params.front ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->setTopic (message_in.params.back ());

          // log this event
          log (message_in);

          break;
        }
        case IRC_Client_IRCMessage::KICK:
        {
          // retrieve nickname string
          IRC_Client_ParametersIterator_t param_iterator =
            message_in.params.begin ();
          param_iterator++;

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (message_in.params.front ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (message_in.params.front ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->remove (*param_iterator);

          // log this event
          log (message_in);

          break;
        }
        case IRC_Client_IRCMessage::PRIVMSG:
        {
          // *TODO*: parse (list of) receiver(s)

          std::string message_text;
          if (!message_in.prefix.origin.empty ())
          {
            message_text += ACE_TEXT_ALWAYS_CHAR ("<");
            message_text += message_in.prefix.origin;
            message_text += ACE_TEXT_ALWAYS_CHAR ("> ");
          } // end IF
          message_text += message_in.params.back ();
          message_text += ACE_TEXT_ALWAYS_CHAR ("\n");

          // private message ?
          std::string target_id;
          if (CBData_.IRCSessionState.nickname == message_in.params.front ())
          {
            // --> send to private conversation handler

            // part of an existing conversation ?

            // retrieve message handler
            if (messageHandlers_.find (message_in.prefix.origin) == messageHandlers_.end ())
              createMessageHandler (message_in.prefix.origin);
          } // end IF

          // channel/nick message ?
          forward (((CBData_.IRCSessionState.nickname == message_in.params.front ()) ? message_in.prefix.origin
                                                                                     : message_in.params.front ()),
                   message_text);

          break;
        }
        case IRC_Client_IRCMessage::NOTICE:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case IRC_Client_IRCMessage::__QUIRK__ERROR:
#else
        case IRC_Client_IRCMessage::ERROR:
#endif
        case IRC_Client_IRCMessage::AWAY:
        {
          log (message_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
          if (command == IRC_Client_IRCMessage::__QUIRK__ERROR)
#else
          if (command == IRC_Client_IRCMessage::ERROR)
#endif
            error (message_in); // --> show on statusbar as well...

          break;
        }
        case IRC_Client_IRCMessage::PING:
          break;
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown command (was: \"%s\"), continuing\n"),
                      ACE_TEXT (message_in.command.string->c_str ())));

          message_in.dump_state ();

          break;
        }
      } // end SWITCH

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid command type (was: %u), continuing\n"),
                  message_in.command.discriminator));
      break;
    }
  } // end SWITCH
}

void
IRC_Client_GUI_Connection::end ()
{
  NETWORK_TRACE(ACE_TEXT("IRC_Client_GUI_Connection::end"));

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("connection lost...\n")));

  // clean up
  delete this;
}

IRC_Client_IIRCControl*
IRC_Client_GUI_Connection::getController ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::getController"));

  // sanity check(s)
  ACE_ASSERT (CBData_.controller);

  return CBData_.controller;
}

IRC_Client_GUI_MessageHandler*
IRC_Client_GUI_Connection::getHandler (const std::string& id_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::getHandler"));

  // existing conversation ? --> retrieve message handler

  MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.find (id_in);
  if (iterator != messageHandlers_.end ())
    return (*iterator).second;

  return NULL;
}

std::string
IRC_Client_GUI_Connection::getNickname () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::getNickname"));

  // sanity check(s)
  ACE_ASSERT (!CBData_.IRCSessionState.nickname.empty ());

  return CBData_.IRCSessionState.nickname;
}

std::string
IRC_Client_GUI_Connection::getLabel () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::getLabel"));

  return label_;
}

std::string
IRC_Client_GUI_Connection::getActiveID ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::getActiveID"));

  // initialize result
  std::string result;

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  //ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  Common_UI_GTKBuildersIterator_t iterator =
      CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (label_.c_str ())));
    return result;
  } // end IF

  // retrieve server tab channel tabs handle
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);
  gint page_num = gtk_notebook_get_current_page (notebook_p);
  // sanity check(s)
  if (page_num == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_notebook_get_current_page(%@): no pages, aborting\n"),
                notebook_p));
    return result;
  } // end IF
  // server log ? --> no active handler --> return empty string
  if (page_num == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no active handler, aborting\n")));
    return result;
  } // end IF

  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   page_num);
  ACE_ASSERT (widget_p);

  for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
       iterator != messageHandlers_.end ();
       iterator++)
  {
    if ((*iterator).second->getTopLevelPageChild () == widget_p)
    {
      result = (*iterator).first;

      break;
    } // end IF
  } // end FOR

  // sanity check
  if (result.empty ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to lookup active handler, aborting\n")));

  return result;
}

IRC_Client_GUI_MessageHandler*
IRC_Client_GUI_Connection::getActiveHandler (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::getActiveHandler"));

  IRC_Client_GUI_MessageHandler* return_value = NULL;

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  if (lockedAccess_in)
    CBData_.GTKState->lock.acquire ();

  Common_UI_GTKBuildersIterator_t iterator =
    CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (label_.c_str ())));

    // clean up
    if (lockedAccess_in)
      CBData_.GTKState->lock.release ();

    return NULL;
  } // end IF

  // retrieve server tab channel tabs handle
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);
  gint page_num = gtk_notebook_get_current_page (notebook_p);
  if (page_num == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_notebook_get_current_page(%@): no pages, aborting\n"),
                notebook_p));

    // clean up
    if (lockedAccess_in)
      CBData_.GTKState->lock.release ();

    return NULL;
  } // end IF
  GtkWidget* widget_p = gtk_notebook_get_nth_page (notebook_p,
                                                   page_num);
  ACE_ASSERT (widget_p);

  GtkWidget* widget_2 = NULL;
  for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
       iterator != messageHandlers_.end ();
       iterator++)
  {
    // server log ?
    if ((page_num == 0) &&
        (*iterator).first.empty ())
    {
      return_value = (*iterator).second;
      break;
    } // end IF

    widget_2 = (*iterator).second->getTopLevelPageChild ();
    if (widget_2 == widget_p)
    {
      return_value = (*iterator).second;
      break;
    } // end IF
  } // end FOR
  ACE_ASSERT (return_value);

  // clean up
  if (lockedAccess_in)
    CBData_.GTKState->lock.release ();

  return return_value;
}

void
IRC_Client_GUI_Connection::forward (const std::string& channel_in,
                                    const std::string& messageText_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::forward"));

  // --> pass to channel log

  // retrieve message handler
  MESSAGE_HANDLERSITERATOR_T handler_iterator =
    messageHandlers_.find (channel_in);
  if (handler_iterator == messageHandlers_.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                ACE_TEXT (channel_in.c_str ())));
    return;
  } // end IF

  (*handler_iterator).second->queueForDisplay (messageText_in);
}

void
IRC_Client_GUI_Connection::log (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::log"));

  // --> pass to server log

  // retrieve message handler
  MESSAGE_HANDLERSITERATOR_T handler_iterator =
    messageHandlers_.find (std::string ());
  ACE_ASSERT (handler_iterator != messageHandlers_.end ());
  (*handler_iterator).second->queueForDisplay (message_in);
}

void
IRC_Client_GUI_Connection::log (const IRC_Client_IRCMessage& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::log"));

  // --> pass to server log

  // retrieve message handler
  MESSAGE_HANDLERSITERATOR_T handler_iterator =
    messageHandlers_.find (std::string ());
  ACE_ASSERT (handler_iterator != messageHandlers_.end ());
  (*handler_iterator).second->queueForDisplay (IRC_Client_Tools::IRCMessage2String (message_in));
}

void
IRC_Client_GUI_Connection::error (const IRC_Client_IRCMessage& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::error"));

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  Common_UI_GTKBuildersIterator_t iterator =
    CBData_.GTKState->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (label_.c_str ())));
    return;
  } // end IF

  gdk_threads_enter ();

  // error --> print on statusbar
  GtkStatusbar* statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_STATUSBAR)));
  ACE_ASSERT (statusbar_p);

  gtk_statusbar_push (statusbar_p,
                      contextID_,
                      IRC_Client_Tools::dump (message_in).c_str ());

  gdk_threads_leave ();
}

guint
IRC_Client_GUI_Connection::exists (const std::string& id_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::exists"));

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  Common_UI_GTKBuildersIterator_t iterator =
      CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (label_.c_str ())));
    return -1;
  } // end IF

  // retrieve server tab channel tabs handle
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);

  MESSAGE_HANDLERSITERATOR_T iterator_2 = messageHandlers_.find (id_in);
  // sanity check
  if ((messageHandlers_.empty ()) ||
      (iterator_2 == messageHandlers_.end ()))
    return -1;

  // *NOTE*: page_num 0 is the server log: ALWAYS !
  GtkWidget* widget_p = NULL;
  gint num_pages = gtk_notebook_get_n_pages (notebook_p);
  for (gint page_num = 0;
       page_num < num_pages;
       page_num++)
  {
    // *NOTE*: channel id's are unique, but notebook page numbers
    // change as channels are joined/parted and private conversations "come and
    // go", (for lack of a better metaphor)...
    widget_p = gtk_notebook_get_nth_page (notebook_p,
                                          page_num);
    ACE_ASSERT (widget_p);

    if ((*iterator_2).second->getTopLevelPageChild () == widget_p)
      return page_num;
  } // end FOR

  return -1;
}

void
IRC_Client_GUI_Connection::channels (string_list_t& channels_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::channels"));

  // initialize return value
  channels_out.clear ();

  for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
       iterator != messageHandlers_.end ();
       iterator++)
    if (IRC_Client_Tools::isValidIRCChannelName ((*iterator).first))
      channels_out.push_back ((*iterator).first);
}

void
IRC_Client_GUI_Connection::createMessageHandler (const std::string& id_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::createMessageHandler"));

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  Common_UI_GTKBuildersIterator_t iterator =
    CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (label_.c_str ())));
    return;
  } // end IF

  gdk_threads_enter ();

  // retrieve channel tabs
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);

  // create new IRC_Client_GUI_MessageHandler
  IRC_Client_GUI_MessageHandler* message_handler_p = NULL;
  ACE_NEW_NORETURN (message_handler_p,
                    IRC_Client_GUI_MessageHandler (CBData_.GTKState,
                                                   this,
                                                   CBData_.controller,
                                                   id_in,
                                                   UIFileDirectory_,
                                                   notebook_p));
  if (!message_handler_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));

    // clean up
    gdk_threads_leave ();

    return;
  } // end IF

  messageHandlers_.insert (std::make_pair (id_in, message_handler_p));

  // check whether this is the first channel of the first connection
  // --> enable corresponding widget(s) in the main UI
  if ((CBData_.connections->size () == 1) &&
      (messageHandlers_.size () == 2)) // server log + first channel
  {
    iterator =
      CBData_.GTKState->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
    // sanity check(s)
    ACE_ASSERT (iterator != CBData_.GTKState->builders.end ());
    GtkHBox* hbox_p =
      GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_SEND)));
    ACE_ASSERT (hbox_p);
    gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), TRUE);
  } // end IF

  gdk_threads_leave ();
}

void
IRC_Client_GUI_Connection::terminateMessageHandler (const std::string& id_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::terminateMessageHandler"));

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  Common_UI_GTKBuildersIterator_t iterator =
      CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (label_.c_str ())));
    return;
  } // end IF

  // retrieve message handler
  MESSAGE_HANDLERSITERATOR_T iterator_2 = messageHandlers_.find (id_in);
  if (iterator_2 == messageHandlers_.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no handler for id (was: \"%s\"), returning\n"),
                ACE_TEXT (id_in.c_str ())));
    return;
  } // end IF

  gdk_threads_enter ();

  // retrieve channel tabs
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);

  // clean up
  delete (*iterator_2).second;
  messageHandlers_.erase (iterator_2);

  // check whether this was the last handler of the last connection
  // --> disable corresponding widgets in the main UI
  if ((CBData_.connections->size () == 1) &&
      (messageHandlers_.size () == 1)) // server log
  {
    iterator =
      CBData_.GTKState->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
    // sanity check(s)
    ACE_ASSERT (iterator != CBData_.GTKState->builders.end ());
    GtkHBox* hbox_p =
      GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_HBOX_SEND)));
    ACE_ASSERT (hbox_p);
    gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), FALSE);
  } // end IF

  gdk_threads_leave ();
}

void
IRC_Client_GUI_Connection::updateModeButtons ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_Connection::updateModeButtons"));

  // sanity check(s)
  ACE_ASSERT (CBData_.GTKState);

  ACE_Guard<ACE_Thread_Mutex> aGuard (CBData_.GTKState->lock);

  Common_UI_GTKBuildersIterator_t iterator =
      CBData_.GTKState->builders.find (label_);
  // sanity check(s)
  if (iterator == CBData_.GTKState->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (label_.c_str ())));
    return;
  } // end IF

  gdk_threads_enter ();

  // display (changed) user modes
  GtkToggleButton* togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
  ACE_ASSERT (togglebutton_p);
  gtk_toggle_button_set_active (togglebutton_p,
                                CBData_.IRCSessionState.userModes[USERMODE_AWAY]);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE)));
  ACE_ASSERT (togglebutton_p);
  gtk_toggle_button_set_active (togglebutton_p,
                                CBData_.IRCSessionState.userModes[USERMODE_INVISIBLE]);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES)));
  ACE_ASSERT (togglebutton_p);
  gtk_toggle_button_set_active (togglebutton_p,
                                CBData_.IRCSessionState.userModes[USERMODE_RECVNOTICES]);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_OPERATOR)));
  ACE_ASSERT (togglebutton_p);
  gtk_toggle_button_set_active (togglebutton_p,
                                CBData_.IRCSessionState.userModes[USERMODE_OPERATOR]);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_RESTRICTED)));
  ACE_ASSERT (togglebutton_p);
  gtk_toggle_button_set_active (togglebutton_p,
                                CBData_.IRCSessionState.userModes[USERMODE_RESTRICTEDCONN]);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_LOCALOPERATOR)));
  ACE_ASSERT (togglebutton_p);
  gtk_toggle_button_set_active (togglebutton_p,
                                CBData_.IRCSessionState.userModes[USERMODE_LOCALOPERATOR]);
  togglebutton_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS)));
  ACE_ASSERT (togglebutton_p);
  gtk_toggle_button_set_active (togglebutton_p,
                                CBData_.IRCSessionState.userModes[USERMODE_RECVWALLOPS]);

  gdk_threads_leave ();
}
