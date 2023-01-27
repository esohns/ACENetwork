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

#include <ace/Synch_Traits.h>

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data.h"

#include "ppspp_common.h"

// forward declarations
struct HTTP_ConnectionState;
struct HTTP_ModuleHandlerConfiguration;
//template <typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename SessionMessageType,
//          typename DataType>
//class HTTP_Message_T;
//class HTTP_SessionMessage;
//template <typename StreamStateType,
//          typename ConfigurationType,
//          typename StatisticContainerType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType>
//class HTTP_Stream_T;
struct HTTP_StreamConfiguration;

//typedef Stream_IModuleHandler_T<HTTP_ModuleHandlerConfiguration> HTTP_IModuleHandler_t;
//typedef Stream_IModule_T<ACE_MT_SYNCH,
//                         Common_TimePolicy_t,
//                         Stream_ModuleConfiguration,
//                         HTTP_ModuleHandlerConfiguration> HTTP_IModule_t;
//typedef HTTP_Message_T<Stream_AllocatorConfiguration,
//                       HTTP_MessageData_t> HTTP_Message_t;

struct HTTP_Stream_SessionData
 : Stream_SessionData
{
  inline HTTP_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   //, currentStatistic ()
  {};

  HTTP_ConnectionState*   connectionState;

  //HTTP_RuntimeStatistic_t currentStatistic;
};

struct HTTP_Stream_UserData
 : Stream_UserData
{
  inline HTTP_Stream_UserData ()
   : Stream_UserData ()
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  // *TODO*: remove these ASAP
  Stream_ModuleConfiguration*     moduleConfiguration;
  HTTP_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct HTTP_StreamState
 : Stream_State
{
  inline HTTP_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  HTTP_Stream_SessionData* currentSessionData;
  HTTP_Stream_UserData*    userData;
};

typedef Stream_SessionData_T<HTTP_Stream_SessionData> HTTP_Stream_SessionData_t;

//typedef Common_INotify_T<unsigned int,
//                         HTTP_Stream_SessionData,
//                         HTTP_Record,
//                         HTTP_SessionMessage> HTTP_IStreamNotify_t;
typedef Stream_INotify_T<Stream_SessionMessageType> HTTP_Stream_INotify_t;

//typedef HTTP_Stream_T<HTTP_StreamState,
//                      HTTP_StreamConfiguration,
//                      HTTP_RuntimeStatistic_t,
//                      HTTP_ModuleHandlerConfiguration,
//                      HTTP_Stream_SessionData,
//                      HTTP_Stream_SessionData_t,
//                      ACE_Message_Block,
//                      HTTP_Message_t,
//                      HTTP_SessionMessage> HTTP_Stream_t;

#endif
