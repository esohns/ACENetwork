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

#include <ace/Synch_Traits.h>

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_cachedmessageallocator.h"
#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "irc_icontrol.h"
//#include "irc_message.h"
#include "irc_record.h"
#include "irc_stream_common.h"

#include "IRC_client_common.h"

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

struct IRC_Client_StreamState
 : IRC_StreamState
{
  inline IRC_Client_StreamState ()
   : IRC_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct IRC_Client_SessionData* currentSessionData;
  struct IRC_Client_UserData*    userData;
};

typedef Stream_INotify_T<enum Stream_SessionMessageType> IRC_Client_IStreamNotify_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct IRC_Client_SessionData,
                                    enum Stream_SessionMessageType,
                                    IRC_Message,
                                    IRC_Client_SessionMessage> IRC_Client_ISessionNotify_t;
typedef std::list<IRC_Client_ISessionNotify_t*> IRC_Client_ISubscribers_t;
typedef IRC_Client_ISubscribers_t::const_iterator IRC_Client_ISubscribersIterator_t;
struct IRC_Client_ModuleHandlerConfiguration
 : IRC_ModuleHandlerConfiguration
{
  inline IRC_Client_ModuleHandlerConfiguration ()
   : IRC_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
   , subscriber (NULL)
   , subscribers (NULL)
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  /* handler */
  IRC_Client_ISessionNotify_t*               subscriber; // (initial) subscriber
  IRC_Client_ISubscribers_t*                 subscribers;

  struct IRC_Client_ConnectionConfiguration* connectionConfiguration;

  struct IRC_Client_UserData*                userData;
};

struct IRC_Client_StreamConfiguration
 : Stream_Configuration
{
  inline IRC_Client_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
   , protocolConfiguration (NULL)
   , userData (NULL)
  {};

  struct Stream_ModuleConfiguration*            moduleConfiguration;        // stream module configuration
  struct IRC_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration; // module handler configuration
  struct IRC_ProtocolConfiguration*             protocolConfiguration;      // protocol configuration

  struct IRC_Client_UserData*                   userData;
};

typedef Stream_SessionData_T<struct IRC_Client_SessionData> IRC_Client_SessionData_t;

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> IRC_Client_ControlMessage_t;

typedef Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
                                        struct Stream_AllocatorConfiguration,
                                        IRC_Client_ControlMessage_t,
                                        IRC_Message,
                                        IRC_Client_SessionMessage> IRC_Client_MessageAllocator_t;

//typedef Common_INotify_T<unsigned int,
//                         struct IRC_Client_SessionData,
//                         IRC_Message,
//                         IRC_Client_SessionMessage> IRC_Client_IStreamNotify_t;
//typedef IRC_IControl_T<IRC_Client_IStreamNotify_t> IRC_Client_IControl_t;

#endif
