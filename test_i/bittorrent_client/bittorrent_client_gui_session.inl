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
                                ConnectionCBDataType>::BitTorrent_Client_GUI_Session_T (struct BitTorrent_Client_UI_CBData& CBData_inout,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
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
  ACE_ASSERT (Common_File_Tools::isDirectory (UIFileDirectory_in));

  // initialize cb data
  CBData_inout.configuration->sessionConfiguration.subscriber = this;

  CBData_.controller = controller_in;
  CBData_.CBData = &CBData_inout;
  CBData_.handler = this;
  CBData_.label = label_in;

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  GError* error_p = NULL;

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
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

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
#if GTK_CHECK_VERSION (3,6,0)
#else
    gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("loaded widgets tree \"%s\": \"%s\"\n"),
              ACE_TEXT (CBData_.label.c_str ()),
              ACE_TEXT (ui_definition_filename.c_str ())));

  // connect signal(s)
  // step1: connect signals/slots
  gtk_builder_connect_signals (builder_p,
                               &CBData_);

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_COMBOBOX_CONNECTION)));
  ACE_ASSERT (combo_box_p);
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    goto error;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              TRUE);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  ACE_TEXT ("text"), 0,
                                  NULL);

  GtkTextBuffer* text_buffer_p =
    GTK_TEXT_BUFFER (gtk_builder_get_object (builder_p,
                                             ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TEXTBUFFER_SESSION)));
  ACE_ASSERT (text_buffer_p);
  GtkTextIter text_iter;
  gtk_text_buffer_get_end_iter (text_buffer_p,
                                &text_iter);
  gtk_text_buffer_create_mark (text_buffer_p,
                               ACE_TEXT_ALWAYS_CHAR ("scroll"),
                               &text_iter,
                               TRUE);
  GtkTextView* text_view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object (builder_p,
                                           ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_TEXTVIEW_SESSION)));
  ACE_ASSERT (text_view_p);
  gtk_text_view_set_buffer (text_view_p, text_buffer_p);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.builders[CBData_.label] =
      std::make_pair (ui_definition_filename, builder_p);
  } // end lock scope
#endif // GTK_USE

  // start session
  ACE_ASSERT (CBData_.controller);
  try {
    CBData_.controller->request (metaInfoFileName_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::request(\"%s\"), returning\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
#if defined (GTK_USE)
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.builders.erase (CBData_.label);
#endif // GTK_USE
    goto error;
  }

  ACE_ASSERT (!CBData_.session);
  CBData_.session = CBData_.controller->get (metaInfoFileName_in);
  if (!CBData_.session)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve session handle (metainfo file was: \"%s\"), aborting\n"),
                ACE_TEXT (metaInfoFileName_in.c_str ())));
#if defined (GTK_USE)
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.builders.erase (CBData_.label);
#endif // GTK_USE
    goto error;
  } // end IF

#if defined (GTK_USE)
#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_enter ();
#endif // GTK_CHECK_VERSION (3,6,0)

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    CBData_.eventSourceId =
        g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                         idle_add_session_cb,
                         &CBData_,
                         NULL);
    if (!CBData_.eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add_full(idle_add_session_cb): \"%m\", returning\n")));
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.builders.erase (CBData_.label);
      goto error;
    } // end IF
    state_r.eventSourceIds.insert (CBData_.eventSourceId);
    CBData_.CBData->progressData.pendingActions[CBData_.eventSourceId] =
        ACE_Thread_ID (0, 0);
  } // end lock scope

#if GTK_CHECK_VERSION (3,6,0)
#else
  gdk_threads_leave ();
#endif // GTK_CHECK_VERSION (3,6,0)
#endif // GTK_USE

  return;

error:
  if (CBData_.session)
  {
    CBData_.session->close (true);
    delete CBData_.session; CBData_.session = NULL;
  } // end IF
#if defined (GTK_USE)
  g_object_unref (G_OBJECT (builder_p)); builder_p = NULL;
#endif // GTK_USE
}

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::~BitTorrent_Client_GUI_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::~BitTorrent_Client_GUI_Session_T"));

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // remove builder
  Common_UI_GTK_BuildersIterator_t iterator =
    state_r.builders.find (CBData_.label);
  // sanity check(s)
  if (iterator == state_r.builders.end ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("session (was: \"%s\") builder not found, returning\n"),
                ACE_TEXT (CBData_.label.c_str ())));
    return;
  } // end IF
  g_object_unref (G_OBJECT ((*iterator).second.second));
  state_r.builders.erase (iterator);
#endif // GTK_USE
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
  ACE_ASSERT (CBData_.session);

  // step1: close connections
  CBData_.session->close (false);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // step2: remove session from the UI
  guint event_source_id =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_remove_session_cb,
                       &CBData_,
                       NULL);
  ACE_ASSERT (event_source_id);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
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

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  struct BitTorrent_Client_UI_SessionProgressData* cb_data_p = NULL;
  ACE_NEW_NORETURN (cb_data_p,
                    struct BitTorrent_Client_UI_SessionProgressData);
  ACE_ASSERT (cb_data_p);
  cb_data_p->label = CBData_.label;
  cb_data_p->message = message_in;

  guint event_source_id =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_log_progress_cb,
                       cb_data_p,
                       NULL);
  ACE_ASSERT (event_source_id);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
}

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
void
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::pieceComplete (unsigned int pieceIndex_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::pieceComplete"));

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  struct BitTorrent_Client_UI_SessionProgressData* cb_data_p = NULL;
  ACE_NEW_NORETURN (cb_data_p,
                    struct BitTorrent_Client_UI_SessionProgressData);
  ACE_ASSERT (cb_data_p);
  cb_data_p->label = CBData_.label;
  cb_data_p->pieceIndex = static_cast<int> (pieceIndex_in);

  guint event_source_id =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_piece_complete_progress_cb,
                       cb_data_p,
                       NULL);
  ACE_ASSERT (event_source_id);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
}

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
void
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::complete (bool cancelled_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::complete"));

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  struct BitTorrent_Client_UI_SessionProgressData* cb_data_p = NULL;
  ACE_NEW_NORETURN (cb_data_p,
                    struct BitTorrent_Client_UI_SessionProgressData);
  ACE_ASSERT (cb_data_p);
  cb_data_p->label = CBData_.label;
  cb_data_p->cancelled = cancelled_in;

  guint event_source_id =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_complete_progress_cb,
                       cb_data_p,
                       NULL);
  ACE_ASSERT (event_source_id);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
}
