﻿/***************************************************************************
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

#include <sstream>

#include <ace/ACE.h>
#include <ace/Log_Msg.h>

#include "common_file_tools.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_tools.h"

#include "net_macros.h"

#include "bittorrent_tools.h"

#include "bittorrent_client_network.h"
#include "bittorrent_client_stream_common.h"
#include "bittorrent_client_tools.h"

#include "bittorrent_client_gui_callbacks.h"
#include "bittorrent_client_gui_defines.h"
#include "bittorrent_client_gui_messagehandler.h"

template <typename ConnectionType>
BitTorrent_Client_GUI_Session_T<ConnectionType>::BitTorrent_Client_GUI_Session_T (struct BitTorrent_Client_GTK_SessionCBData* CBData_in,
                                                                                  BitTorrent_Client_GUI_Sessions_t* sessions_in,
                                                                                  guint contextID_in,
                                                                                  const std::string& label_in,
                                                                                  const std::string& UIFileDirectory_in)
 : closing_ (false)
 , CBData_ (CBData_in)
 , contextID_ (contextID_in)
 , isFirstUsersMsg_ (true)
 , sessionState_ (NULL)
 , UIFileDirectory_ (UIFileDirectory_in)
 , lock_ ()
 , messageHandlers_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::BitTorrent_Client_GUI_Session_T"));

  // sanity check(s)
  ACE_ASSERT (CBData_in);
  ACE_ASSERT (sessions_in);
  if (!Common_File_Tools::isDirectory (UIFileDirectory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: \"%s\"): not a directory, returning\n"),
                ACE_TEXT (UIFileDirectory_in.c_str ())));
    return;
  } // end IF

  // initialize cb data
  CBData_->sessions = sessions_in;
  CBData_->eventSourceID = 0;
  //   CBData_->nick.clear(); // cannot set this now...
  CBData_->label = label_in;
  ACE_TCHAR time_stamp[27]; // ISO-8601 format
  ACE_OS::memset (&time_stamp, 0, sizeof (time_stamp));
  ACE_TCHAR* result_p = ACE::timestamp (COMMON_TIME_NOW,
                                        time_stamp,
                                        sizeof (time_stamp),
                                        false);
  if (result_p == NULL)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::timestamp(): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->timeStamp = ACE_TEXT_ALWAYS_CHAR (time_stamp);

  // create new GtkBuilder
  GtkBuilder* builder_p = gtk_builder_new ();
  if (!builder_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  std::string ui_definition_filename = UIFileDirectory_;
  ui_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_filename += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_UI_CONNECTION_FILE);
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
  GError* error_p = NULL;
  gdk_threads_enter ();
  gtk_builder_add_from_file (builder_p,
                             ui_definition_filename.c_str (),
                             &error_p);
  if (error_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_add_from_file(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (ui_definition_filename.c_str ()),
                ACE_TEXT (error_p->message)));

    // clean up
    g_error_free (error_p);
    g_object_unref (G_OBJECT (builder_p));
    gdk_threads_leave ();

    return;
  } // end IF
  gdk_threads_leave ();

  // retrieve server tab channels store
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object (builder_p,
                                            ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_CHANNELS)));
  ACE_ASSERT (list_store_p);
  // make it sort the channels by #members...
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);

  // connect signal(s)
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_DISCONNECT)));
  ACE_ASSERT (button_p);
  gulong result_2 = g_signal_connect (button_p,
                                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                                      G_CALLBACK (button_disconnect_clicked_cb),
                                      &CBData_);
  ACE_ASSERT (result_2);
  GtkEntry* entry_p =
    GTK_ENTRY (gtk_builder_get_object (builder_p,
                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_ENTRY_NICK)));
  ACE_ASSERT (entry_p);
  result_2 = g_signal_connect (entry_p,
                               ACE_TEXT_ALWAYS_CHAR ("focus-in-event"),
                               G_CALLBACK (nickname_entry_kb_focused_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_NICK_ACCEPT)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                               ACE_TEXT_ALWAYS_CHAR ("clicked"),
                               G_CALLBACK (nickname_clicked_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_USERS)));
  ACE_ASSERT (combo_box_p);
  result_2 = g_signal_connect (combo_box_p,
                               ACE_TEXT_ALWAYS_CHAR ("changed"),
                               G_CALLBACK (usersbox_changed_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_USERS_REFRESH)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                               ACE_TEXT_ALWAYS_CHAR ("clicked"),
                               G_CALLBACK (refresh_users_clicked_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  entry_p =
    GTK_ENTRY (gtk_builder_get_object (builder_p,
                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_ENTRY_CONNECTION_CHANNEL)));
  ACE_ASSERT (entry_p);
  result_2 = g_signal_connect (entry_p,
                               ACE_TEXT_ALWAYS_CHAR ("focus-in-event"),
                               G_CALLBACK (channel_entry_kb_focused_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_JOIN)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                               ACE_TEXT_ALWAYS_CHAR ("clicked"),
                               G_CALLBACK (join_clicked_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CHANNELS)));
  ACE_ASSERT (combo_box_p);
  result_2 = g_signal_connect (combo_box_p,
                               ACE_TEXT_ALWAYS_CHAR ("changed"),
                               G_CALLBACK (channelbox_changed_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_CHANNELS_REFRESH)));
  ACE_ASSERT (button_p);
  result_2 = g_signal_connect (button_p,
                               ACE_TEXT_ALWAYS_CHAR ("clicked"),
                               G_CALLBACK (refresh_channels_clicked_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

  // togglebuttons
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
  ACE_ASSERT (toggle_button_p);
  //PangoAttribute* pango_attribute_p = pango_attr_scale_new (PANGO_SCALE_SMALL);
  //if (!pango_attribute_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_attr_scale_new(%f): \"%m\", aborting\n"),
  //              PANGO_SCALE_SMALL));

  //  // clean up
  //  g_object_unref (G_OBJECT (builder_p));

  //  return;
  //} // end IF
  //PangoAttrList* pango_attr_list_p = NULL;
  //// *TODO*: find a better way to do this...
  //const char* markup_p = ACE_TEXT_ALWAYS_CHAR ("<span size=\"smaller\"></span>");
  //if (!pango_parse_markup (markup_p, -1,
  //                         0,
  //                         &pango_attr_list_p,
  //                         NULL,
  //                         NULL,
  //                         &error_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_parse_markup(\"%s\"): \"%s\", aborting\n"),
  //              ACE_TEXT (markup_p),
  //              ACE_TEXT (error_p->message)));

  //  // clean up
  //  g_error_free (error_p);
  //  g_object_unref (G_OBJECT (builder_p));

  //  return;
  //} // end IF
  //GtkLabel* label_p = GTK_LABEL (gtk_bin_get_child (GTK_BIN (toggle_button_p)));
  //ACE_ASSERT (label_p);
  //PangoAttrList* pango_attr_list_2 = NULL;
  //char* text_p = NULL;
  //gunichar accel_char;
  //if (!pango_parse_markup (gtk_label_get_text (label_p), -1,
  //                         '_',
  //                         &pango_attr_list_2,
  //                         &text_p,
  //                         &accel_char,
  //                         &error_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_parse_markup(\"%s\"): \"%s\", aborting\n"),
  //              ACE_TEXT (gtk_label_get_text (label_p)),
  //              ACE_TEXT (error_p->message)));

  //  // clean up
  //  //pango_attr_list_unref (pango_attr_list_p);
  //  g_error_free (error_p);
  //  g_object_unref (G_OBJECT (builder_p));

  //  return;
  //} // end IF
  //PangoAttribute* pango_attribute_2 = pango_attribute_copy (pango_attribute_p);
  //if (!pango_attribute_2)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_attribute_copy(0x%@), aborting\n"),
  //              pango_attribute_p));

  //  // clean up
  //  pango_attr_list_unref (pango_attr_list_2);
  //  g_object_unref (G_OBJECT (builder_p));

  //  return;
  //} // end IF
  //pango_attr_list_change (pango_attr_list_2,
  //                        pango_attribute_2);
  //pango_attribute_destroy (pango_attribute_p);
  //PangoAttrIterator* iterator_p =
  //  pango_attr_list_get_iterator (pango_attr_list_p);
  //if (!iterator_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_attr_list_get_iterator(0x%@): \"%m\", aborting\n"),
  //              pango_attr_list_p));

  //  // clean up
  //  pango_attr_list_unref (pango_attr_list_p);
  //  g_error_free (error_p);
  //  g_object_unref (G_OBJECT (builder_p));

  //  return;
  //} // end IF
  //GSList* list_p = NULL;
  //do
  //{
  //  list_p = pango_attr_iterator_get_attrs (iterator_p);
  //  for (PangoAttribute* pango_attribute_p = NULL;
  //       (list_p = g_slist_next (list_p)) != NULL;
  //       pango_attribute_p = static_cast<PangoAttribute*> (list_p->data))
  //  {
  //    pango_attr_list_change (pango_attr_list_2,
  //                            pango_attribute_p);
  //    pango_attribute_destroy (pango_attribute_p);
  //  } // end FOR
  //  g_slist_free (list_p);

  //  if (!pango_attr_iterator_next (iterator_p))
  //    break; // done
  //} while (true); // end WHILE
  //pango_attr_list_unref (pango_attr_list_p);
  //pango_attr_iterator_destroy (iterator_p);
  //gtk_label_set_attributes (label_p,
  //                          pango_attr_list_2);
  //result_2 = g_signal_connect (toggle_button_p,
  //                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
  //                             G_CALLBACK (user_mode_toggled_cb),
  //                             &CBData_);
  //ACE_ASSERT (result_2);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_INVISIBLE)));
  ACE_ASSERT (toggle_button_p);
  //label_p = GTK_LABEL (gtk_bin_get_child (GTK_BIN (toggle_button_p)));
  //ACE_ASSERT (label_p);
  //if (!pango_parse_markup (gtk_label_get_text (label_p), -1,
  //                         '_',
  //                         &pango_attr_list_2,
  //                         &text_p,
  //                         &accel_char,
  //                         &error_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_parse_markup(\"%s\"): \"%s\", aborting\n"),
  //              ACE_TEXT (gtk_label_get_text (label_p)),
  //              ACE_TEXT (error_p->message)));

  //  // clean up
  //  g_error_free (error_p);
  //  g_object_unref (G_OBJECT (builder_p));

  //  return;
  //} // end IF
  //pango_attribute_2 = pango_attribute_copy (pango_attribute_p);
  //if (!pango_attribute_2)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to pango_attribute_copy(0x%@), aborting\n"),
  //              pango_attribute_p));

  //  // clean up
  //  pango_attr_list_unref (pango_attr_list_2);
  //  g_object_unref (G_OBJECT (builder_p));

  //  return;
  //} // end IF
  //pango_attr_list_change (pango_attr_list_2,
  //                        pango_attribute_2);
  result_2 = g_signal_connect (toggle_button_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (user_mode_toggled_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_OPERATOR)));
  ACE_ASSERT (toggle_button_p);
  result_2 = g_signal_connect (toggle_button_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (user_mode_toggled_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_LOCALOPERATOR)));
  ACE_ASSERT (toggle_button_p);
  result_2 = g_signal_connect (toggle_button_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (user_mode_toggled_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_RESTRICTED)));
  ACE_ASSERT (toggle_button_p);
  result_2 = g_signal_connect (toggle_button_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (user_mode_toggled_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_NOTICES)));
  ACE_ASSERT (toggle_button_p);
  result_2 = g_signal_connect (toggle_button_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (user_mode_toggled_cb),
                               &CBData_);
  ACE_ASSERT (result_2);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_WALLOPS)));
  ACE_ASSERT (toggle_button_p);
  result_2 = g_signal_connect (toggle_button_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (user_mode_toggled_cb),
                               &CBData_);
  ACE_ASSERT (result_2);

  //// actions
  //GtkAction* action_p =
  //  GTK_ACTION (gtk_builder_get_object (builder_p,
  //                                      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_ACTION_AWAY)));
  //ACE_ASSERT (action_p);
  //result_2 = g_signal_connect (action_p,
  //                           ACE_TEXT_ALWAYS_CHAR ("activate"),
  //                           G_CALLBACK (action_away_cb),
  //                           &CBData_);
  //ACE_ASSERT (result_2);

  // retrieve channel tabs
  GtkNotebook* notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object (builder_p,
                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);
  result_2 = g_signal_connect (notebook_p,
                               ACE_TEXT_ALWAYS_CHAR ("switch-page"),
                               G_CALLBACK (switch_channel_cb),
                               &CBData_);
  ACE_ASSERT (result_2);

  // retrieve server log tab child
  GtkScrolledWindow* scrolled_window_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder_p,
                                                 ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_SCROLLEDWINDOW_CONNECTION)));
  ACE_ASSERT (scrolled_window_p);
  // disallow reordering the server log tab
  gtk_notebook_set_tab_reorderable (notebook_p,
                                    GTK_WIDGET (scrolled_window_p),
                                    FALSE);

  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->builders[CBData_->timeStamp] =
        std::make_pair (ui_definition_filename, builder_p);
  } // end lock scope

  // create default BitTorrent_Client_GUI_MessageHandler (== server log)
  BitTorrent_Client_GUI_MessageHandler* message_handler_p = NULL;
  ACE_NEW_NORETURN (message_handler_p,
                    BitTorrent_Client_GUI_MessageHandler (CBData_,
                                                   this,
                                                   CBData_->timeStamp));
  if (!message_handler_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    g_object_unref (G_OBJECT (builder_p));
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->builders.erase (CBData_->timeStamp);

    return;
  } // end IF

  // *NOTE*: the empty channel name string denotes the log handler !
  // *NOTE*: no updates yet --> no need for locking
  // *NOTE*: in theory, there is a race condition as the user may start
  // interacting with the new UI elements by now - as GTK will draw the new elements
  // only after we return, this is not really a problem...
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    std::pair<MESSAGE_HANDLERSITERATOR_T, bool> result_3 =
        messageHandlers_.insert (std::make_pair (std::string (),
                                                 message_handler_p));
    if (!result_3.second)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to insert message handler: \"%m\", returning\n")));

      // clean up
      delete message_handler_p;
      g_object_unref (G_OBJECT (builder_p));

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
        CBData_->builders.erase (CBData_->timeStamp);
      } // end lock scope

      return;
    } // end IF
  } // end lock scope

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventSourceID =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_add_connection_cb,
                       &CBData_,
                       NULL);
    if (!CBData_->eventSourceID)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add_full(idle_add_connection_cb): \"%m\", returning\n")));
      return;
    } // end IF
    CBData_->eventSourceIds.insert (CBData_->eventSourceID);
  } // end lock scope
}

template <typename ConnectionType>
BitTorrent_Client_GUI_Session_T<ConnectionType>::~BitTorrent_Client_GUI_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::~BitTorrent_Client_GUI_Session_T"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

//  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->lock);

  // remove builder
  Common_UI_GTKBuildersIterator_t iterator =
    CBData_->builders.find (CBData_->timeStamp);
  // sanity check(s)
  if (iterator == CBData_->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (CBData_->label.c_str ())));
    return;
  } // end IF
  g_object_unref (G_OBJECT ((*iterator).second.second));
  CBData_->builders.erase (iterator);
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::initialize (BitTorrent_Client_SessionState* sessionState_in,
                                                             BitTorrent_IControl* controller_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::initialize"));

  ACE_ASSERT (controller_in);
  ACE_ASSERT (sessionState_in);

  CBData_->controller = controller_in;
  sessionState_ = sessionState_in;
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::finalize (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::finalize"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  if (lockedAccess_in)
  {
    result = CBData_->lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  // clean up message handlers
  const BitTorrent_Client_GTK_HandlerCBData* cb_data_p = NULL;
  Common_UI_GTKBuildersIterator_t iterator;
  GtkButton* button_p = NULL;
//  gdk_threads_enter ();
  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    for (MESSAGE_HANDLERSITERATOR_T iterator_2 = messageHandlers_.begin ();
         iterator_2 != messageHandlers_.end ();
         ++iterator_2)
    {
      ACE_ASSERT ((*iterator_2).second);
      if ((*iterator_2).second->isServerLog ())
        continue;

      cb_data_p = &(*iterator_2).second->get ();
      ACE_ASSERT (cb_data_p);

      iterator = CBData_->builders.find (cb_data_p->builderLabel);
      // sanity check(s)
      if (iterator == CBData_->builders.end ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("handler (was: \"%s\") builder not found, continuing\n"),
                    ACE_TEXT (CBData_->label.c_str ())));
        continue;
      } // end IF
      button_p =
          GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_PART)));
      ACE_ASSERT (button_p);
      gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
    } // end FOR
  } // end lock scope
//  gdk_threads_leave ();

  if (lockedAccess_in)
  {
    result = CBData_->lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::start (Stream_SessionId_t sessionID_in,
                                                        const BitTorrent_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::start"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionData_in);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  Common_UI_GTKBuildersIterator_t iterator =
    CBData_->builders.find (CBData_->timeStamp);
  // sanity check(s)
  if (iterator == CBData_->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (CBData_->label.c_str ())));
    return;
  } // end IF

  // enable close button
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_DISCONNECT)));
  ACE_ASSERT (button_p);
  gdk_threads_enter ();
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  gdk_threads_leave ();

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("connected...\n")));
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::notify (Stream_SessionId_t sessionID_in,
                                                         const Stream_SessionMessageType& sessionEvent_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::end (Stream_SessionId_t sessionID_in)
{
  NETWORK_TRACE(ACE_TEXT("BitTorrent_Client_GUI_Session_T::end"));

  ACE_UNUSED_ARG (sessionID_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connection \"%s\" closed/lost\n"),
              ACE_TEXT (CBData_->label.c_str ())));

//  // *NOTE*: this is the final invocation from the controller
//  //         --> unsubscribe anyway
//  if (CBData_->controller)
//  {
//    try {
//      CBData_->controller->unsubscribe (this);
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in IRC_IControl::unsubscribe(%@), continuing\n"),
//                  this));
//    }
//    CBData_->controller = NULL;
//  } // end IF

  close ();
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::notify (Stream_SessionId_t sessionID_in,
                                                         const BitTorrent_Client_PeerMessage& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  Common_UI_GTKBuildersIterator_t iterator =
      CBData_->builders.find (CBData_->timeStamp);
  // sanity check(s)
  if (iterator == CBData_->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (CBData_->label.c_str ())));
    return;
  } // end IF

  const IRC_Record& record_r = message_in.get ();
  switch (record_r.command_.discriminator)
  {
    case IRC_Record::Command::NUMERIC:
    {
      switch (record_r.command_.numeric)
      {
        case IRC_Codes::RPL_WELCOME:          //   1
        {
          // *NOTE*: this is the first message in any connection !

          // remember nickname
          ACE_ASSERT (sessionState_);
          sessionState_->nickName = record_r.parameters_.front ();

          gdk_threads_enter ();

          // --> display (starting) nickname
          // set server tab nickname label
          GtkLabel* label_p =
            GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_LABEL_NICK)));
          ACE_ASSERT (label_p);
          // --> see Pango Text Attribute Markup Language...
          std::string nickname_string = ACE_TEXT_ALWAYS_CHAR ("<b><i>nickname</i></b> ");
          nickname_string += record_r.parameters_.front ();
          gtk_label_set_markup (label_p,
                                nickname_string.c_str ());

          // retrieve button handle
          GtkHBox* hbox_p =
            GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_HBOX_NICK_CHANNEL)));
          ACE_ASSERT (hbox_p);
          gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), TRUE);
          hbox_p =
            GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_HBOX_USERMODE)));
          ACE_ASSERT (hbox_p);
          gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), TRUE);

          gdk_threads_leave ();

          // *WARNING*: falls through !
        }
        case IRC_Codes::RPL_YOURHOST:         //   2
        case IRC_Codes::RPL_CREATED:          //   3
        case IRC_Codes::RPL_MYINFO:           //   4
        case IRC_Codes::RPL_PROTOCTL:         //   5
        case IRC_Codes::RPL_SNOMASK:          //   8
        case IRC_Codes::RPL_YOURID:           //  20
        case IRC_Codes::RPL_STATSDLINE:       // 250
        case IRC_Codes::RPL_LUSERCLIENT:      // 251
        case IRC_Codes::RPL_LUSEROP:          // 252
        case IRC_Codes::RPL_LUSERUNKNOWN:     // 253
        case IRC_Codes::RPL_LUSERCHANNELS:    // 254
        case IRC_Codes::RPL_LUSERME:          // 255
        case IRC_Codes::RPL_TRYAGAIN:         // 263
        case IRC_Codes::RPL_LOCALUSERS:       // 265
        case IRC_Codes::RPL_GLOBALUSERS:      // 266
        case IRC_Codes::RPL_INVITING:         // 341
        {
          log (record_r);
          break;
        }
        case IRC_Codes::RPL_USERHOST:         // 302
        {
          // bisect (WS-separated) userhost records from the final parameter

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting records: \"%s\"...\n"),
          //            ACE_TEXT (record_r.parameters_.back ().c_str ())));

          std::string::size_type current_position = 0;
          std::string::size_type last_position = 0;
          std::string record;
          string_list_t list;
          do
          {
            current_position =
                record_r.parameters_.back ().find (' ', last_position);

            record =
              record_r.parameters_.back ().substr (last_position,
                                                   (((current_position == std::string::npos) ? record_r.parameters_.back ().size ()
                                                                                             : current_position) -
                                                    last_position));

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
        case IRC_Codes::RPL_UNAWAY:           // 305
        {
          gdk_threads_enter ();

          // *WARNING*: needs the lock protection, otherwise there is a race...
          ACE_ASSERT (sessionState_);
          sessionState_->away = false;

          // retrieve togglebutton
          GtkToggleButton* toggle_button_p =
            GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
          ACE_ASSERT (toggle_button_p);
          gtk_toggle_button_set_active (toggle_button_p, FALSE);

          gdk_threads_leave ();

          log (record_r);

          break;
        }
        case IRC_Codes::RPL_NOWAWAY:          // 306
        {
          gdk_threads_enter ();

          // *WARNING*: needs the lock protection, otherwise there is a race...
          ACE_ASSERT (sessionState_);
          sessionState_->away = true;

          // retrieve togglebutton
          GtkToggleButton* toggle_button_p =
            GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                       ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TOGGLEBUTTON_USERMODE_AWAY)));
          ACE_ASSERT (toggle_button_p);
          gtk_toggle_button_set_active (toggle_button_p, TRUE);

          gdk_threads_leave ();

          log (record_r);

          break;
        }
        case IRC_Codes::RPL_ENDOFWHO:         // 315
        {
          //log (message_in);

          //gdk_threads_enter ();

          //// retrieve server tab users store
          //GtkComboBox* combobox_p =
          //  GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
          //                                         ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_USERS)));
          //ACE_ASSERT (combobox_p);
          //gtk_widget_set_sensitive (GTK_WIDGET (combobox_p), TRUE);

          //gdk_threads_leave ();

          isFirstUsersMsg_ = true;

          break;
        }
        case IRC_Codes::RPL_LISTSTART:        // 321
        {
          gdk_threads_enter ();

          // retrieve server tab channels store
          GtkListStore* liststore_p =
            GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_CHANNELS)));
          ACE_ASSERT (liststore_p);

          // clear the store
          gtk_list_store_clear (liststore_p);

          gdk_threads_leave ();

          // *WARNING*: falls through !
        }
        case IRC_Codes::RPL_LISTEND:          // 323
        {
          log (record_r);

          //gdk_threads_enter();

          //// retrieve server tab channels store
          //GtkComboBox* combobox_p =
          //  GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
          //                                         ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CHANNELS)));
          //ACE_ASSERT (combobox_p);
          //gtk_widget_set_sensitive (GTK_WIDGET (combobox_p), TRUE);

          //gdk_threads_leave ();

          break;
        }
        case IRC_Codes::RPL_LIST:             // 322
        {
          gdk_threads_enter ();

          // retrieve server tab channels store
          GtkListStore* list_store_p =
            GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_CHANNELS)));
          ACE_ASSERT (list_store_p);

          // convert <# visible>
          IRC_ParametersIterator_t param_iterator =
              record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 3);
          std::advance (param_iterator, 2);
          std::stringstream converter;
          guint num_members = 0;
          converter << *param_iterator;
          converter >> num_members;
          param_iterator--;

          // convert text
          gchar* string_p =
            Common_UI_Tools::Locale2UTF8 (*param_iterator);
          if (!string_p)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
                        ACE_TEXT ((*param_iterator).c_str ())));

            // clean up
            gdk_threads_leave ();

            break;
          } // end IF
          gchar* string_2 = NULL;
          if (record_r.parameters_.size () > 3)
          {
            param_iterator++;
            string_2 = Common_UI_Tools::Locale2UTF8 (*param_iterator);
            if (!string_2)
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
                          ACE_TEXT ((*param_iterator).c_str ())));

              // clean up
              g_free (string_p);
              gdk_threads_leave ();

              break;
            } // end IF
          } // end IF

          GtkTreeIter tree_iter;
          gtk_list_store_append (list_store_p,
                                 &tree_iter);
          gtk_list_store_set (list_store_p, &tree_iter,
                              0, string_p,    // channel name
                              1, num_members, // # visible members
                              2, string_2,    // topic
                              -1);

          // clean up
          g_free (string_p);
          g_free (string_2);
          gdk_threads_leave ();

          break;
        }
        case IRC_Codes::RPL_NOTOPIC:          // 331
        case IRC_Codes::RPL_TOPIC:            // 332
        case IRC_Codes::RPL_TOPICWHOTIME:     // 333
        {
          IRC_ParametersIterator_t iterator_2 =
            record_r.parameters_.begin ();
          iterator_2++;

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (*iterator_2);
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT ((*iterator_2).c_str ())));
            break;
          } // end IF

          if ((record_r.command_.numeric == IRC_Codes::RPL_NOTOPIC) ||
              (record_r.command_.numeric == IRC_Codes::RPL_TOPIC))
            (*handler_iterator).second->setTopic (record_r.parameters_.back ());

          break;
        }
        case IRC_Codes::RPL_WHOREPLY:         // 352
        {
          // bisect user information from parameter strings
          IRC_ParametersIterator_t iterator_2 =
            record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 8);
          std::advance (iterator_2, 5); // nick position
          std::string nickname = *iterator_2;
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
          ws_position = record_r.parameters_.back ().find (' ', 0);
          converter << record_r.parameters_.back ().substr (0, ws_position);
          converter >> hop_count;
          real_name = record_r.parameters_.back ().substr (ws_position + 1);

          gdk_threads_enter ();

          // retrieve server tab users store
          Common_UI_GTKBuildersIterator_t iterator_3 =
            CBData_->builders.find (CBData_->timeStamp);
          // sanity check(s)
          if (iterator_3 == CBData_->builders.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                        ACE_TEXT (CBData_->label.c_str ())));

            // clean up
            gdk_threads_leave ();

            break;
          } // end IF

          GtkListStore* list_store_p =
            GTK_LIST_STORE (gtk_builder_get_object ((*iterator_3).second.second,
                                                    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_LISTSTORE_USERS)));
          ACE_ASSERT (list_store_p);

          if (isFirstUsersMsg_)
          {
            gtk_list_store_clear (list_store_p);

            isFirstUsersMsg_ = false;
          } // end IF

          // ignore own record
          ACE_ASSERT (sessionState_);
          if (nickname == sessionState_->nickName)
          {
            // clean up
            gdk_threads_leave ();

            break;
          } // end IF

          // step1: convert text
          const gchar* string_p = nickname.c_str ();
          //gchar* string_p = Common_UI_Tools::Locale2UTF8 (nick);
          //if (!string_p)
          //{
          //  ACE_DEBUG ((LM_ERROR,
          //              ACE_TEXT ("failed to convert nickname: \"%s\", aborting\n"),
          //              ACE_TEXT (nick.c_str ())));

          //  // clean up
          //  gdk_threads_leave ();

          //  break;
          //} // end IF
          const gchar* string_2 = real_name.c_str ();
          //gchar* string_2 = Common_UI_Tools::Locale2UTF8 (real_name);
          //if (!string_2)
          //{
          //  ACE_DEBUG ((LM_ERROR,
          //              ACE_TEXT ("failed to convert name: \"%s\", aborting\n"),
          //              ACE_TEXT (real_name.c_str ())));

          //  // clean up
          //  g_free (string_p);
          //  gdk_threads_leave ();

          //  break;
          //} // end IF

          // step2: append new (text) entry
          GtkTreeIter tree_iter;
          gtk_list_store_append (list_store_p, &tree_iter);
          gtk_list_store_set (list_store_p, &tree_iter,
                              0, string_p, // column 0
                              1, away,
                              2, is_IRCoperator,
                              3, is_operator,
                              4, is_voiced,
                              5, hop_count,
                              6, string_2,
                              -1);

          // clean up
          //g_free (string_p);
          //g_free (string_2);
          gdk_threads_leave ();

          break;
        }
        case IRC_Codes::RPL_NAMREPLY:         // 353
        {
          // bisect (WS-separated) nicknames from the final parameter string

          // *NOTE*: UnrealIRCd 3.2.10.4 has trailing whitespace...
          std::string& back =
              const_cast<IRC_Record&> (record_r).parameters_.back ();

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("bisecting nicknames: \"%s\"...\n"),
          //            ACE_TEXT (back.c_str ())));

          // step1: trim trailing WS
          size_t position = back.find_last_not_of (' ', std::string::npos);
          if (position != std::string::npos)
            back.erase (++position, std::string::npos);

          std::istringstream converter (back);
          std::string nickname;
          string_list_t list;
          bool is_operator = false;
          while (!converter.eof ())
          {
            converter >> nickname;

            // check whether user is a channel operator
            ACE_ASSERT (sessionState_);
            if (nickname.find (sessionState_->nickName) != std::string::npos)
              is_operator = ((nickname[0] == '@') &&
                             (nickname.size () == (sessionState_->nickName.size () + 1)));

            list.push_back (nickname);
          } // end WHILE

          // retrieve channel name
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
          ACE_ASSERT (record_r.parameters_.size () >= 3);
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

          (*handler_iterator).second->members (list, false);

          // user is operator ? --> set channel mode accordingly
          if (is_operator)
          {
            // *NOTE*: ops always have a voice...
            std::string op_mode = ACE_TEXT_ALWAYS_CHAR ("+ov");
            (*handler_iterator).second->setModes (op_mode,
                                                  std::string (), // none
                                                  false);
          } // end IF

          break;
        }
        case IRC_Codes::RPL_ENDOFNAMES:       // 366
        {
          // retrieve channel name
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
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

          (*handler_iterator).second->endMembers (false);

          break;
        }
        case IRC_Codes::RPL_ENDOFBANLIST:     // 368
        case IRC_Codes::RPL_MOTD:             // 372
        case IRC_Codes::RPL_MOTDSTART:        // 375
        case IRC_Codes::RPL_ENDOFMOTD:        // 376
        case IRC_Codes::RPL_HOSTHIDDEN:       // 396
        case IRC_Codes::ERR_NOSUCHNICK:       // 401
        case IRC_Codes::ERR_UNKNOWNCOMMAND:   // 421
        case IRC_Codes::ERR_NOMOTD:           // 422
        case IRC_Codes::ERR_ERRONEUSNICKNAME: // 432
        case IRC_Codes::ERR_NICKNAMEINUSE:    // 433
        case IRC_Codes::ERR_NOTREGISTERED:    // 451
        case IRC_Codes::ERR_NEEDMOREPARAMS:   // 461
        case IRC_Codes::ERR_ALREADYREGISTRED: // 462
        case IRC_Codes::ERR_YOUREBANNEDCREEP: // 465
        case IRC_Codes::ERR_BADCHANNAME:      // 479
        case IRC_Codes::ERR_CHANOPRIVSNEEDED: // 482
        case IRC_Codes::ERR_UMODEUNKNOWNFLAG: // 501
        {
          log (record_r);

          if ((record_r.command_.numeric == IRC_Codes::ERR_NOSUCHNICK)       ||
              (record_r.command_.numeric == IRC_Codes::ERR_UNKNOWNCOMMAND)   ||
              (record_r.command_.numeric == IRC_Codes::ERR_ERRONEUSNICKNAME) ||
              (record_r.command_.numeric == IRC_Codes::ERR_NICKNAMEINUSE)    ||
              (record_r.command_.numeric == IRC_Codes::ERR_NOTREGISTERED)    ||
              (record_r.command_.numeric == IRC_Codes::ERR_ALREADYREGISTRED) ||
              (record_r.command_.numeric == IRC_Codes::ERR_YOUREBANNEDCREEP) ||
              (record_r.command_.numeric == IRC_Codes::ERR_BADCHANNAME)      ||
              (record_r.command_.numeric == IRC_Codes::ERR_CHANOPRIVSNEEDED) ||
              (record_r.command_.numeric == IRC_Codes::ERR_UMODEUNKNOWNFLAG))
            error (record_r, // show in statusbar as well...
                   false);

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("invalid/unknown (numeric) command/reply (was: \"%s\" (%u)), continuing\n"),
                      ACE_TEXT (IRC_Tools::Command2String (record_r.command_.numeric).c_str ()),
                      record_r.command_.numeric));

          message_in.dump_state ();

          break;
        }
      } // end SWITCH

      break;
    }
    case IRC_Record::Command::STRING:
    {
      IRC_Record::CommandType command =
        IRC_Tools::Command2Type (*record_r.command_.string);
      switch (command)
      {
        case IRC_Record::NICK:
        {
          // remember changed nickname...
          ACE_ASSERT (sessionState_);
          std::string old_nickname = sessionState_->nickName;
          sessionState_->nickName = record_r.parameters_.front ();

          // --> display (changed) nickname
          // step1: set server tab nickname label
          gdk_threads_enter ();

          GtkLabel* label_p =
            GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_LABEL_NICK)));
          ACE_ASSERT (label_p);
          // --> see Pango Text Attribute Markup Language...
          std::string nickname_string = ACE_TEXT_ALWAYS_CHAR ("<b><i>nickname</i></b> ");
          nickname_string += sessionState_->nickName;
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

            (*iterator).second->update (old_nickname);
          } // end FOR

          // *WARNING*: falls through !
        }
        case IRC_Record::USER:
        case IRC_Record::QUIT:
        {
          log (record_r);

          ACE_ASSERT (sessionState_);
          if ((record_r.prefix_.origin == sessionState_->nickName) &&
              (command == IRC_Record::QUIT))
            error (record_r, // --> show on statusbar as well...
                   false);

          break;
        }
        case IRC_Record::JOIN:
        {
          log (record_r);

          // there are two possibilities:
          // - reply from a successful join request
          // - stranger entering the channel

          // reply from a successful join request ?
          ACE_ASSERT (sessionState_);
          if (record_r.prefix_.origin == sessionState_->nickName)
          {
            createMessageHandler (record_r.parameters_.front (),
                                  false);

//            // query channel members
//            string_list_t channels;
//            channels.push_back (record_r.parameters_.front ());
//            ACE_ASSERT (CBData_->controller);
//            try
//            {
//              CBData_->controller->names (channels);
//            }
//            catch (...)
//            {
//              ACE_DEBUG ((LM_ERROR,
//                          ACE_TEXT ("caught exception in BitTorrent_Client_IIRCControl::names(), continuing\n")));
//            }

            break;
          } // end IF

          // someone joined a common channel...

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.back ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.back ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->add (record_r.prefix_.origin,
                                           false);

          break;
        }
        case IRC_Record::PART:
        {
          log (record_r);

          // there are two possibilities:
          // - reply from a (successful) part request
          // - someone left a common channel

          // reply from a successful part request ?
          ACE_ASSERT (sessionState_);
          if (record_r.prefix_.origin == sessionState_->nickName)
          {
            terminateMessageHandler (record_r.parameters_.back (),
                                     false);
            break;
          } // end IF

          // someone left a common channel...

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.back ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.back ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->remove (record_r.prefix_.origin,
                                              false);

          break;
        }
        case IRC_Record::MODE:
        {
          log (record_r);

          // there are two possibilities:
          // - user mode message
          // - channel mode message

          // retrieve mode string
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
          param_iterator++;

          ACE_ASSERT (sessionState_);
          if (record_r.parameters_.front () == sessionState_->nickName)
          {
            // --> user mode
            // *WARNING*: needs the lock protection, otherwise there is a race...
            CBData_->acknowledgements +=
              IRC_Tools::merge (record_r.parameters_.back (),
                                       sessionState_->userModes);

            guint event_source_id =
                g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                                 idle_update_user_modes_cb,
                                 &CBData_,
                                 NULL);
            if (!event_source_id)
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to g_idle_add_full(idle_update_user_modes_cb): \"%m\", returning\n")));
              break;
            } // end IF
            // *TODO*: this id is never removed from the list
            CBData_->eventSourceIds.insert (event_source_id);
          } // end IF
          else
          {
            // --> channel mode

            // retrieve message handler
            MESSAGE_HANDLERSITERATOR_T handler_iterator =
              messageHandlers_.find (record_r.parameters_.front ());
            if (handler_iterator == messageHandlers_.end ())
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                          ACE_TEXT (record_r.parameters_.front ().c_str ())));
              break;
            } // end IF

            // *WARNING*: needs the lock protection, otherwise there is a race...
            (*handler_iterator).second->setModes (*param_iterator,
                                                  ((*param_iterator == record_r.parameters_.back ()) ? std::string ()
                                                                                                  : record_r.parameters_.back ()),
                                                  false);
          } // end ELSE

          break;
        }
        case IRC_Record::TOPIC:
        {
          log (record_r);

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.front ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.front ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->setTopic (record_r.parameters_.back ());

          break;
        }
        case IRC_Record::KICK:
        {
          log (record_r);

          // retrieve nickname string
          IRC_ParametersIterator_t param_iterator =
            record_r.parameters_.begin ();
          param_iterator++;

          // retrieve message handler
          MESSAGE_HANDLERSITERATOR_T handler_iterator =
            messageHandlers_.find (record_r.parameters_.front ());
          if (handler_iterator == messageHandlers_.end ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("no handler for channel (was: \"%s\"), returning\n"),
                        ACE_TEXT (record_r.parameters_.front ().c_str ())));
            break;
          } // end IF

          (*handler_iterator).second->remove (*param_iterator,
                                              false);

          break;
        }
        case IRC_Record::PRIVMSG:
        {
          // *TODO*: parse (list of) receiver(s)

          std::string message_text;
          if (!record_r.prefix_.origin.empty ())
          {
            message_text += ACE_TEXT_ALWAYS_CHAR ("<");
            message_text += record_r.prefix_.origin;
            message_text += ACE_TEXT_ALWAYS_CHAR ("> ");
          } // end IF
          message_text += record_r.parameters_.back ();

          // private message ?
          std::string target_id;
          ACE_ASSERT (sessionState_);
          if (sessionState_->nickName == record_r.parameters_.front ())
          {
            // --> send to private conversation handler

            // part of an existing conversation ?

            // retrieve message handler
            if (messageHandlers_.find (record_r.prefix_.origin) == messageHandlers_.end ())
              createMessageHandler (record_r.prefix_.origin,
                                    false);
          } // end IF

          // channel/nick message ?
          forward (((sessionState_->nickName == record_r.parameters_.front ()) ? record_r.prefix_.origin
                                                                            : record_r.parameters_.front ()),
                   message_text);

          break;
        }
        case IRC_Record::NOTICE:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case IRC_Record::__QUIRK__ERROR:
#else
        case IRC_Record::ERROR:
#endif
        case IRC_Record::AWAY:
        {
          log (record_r);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
          if (command == IRC_Record::__QUIRK__ERROR)
#else
          if (command == IRC_Record::ERROR)
#endif
            error (record_r, // --> show on statusbar as well...
                   false);

          break;
        }
        case IRC_Record::PING:
          break;
        default:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("unknown/invalid command (was: \"%s\"), continuing\n"),
                      ACE_TEXT (record_r.command_.string->c_str ())));

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
                  record_r.command_.discriminator));
      break;
    }
  } // end SWITCH
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::notify (Stream_SessionId_t sessionID_in,
                                                         const BitTorrent_Client_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE(ACE_TEXT("BitTorrent_Client_GUI_Session_T::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionMessage_in);
}

template <typename ConnectionType>
const struct BitTorrent_Client_GTK_SessionCBData&
BitTorrent_Client_GUI_Session_T<ConnectionType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::get"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  return *CBData_;
}

template <typename ConnectionType>
BitTorrent_Client_GUI_MessageHandler*
BitTorrent_Client_GUI_Session_T<ConnectionType>::getHandler (const std::string& id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::getHandler"));

  // existing conversation ? --> retrieve message handler

  MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.find (id_in);
  if (iterator != messageHandlers_.end ())
    return (*iterator).second;

  return NULL;
}

//void
//BitTorrent_Client_GUI_Session_T::current (std::string& nickname_out,
//                                    std::string& channel_out) const
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::current"));
//
//  // sanity check(s)
//  ACE_ASSERT (sessionState_);
//
//  nickname_out = sessionState_->nickName;
//  // *TODO*: keep this information synchronized
//  channel_out = sessionState_->channel;
//}
template <typename ConnectionType>
const BitTorrent_Client_SessionState&
BitTorrent_Client_GUI_Session_T<ConnectionType>::state () const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::state"));

  // sanity check(s)
  ACE_ASSERT (sessionState_);

  return *sessionState_;
}

template <typename ConnectionType>
BitTorrent_Client_GUI_MessageHandler*
BitTorrent_Client_GUI_Session_T<ConnectionType>::getActiveHandler (bool lockedAccess_in,
                                                                   bool gdkLockedAccess_in) const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::getActiveHandler"));

  BitTorrent_Client_GUI_MessageHandler* return_value = NULL;

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  if (lockedAccess_in)
  {
    result = CBData_->lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  GtkNotebook* notebook_p = NULL;
  gint page_number = -1;
  GtkWidget* widget_p = NULL;
  bool server_log_active = false;
  GtkWidget* server_log_p = NULL;
  GtkWidget* widget_2 = NULL;

  Common_UI_GTKBuildersIterator_t iterator =
    CBData_->builders.find (CBData_->timeStamp);
  // sanity check(s)
  if (iterator == CBData_->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                ACE_TEXT (CBData_->label.c_str ())));
    goto clean_up;
  } // end IF

  if (gdkLockedAccess_in)
    gdk_threads_enter ();
  // retrieve server tab channel tabs handle
  notebook_p =
    GTK_NOTEBOOK (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
  ACE_ASSERT (notebook_p);
  page_number = gtk_notebook_get_current_page (notebook_p);
  if (page_number == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_notebook_get_current_page(%@): no pages, aborting\n"),
                notebook_p));

    // clean up
    if (gdkLockedAccess_in)
      gdk_threads_leave ();

    goto clean_up;
  } // end IF
  widget_p = gtk_notebook_get_nth_page (notebook_p,
                                        page_number);
  ACE_ASSERT (widget_p);

  server_log_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_SCROLLEDWINDOW_CONNECTION)));
  ACE_ASSERT (server_log_p);
  server_log_active = (widget_p == server_log_p);

  { // synch access
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    for (MESSAGE_HANDLERSCONSTITERATOR_T iterator = messageHandlers_.begin ();
         iterator != messageHandlers_.end ();
         iterator++)
    {
      ACE_ASSERT ((*iterator).second);

      // server log ?
      if (server_log_active &&
          (*iterator).second->isServerLog ())
      {
        return_value = (*iterator).second;
        break;
      } // end IF

      widget_2 = (*iterator).second->getTopLevelPageChild (false);
      if (widget_2 == widget_p)
      {
        return_value = (*iterator).second;
        break;
      } // end IF
    } // end FOR
  } // end lock scope
  if (gdkLockedAccess_in)
    gdk_threads_leave ();

clean_up:
  if (lockedAccess_in)
  {
    result = CBData_->lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF

  return return_value;
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::close"));

  BitTorrent_Client_GTK_HandlerCBData* cb_data_p = NULL;

  // clean up message handlers
  { // synch access
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
         iterator != messageHandlers_.end ();
         iterator++)
    {
      cb_data_p =
          &const_cast<BitTorrent_Client_GTK_HandlerCBData&> ((*iterator).second->get ());
      ACE_ASSERT (cb_data_p);
      cb_data_p->eventSourceID =
        g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                         idle_remove_channel_cb,
                         cb_data_p,
                         NULL);
      if (cb_data_p->eventSourceID)
        CBData_->eventSourceIds.insert (cb_data_p->eventSourceID);
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add_full(idle_remove_channel_cb): \"%m\", continuing\n")));
    } // end FOR
    messageHandlers_.clear ();
    closing_ = true;
  } // end lock scope
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::forward (const std::string& channel_in,
                                                          const std::string& messageText_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::forward"));

  // --> pass to channel log

  // retrieve message handler
  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    if (closing_)
      return; // done

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
  } // end lock scope
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::log (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::log"));

  // --> pass to server log

  // retrieve message handler
  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    if (closing_)
      return; // done

    MESSAGE_HANDLERSITERATOR_T handler_iterator =
      messageHandlers_.find (std::string ());
    ACE_ASSERT (handler_iterator != messageHandlers_.end ());
    (*handler_iterator).second->queueForDisplay (message_in);
  } // end lock scope
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::log (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::log"));

  // --> pass to server log

  // retrieve message handler
  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

    if (closing_)
      return; // done

    MESSAGE_HANDLERSITERATOR_T handler_iterator =
      messageHandlers_.find (std::string ());
    ACE_ASSERT (handler_iterator != messageHandlers_.end ());
    (*handler_iterator).second->queueForDisplay (IRC_Tools::Record2String (message_in));
  } // end lock scope
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::error (const struct BitTorrent_Record& record_in,
                                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::error"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  if (lockedAccess_in)
  {
    result = CBData_->lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  Common_UI_GTKBuildersIterator_t iterator =
    CBData_->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
  // sanity check(s)
  if (iterator == CBData_->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") main builder not found, returning\n"),
                ACE_TEXT (CBData_->label.c_str ())));
    return;
  } // end IF

  gdk_threads_enter ();
  // error --> print on statusbar
  GtkStatusbar* statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR)));
  ACE_ASSERT (statusbar_p);
  gtk_statusbar_push (statusbar_p,
                      contextID_,
                      BitTorrent_Tools::dump (message_in).c_str ());
  gdk_threads_leave ();

  if (lockedAccess_in)
  {
    result = CBData_->lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

template <typename ConnectionType>
gint
BitTorrent_Client_GUI_Session_T<ConnectionType>::exists (const std::string& id_in,
                                                         bool gdkLockedAccess_in) const
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::exists"));

  gint result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, CBData_->lock, result);

  { // synch access
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, result);

    // sanity check
    if (messageHandlers_.empty ())
      return result;

    MESSAGE_HANDLERSCONSTITERATOR_T iterator = messageHandlers_.find (id_in);
    if (iterator == messageHandlers_.end ())
      return result;

    Common_UI_GTKBuildersIterator_t iterator_2 =
        CBData_->builders.find (CBData_->timeStamp);
    // sanity check(s)
    if (iterator_2 == CBData_->builders.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection (was: \"%s\") builder not found, aborting\n"),
                  ACE_TEXT (CBData_->label.c_str ())));
      return result;
    } // end IF

    if (gdkLockedAccess_in)
      gdk_threads_enter ();
    // retrieve server tab channel tabs handle
    GtkNotebook* notebook_p =
        GTK_NOTEBOOK (gtk_builder_get_object ((*iterator_2).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_CHANNELS)));
    ACE_ASSERT (notebook_p);

    GtkWidget* widget_p = NULL;
    gint num_pages = gtk_notebook_get_n_pages (notebook_p);
    for (gint page_number = 0;
         page_number < num_pages;
         page_number++)
    {
      // *NOTE*: channel ids are unique, but notebook page numbers may change as
      //         channels/private conversations are joined/parted and/or tabs
      //         are rearranged
      widget_p = gtk_notebook_get_nth_page (notebook_p,
                                            page_number);
      ACE_ASSERT (widget_p);

      if ((*iterator).second->getTopLevelPageChild (false) == widget_p)
      {
        result = page_number;
        break;
      } // end IF
    } // end FOR
    if (gdkLockedAccess_in)
      gdk_threads_leave ();
  } // end lock scope

  return result;
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::channels (string_list_t& channels_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::channels"));

  // initialize return value
  channels_out.clear ();

  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    for (MESSAGE_HANDLERSITERATOR_T iterator = messageHandlers_.begin ();
         iterator != messageHandlers_.end ();
         iterator++)
      if (IRC_Tools::isValidChannelName ((*iterator).first))
        channels_out.push_back ((*iterator).first);
  } // end lock scope
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::createMessageHandler (const std::string& id_in,
                                                                       bool lockedAccess_in,
                                                                       bool gdkLockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::createMessageHandler"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->connections);

  int result = -1;
  BitTorrent_Client_GUI_MessageHandler* message_handler_p = NULL;

  if (lockedAccess_in)
  {
    result = CBData_->lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  Common_UI_GTKBuildersIterator_t iterator =
    CBData_->builders.find (CBData_->timeStamp);
  // sanity check(s)
  if (iterator == CBData_->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (CBData_->label.c_str ())));
    goto clean_up;
  } // end IF

  // create new BitTorrent_Client_GUI_MessageHandler
//  gdk_threads_enter ();
  ACE_NEW_NORETURN (message_handler_p,
                    BitTorrent_Client_GUI_MessageHandler (CBData_,
                                                          this,
                                                          CBData_->controller,
                                                          id_in,
                                                          UIFileDirectory_,
                                                          CBData_->timeStamp,
                                                          gdkLockedAccess_in));
  if (!message_handler_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));

    // clean up
//    gdk_threads_leave ();

    goto clean_up;
  } // end IF
  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    messageHandlers_.insert (std::make_pair (id_in, message_handler_p));

    // check whether this is the first channel of the first connection
    // --> enable corresponding widget(s) in the main UI
    if ((CBData_->connections->size () == 1) &&
        (messageHandlers_.size () == 2)) // server log + first channel
    {
      iterator =
        CBData_->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
      // sanity check(s)
      ACE_ASSERT (iterator != CBData_->builders.end ());
      gdk_threads_enter ();
      GtkHBox* hbox_p =
        GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_HBOX_SEND)));
      ACE_ASSERT (hbox_p);
      gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), TRUE);
      gdk_threads_leave ();
    } // end IF
  } // end lock scope

clean_up:
  if (lockedAccess_in)
  {
    result = CBData_->lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

template <typename ConnectionType>
void
BitTorrent_Client_GUI_Session_T<ConnectionType>::terminateMessageHandler (const std::string& id_in,
                                                                          bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::terminateMessageHandler"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  if (lockedAccess_in)
  {
    result = CBData_->lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  // retrieve message handler
  BitTorrent_Client_GTK_HandlerCBData* cb_data_p = NULL;
  { // synch access
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    MESSAGE_HANDLERSITERATOR_T iterator_2 = messageHandlers_.find (id_in);
    if (iterator_2 == messageHandlers_.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no handler for id (was: \"%s\"), returning\n"),
                  ACE_TEXT (id_in.c_str ())));
      goto clean_up;
    } // end IF
    ACE_ASSERT ((*iterator_2).second);
    cb_data_p =
        &const_cast<BitTorrent_Client_GTK_HandlerCBData&> ((*iterator_2).second->get ());
    messageHandlers_.erase (iterator_2);
  } // end lock scope
  ACE_ASSERT (cb_data_p);

  cb_data_p->eventSourceID =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_remove_channel_cb,
                       cb_data_p,
                       NULL);
  if (cb_data_p->eventSourceID)
    CBData_->eventSourceIds.insert (cb_data_p->eventSourceID);
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add_full(idle_remove_channel_cb): \"%m\", continuing\n")));

  // check whether this was the last handler of the last connection
  // --> disable corresponding widgets in the main UI
  if ((CBData_->connections->size () == 1) &&
      (messageHandlers_.size () == 1)) // server log
  {
    Common_UI_GTKBuildersIterator_t iterator =
      CBData_->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
    // sanity check(s)
    ACE_ASSERT (iterator != CBData_->builders.end ());
    gdk_threads_enter ();
    GtkHBox* hbox_p =
      GTK_HBOX (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_HBOX_SEND)));
    ACE_ASSERT (hbox_p);
    gtk_widget_set_sensitive (GTK_WIDGET (hbox_p), FALSE);
    gdk_threads_leave ();
  } // end IF

clean_up:
  if (lockedAccess_in)
  {
    result = CBData_->lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}