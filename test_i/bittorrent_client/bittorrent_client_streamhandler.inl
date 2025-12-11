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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"
#include "ace/Synch_Traits.h"

#include "common_parser_bencoding_tools.h"

#include "common_ui_common.h"

#include "net_macros.h"

#include "bittorrent_message.h"
#include "bittorrent_bencoding_parser_driver.h"
#include "bittorrent_sessionmessage.h"

#include "bittorrent_client_gui_callbacks.h"
#include "bittorrent_client_gui_common.h"

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
BitTorrent_Client_PeerStreamHandler_T<SessionDataType,
                                      UserDataType,
                                      SessionInterfaceType,
                                      CBDataType>::BitTorrent_Client_PeerStreamHandler_T (SessionInterfaceType* interfaceHandle_in,
                                                                                          CBDataType* CBData_in)
 : inherited (interfaceHandle_in,
              CBData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStreamHandler_T::BitTorrent_Client_PeerStreamHandler_T"));

}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_PeerStreamHandler_T<SessionDataType,
                                      UserDataType,
                                      SessionInterfaceType,
                                      CBDataType>::start (Stream_SessionId_t sessionId_in,
                                                          const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStreamHandler_T::start"));

  inherited::start (sessionId_in,
                    sessionData_in);

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
    } // end lock scope

//     struct BitTorrent_Client_UI_SessionConnectionCBData* session_connection_cb_data_p =
//       NULL;
//     ACE_NEW_NORETURN (session_connection_cb_data_p,
//                       struct BitTorrent_Client_UI_SessionConnectionCBData ());
//     ACE_ASSERT (session_connection_cb_data_p);
//     session_connection_cb_data_p->CBData = inherited::CBData_;
//     ACE_ASSERT (!sessionData_in.connectionStates.empty ());
//     session_connection_cb_data_p->connectionHandle =
//         sessionData_in.connectionStates.begin ()->second->handle;
//     guint event_source_id = g_idle_add (idle_add_session_connection_cb,
//                                         session_connection_cb_data_p);
//     if (event_source_id == 0)
//     {
//       ACE_DEBUG ((LM_ERROR,
//                   ACE_TEXT ("failed to g_idle_add(idle_add_session_connection_cb): \"%m\", returning\n")));
//       return;
//     } // end IF
//     state_r.eventSourceIds.insert (event_source_id);
 #endif // GTK_USE
  } // end IF
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_PeerStreamHandler_T<SessionDataType,
                                      UserDataType,
                                      SessionInterfaceType,
                                      CBDataType>::notify (Stream_SessionId_t sessionId_in,
                                                           const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_PeerStreamHandler_T<SessionDataType,
                                      UserDataType,
                                      SessionInterfaceType,
                                      CBDataType>::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStreamHandler_T::end"));

  typename inherited::SESSION_DATA_ITERATOR_T iterator =
    inherited::sessionData_.find (sessionId_in);
  ACE_ASSERT (iterator != inherited::sessionData_.end ());

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
    } // end lock scope

    // struct BitTorrent_Client_UI_SessionConnectionCBData* session_connection_cb_data_p =
    //   NULL;
    // ACE_NEW_NORETURN (session_connection_cb_data_p,
    //                   struct BitTorrent_Client_UI_SessionConnectionCBData ());
    // ACE_ASSERT (session_connection_cb_data_p);
    // session_connection_cb_data_p->CBData = inherited::CBData_;
    // ACE_ASSERT (!(*iterator).second->connectionStates.empty ());
    // session_connection_cb_data_p->connectionHandle =
    //   (*iterator).second->connectionStates.begin ()->second->handle;
    // guint event_source_id = g_idle_add (idle_remove_session_connection_cb,
    //                                     session_connection_cb_data_p);
    // if (event_source_id == 0)
    // {
    //   ACE_DEBUG ((LM_ERROR,
    //               ACE_TEXT ("failed to g_idle_add(idle_remove_session_connection_cb): \"%m\", returning\n")));
    //   return;
    // } // end IF
    // state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
  } // end IF

  inherited::end (sessionId_in);
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_PeerStreamHandler_T<SessionDataType,
                                      UserDataType,
                                      SessionInterfaceType,
                                      CBDataType>::notify (Stream_SessionId_t sessionId_in,
                                                           const BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                                                                      UserDataType>& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStreamHandler_T::notify"));

  inherited::notify (sessionId_in,
                     message_in);

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_DATA);
    } // end lock scope
