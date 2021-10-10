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

#include <sstream>

#include "ace/ACE.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_tools.h"

#include "net_macros.h"

#include "bittorrent_tools.h"

#include "bittorrent_client_network.h"
#include "bittorrent_client_session_common.h"
#include "bittorrent_client_stream_common.h"
#include "bittorrent_client_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "bittorrent_client_gui_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "bittorrent_client_gui_defines.h"

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::BitTorrent_Client_GUI_Session_T (const struct BitTorrent_Client_Configuration& configuration_in,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                                                                        const Common_UI_GTK_State_t& GTKState_in,
                                                                                        guint contextId_in,
#endif // GTK_USE
#endif // GUI_SUPPORT
                                                                                        const std::string& label_in,
#if defined (GUI_SUPPORT)
                                                                                        const std::string& UIFileDirectory_in,
#endif // GUI_SUPPORT
                                                                                        BitTorrent_Client_IControl_t* controller_in,
                                                                                        const std::string& metaInfoFileName_in)
 : closing_ (false)
 , CBData_ ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 , contextId_ (contextId_in)
#endif // GTK_USE
 , UIFileDirectory_ (UIFileDirectory_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::BitTorrent_Client_GUI_Session_T"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  if (!Common_File_Tools::isDirectory (UIFileDirectory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: \"%s\"): not a directory, returning\n"),
                ACE_TEXT (UIFileDirectory_in.c_str ())));
    return;
  } // end IF
#endif // GUI_SUPPORT

  // initialize cb data
  CBData_.configuration =
      &const_cast<struct BitTorrent_Client_Configuration&> (configuration_in);
  CBData_.controller = controller_in;
  CBData_.eventSourceId = 0;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  CBData_.state = &const_cast<Common_UI_GTK_State_t&> (GTKState_in);
#endif // GTK_USE
#endif // GUI_SUPPORT
  CBData_.handler = this;
  CBData_.label = label_in;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  // create new GtkBuilder
  GtkBuilder* builder_p = gtk_builder_new ();
  if (!builder_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF

  GError* error_p = NULL;
  GtkButton* button_p = NULL;
  GtkComboBox* combo_box_p = NULL;
  gulong result = 0;

  std::string ui_definition_filename = UIFileDirectory_;
  ui_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_filename +=
      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_UI_SESSION_FILE);
  if (!Common_File_Tools::isReadable (ui_definition_filename))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid UI file (was: \"%s\"): not readable, returning\n"),
                ACE_TEXT (ui_definition_filename.c_str ())));
    goto error;
  } // end IF

  // load widget tree
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
    g_error_free (error_p); error_p = NULL;
    gdk_threads_leave ();
    goto error;
  } // end IF
  gdk_threads_leave ();

  // connect signal(s)
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_TAB_SESSION_CLOSE)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_session_close_clicked_cb),
                             &CBData_);
  ACE_ASSERT (result);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CONNECTIONS)));
  ACE_ASSERT (combo_box_p);
  result = g_signal_connect (combo_box_p,
                             ACE_TEXT_ALWAYS_CHAR ("changed"),
                             G_CALLBACK (combobox_connections_changed_cb),
                             &CBData_);
  ACE_ASSERT (result);
  button_p =
    GTK_BUTTON (gtk_builder_get_object (builder_p,
                                        ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_BUTTON_CONNECTION_CLOSE)));
  ACE_ASSERT (button_p);
  result = g_signal_connect (button_p,
                             ACE_TEXT_ALWAYS_CHAR ("clicked"),
                             G_CALLBACK (button_connection_close_clicked_cb),
                             &CBData_);
  ACE_ASSERT (result);

//  // retrieve connection tabs
//  GtkNotebook* notebook_p =
//    GTK_NOTEBOOK (gtk_builder_get_object (builder_p,
//                                          ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_NOTEBOOK_SESSIONS)));
//  ACE_ASSERT (notebook_p);
//  result_2 = g_signal_connect (notebook_p,
//                               ACE_TEXT_ALWAYS_CHAR ("switch-page"),
//                               G_CALLBACK (switch_channel_cb),
//                               &CBData_);
//  ACE_ASSERT (result_2);

//  // retrieve session log tab child
//  GtkScrolledWindow* scrolled_window_p =
//    GTK_SCROLLED_WINDOW (gtk_builder_get_object (builder_p,
//                                                 ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_SCROLLEDWINDOW_SESSION)));
//  ACE_ASSERT (scrolled_window_p);
//  // disallow reordering the application log tab
//  gtk_notebook_set_tab_reorderable (notebook_p,
//                                    GTK_WIDGET (scrolled_window_p),
//                                    FALSE);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);
    CBData_.state->builders[CBData_.label] =
        std::make_pair (ui_definition_filename, builder_p);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT

  // start session
  ACE_ASSERT (CBData_.controller);
  try {
    CBData_.controller->request (metaInfoFileName_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::request(\"%s\"), returning\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);
    CBData_.state->builders.erase (CBData_.label);
#endif // GTK_USE
#endif // GUI_SUPPORT
    goto error;
  }

  ACE_ASSERT (!CBData_.session);
  CBData_.session = CBData_.controller->get (metaInfoFileName_in);
  if (!CBData_.session)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve session handle (metainfo file was: \"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);
    CBData_.state->builders.erase (CBData_.label);
#endif // GTK_USE
#endif // GUI_SUPPORT
    goto error;
  } // end IF

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);
    CBData_.eventSourceId =
        g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                         idle_add_session_cb,
                         &CBData_,
                         NULL);
    if (!CBData_.eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add_full(idle_add_session_cb): \"%m\", returning\n")));
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);
      CBData_.state->builders.erase (CBData_.label);
      goto error;
    } // end IF
    CBData_.state->eventSourceIds.insert (CBData_.eventSourceId);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT

  return;

