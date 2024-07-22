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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "common_parser_bencoding_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#if defined (GTK_USE)
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "net_macros.h"

#include "bittorrent_message.h"
#include "bittorrent_bencoding_parser_driver.h"
#include "bittorrent_sessionmessage.h"

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType
#if defined (GUI_SUPPORT)
                               ,CBDataType>::BitTorrent_PeerStreamHandler_T (SessionInterfaceType* interfaceHandle_in,
                                                                             CBDataType* CBData_in)
#else
                               >::BitTorrent_PeerStreamHandler_T (SessionInterfaceType* interfaceHandle_in)
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , session_ (interfaceHandle_in)
#else
 : session_ (interfaceHandle_in)
#endif // GUI_SUPPORT
 , lock_ ()
 , sessionData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::BitTorrent_PeerStreamHandler_T"));

  // sanity check(s)
  ACE_ASSERT (session_);
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType
#if defined (GUI_SUPPORT)
                               ,CBDataType>::start (Stream_SessionId_t sessionId_in,
#else
                               >::start (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                                    const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::start"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    // sanity check(s)
    ACE_ASSERT (sessionData_.find (sessionId_in) == sessionData_.end ());
    sessionData_.insert (std::make_pair (sessionId_in,
                                         &const_cast<SessionDataType&> (sessionData_in)));
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (session_);

  try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    session_->connect (reinterpret_cast<Net_ConnectionId_t> (sessionData_in.connectionStates.begin ()->second->handle));
#else
    session_->connect (static_cast<Net_ConnectionId_t> (sessionData_in.connectionStates.begin ()->second->handle));
#endif // ACE_WIN32 || ACE_WIN64
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::connect(), continuing\n")));
  }
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType
#if defined (GUI_SUPPORT)
                               ,CBDataType>::notify (Stream_SessionId_t sessionId_in,
#else
                               >::notify (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                                     const enum Stream_SessionMessageType& sessionEvent_in,
                                                     bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType
#if defined (GUI_SUPPORT)
                               ,CBDataType>::end (Stream_SessionId_t sessionId_in)
#else
                               >::end (Stream_SessionId_t sessionId_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::end"));

  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionId_in);
    ACE_ASSERT (iterator != sessionData_.end ());
    ACE_ASSERT ((*iterator).second);
    handle_h = (*iterator).second->connectionStates.begin ()->second->handle;
    sessionData_.erase (iterator);
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (session_);
  ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);

  try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    session_->disconnect (reinterpret_cast<Net_ConnectionId_t> (handle_h));
#else
    session_->disconnect (static_cast<Net_ConnectionId_t> (handle_h));
#endif // ACE_WIN32 || ACE_WIN64
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::disconnect(), continuing\n")));
  }
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType
#if defined (GUI_SUPPORT)
                               ,CBDataType>::notify (Stream_SessionId_t sessionId_in,
#else
                               >::notify (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                                     const BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                                                                UserDataType>& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::notify"));

  // sanity check(s)
  ACE_ASSERT (session_);

  ACE_HANDLE handle_h = ACE_INVALID_HANDLE;

  // notify handshake ?
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionId_in);
    ACE_ASSERT (iterator != sessionData_.end ());
    ACE_ASSERT ((*iterator).second);
    handle_h = (*iterator).second->connectionStates.begin ()->second->handle;
    ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);
    if (!(*iterator).second->forwardedHandshake)
    {
      (*iterator).second->forwardedHandshake = true;
      ACE_ASSERT ((*iterator).second->handshake);
      try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        session_->notify (reinterpret_cast<Net_ConnectionId_t> (handle_h),
#else
        session_->notify (static_cast<Net_ConnectionId_t> (handle_h),
#endif // ACE_WIN32 || ACE_WIN64
                          *(*iterator).second->handshake);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_ISession_T::notify(), continuing\n")));
      }
    } // end IF
  } // end lock scope

  const typename BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                      UserDataType>::DATA_T& data_container_r =
    message_in.getR ();
  const struct BitTorrent_PeerRecord& record_r = data_container_r.getR ();
  try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        session_->notify (reinterpret_cast<Net_ConnectionId_t> (handle_h),
#else
        session_->notify (static_cast<Net_ConnectionId_t> (handle_h),
#endif // ACE_WIN32 || ACE_WIN64
                          record_r,
                          (record_r.type == BITTORRENT_MESSAGETYPE_PIECE) ? &const_cast<BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                                                                                             UserDataType>&> (message_in)
                                                                          : NULL);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::notify(), continuing\n")));
  }
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_PeerStreamHandler_T<SessionDataType,
                               UserDataType,
                               SessionInterfaceType
