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

#include <ace/Synch_Traits.h>

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data.h"

#include "bittorrent_common.h"
//#include "bittorrent_icontrol.h"
#include "bittorrent_stream_common.h"

#include "bittorrent_client_common.h"

// forward declaration(s)
struct BitTorrent_Client_SessionData;
//class BitTorrent_Client_SessionMessage;
struct BitTorrent_Client_UserData;

struct BitTorrent_Client_StreamState
 : BitTorrent_StreamState
{
  inline BitTorrent_Client_StreamState ()
   : BitTorrent_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  BitTorrent_Client_SessionData* currentSessionData;
  BitTorrent_Client_UserData*    userData;
};

typedef Stream_SessionData_T<BitTorrent_Client_SessionData> BitTorrent_Client_SessionData_t;

//typedef Common_INotify_T<unsigned int,
//                         BitTorrent_Client_SessionData,
//                         BitTorrent_Message,
//                         BitTorrent_Client_SessionMessage> BitTorrent_Client_IStreamNotify_t;
//typedef BitTorrent_IControl_T<BitTorrent_Client_IStreamNotify_t> BitTorrent_Client_IControl_t;

#endif