error:
  if (CBData_.session)
  {
    CBData_.session->close (true);
    delete CBData_.session;
    CBData_.session = NULL;
  } // end IF
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  g_object_unref (G_OBJECT (builder_p)); builder_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT
}

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::~BitTorrent_Client_GUI_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::~BitTorrent_Client_GUI_Session_T"));

  // sanity check(s)
  ACE_ASSERT (CBData_.state);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  // remove builder
  Common_UI_GTK_BuildersIterator_t iterator =
    CBData_.state->builders.find (CBData_.label);
  // sanity check(s)
  if (iterator == CBData_.state->builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("session (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (CBData_.label.c_str ())));
    return;
  } // end IF
  g_object_unref (G_OBJECT ((*iterator).second.second));
  CBData_.state->builders.erase (iterator);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
void
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::close ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::close"));

  // sanity check(s)
  ACE_ASSERT (CBData_.state);
  ACE_ASSERT (CBData_.session);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);
    // step1: close connections
    CBData_.session->close (false);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    // step2: remove session from the UI
    guint event_source_id =
        g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                         idle_remove_session_cb,
                         CBData_,
                         NULL);
    if (event_source_id)
      CBData_.state->eventSourceIds.insert (event_source_id);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add_full(idle_remove_connection_cb): \"%m\", continuing\n")));
#endif // GTK_USE
#endif // GUI_SUPPORT
  } // end lock scope
}

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
void
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::log (const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::log"));

  // sanity check(s)
  ACE_ASSERT (CBData_.state);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  gdk_threads_enter ();
  Common_UI_GTK_BuildersIterator_t iterator =
      CBData_.state->builders.find (CBData_.label);
  // sanity check(s)
  ACE_ASSERT (iterator != CBData_.state->builders.end ());

  GtkTextView* text_view_p =
      GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TEXTVIEW_SESSION)));
  ACE_ASSERT (text_view_p);

  // always insert new text at the END of the buffer
  GtkTextIter text_iter;
  gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer (text_view_p),
                                &text_iter);

  //gchar* string_p = NULL;
//  std::string message_text;
  GtkTextMark* text_mark_p = NULL;

  //// step1: convert text (GTK uses UTF-8 to represent strings)
  //string_p =
  //  Common_UI_Tools::Locale2UTF8 (message_in);
  //if (!string_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Common_UI_Tools::Locale2UTF8(\"%s\"), returning\n"),
  //              ACE_TEXT (message_in.c_str ())));
  //  return;
  //} // end IF

  // step2: display text
  //gtk_text_buffer_insert (gtk_text_view_get_buffer (view_), &text_iterator,
  //                        string_p, -1);
  gtk_text_buffer_insert (gtk_text_view_get_buffer (text_view_p), &text_iter,
                          message_in.c_str (), -1);

  //// clean up
  //g_free (string_p);

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
#endif // GTK_USE
#endif // GUI_SUPPORT
}

//template <typename ConnectionType>
//void
//BitTorrent_Client_GUI_Session_T<ConnectionType>::log (const struct BitTorrent_Record& record_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::log"));

//  // --> pass to server log

//  // retrieve message handler
//  { // synch access
//    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

//    if (closing_)
//      return; // done

//    MESSAGE_HANDLERSITERATOR_T handler_iterator =
//      messageHandlers_.find (std::string ());
//    ACE_ASSERT (handler_iterator != messageHandlers_.end ());
//    (*handler_iterator).second->queueForDisplay (IRC_Tools::Record2String (message_in));
//  } // end lock scope
//}

//template <typename ConnectionType>
//void
//BitTorrent_Client_GUI_Session_T<ConnectionType>::error (const struct BitTorrent_Record& record_in,
//                                                        bool lockedAccess_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::error"));

//  // sanity check(s)
//  ACE_ASSERT (CBData_);

//  int result = -1;
//  if (lockedAccess_in)
//  {
//    result = CBData_.lock.acquire ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
//  } // end IF

//  Common_UI_GTK_BuildersIterator_t iterator =
//    CBData_.builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
//  // sanity check(s)
//  if (iterator == CBData_.builders.end ())
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("connection (was: \"%s\") main builder not found, returning\n"),
//                ACE_TEXT (CBData_.label.c_str ())));
//    return;
//  } // end IF

