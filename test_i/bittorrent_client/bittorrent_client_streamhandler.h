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

#ifndef BITTORRENT_CLIENT_STREAMHANDLER_H
#define BITTORRENT_CLIENT_STREAMHANDLER_H

#include <map>

#include "ace/Global_Macros.h"

#include "common.h"
#include "common_iinitialize.h"

#include "stream_common.h"
#include "stream_isessionnotify.h"

#include "bittorrent_common.h"
#include "bittorrent_stream_common.h"
#include "bittorrent_streamhandler.h"

// forward declarations
template <typename SessionDataType,
          typename UserDataType>
class BitTorrent_Message_T;
template <typename SessionDataType,
          typename UserDataType>
class BitTorrent_SessionMessage_T;
template <typename SessionMessageType>
class BitTorrent_Bencoding_ParserDriver_T;

template <typename SessionDataType,     // not (!) reference-counted
          typename UserDataType,
          typename SessionInterfaceType // derived from Net_ISession_T
          ////////////////////////////////
#if defined (GUI_SUPPORT)
          ,typename CBDataType>         // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
class BitTorrent_Client_PeerStreamHandler_T
 : public BitTorrent_PeerStreamHandler_T<SessionDataType,
                                         UserDataType,
                                         SessionInterfaceType
#if defined (GUI_SUPPORT)
                                         ,CBDataType>
#else
                                         >
#endif // GUI_SUPPORT
{
 public:
  BitTorrent_Client_PeerStreamHandler_T (SessionInterfaceType* // session handle
#if defined (GUI_SUPPORT)
                                         ,CBDataType* = NULL); // ui feedback data handle
#else
                                        );
#endif // GUI_SUPPORT
  inline virtual ~BitTorrent_Client_PeerStreamHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,                         // session id
                       const BitTorrent_Message_T<Stream_SessionData_T<SessionDataType>,
                                                  UserDataType>&); // (protocol) message
  virtual void notify (Stream_SessionId_t,                                // session id
                       const BitTorrent_SessionMessage_T<SessionDataType,
                                                         UserDataType>&); // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerStreamHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerStreamHandler_T (const BitTorrent_Client_PeerStreamHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_PeerStreamHandler_T& operator= (const BitTorrent_Client_PeerStreamHandler_T&))

  typedef std::map<Stream_SessionId_t, SessionDataType*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;

#if defined (GUI_SUPPORT)
  CBDataType*           CBData_;
#endif // GUI_SUPPORT
  SessionInterfaceType* session_;
  SESSION_DATA_T        sessionData_;
};

//////////////////////////////////////////

template <typename SessionDataType,     // not (!) reference-counted
          typename UserDataType,
          typename SessionInterfaceType // derived from Net_ISession_T
          ////////////////////////////////
#if defined (GUI_SUPPORT)
          ,typename CBDataType>         // ui feedback data type
#else
          >
#endif // GUI_SUPPORT
class BitTorrent_Client_TrackerStreamHandler_T
 : public BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                            UserDataType,
                                            SessionInterfaceType
#if defined (GUI_SUPPORT)
                                            ,CBDataType>
#else
                                            >
#endif // GUI_SUPPORT
 //, public Common_IInitialize_T<struct Common_ParserConfiguration>
{
 typedef BitTorrent_TrackerStreamHandler_T<SessionDataType,
                                           UserDataType,
                                           SessionInterfaceType
#if defined (GUI_SUPPORT)
                                           ,CBDataType> inherited;
#else
                                           > inherited;
#endif // GUI_SUPPORT

 public:
  BitTorrent_Client_TrackerStreamHandler_T (SessionInterfaceType* // session handle
#if defined (GUI_SUPPORT)
                                            ,CBDataType* = NULL); // ui feedback data handle
#else
                                           );
#endif // GUI_SUPPORT
  inline virtual ~BitTorrent_Client_TrackerStreamHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,                                // session id
                       const BitTorrent_TrackerMessage_T<Stream_SessionData_T<SessionDataType>,
                                                         UserDataType>&); // (protocol) message
  virtual void notify (Stream_SessionId_t,                                // session id
                       const BitTorrent_SessionMessage_T<SessionDataType,
                                                         UserDataType>&); // session message

  // implement Common_IInitialize_T
  //inline virtual bool initialize (const struct Common_ParserConfiguration& configuration_in) { configuration_ = &const_cast<struct Common_ParserConfiguration&> (configuration_in); return true; }

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerStreamHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerStreamHandler_T (const BitTorrent_Client_TrackerStreamHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Client_TrackerStreamHandler_T& operator= (const BitTorrent_Client_TrackerStreamHandler_T&))

  // convenient types
  typedef std::map<Stream_SessionId_t, SessionDataType*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;
  typedef BitTorrent_TrackerMessage_T<Stream_SessionData_T<SessionDataType>,
                                      UserDataType> MESSAGE_T;
  typedef BitTorrent_SessionMessage_T<SessionDataType,
                                      UserDataType> SESSION_MESSAGE_T;
  typedef BitTorrent_Bencoding_ParserDriver_T<SESSION_MESSAGE_T> PARSER_T;

//  struct Common_ParserConfiguration* configuration_;
#if defined (GUI_SUPPORT)
  CBDataType*                        CBData_;
#endif // GUI_SUPPORT
  SessionInterfaceType*              session_;
  SESSION_DATA_T                     sessionData_;
};

// include template definition
#include "bittorrent_client_streamhandler.inl"

#endif
