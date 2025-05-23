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

#include "IRC_client_gui_messagehandler.h"

#include "common_file_tools.h"

#include "net_macros.h"

//////////////////////////////////////////

#if defined (GTK_SUPPORT)
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection* connection_in,
                                                                                           const std::string& timeStamp_in,
                                                                                           struct IRC_Client_UI_HandlerCBData* CBData_in)
 : CBData_ (CBData_in)
 , isFirstMemberListMsg_ (true)
 , isPrivateDialog_ (false)
 , messageQueue_ ()
 , messageQueueLock_ ()
 , view_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::IRC_Client_GUI_MessageHandler_T"));

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (CBData_in);

  // initialize cb data
  //CBData_->acknowledgements = 0;
  //CBData_->builderLabel ();
  //CBData_->channelModes = 0;
  CBData_->connection = connection_in;
  CBData_->controller = NULL;
  //CBData_->eventSourceId = 0;
  CBData_->handler = this;
  CBData_->id.clear ();
  //  CBData_->parameters ();
  CBData_->timeStamp = timeStamp_in;

  // step1: retrieve server log view
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_->timeStamp);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (timestamp was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (CBData_->timeStamp.c_str ())));
    return;
  } // end IF

  // step2: setup auto-scrolling
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
  view_ =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TEXTVIEW_CONNECTION)));
  ACE_ASSERT (view_);

  GtkTextIter text_iter;
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iter);
  gtk_text_buffer_create_mark (buffer_p,
                               ACE_TEXT_ALWAYS_CHAR ("scroll"),
                               &text_iter,
                               TRUE);
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
}

IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::IRC_Client_GUI_MessageHandler_T (IRC_Client_GUI_IConnection* connection_in,
                                                                                           IRC_IControl* controller_in,
                                                                                           const std::string& id_in,
                                                                                           const std::string& UIFileDirectory_in,
                                                                                           const std::string& timestamp_in,
                                                                                           struct IRC_Client_UI_HandlerCBData* CBData_in,
                                                                                           bool gdkLockedAccess_in)
 : CBData_ (CBData_in)
 , isFirstMemberListMsg_ (true)
 , isPrivateDialog_ (false)
 , messageQueue_ ()
 , messageQueueLock_ ()
 , view_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::IRC_Client_GUI_MessageHandler_T"));

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (controller_in);
  ACE_ASSERT (!id_in.empty ());
  if (!Common_File_Tools::isDirectory (UIFileDirectory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: \"%s\"): not a directory, returning\n"),
                ACE_TEXT (UIFileDirectory_in.c_str ())));
    return;
  } // end IF
  ACE_ASSERT (CBData_);

  // initialize cb data
  //CBData_->acknowledgements = 0;
  //CBData_->builderLabel ();
  //CBData_->channelModes = 0;
  CBData_->connection = connection_in;
  CBData_->controller = controller_in;
  //CBData_->eventSourceId = 0;
  CBData_->handler = this;
  CBData_->id = id_in;
//  CBData_->parameters ();
  CBData_->timeStamp = timestamp_in;

//  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->UIState->lock);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // step1: retrieve server log view
    Common_UI_GTK_BuildersIterator_t iterator =
      state_r.builders.find (CBData_->timeStamp);
    // sanity check(s)
    if (iterator == state_r.builders.end ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection (timestamp was: \"%s\") builder not found, returning\n"),
                  ACE_TEXT (CBData_->timeStamp.c_str ())));
      return;
    } // end IF
    if (gdkLockedAccess_in)
    {
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
    } // end IF
    view_ =
      GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TEXTVIEW_CONNECTION)));
    if (gdkLockedAccess_in)
    {
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    } // end IF
    ACE_ASSERT (view_);
