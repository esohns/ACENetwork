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

#ifndef BITTORRENT_CLIENT_STREAM_COMMON_H
#define BITTORRENT_CLIENT_STREAM_COMMON_H

#include "stream_cachedmessageallocator.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"
#include "bittorrent_stream_common.h"
#include "bittorrent_streamhandler.h"

#include "test_i_gtk_common.h"

struct BitTorrent_Client_ConnectionState;
struct BitTorrent_Client_SessionData
 : BitTorrent_SessionData
{
  inline BitTorrent_Client_SessionData ()
   : BitTorrent_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};

  struct BitTorrent_Client_ConnectionState* connectionState;

  struct BitTorrent_Client_UserData*        userData;
};
typedef Stream_SessionData_T<struct BitTorrent_Client_SessionData> BitTorrent_Client_SessionData_t;

template <typename SessionDataType>
class BitTorrent_Message_T;
typedef BitTorrent_Message_T<struct BitTorrent_Client_SessionData> BitTorrent_Client_Message_t;
class BitTorrent_Client_SessionMessage;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct BitTorrent_Client_SessionData,
                                    enum Stream_SessionMessageType,
                                    BitTorrent_Client_Message_t,
                                    BitTorrent_Client_SessionMessage> BitTorrent_Client_IStreamNotify_t;

struct BitTorrent_Client_StreamConfiguration;
struct BitTorrent_Client_UserData;
struct BitTorrent_Client_ModuleHandlerConfiguration
 : BitTorrent_ModuleHandlerConfiguration
{
  inline BitTorrent_Client_ModuleHandlerConfiguration ()
   : BitTorrent_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , userData (NULL)
  {};

  struct BitTorrent_Client_StreamConfiguration* streamConfiguration;
  BitTorrent_Client_IStreamNotify_t*            subscriber; // (initial) subscriber

  struct BitTorrent_Client_UserData*            userData;
};

struct BitTorrent_Client_StreamState
 : BitTorrent_StreamState
{
  inline BitTorrent_Client_StreamState ()
   : BitTorrent_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct BitTorrent_Client_SessionData* currentSessionData;

  struct BitTorrent_Client_UserData*    userData;
};

struct BitTorrent_Client_StreamConfiguration
 : BitTorrent_StreamConfiguration
{
  inline BitTorrent_Client_StreamConfiguration ()
   : BitTorrent_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
   , userData (NULL)
  {}

  struct BitTorrent_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration; // module handler configuration

  struct BitTorrent_Client_UserData*                   userData;
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                struct Stream_AllocatorConfiguration,
                                BitTorrent_Client_Message_t,
                                BitTorrent_Client_SessionMessage> BitTorrent_Client_ControlMessage_t;

typedef Stream_CachedMessageAllocator_T<struct Stream_AllocatorConfiguration,
                                        BitTorrent_Client_ControlMessage_t,
                                        BitTorrent_Client_Message_t,
                                        BitTorrent_Client_SessionMessage> BitTorrent_Client_MessageAllocator_t;

typedef BitTorrent_StreamHandler_T<struct BitTorrent_Client_SessionData,
                                   struct Test_I_GTK_CBData> BitTorrent_StreamHandler_t;

#endif