//  gdk_threads_enter ();
//  // error --> print on statusbar
//  GtkStatusbar* statusbar_p =
//    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
//                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_STATUSBAR)));
//  ACE_ASSERT (statusbar_p);
//  gtk_statusbar_push (statusbar_p,
//                      contextID_,
//                      BitTorrent_Tools::dump (message_in).c_str ());
//  gdk_threads_leave ();

//  if (lockedAccess_in)
//  {
//    result = CBData_.lock.release ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
//  } // end IF
//}

//template <typename SessionInterfaceType,
//          typename ConnectionType,
//          typename ConnectionCBDataType>
//void
//BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
//                                ConnectionType,
//                                ConnectionCBDataType>::createConnection (const ACE_INET_Addr& address_in,
//                                                                         bool lockedAccess_in,
//                                                                         bool gdkLockedAccess_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::createConnection"));

//  // sanity check(s)
//  ACE_ASSERT (CBData_);
//  ACE_ASSERT (CBData_.state);

//  int result = -1;
//  ConnectionType* connection_p = NULL;

//  if (lockedAccess_in)
//  {
//    result = CBData_.state->lock.acquire ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Mutex::acquire(): \"%m\", continuing\n")));
//  } // end IF

//  Common_UI_GTK_BuildersIterator_t iterator =
//    CBData_.state->builders.find (CBData_.label);
//  // sanity check(s)
//  if (iterator == CBData_.state->builders.end ())
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("session builder (was: \"%s\") not found, returning\n"),
//                ACE_TEXT (CBData_.label.c_str ())));
//    goto clean_up;
//  } // end IF

//  // create new ConnectionType
////  gdk_threads_enter ();
//  // *TODO*: remove type inferences
////  ACE_NEW_NORETURN (connection_p,
////                    ConnectionType (CBData_,
////                                    this,
////                                    CBData_.controller,
////                                    address_in,
////                                    UIFileDirectory_,
////                                    CBData_.label,
////                                    gdkLockedAccess_in));
////  if (!connection_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate memory, returning\n")));

////    // clean up
//////    gdk_threads_leave ();

////    goto clean_up;
////  } // end IF

////    connections_.insert (std::make_pair (address_in, connection_p));

//  // check whether this is the first connection
//  // --> enable corresponding widget(s) in the main UI
//  if (CBData_.connections.size () == 1)
//  {
//    iterator =
//        CBData_.state->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
//    // sanity check(s)
//    ACE_ASSERT (iterator != CBData_.state->builders.end ());
//    gdk_threads_enter ();
//    GtkComboBox* combo_box_p =
//        GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
//                                               ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CONNECTIONS)));
//    ACE_ASSERT (combo_box_p);
//    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
//    gdk_threads_leave ();
//  } // end IF

//clean_up:
//  if (lockedAccess_in)
//  {
//    result = CBData_.state->lock.release ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Thread_Mutex::release(): \"%m\", continuing\n")));
//  } // end IF
//}

//template <typename SessionInterfaceType,
//          typename ConnectionType,
//          typename ConnectionCBDataType>
//void
//BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
//                                ConnectionType,
//                                ConnectionCBDataType>::terminateConnection (const ACE_INET_Addr& address_in,
//                                                                            bool lockedAccess_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::terminateConnection"));

//  // sanity check(s)
//  ACE_ASSERT (CBData_);
//  ACE_ASSERT (CBData_.state);

//  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_.state->lock);

//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//  ACE_INET_Addr local_address, remote_address;
//  BitTorrent_Client_GUI_ConnectionsIterator_t iterator;
//  for (iterator = CBData_.connections.begin ();
//       iterator != CBData_.connections.end ();
//       ++iterator)
//  {
//    (*iterator).second->info (handle,
//                              local_address,
//                              remote_address);
//    if (remote_address == address_in)
//      break;
//  } // end FOR
//  if (iterator == CBData_.connections.end ())
//  {
//    ACE_TCHAR buffer[BUFSIZ];
//    int result = address_in.addr_to_string (buffer,
//                                            sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("unknown/invalid connection (address was: \"%s\"), aborting\n"),
//                buffer));
//    return;
//  } // end IF

//  (*iterator).second->close ();
//  (*iterator).second->decrease ();
//  CBData_.connections.erase (iterator);

////  guint event_source_id =
////      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
////                       idle_remove_connection_cb,
////                       CBData_,
////                       NULL);
////  if (event_source_id)
////    CBData_.eventSourceIds.insert (event_source_id);
////  else
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to g_idle_add_full(idle_remove_connection_cb): \"%m\", continuing\n")));

//  // check whether this was the last connection
//  // --> disable corresponding widgets in the main UI
//  if (!CBData_.connections.size ())
//  {
//    Common_UI_GTK_BuildersIterator_t iterator =
//      CBData_.state->builders.find (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN);
//    // sanity check(s)
//    ACE_ASSERT (iterator != CBData_.state->builders.end ());
//    gdk_threads_enter ();
//    GtkComboBox* combo_box_p =
//      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
//                                             ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CONNECTIONS)));
//    ACE_ASSERT (combo_box_p);
//    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), FALSE);
//    gdk_threads_leave ();
//  } // end IF
//}