//  } // end lock scope

  // step2: create new GtkBuilder
  GtkBuilder* builder_p = gtk_builder_new ();
  if (!builder_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  std::string ui_definition_filename = UIFileDirectory_in;
  ui_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_filename +=
      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_CHANNEL_FILE);
  if (!Common_File_Tools::isReadable (ui_definition_filename))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid UI file (was: \"%s\"): not readable, returning\n"),
                ACE_TEXT (ui_definition_filename.c_str ())));
    g_object_unref (G_OBJECT (builder_p)); builder_p = NULL;
    return;
  } // end IF

  // load widget tree
  GError* error_p = NULL;
  if (gdkLockedAccess_in)
  {
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)
  } // end IF
  gtk_builder_add_from_file (builder_p,
                             ui_definition_filename.c_str (),
                             &error_p);
  if (error_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_add_from_file(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT (ui_definition_filename.c_str ()),
                ACE_TEXT (error_p->message)));
    g_error_free (error_p); error_p = NULL;
    g_object_unref (G_OBJECT (builder_p));
    if (gdkLockedAccess_in)
    {
#if GTK_CHECK_VERSION (3,6,0)
#else
      gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    } // end IF
    return;
  } // end IF
  if (gdkLockedAccess_in)
  {
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
  } // end IF

  // setup auto-scrolling in textview
  GtkTextView* text_view_p =
      GTK_TEXT_VIEW (gtk_builder_get_object (builder_p,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TEXTVIEW_CHANNEL)));
  ACE_ASSERT (text_view_p);
  GtkTextIter text_iter;
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (text_view_p);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iter);
  gtk_text_buffer_create_mark (buffer_p,
                               ACE_TEXT_ALWAYS_CHAR ("scroll"),
                               &text_iter,
                               TRUE);

  // enable multi-selection in treeview
  GtkTreeView* tree_view_p =
    GTK_TREE_VIEW (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TREEVIEW_CHANNEL)));
  ACE_ASSERT (tree_view_p);
  GtkTreeSelection* tree_selection_p =
    gtk_tree_view_get_selection (tree_view_p);
  ACE_ASSERT (tree_selection_p);
  gtk_tree_selection_set_mode (tree_selection_p,
                               GTK_SELECTION_MULTIPLE);

  // add the invite_channel_members_menu to the "Invite" menu item
  GtkMenu* menu_p =
    GTK_MENU (gtk_builder_get_object (builder_p,
                                      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENU_CHANNEL_INVITE)));
  ACE_ASSERT (menu_p);
  GtkMenuItem* menu_item_p =
    GTK_MENU_ITEM (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_MENUITEM_INVITE)));
  ACE_ASSERT (menu_item_p);
  gtk_menu_item_set_submenu (menu_item_p, GTK_WIDGET (menu_p));

  // connect signal(s)
  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_BUTTON_PART)));
  ACE_ASSERT (button_p);
  gulong result = g_signal_connect (button_p,
                                    ACE_TEXT_ALWAYS_CHAR ("clicked"),
                                    G_CALLBACK (part_clicked_cb),
                                    CBData_);
  ACE_ASSERT (result);

  // togglebuttons
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_ANONYMOUS)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BAN)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_INVITEONLY)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_KEY)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_USERLIMIT)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_MODERATED)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_BLOCKFOREIGN)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_OPERATOR)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_PRIVATE)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_QUIET)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_REOP)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_SECRET)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_RESTRICTOPIC)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object (builder_p,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TOGGLEBUTTON_CHANNELMODE_VOICE)));
  ACE_ASSERT (toggle_button_p);
  result = g_signal_connect (toggle_button_p,
                             ACE_TEXT_ALWAYS_CHAR ("toggled"),
                             G_CALLBACK (channel_mode_toggled_cb),
                             CBData_);
  ACE_ASSERT (result);

  // topic label
  GtkEventBox* event_box_p =
    GTK_EVENT_BOX (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_EVENTBOX_TOPIC)));
  ACE_ASSERT (event_box_p);
  result = g_signal_connect (event_box_p,
                             ACE_TEXT_ALWAYS_CHAR ("button-press-event"),
                             G_CALLBACK (topic_clicked_cb),
                             CBData_);
  ACE_ASSERT (result);

  // context menu in treeview
  result = g_signal_connect (tree_view_p,
                             ACE_TEXT_ALWAYS_CHAR ("button-press-event"),
                             G_CALLBACK (members_clicked_cb),
                             CBData_);
  ACE_ASSERT (result);
  // actions in treeview
  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_MESSAGE)));
  ACE_ASSERT (action_p);
  result = g_signal_connect (action_p,
                             ACE_TEXT_ALWAYS_CHAR ("activate"),
                             G_CALLBACK (action_msg_cb),
                             CBData_);
  ACE_ASSERT (result);
  action_p =
    GTK_ACTION (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_INVITE)));
  ACE_ASSERT (action_p);
  result = g_signal_connect (action_p,
                             ACE_TEXT_ALWAYS_CHAR ("activate"),
                             G_CALLBACK (action_invite_cb),
                             CBData_);
  ACE_ASSERT (result);
  action_p =
    GTK_ACTION (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_INFO)));
  ACE_ASSERT (action_p);
  result = g_signal_connect (action_p,
                             ACE_TEXT_ALWAYS_CHAR ("activate"),
                             G_CALLBACK (action_info_cb),
                             CBData_);
  ACE_ASSERT (result);
  action_p =
    GTK_ACTION (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_KICK)));
  ACE_ASSERT (action_p);
  result = g_signal_connect (action_p,
                             ACE_TEXT_ALWAYS_CHAR ("activate"),
                             G_CALLBACK (action_kick_cb),
                             CBData_);
  ACE_ASSERT (result);
  action_p =
    GTK_ACTION (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_ACTION_BAN)));
  ACE_ASSERT (action_p);
  result = g_signal_connect (action_p,
                             ACE_TEXT_ALWAYS_CHAR ("activate"),
                             G_CALLBACK (action_ban_cb),
                             CBData_);
  ACE_ASSERT (result);
  ACE_UNUSED_ARG (result);

  // *TODO*: there must be a better way to do this
  //         (see: IRC_client_gui_callbacks.cpp:2236, 2347, ...)
  std::string page_tab_label_string;
  if (!IRC_Tools::isValidChannelName (CBData_->id))
  {
    // --> private conversation window, modify label accordingly
    isPrivateDialog_ = true;
    page_tab_label_string = ACE_TEXT_ALWAYS_CHAR ("<b>");
    page_tab_label_string += CBData_->id;
    page_tab_label_string += ACE_TEXT_ALWAYS_CHAR ("</b>");
  } // end IF
  else
    page_tab_label_string = CBData_->id;
  CBData_->builderLabel = CBData_->timeStamp;
  CBData_->builderLabel += ACE_TEXT_ALWAYS_CHAR ("::");
  CBData_->builderLabel += page_tab_label_string;

