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

#ifndef WEBSOCKET_STREAM_COMMON_H
#define WEBSOCKET_STREAM_COMMON_H

#include "ace/Synch_Traits.h"

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_configuration.h"
#include "stream_imodule.h"
#include "stream_session_data.h"

#include "stream_net_common.h"

#include "net_defines.h"

//#include "http_common.h"

#include "ws_common.h"
#include "ws_message.h"
//#include "ws_stream.h"

// forward declarations
struct WebSocket_ConnectionState;
struct WebSocket_ModuleHandlerConfiguration;
struct WebSocket_StreamConfiguration;

typedef WebSocket_Message_T<struct WebSocket_Record,
                            enum Stream_MessageType> WebSocket_Message_t;

struct WebSocket_SessionData
 : Stream_SessionData
{
  WebSocket_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
  {};

  struct WebSocket_ConnectionState* connectionState;
};
typedef Stream_SessionData_T<struct WebSocket_SessionData> WebSocket_SessionData_t;

struct WebSocket_StreamState
 : Stream_State
{
  WebSocket_StreamState ()
   : Stream_State ()
   //, sessionData (NULL)
  {};

  //struct WebSocket_SessionData* sessionData;
};

//struct WebSocket_ProtocolConfiguration;
struct WebSocket_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  WebSocket_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
  ////////////////////////////////////////
   , parserConfiguration (NULL)
   , waitForConnect (true)
  {
    printFinalReport = true;
  };

  //HTTP_Form_t                           HTTPForm; // HTTP get module
  //HTTP_Headers_t                        HTTPHeaders; // HTTP get module
  struct HTTP_ParserConfiguration* parserConfiguration; // parser module(s)
  bool                             waitForConnect; // HTTP get module
};

//struct WebSocket_ProtocolConfiguration;
struct WebSocket_StreamConfiguration
 : Stream_Net_StreamConfiguration
{
  WebSocket_StreamConfiguration ()
   : Stream_Net_StreamConfiguration ()
   //, protocolConfiguration (NULL)
   //, userData (NULL)
  {};

  //struct WebSocket_ProtocolConfiguration* protocolConfiguration;       // protocol configuration

  //struct WebSocket_Stream_UserData*       userData;
};

//typedef Common_INotify_T<unsigned int,
//                         WebSocket_Stream_SessionData,
//                         WebSocket_Record,
//                         WebSocket_SessionMessage> WebSocket_IStreamNotify_t;
//typedef Stream_INotify_T<enum Stream_SessionMessageType> WebSocket_Stream_INotify_t;

#endif
