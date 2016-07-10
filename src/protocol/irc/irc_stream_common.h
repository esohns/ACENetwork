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

#ifndef IRC_STREAM_COMMON_H
#define IRC_STREAM_COMMON_H

#include "ace/Synch_Traits.h"

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data.h"

#include "irc_common.h"
#include "irc_record.h"

// forward declarations
struct IRC_ConnectionState;
struct IRC_ModuleHandlerConfiguration;
//struct IRC_State;
class IRC_Message;
class IRC_SessionMessage;
template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class IRC_Stream_T;
struct IRC_StreamConfiguration;

//typedef Stream_IModuleHandler_T<IRC_ModuleHandlerConfiguration> IRC_IModuleHandler_t;
typedef Stream_IModule_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t,
                         Stream_ModuleConfiguration,
                         IRC_ModuleHandlerConfiguration> IRC_IModule_t;

struct IRC_Stream_UserData
 : Stream_UserData
{
  inline IRC_Stream_UserData ()
   : Stream_UserData ()
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  // *TODO*: remove these ASAP
  Stream_ModuleConfiguration*     moduleConfiguration;
  IRC_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct IRC_Stream_SessionData
 : Stream_SessionData
{
  inline IRC_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   //, currentStatistic ()
   , userData (NULL)
  {};

  IRC_ConnectionState*   connectionState;

  //IRC_RuntimeStatistic_t currentStatistic;

  IRC_Stream_UserData*   userData;
};

struct IRC_StreamState
 : Stream_State
{
  inline IRC_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  IRC_Stream_SessionData* currentSessionData;
  IRC_Stream_UserData*    userData;
};

typedef Stream_SessionData_T<IRC_Stream_SessionData> IRC_Stream_SessionData_t;

typedef Common_INotify_T<unsigned int,
                         IRC_Stream_SessionData,
                         IRC_Record,
                         IRC_SessionMessage> IRC_IStreamNotify_t;

//typedef IRC_IControl_T<IRC_IStreamNotify_t> IRC_IControl_t;

typedef IRC_Stream_T<IRC_StreamState,
                     IRC_StreamConfiguration,
                     IRC_RuntimeStatistic_t,
                     IRC_ModuleHandlerConfiguration,
                     IRC_Stream_SessionData,
                     IRC_Stream_SessionData_t,
                     ACE_Message_Block,
                     IRC_Message,
                     IRC_SessionMessage> IRC_Stream_t;

#endif