//  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (state_r.lock);
    state_r.builders[CBData_->builderLabel] =
        std::make_pair (ui_definition_filename, builder_p);

    CBData_->eventSourceId =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_add_channel_cb,
                       CBData_,
                       NULL);
    if (!CBData_->eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add_full(idle_add_channel_cb): \"%m\", returning\n")));
      return;
    } // end IF
    // *TODO*: this id is never removed from the list again...
    state_r.eventSourceIds.insert (CBData_->eventSourceId);
//  } // end lock scope
}

IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::~IRC_Client_GUI_MessageHandler_T ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::~IRC_Client_GUI_MessageHandler_T"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

//  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->UIState->lock);

  // remove builder ?
  if (!isServerLog ())
  {
    Common_UI_GTK_BuildersIterator_t iterator =
        state_r.builders.find (CBData_->builderLabel);
    if (iterator != state_r.builders.end ())
    {
      g_object_unref (G_OBJECT ((*iterator).second.second));
      state_r.builders.erase (iterator);
    } // end IF
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("handler (was: \"%s\") builder not found, continuing\n"),
                  ACE_TEXT (CBData_->id.c_str ())));
  } // end IF

  // remove (queued) gtk events
  unsigned int removed_events = 0;
  while (g_idle_remove_by_data (&CBData_))
    removed_events++;
  if (removed_events)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: removed %u queued event(s)\n"),
                (isServerLog () ? ACE_TEXT (IRC_CLIENT_GUI_GTK_LABEL_DEF_LOG_TEXT)
                                : ACE_TEXT (CBData_->id.c_str ())),
                removed_events));
