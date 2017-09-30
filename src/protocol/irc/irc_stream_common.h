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
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_iallocator.h"
#include "stream_imodule.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "irc_common.h"
#include "irc_message.h"
#include "irc_record.h"
#include "irc_sessionmessage.h"

// forward declarations
struct IRC_ConnectionState;
struct IRC_ModuleHandlerConfiguration;
struct IRC_Stream_SessionData;
typedef Stream_SessionData_T<struct IRC_Stream_SessionData> IRC_Stream_SessionData_t;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct IRC_AllocatorConfiguration> IRC_ControlMessage_t;

typedef IRC_SessionMessage_T<IRC_Stream_SessionData_t,
                             struct Stream_UserData> IRC_SessionMessage_t;
template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
class IRC_Stream_T;
struct IRC_StreamConfiguration;

//typedef Stream_IModuleHandler_T<struct IRC_ModuleHandlerConfiguration> IRC_IModuleHandler_t;
//typedef Stream_IModule_T<ACE_MT_SYNCH,
//                         Common_TimePolicy_t,
//                         struct Stream_ModuleConfiguration,
//                         struct IRC_ModuleHandlerConfiguration> IRC_IModule_t;
typedef Stream_INotify_T<enum Stream_SessionMessageType> IRC_Stream_INotify_t;

//struct IRC_UserData
// : Stream_UserData
//{
//  inline IRC_UserData ()
//   : Stream_UserData ()
//   , moduleConfiguration (NULL)
//   , moduleHandlerConfiguration (NULL)
//  {};
//
//  // *TODO*: remove these ASAP
//  struct Stream_ModuleConfiguration*     moduleConfiguration;
//  struct IRC_ModuleHandlerConfiguration* moduleHandlerConfiguration;
//};

struct IRC_Stream_SessionData
 : Stream_SessionData
{
  IRC_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , statistic ()
   //, userData (NULL)
  {};

  struct IRC_ConnectionState* connectionState;

  IRC_Statistic_t             statistic;

  //struct IRC_UserData*        userData;
};

struct IRC_StreamState
 : Stream_State
{
  IRC_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   //, userData (NULL)
  {};

  struct IRC_Stream_SessionData* currentSessionData;

  //struct IRC_UserData*           userData;
};

//////////////////////////////////////////

typedef Stream_SessionData_T<struct IRC_Stream_SessionData> IRC_Stream_SessionData_t;

//typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
//                                struct Stream_AllocatorConfiguration,
//                                IRC_Message,
//                                IRC_SessionMessage_t> IRC_ControlMessage_t;

//typedef Common_INotify_T<unsigned int,
//                         struct IRC_Stream_SessionData,
//                         IRC_Record,
//                         IRC_SessionMessage> IRC_IStreamNotify_t;

//typedef IRC_IControl_T<IRC_IStreamNotify_t> IRC_IControl_t;

typedef IRC_Stream_T<struct IRC_StreamState,
                     struct IRC_StreamConfiguration,
                     IRC_Statistic_t,
                     Common_Timer_Manager_t,
                     struct IRC_ModuleHandlerConfiguration,
                     struct IRC_Stream_SessionData,
                     IRC_Stream_SessionData_t,
                     ACE_Message_Block,
                     IRC_Message,
                     IRC_SessionMessage_t,
                     struct Stream_UserData> IRC_Stream_t;
//typedef IRC_Stream_T<struct IRC_StreamState,
//                     struct IRC_StreamConfiguration,
//                     IRC_Statistic_t,
//                     IRC_StatisticHandler_t,
//                     struct IRC_ModuleHandlerConfiguration,
//                     struct IRC_Stream_SessionData,
//                     IRC_Stream_SessionData_t,
//                     ACE_Message_Block,
//                     IRC_Message,
//                     IRC_SessionMessage_t,
//                     struct Stream_UserData> IRC_AsynchStream_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct IRC_Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    IRC_Message,
                                    IRC_SessionMessage_t> IRC_ISessionNotify_t;

#endif
