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

#if defined (GTK_SUPPORT)
#include "bittorrent_client_gui_callbacks.h"
#endif // GTK_SUPPORT
#include "bittorrent_client_gui_defines.h"

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::BitTorrent_Client_GUI_Session_T (struct BitTorrent_Client_UI_CBData& CBData_inout,
#if defined (GTK_USE)
                                                                                        guint contextId_in,
#endif // GTK_USE
                                                                                        const std::string& label_in,
                                                                                        const std::string& UIFileDirectory_in,
                                                                                        BitTorrent_Client_IControl_t* controller_in,
                                                                                        const std::string& metaInfoFileName_in)
 : closing_ (false)
 , CBData_ ()
#if defined (GTK_USE)
 , contextId_ (contextId_in)
#endif // GTK_USE
 , UIFileDirectory_ (UIFileDirectory_in)
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
  CBData_.UIFileDirectory = UIFileDirectory_in;

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    CBData_.eventSourceId =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_load_session_ui_cb,
                       &CBData_,
                       NULL);
    if (!CBData_.eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add_full(idle_load_session_ui_cb): \"%m\", returning\n")));
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      goto error;
    } // end IF
    state_r.eventSourceIds.insert (CBData_.eventSourceId);
    CBData_.CBData->progressData.pendingActions[CBData_.eventSourceId] =
      ACE_Thread_ID (0, 0);
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

template <typename SessionInterfaceType,
          typename ConnectionType,
          typename ConnectionCBDataType>
void
BitTorrent_Client_GUI_Session_T<SessionInterfaceType,
                                ConnectionType,
                                ConnectionCBDataType>::peerConnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::peerConnect"));

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  struct BitTorrent_Client_UI_SessionConnectionCBData* session_connection_cb_data_p =
    NULL;
  ACE_NEW_NORETURN (session_connection_cb_data_p,
                    struct BitTorrent_Client_UI_SessionConnectionCBData ());
  ACE_ASSERT (session_connection_cb_data_p);
  session_connection_cb_data_p->CBData = &CBData_;

  ConnectionType* iconnection_p =
    BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ()->get (id_in);
  if (unlikely (!iconnection_p))
  {
    delete session_connection_cb_data_p;
    return;
  } // end IF
  ACE_INET_Addr local_address, peer_address;
  iconnection_p->info (session_connection_cb_data_p->connectionHandle,
                       local_address, peer_address);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    guint event_source_id =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_add_session_connection_cb,
                       session_connection_cb_data_p,
                       NULL);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_add_session_connection_cb): \"%m\", returning\n")));
      iconnection_p->decrease ();
      delete session_connection_cb_data_p;
      return;
    } // end IF
    iconnection_p->decrease (); iconnection_p = NULL;

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
                                ConnectionCBDataType>::peerDisconnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_GUI_Session_T::peerDisconnect"));

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  struct BitTorrent_Client_UI_SessionConnectionCBData* session_connection_cb_data_p =
    NULL;
  ACE_NEW_NORETURN (session_connection_cb_data_p,
                    struct BitTorrent_Client_UI_SessionConnectionCBData ());
  ACE_ASSERT (session_connection_cb_data_p);
  session_connection_cb_data_p->CBData = &CBData_;

  ConnectionType* iconnection_p =
    BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ()->get (id_in);
  if (unlikely (!iconnection_p))
  {
    delete session_connection_cb_data_p;
    return;
  } // end IF
  ACE_INET_Addr local_address, peer_address;
  iconnection_p->info (session_connection_cb_data_p->connectionHandle,
                       local_address, peer_address);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    guint event_source_id =
      g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
                       idle_remove_session_connection_cb,
                       session_connection_cb_data_p,
                       NULL);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_remove_session_connection_cb): \"%m\", returning\n")));
      iconnection_p->decrease ();
      delete session_connection_cb_data_p;
      return;
    } // end IF
    iconnection_p->decrease (); iconnection_p = NULL;

    state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
}
