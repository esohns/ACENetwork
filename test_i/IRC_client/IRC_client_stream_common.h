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

#ifndef IRC_CLIENT_STREAM_COMMON_H
#define IRC_CLIENT_STREAM_COMMON_H

#include <list>
#include <map>
#include <string>

#include "ace/Synch_Traits.h"

#include "common_inotify.h"
#include "common_time_common.h"

#include "common_timer_manager_common.h"

#include "stream_cachedmessageallocator.h"
#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_configuration.h"

#include "irc_icontrol.h"
#include "irc_common.h"
#include "irc_configuration.h"
#include "irc_record.h"
#include "irc_stream_common.h"
#include "irc_network.h"

#include "test_i_stream_common.h"

// forward declaration(s)
struct IRC_Client_SessionData;
class IRC_Client_SessionMessage;
struct IRC_Client_UserData;
class IRC_Message;

//typedef Stream_IModuleHandler_T<struct IRC_Client_ModuleHandlerConfiguration> IRC_Client_IModuleHandler_t;
//typedef Stream_IModule_T<ACE_MT_SYNCH,
//                         Common_TimePolicy_t,
//                         struct Stream_ModuleConfiguration,
//                         struct IRC_Client_ModuleHandlerConfiguration> IRC_Client_IModule_t;

//struct IRC_Client_SessionState;
struct IRC_Client_SessionData
 : Test_I_StreamSessionData
{
  IRC_Client_SessionData ()
   : Test_I_StreamSessionData ()
   , connection (NULL)
   , sessionState (NULL)
  {}

  IRC_IConnection_t*       connection;
  struct IRC_SessionState* sessionState;
};
typedef Stream_SessionData_T<struct IRC_Client_SessionData> IRC_Client_SessionData_t;

struct IRC_Client_StreamState
 : IRC_StreamState
{
  IRC_Client_StreamState ()
   : IRC_StreamState ()
   , sessionData (NULL)
  {}

  struct IRC_Client_SessionData* sessionData;
};

//struct IRC_Client_ConnectionConfiguration;
//typedef std::map<std::string,
//                 struct IRC_Client_ConnectionConfiguration> IRC_Client_ConnectionConfigurations_t;
//typedef IRC_Client_ConnectionConfigurations_t::iterator IRC_Client_ConnectionConfigurationIterator_t;
//typedef Stream_INotify_T<enum Stream_SessionMessageType> IRC_Client_IStreamNotify_t;
typedef Stream_ISessionDataNotify_T<struct IRC_Client_SessionData,
                                    enum Stream_SessionMessageType,
                                    IRC_Message,
                                    IRC_Client_SessionMessage> IRC_Client_ISessionNotify_t;
typedef std::list<IRC_Client_ISessionNotify_t*> IRC_Client_ISubscribers_t;
typedef IRC_Client_ISubscribers_t::const_iterator IRC_Client_ISubscribersIterator_t;
//extern const char stream_name_string_[];
struct IRC_Client_StreamConfiguration;
struct IRC_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct IRC_Client_StreamConfiguration,
                               struct IRC_Client_ModuleHandlerConfiguration> IRC_Client_StreamConfiguration_t;
//typedef Net_ConnectionConfiguration_T<struct IRC_AllocatorConfiguration,
//                                      IRC_Client_StreamConfiguration_t,
//                                      NET_TRANSPORTLAYER_TCP> IRC_Client_ConnectionConfiguration_t;
//typedef std::map<std::string,
//                 IRC_Client_ConnectionConfiguration_t> IRC_Client_ConnectionConfigurations_t;
struct IRC_Client_ModuleHandlerConfiguration
 : IRC_ModuleHandlerConfiguration
{
  IRC_Client_ModuleHandlerConfiguration ()
   : IRC_ModuleHandlerConfiguration ()
  ////////////////////////////////////////
   , connectionConfigurations (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

  Net_ConnectionConfigurations_t*   connectionConfigurations;
  IRC_Client_StreamConfiguration_t* streamConfiguration;
  IRC_Client_ISessionNotify_t*      subscriber; // (initial) subscriber
  IRC_Client_ISubscribers_t*        subscribers;
};

struct IRC_Client_StreamConfiguration
 : IRC_StreamConfiguration
{
  IRC_Client_StreamConfiguration ()
   : IRC_StreamConfiguration ()
  {}
};

typedef Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
                                        struct Common_AllocatorConfiguration,
                                        Stream_ControlMessage_t,
                                        IRC_Message,
                                        IRC_Client_SessionMessage> IRC_Client_MessageAllocator_t;

#endif