#if defined (GUI_SUPPORT)
                               ,CBDataType>::notify (Stream_SessionId_t sessionId_in,
#else
                               >::notify (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                                     const BitTorrent_SessionMessage_T<SessionDataType,
                                                                                       UserDataType>& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_PeerStreamHandler_T::notify"));

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionId_in);

  // sanity check(s)
  // *NOTE*: the current implementation isn't fully synchronized in the sense
  //         that messages may arrive 'out-of-session' (e.g.
  //         STREAM_SESSION_MESSAGE_DISCONNECT after STREAM_SESSION_MESSAGE_END)
  if (iterator == sessionData_.end ())
    return;
}

//////////////////////////////////////////

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType
#if defined (GUI_SUPPORT)
                                  ,CBDataType>::BitTorrent_TrackerStreamHandler_T (SessionInterfaceType* interfaceHandle_in,
                                                                                   CBDataType* CBData_in)
#else
                                  >::BitTorrent_TrackerStreamHandler_T (SessionInterfaceType* interfaceHandle_in)
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , session_ (interfaceHandle_in)
#else
 : session_ (interfaceHandle_in)
#endif // GUI_SUPPORT
 , sessionData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::BitTorrent_TrackerStreamHandler_T"));

  // sanity check(s)
  ACE_ASSERT (session_);
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType
#if defined (GUI_SUPPORT)
                                  ,CBDataType>::start (Stream_SessionId_t sessionId_in,
#else
                                  >::start (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                                       const SessionDataType& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::start"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (iterator == sessionData_.end ());
  ACE_ASSERT (session_);

  sessionData_.insert (std::make_pair (sessionId_in,
                                       &const_cast<SessionDataType&> (sessionData_in)));

  try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    session_->trackerConnect (reinterpret_cast<Net_ConnectionId_t> (sessionData_in.connectionStates.begin ()->second->handle));
#else
    session_->trackerConnect (static_cast<Net_ConnectionId_t> (sessionData_in.connectionStates.begin ()->second->handle));
#endif // ACE_WIN32 || ACE_WIN64
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::trackerConnect(), continuing\n")));
  }
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType
#if defined (GUI_SUPPORT)
                                  ,CBDataType>::notify (Stream_SessionId_t sessionId_in,
#else
                                  >::notify (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                                        const enum Stream_SessionMessageType& sessionEvent_in,
                                                        bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType
#if defined (GUI_SUPPORT)
                                  ,CBDataType>::end (Stream_SessionId_t sessionId_in)
#else
                                  >::end (Stream_SessionId_t sessionId_in)
#endif // GUI_SUPPORT
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::end"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (iterator != sessionData_.end ());
  ACE_ASSERT (session_);

  ACE_HANDLE handle_h = (*iterator).second->connectionStates.begin ()->second->handle;

  sessionData_.erase (iterator);

  try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    session_->trackerDisconnect (reinterpret_cast<Net_ConnectionId_t> (handle_h));
#else
    session_->trackerDisconnect (static_cast<Net_ConnectionId_t> (handle_h));
#endif // ACE_WIN32 || ACE_WIN64
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::trackerDisconnect(), continuing\n")));
  }
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType
#if defined (GUI_SUPPORT)
                                  ,CBDataType>::notify (Stream_SessionId_t sessionId_in,
#else
                                  >::notify (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                             const BitTorrent_TrackerMessage_T<Stream_SessionData_T<SessionDataType>,
                                                                               UserDataType>& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::notify"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (iterator != sessionData_.end ());
  ACE_ASSERT (session_);

  ACE_HANDLE handle_h =
      (*iterator).second->connectionStates.begin ()->second->handle;

  const typename MESSAGE_T::DATA_T& data_container_r = message_in.getR ();
  const struct HTTP_Record& record_r = data_container_r.getR ();

  // handle redirects
  switch (record_r.status)
  {
    case HTTP_Codes::HTTP_STATUS_OK:
      break;
    case HTTP_Codes::HTTP_STATUS_MULTIPLECHOICES:
    case HTTP_Codes::HTTP_STATUS_MOVEDPERMANENTLY:
    case HTTP_Codes::HTTP_STATUS_MOVEDTEMPORARILY:
    case HTTP_Codes::HTTP_STATUS_NOTMODIFIED:
    case HTTP_Codes::HTTP_STATUS_USEPROXY:
    case HTTP_Codes::HTTP_STATUS_SWITCHPROXY:
    case HTTP_Codes::HTTP_STATUS_TEMPORARYREDIRECT:
    case HTTP_Codes::HTTP_STATUS_PERMANENTREDIRECT:
    {
      // step1: redirected --> extract location
      HTTP_HeadersConstIterator_t iterator_2 =
          record_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_LOCATION_STRING));
      if (iterator_2 == record_r.headers.end ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("missing \"%s\" HTTP header, returning\n"),
                    ACE_TEXT (HTTP_PRT_HEADER_LOCATION_STRING)));
        return;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("tracker has been redirected to \"%s\" (status was: %d)\n"),
                  ACE_TEXT ((*iterator_2).second.c_str ()),
                  record_r.status));
      try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        session_->trackerRedirect (reinterpret_cast<Net_ConnectionId_t> (handle_h),
#else
        session_->trackerRedirect (static_cast<Net_ConnectionId_t> (handle_h),
#endif // ACE_WIN32 || ACE_WIN64
                                   (*iterator_2).second);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in BitTorrent_ISession_T::trackerRedirect(), returning\n")));
      }
      return;
    }
    default:
    {
      try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        session_->trackerError (reinterpret_cast<Net_ConnectionId_t> (handle_h),
#else
        session_->trackerError (static_cast<Net_ConnectionId_t> (handle_h),
#endif // ACE_WIN32 || ACE_WIN64
                                record_r);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in BitTorrent_ISession_T::trackerError(), returning\n")));
      }
      return;
    }
  } // end SWITCH

  // parse bencoded record
  struct Common_FlexBisonParserConfiguration parser_configuration = *configuration_;
  parser_configuration.block = false;
  parser_configuration.messageQueue = NULL;
