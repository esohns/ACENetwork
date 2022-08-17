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

#ifndef HTTP_STREAM_COMMON_H
#define HTTP_STREAM_COMMON_H

#include "ace/Synch_Traits.h"

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_configuration.h"
#include "stream_imodule.h"
#include "stream_session_data.h"

#include "net_defines.h"

#include "http_common.h"
#include "http_message.h"
//#include "http_stream.h"

// forward declarations
struct HTTP_ConnectionState;
struct HTTP_ModuleHandlerConfiguration;
struct HTTP_StreamConfiguration;


typedef HTTP_Message_T<struct HTTP_Record,
                       enum Stream_MessageType> HTTP_Message_t;

struct HTTP_Stream_SessionData
 : Stream_SessionData
{
  HTTP_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   //, statistic ()
  {};

  struct HTTP_ConnectionState*   connectionState;

  //HTTP_Statistic_t statistic;
};
typedef Stream_SessionData_T<struct HTTP_Stream_SessionData> HTTP_Stream_SessionData_t;

struct HTTP_StreamState
 : Stream_State
{
  HTTP_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {};

  struct HTTP_Stream_SessionData* sessionData;
};

//struct HTTP_ProtocolConfiguration;
struct HTTP_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  HTTP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
  ////////////////////////////////////////
   , closeAfterReception (HTTP_DEFAULT_CLOSE_AFTER_RECEPTION)
   , crunchMessages (HTTP_DEFAULT_CRUNCH_MESSAGES)
   , HTTPForm ()
   , HTTPHeaders ()
   , parserConfiguration (NULL)
   , printProgressDot (false)
   , URL ()
   , waitForConnect (true)
  {
    printFinalReport = true;
  };

  bool                             closeAfterReception; // HTTP get module
  bool                             crunchMessages; // HTTP parser module
  HTTP_Form_t                      HTTPForm; // HTTP get module
  HTTP_Headers_t                   HTTPHeaders; // HTTP get module
  struct HTTP_ParserConfiguration* parserConfiguration; // parser module(s)
  bool                             printProgressDot; // file writer module
  std::string                      URL;
  bool                             waitForConnect; // HTTP get module
};

//struct HTTP_ProtocolConfiguration;
struct HTTP_StreamConfiguration
 : Stream_Configuration
{
  HTTP_StreamConfiguration ()
   : Stream_Configuration ()
   //, protocolConfiguration (NULL)
   //, userData (NULL)
  {};

  //struct HTTP_ProtocolConfiguration* protocolConfiguration;       // protocol configuration

  //struct HTTP_Stream_UserData*       userData;
};

//typedef Common_INotify_T<unsigned int,
//                         HTTP_Stream_SessionData,
//                         HTTP_Record,
//                         HTTP_SessionMessage> HTTP_IStreamNotify_t;
//typedef Stream_INotify_T<enum Stream_SessionMessageType> HTTP_Stream_INotify_t;

#endif