//  if (CBData_->eventSourceId)
//    CBData_->UIState->eventSourceIds.erase (CBData_->eventSourceId);
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::queueForDisplay (const std::string& text_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::queueForDisplay"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, messageQueueLock_);
    messageQueue_.push_front (text_in);
  } // end lock scope
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::update ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::update"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // always insert new text at the END of the buffer
  GtkTextView* text_view_p = NULL;

  //  gdk_threads_enter ();
  if (isServerLog ())
    text_view_p = view_;
  else
  {
    Common_UI_GTK_BuildersIterator_t iterator =
        state_r.builders.find (CBData_->builderLabel);
    // sanity check(s)
    ACE_ASSERT (iterator != state_r.builders.end ());

    text_view_p =
        GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TEXTVIEW_CHANNEL)));
  } // end ELSE
  ACE_ASSERT (text_view_p);

  GtkTextIter text_iter;
  gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer (text_view_p),
                                &text_iter);

  //gchar* string_p = NULL;
  std::string message_text;
  GtkTextMark* text_mark_p = NULL;
  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (messageQueueLock_);
    // sanity check
    for (IRC_Client_MessageQueueReverseIterator_t iterator = messageQueue_.rbegin ();
         iterator != messageQueue_.rend ();
         ++iterator)
    {
      message_text = *iterator + '\n';

      //// step1: convert text (GTK uses UTF-8 to represent strings)
      //string_p =
      //  Common_UI_GTK_Tools::Locale2UTF8 (message_text);
      //if (!string_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to Common_UI_GTK_Tools::Locale2UTF8(\"%s\"), returning\n"),
      //              ACE_TEXT (message_text.c_str ())));
      //  return;
      //} // end IF

      // step2: display text
      //gtk_text_buffer_insert (gtk_text_view_get_buffer (view_), &text_iterator,
      //                        string_p, -1);
      gtk_text_buffer_insert (gtk_text_view_get_buffer (text_view_p), &text_iter,
                              message_text.c_str (), -1);
      //g_free (string_p); string_p = NULL;

    //   // get the new "end"...
    //   gtk_text_buffer_get_end_iter(myTargetBuffer,
    //                                &iter);
      // move the iterator to the beginning of line, so the view doesn't scroll
      // in horizontal direction
      gtk_text_iter_set_line_offset (&text_iter, 0);

      // ...and place the mark at iter. The mark will stay there after text is
      // inserted at the end, because it has right gravity
      text_mark_p =
        gtk_text_buffer_get_mark (gtk_text_view_get_buffer (text_view_p),
                                  ACE_TEXT_ALWAYS_CHAR ("scroll"));
      ACE_ASSERT (text_mark_p);
      gtk_text_buffer_move_mark (gtk_text_view_get_buffer (text_view_p),
                                 text_mark_p,
                                 &text_iter);

      // scroll the mark onscreen
      gtk_text_view_scroll_mark_onscreen (text_view_p,
                                          text_mark_p);
    } // end FOR
    messageQueue_.clear ();
  } // end lock scope

  // redraw view area...
//   // sanity check(s)
//   ACE_ASSERT(myBuilder);
// //   GtkScrolledWindow* scrolledwindow = NULL;
//   GtkWindow* dialog = NULL;
//   dialog = GTK_WINDOW(gtk_builder_get_object(myBuilder,
//                                        ACE_TEXT_ALWAYS_CHAR("dialog")));
//   ACE_ASSERT(dialog);
//   GdkRegion* region = NULL;
//   region = gdk_drawable_get_clip_region(GTK_WIDGET(dialog)->window);
//   ACE_ASSERT(region);
//   gdk_window_invalidate_region(GTK_WIDGET(dialog)->window,
//                                region,
//                                TRUE);
  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (text_view_p)),
                              NULL,
                              TRUE);
//   gdk_region_destroy(region);
//   gtk_widget_queue_draw(GTK_WIDGET(view_));
  gdk_window_process_updates (gtk_widget_get_window (GTK_WIDGET (text_view_p)),
                              TRUE);
//   gdk_window_process_all_updates();

//  gdk_threads_leave ();
}

