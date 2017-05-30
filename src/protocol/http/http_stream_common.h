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

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data.h"

#include "http_common.h"
#include "http_stream.h"

// forward declarations
struct HTTP_ConnectionState;
struct HTTP_ModuleHandlerConfiguration;
struct HTTP_StreamConfiguration;

typedef HTTP_Message_T<struct Stream_AllocatorConfiguration,
                       HTTP_MessageData_t> HTTP_Message_t;
//typedef Stream_ControlMessage_T<enum Stream_ControlType,
//                                struct Stream_AllocatorConfiguration,
//                                HTTP_Message_t,
//                                HTTP_SessionMessage> HTTP_ControlMessage_t;

struct HTTP_Stream_SessionData
 : Stream_SessionData
{
  inline HTTP_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   //, currentStatistic ()
  {};

  struct HTTP_ConnectionState*   connectionState;

  //HTTP_RuntimeStatistic_t currentStatistic;
};
typedef Stream_SessionData_T<struct HTTP_Stream_SessionData> HTTP_Stream_SessionData_t;

struct HTTP_Stream_UserData
 : Stream_UserData
{
  inline HTTP_Stream_UserData ()
   : Stream_UserData ()
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  // *TODO*: remove these ASAP
  struct Stream_ModuleConfiguration*      moduleConfiguration;
  struct HTTP_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct HTTP_StreamState
 : Stream_State
{
  inline HTTP_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct HTTP_Stream_SessionData* currentSessionData;
  struct HTTP_Stream_UserData*    userData;
};

//struct HTTP_ProtocolConfiguration;
struct HTTP_ModuleHandlerConfiguration
 : public Stream_ModuleHandlerConfiguration
{
  inline HTTP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   //////////////////////////////////////
   , printProgressDot (false)
   , pushStatisticMessages (true)
   //, protocolConfiguration (NULL)
   , URL ()
  {
    crunchMessages = HTTP_DEFAULT_CRUNCH_MESSAGES; // http parser module
    printFinalReport = true;
  };

  bool                               printProgressDot; // file writer module
  bool                               pushStatisticMessages;

  //struct HTTP_ProtocolConfiguration* protocolConfiguration;
  std::string                        URL;
};

typedef std::map<std::string,
                 struct HTTP_ModuleHandlerConfiguration*> HTTP_ModuleHandlerConfigurations_t;
typedef HTTP_ModuleHandlerConfigurations_t::iterator HTTP_ModuleHandlerConfigurationsIterator_t;
typedef HTTP_ModuleHandlerConfigurations_t::const_iterator HTTP_ModuleHandlerConfigurationsConstIterator_t;

//struct HTTP_ProtocolConfiguration;
struct HTTP_StreamConfiguration
 : Stream_Configuration
{
  inline HTTP_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration ()
   , moduleHandlerConfigurations ()
   //, protocolConfiguration (NULL)
   , userData (NULL)
  {};

  struct Stream_ModuleConfiguration* moduleConfiguration;         // stream module configuration
  HTTP_ModuleHandlerConfigurations_t moduleHandlerConfigurations; // module handler configuration
  //struct HTTP_ProtocolConfiguration* protocolConfiguration;       // protocol configuration

  struct HTTP_Stream_UserData*            userData;
};

//typedef Common_INotify_T<unsigned int,
//                         HTTP_Stream_SessionData,
//                         HTTP_Record,
//                         HTTP_SessionMessage> HTTP_IStreamNotify_t;
typedef Stream_INotify_T<enum Stream_SessionMessageType> HTTP_Stream_INotify_t;

typedef HTTP_Stream_T<struct HTTP_StreamState,
                      struct HTTP_StreamConfiguration,
                      HTTP_RuntimeStatistic_t,
                      struct HTTP_ModuleHandlerConfiguration,
                      struct HTTP_Stream_SessionData,
                      HTTP_Stream_SessionData_t,
                      ACE_Message_Block,
                      HTTP_Message_t,
                      HTTP_SessionMessage,
                      struct HTTP_Stream_UserData> HTTP_Stream_t;

#endif