#endif // GTK_USE
    inherited::CBData_->progressData.transferred += message_in.total_length ();
  } // end IF
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_PeerStreamHandler_T<SessionDataType,
                                      UserDataType,
                                      SessionInterfaceType,
                                      CBDataType>::notify (Stream_SessionId_t sessionId_in,
                                                           const BitTorrent_SessionMessage_T<SessionDataType,
                                                                                             UserDataType>& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_PeerStreamHandler_T::notify"));

  inherited::notify (sessionId_in,
                     sessionMessage_in);

  // sanity check(s)
  typename inherited::SESSION_DATA_ITERATOR_T iterator =
    inherited::sessionData_.find (sessionId_in);
  ACE_ASSERT (iterator != inherited::sessionData_.end ());

  int result = -1;

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
      enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
      switch (sessionMessage_in.type ())
      {
        case STREAM_SESSION_MESSAGE_CONNECT:
        {
// #if defined (GTK_USE)
//           struct BitTorrent_Client_UI_SessionConnectionCBData* session_connection_cb_data_p =
//             NULL;
//           ACE_NEW_NORETURN (session_connection_cb_data_p,
//                             struct BitTorrent_Client_UI_SessionConnectionCBData ());
//           ACE_ASSERT (session_connection_cb_data_p);
//           session_connection_cb_data_p->CBData = inherited::CBData_;
//           ACE_ASSERT (!(*iterator).second->connectionStates.empty ());
//           session_connection_cb_data_p->connectionHandle =
//             (*iterator).second->connectionStates.begin ()->second->handle;
//           guint event_source_id = g_idle_add (idle_add_session_connection_cb,
//                                               session_connection_cb_data_p);
//           if (event_source_id == 0)
//           {
//             ACE_DEBUG ((LM_ERROR,
//                         ACE_TEXT ("failed to g_idle_add(idle_add_session_connection_cb): \"%m\", returning\n")));
//             return;
//           } // end IF
//           inherited::CBData_->eventSourceIds.insert (event_source_id);
// #endif // GTK_USE
          event_e = COMMON_UI_EVENT_CONNECT;
          break;
        }
        case STREAM_SESSION_MESSAGE_DISCONNECT:
        {
// #if defined (GTK_USE)
          // struct BitTorrent_Client_UI_SessionConnectionCBData* session_connection_cb_data_p =
          //   NULL;
          // ACE_NEW_NORETURN (session_connection_cb_data_p,
          //                   struct BitTorrent_Client_UI_SessionConnectionCBData ());
          // ACE_ASSERT (session_connection_cb_data_p);
          // session_connection_cb_data_p->CBData = inherited::CBData_;
          // ACE_ASSERT (!(*iterator).second->connectionStates.empty ());
          // session_connection_cb_data_p->connectionHandle =
          //   (*iterator).second->connectionStates.begin ()->second->handle;
          // guint event_source_id = g_idle_add (idle_remove_session_connection_cb,
          //                                     session_connection_cb_data_p);
          // if (event_source_id == 0)
          // {
          //   ACE_DEBUG ((LM_ERROR,
          //               ACE_TEXT ("failed to g_idle_add(idle_remove_session_connection_cb): \"%m\", returning\n")));
          //   return;
          // } // end IF
          // inherited::CBData_->eventSourceIds.insert (event_source_id);
// #endif // GTK_USE
          event_e = COMMON_UI_EVENT_DISCONNECT;
          break;
        }
        case STREAM_SESSION_MESSAGE_STATISTIC:
        {
          if ((*iterator).second->lock)
          {
            result = (*iterator).second->lock->acquire ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                         ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
          } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
          inherited::CBData_->progressData.statistic =
            (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

          if ((*iterator).second->lock)
          {
            result = (*iterator).second->lock->release ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                         ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
          } // end IF

          event_e = COMMON_UI_EVENT_STATISTIC;
          break;
        }
        case STREAM_SESSION_MESSAGE_STEP_DATA:
          event_e = COMMON_UI_EVENT_STEP;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                     ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                     sessionMessage_in.type ()));
          return;
        }
      } // end SWITCH
#if defined (GTK_USE)
      state_r.eventStack.push (event_e);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