GtkWidget*
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::getTopLevelPageChild (bool lockedAccess_in) const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::getTopLevelPageChild"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  GtkWidget* widget_p = NULL;

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  if (lockedAccess_in)
  {
    result = state_r.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
  } // end IF

  // *NOTE*: the server log handler doesn't have a builder
  if (CBData_->id.empty ())
  {
    // sanity check(s)
    ACE_ASSERT (view_);

    widget_p =  gtk_widget_get_ancestor (GTK_WIDGET (view_),
                                         GTK_TYPE_WIDGET);
  } // end IF
  else
  {
    Common_UI_GTK_BuildersIterator_t iterator =
      state_r.builders.find (CBData_->builderLabel);
    // sanity check(s)
    ACE_ASSERT (iterator != state_r.builders.end ());

    widget_p =
        GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_VBOX_CHANNEL)));
  } // end ELSE
  ACE_ASSERT (widget_p);

  if (lockedAccess_in)
  {
    result = state_r.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF

  return widget_p;
}

// const std::string
// IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::getChannel() const
// {
//   NETWORK_TRACE(ACE_TEXT("IRC_Client_GUI_MessageHandler_T::getChannel"));
//
//   // sanity check: 'this' might be a private message handler !...
//   ACE_ASSERT(RPG_Net_Protocol_Tools::isValidIRCChannelName(CBData_->id));
//
//   return CBData_->id;
// }

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::setTopic (const std::string& topic_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::setTopic"));

  ACE_UNUSED_ARG (topic_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->UIState->lock);

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

  // retrieve label handle
  GtkLabel* label_p =
      GTK_LABEL (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LABEL_TOPIC)));
  ACE_ASSERT (label_p);
  gtk_label_set_text (label_p,
                      topic_in.c_str ());

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::setModes (const std::string& modes_in,
                                                                    const std::string& parameter_in,
                                                                    bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::setModes"));

  ACE_UNUSED_ARG (parameter_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  //  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->UIState->lock);
  int result = -1;
  if (lockedAccess_in)
  {
    result = state_r.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
  } // end IF

  CBData_->acknowledgements += IRC_Tools::merge (modes_in,
                                                 CBData_->channelModes);
  CBData_->eventSourceId =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_update_channel_modes_cb,
                       CBData_,
                       NULL);
  if (CBData_->eventSourceId)
    state_r.eventSourceIds.insert (CBData_->eventSourceId);
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add_full(idle_update_channel_modes_cb): \"%m\", continuing\n")));

  if (lockedAccess_in)
  {
    result = state_r.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::clearMembers (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::clearMembers"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

//  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->UIState->lock);
  int result = -1;
  if (lockedAccess_in)
  {
    result = state_r.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

  // retrieve channel liststore handle
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNEL)));
  ACE_ASSERT (list_store_p);

  // clear liststore
  gtk_list_store_clear (list_store_p);

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  if (lockedAccess_in)
  {
    result = state_r.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::update (const std::string& currentNickName_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::update"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->connection);

  std::string new_nick_name;//, channel;
  //CBData_->connection->current (new_nick_name,
  //                             channel);
  const struct IRC_SessionState& session_state_r =
    CBData_->connection->state ();
  new_nick_name = session_state_r.nickName;
  if (CBData_->channelModes.test (CHANNELMODE_OPERATOR))
    new_nick_name.insert (new_nick_name.begin (), '@');

  remove (currentNickName_in, true);
  add (new_nick_name, true);
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::add (const std::string& nickname_in,
                                                               bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::add"));

  ACE_UNUSED_ARG (nickname_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  int result = -1;
  if (lockedAccess_in)
  {
    result = state_r.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

  // retrieve channel liststore handle
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNEL)));
  ACE_ASSERT (list_store_p);

  //// step1: convert text
  const gchar* string_p = nickname_in.c_str ();
  //gchar* string_p = Common_UI_GTK_Tools::Locale2UTF8 (nickname_in);
  //if (!string_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Common_UI_GTK_Tools::Locale2UTF8(\"%s\"): \"%m\", returning\n"),
  //              ACE_TEXT (nickname_in.c_str ())));
  //  gdk_threads_leave ();
  //  return;
  //} // end IF

  // step2: append new (text) entry
  GtkTreeIter tree_iter;
  gtk_list_store_append (list_store_p, &tree_iter);
  gtk_list_store_set (list_store_p, &tree_iter,
                      0, string_p, // column 0
                      -1);

  // clean up
  //g_free (string_p);
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  if (lockedAccess_in)
  {
    result = state_r.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::remove (const std::string& nick_in,
                                                                  bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::remove"));

  ACE_UNUSED_ARG (nick_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  if (lockedAccess_in)
  {
    result = state_r.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  bool found_row = false;
  GtkTreeIter tree_iter;
  gchar* string_2 = NULL;

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

  // retrieve channel liststore handle
  GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNEL)));
  ACE_ASSERT (list_store_p);

  // step1: convert text
  gchar* string_p = Common_UI_GTK_Tools::localeToUTF8 (nick_in);
  if (!string_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_GTK_Tools::localeToUTF8(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (nick_in.c_str ())));
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    goto clean_up;
  } // end IF

  // step2: find matching entry
  if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store_p),
                                      &tree_iter))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_tree_model_get_iter_first(%@), returning\n"),
                list_store_p));
    g_free (string_p); string_p = NULL;
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    goto clean_up;
  } // end IF
  //   GValue current_value;
  do
  {
    string_2 = NULL;

    // retrieve value
//     gtk_tree_model_get_value(GTK_TREE_MODEL(list_store_p),
//                              current_iter,
//                              0, &current_value);
    gtk_tree_model_get (GTK_TREE_MODEL (list_store_p),
                        &tree_iter,
                        0, &string_2,
                        -1);
    if (g_str_equal (string_2,
                     string_p)       ||
        (g_str_has_suffix (string_2,
                           string_p) &&
         ((string_2[0] == '@'))))
      found_row = true;
    g_free (string_2); string_2 = NULL;
    if (found_row)
      break; // found value
  } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store_p),
                                     &tree_iter));
  g_free (string_p); string_p = NULL;

  if (found_row)
    gtk_list_store_remove (list_store_p,
                           &tree_iter);
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to remove nickname (was: \"%s\"), continuing\n"),
                ACE_TEXT (nick_in.c_str ())));

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

