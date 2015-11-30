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

#ifndef NET_STREAM_COMMON_H
#define NET_STREAM_COMMON_H

#include <list>

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_inotify.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data.h"
#include "stream_messageallocatorheap_base.h"

#include "test_u_message.h"

// forward declarations
typedef Stream_Statistic Net_RuntimeStatistic_t;
struct Net_Configuration;
struct Net_ConnectionState;
class Net_SessionMessage;
struct Net_UserData;

struct Net_StreamSessionData
 : Stream_SessionData
{
  inline Net_StreamSessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , currentStatistic ()
  {};

  Net_ConnectionState*   connectionState;

  Net_RuntimeStatistic_t currentStatistic;
};

struct Net_StreamState
 : Stream_State
{
  inline Net_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Net_StreamSessionData* currentSessionData;
  Net_UserData*          userData;
};

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,

                                          Net_Message,
                                          Net_SessionMessage> Net_StreamMessageAllocator_t;

typedef Stream_SessionData_T<Net_StreamSessionData> Net_StreamSessionData_t;

typedef Stream_IModuleHandler_T<Stream_ModuleHandlerConfiguration> Net_IModuleHandler_t;
typedef Stream_IModule_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t,
                         Stream_ModuleConfiguration,
                         Stream_ModuleHandlerConfiguration> Net_IModule_t;

typedef Common_INotify_T<Net_StreamSessionData,
                         Net_Message,
                         Net_SessionMessage> Net_IStreamNotify_t;
typedef std::list<Net_IStreamNotify_t*> Net_Subscribers_t;
typedef Net_Subscribers_t::iterator Net_SubscribersIterator_t;

typedef Common_ISubscribe_T<Net_IStreamNotify_t> Net_ISubscribe_t;

#endif