#if defined (_DEBUG)
  PARSER_T parser (parser_configuration.debugScanner,
                   parser_configuration.debugParser);
#else
  PARSER_T parser (false, false);
#endif // _DEBUG
  if (!parser.initialize (parser_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Bencoding_ParserDriver_T::initialize(), returning\n")));
    return;
  } // end IF
  if (!parser.parse (&const_cast<MESSAGE_T&> (message_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_ParserBase_T::parse(), returning\n")));
    try {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      session_->trackerError (reinterpret_cast<Net_ConnectionId_t> (handle_h),
#else
      session_->trackerError (static_cast<Net_ConnectionId_t> (handle_h),
#endif // ACE_WIN32 || ACE_WIN64
                              record_r);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in BitTorrent_ISession_T::trackerError(), returning\n")));
    }
    return;
  } // end IF
  ACE_ASSERT (parser.bencoding_);
  ACE_ASSERT (parser.bencoding_->type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
//#if defined (_DEBUG)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s\n"),
//              ACE_TEXT (Common_Parser_Bencoding_Tools::DictionaryToString (*parser.bencoding_).c_str ())));
//#endif // _DEBUG
  try {
    session_->notify (*parser.bencoding_->dictionary);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_ISession_T::notify(), continuing\n")));
  }
  delete parser.bencoding_; parser.bencoding_ = NULL;
}

template <typename SessionDataType,
          typename UserDataType,
          typename SessionInterfaceType
#if defined (GUI_SUPPORT)
          ,typename CBDataType>
#else
          >
#endif // GUI_SUPPORT
void
BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                  UserDataType,
                                  SessionInterfaceType
#if defined (GUI_SUPPORT)
                                  ,CBDataType>::notify (Stream_SessionId_t sessionId_in,
#else
                                  >::notify (Stream_SessionId_t sessionId_in,
#endif // GUI_SUPPORT
                                                        const BitTorrent_SessionMessage_T<SessionDataType,
                                                                                          UserDataType>& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_TrackerStreamHandler_T::notify"));

  SESSION_DATA_ITERATOR_T iterator = sessionData_.find (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (iterator != sessionData_.end ());
}