clean_up:
  if (lockedAccess_in)
  {
    result = state_r.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::members (const string_list_t& list_in,
                                                                   bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::members"));

  ACE_UNUSED_ARG (list_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  int result = -1;
  if (lockedAccess_in)
  {
    result = state_r.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  if (isFirstMemberListMsg_)
  {
    clearMembers (lockedAccess_in);
    isFirstMemberListMsg_ = false;
  } // end IF

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

  // retrieve channel liststore handle
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_LISTSTORE_CHANNEL)));
  ACE_ASSERT (list_store_p);

  GtkTreeIter tree_iter;
  const gchar* string_p = NULL;
  for (string_list_const_iterator_t iterator = list_in.begin ();
       iterator != list_in.end ();
       iterator++)
  {
    // step1: convert text
    string_p = (*iterator).c_str ();
    //string_p = Common_UI_GTK_Tools::Locale2UTF8 (*iterator);
    //if (!string_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Common_UI_GTK_Tools::Locale2UTF8(\"%s\"): \"%m\", returning\n"),
    //              ACE_TEXT ((*iterator).c_str ())));
    //  gdk_threads_leave ();
    //  return;
    //} // end IF

    // step2: append new (text) entry
    gtk_list_store_append (list_store_p, &tree_iter);
    gtk_list_store_set (list_store_p, &tree_iter,
                        0, string_p, // column 0
                        -1);
    //g_free (string_p); string_p = NULL;
  } // end FOR
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  if (lockedAccess_in)
  {
    result = state_r.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}

void
IRC_Client_GUI_MessageHandler_T<COMMON_UI_FRAMEWORK_GTK>::endMembers (bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_GUI_MessageHandler_T::endMembers"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  int result = -1;
  if (lockedAccess_in)
  {
    result = state_r.lock.acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
  } // end IF

  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_->builderLabel);
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  isFirstMemberListMsg_ = true;

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

  // retrieve treeview handle
  GtkTreeView* tree_view_p =
      GTK_TREE_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_TREEVIEW_CHANNEL)));
  ACE_ASSERT (tree_view_p);
  gtk_widget_set_sensitive (GTK_WIDGET (tree_view_p), TRUE);

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  if (lockedAccess_in)
  {
    result = state_r.lock.release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
  } // end IF
}
#endif // GTK_SUPPORT