//////////////////////////////////////////

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
BitTorrent_Client_TrackerStreamHandler_T<SessionDataType,
                                         UserDataType,
                                         SessionInterfaceType,
                                         CBDataType>::BitTorrent_Client_TrackerStreamHandler_T (SessionInterfaceType* interfaceHandle_in,
                                                                                                CBDataType* CBData_in)
 : inherited (interfaceHandle_in,
              CBData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStreamHandler_T::BitTorrent_Client_TrackerStreamHandler_T"));

}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_TrackerStreamHandler_T<SessionDataType,
                                         UserDataType,
                                         SessionInterfaceType,
                                         CBDataType>::start (Stream_SessionId_t sessionId_in,
                                                             const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStreamHandler_T::start"));

  inherited::start (sessionId_in,
                    sessionData_in);

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      //  CBData_->progressData.transferred = 0;
      state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
    } // end lock scope

//    guint event_source_id = g_idle_add (idle_start_UI_cb,
//                                        inherited::CBData_);
//    if (event_source_id == 0)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_send_UI_cb): \"%m\", returning\n")));
//      return;
//    } // end IF
//    inherited::CBData_->eventSourceIds.insert (event_source_id);
#endif // GTK_USE
  } // end IF
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_TrackerStreamHandler_T<SessionDataType,
                                         UserDataType,
                                         SessionInterfaceType,
                                         CBDataType>::notify (Stream_SessionId_t sessionId_in,
                                                              const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_TrackerStreamHandler_T<SessionDataType,
                                         UserDataType,
                                         SessionInterfaceType,
                                         CBDataType>::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStreamHandler_T::end"));

  inherited::end (sessionId_in);

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
    } // end lock scope

//    guint event_source_id = g_idle_add (idle_end_UI_cb,
//                                        inherited::CBData_);
//    if (event_source_id == 0)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_end_UI_cb): \"%m\", returning\n")));
//      return;
//    } // end IF
//    inherited::CBData_->eventSourceIds.insert (event_source_id);
#endif // GTK_USE
  } // end IF
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_TrackerStreamHandler_T<SessionDataType,
                                         UserDataType,
                                         SessionInterfaceType,
                                         CBDataType>::notify (Stream_SessionId_t sessionId_in,
                                                              const BitTorrent_TrackerMessage_T<Stream_SessionData_T<SessionDataType>,
                                                                                                UserDataType>& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStreamHandler_T::notify"));

  inherited::notify (sessionId_in,
                     message_in);

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_DATA);
    } // end lock scope
#endif // GTK_USE
    inherited::CBData_->progressData.transferred += message_in.total_length ();
  } // end IF
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType,
          typename CBDataType>
void
BitTorrent_Client_TrackerStreamHandler_T<SessionDataType,
                                         UserDataType,
                                         SessionInterfaceType,
                                         CBDataType>::notify (Stream_SessionId_t sessionId_in,
                                                              const BitTorrent_SessionMessage_T<SessionDataType,
                                                                                                UserDataType>& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_TrackerStreamHandler_T::notify"));

  inherited::notify (sessionId_in,
                     sessionMessage_in);

  typename inherited::SESSION_DATA_ITERATOR_T iterator =
      inherited::sessionData_.find (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (iterator != inherited::sessionData_.end ());

  int result = -1;

  if (inherited::CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_Manager_t* gtk_manager_p =
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
        const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
      enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
      switch (sessionMessage_in.type ())
      {
        case STREAM_SESSION_MESSAGE_STATISTIC:
        {
          if ((*iterator).second->lock)
          {
            result = (*iterator).second->lock->acquire ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                         ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
          } // end IF

#if defined (GTK_USE) || defined (WXWIDGETS_USE)
          inherited::CBData_->progressData.statistic =
              (*iterator).second->statistic;
#endif // GTK_USE || WXWIDGETS_USE

          if ((*iterator).second->lock)
          {
            result = (*iterator).second->lock->release ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                         ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
          } // end IF

          event_e = COMMON_UI_EVENT_STATISTIC;
          break;
        }
        case STREAM_SESSION_MESSAGE_STEP_DATA:
          event_e = COMMON_UI_EVENT_STEP;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                     ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                     sessionMessage_in.type ()));
          return;
        }
      } // end SWITCH
#if defined (GTK_USE)
      state_r.eventStack.push (event_e);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}
