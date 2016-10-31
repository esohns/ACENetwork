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

#ifndef BITTORRENT_STREAMHANDLER_H
#define BITTORRENT_STREAMHANDLER_H

#include <map>

#include <ace/Global_Macros.h>

#include "stream_common.h"
#include "stream_isessionnotify.h"

#include "bittorrent_common.h"
#include "bittorrent_stream_common.h"

// forward declarations
template <typename SessionDataType>
class BitTorrent_Message_T;
template <typename SessionDataType>
class BitTorrent_SessionMessage_T;

template <typename SessionDataType,
          ////////////////////////////////
          typename CBDataType> // gtk ui feedback
class BitTorrent_StreamHandler_T
 : public Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                      SessionDataType,
                                      enum Stream_SessionMessageType,
                                      BitTorrent_Message_T<SessionDataType>,
                                      BitTorrent_SessionMessage_T<SessionDataType>>
{
 public:
  BitTorrent_StreamHandler_T ();
  virtual ~BitTorrent_StreamHandler_T ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,                            // session id
                       const BitTorrent_Message_T<SessionDataType>&); // (protocol) message
  virtual void notify (Stream_SessionId_t,                                   // session id
                       const BitTorrent_SessionMessage_T<SessionDataType>&); // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_StreamHandler_T (const BitTorrent_StreamHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_StreamHandler_T& operator= (const BitTorrent_StreamHandler_T&))

  typedef std::map<Stream_SessionId_t, SessionDataType*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;

  CBDataType*    CBData_;
  SESSION_DATA_T sessionData_;
};

// include template definition
#include "bittorrent_streamhandler.inl"

#endif
