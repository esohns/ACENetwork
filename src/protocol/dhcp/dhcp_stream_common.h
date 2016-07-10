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

#ifndef DHCP_STREAM_COMMON_H
#define DHCP_STREAM_COMMON_H

#include "ace/Synch_Traits.h"

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data.h"

#include "dhcp_common.h"

// forward declarations
struct DHCP_ConnectionState;
struct DHCP_ModuleHandlerConfiguration;
template <typename AllocatorConfigurationType>
class DHCP_Message_T;
class DHCP_SessionMessage;
template <typename StreamStateType,
          typename ConfigurationType,
          typename StatisticContainerType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class DHCP_Stream_T;
struct DHCP_StreamConfiguration;

//typedef Stream_IModuleHandler_T<DHCP_ModuleHandlerConfiguration> DHCP_IModuleHandler_t;
typedef Stream_IModule_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t,
                         Stream_ModuleConfiguration,
                         DHCP_ModuleHandlerConfiguration> DHCP_IModule_t;
typedef DHCP_Message_T<Stream_AllocatorConfiguration> DHCP_Message_t;

struct DHCP_Stream_SessionData
 : Stream_SessionData
{
  inline DHCP_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   //, currentStatistic ()
  {};

  DHCP_ConnectionState*   connectionState;

  //DHCP_RuntimeStatistic_t currentStatistic;
};

struct DHCP_Stream_UserData
 : Stream_UserData
{
  inline DHCP_Stream_UserData ()
   : Stream_UserData ()
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  // *TODO*: remove these ASAP
  Stream_ModuleConfiguration*     moduleConfiguration;
  DHCP_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct DHCP_StreamState
 : Stream_State
{
  inline DHCP_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  DHCP_Stream_SessionData* currentSessionData;
  DHCP_Stream_UserData*    userData;
};

typedef Stream_SessionData_T<DHCP_Stream_SessionData> DHCP_Stream_SessionData_t;

typedef Common_INotify_T<unsigned int,
                         DHCP_Stream_SessionData,
                         DHCP_Record,
                         DHCP_SessionMessage> DHCP_IStreamNotify_t;

typedef DHCP_Stream_T<DHCP_StreamState,
                      DHCP_StreamConfiguration,
                      DHCP_RuntimeStatistic_t,
                      DHCP_ModuleHandlerConfiguration,
                      DHCP_Stream_SessionData,
                      DHCP_Stream_SessionData_t,
                      ACE_Message_Block,
                      DHCP_Message_t,
                      DHCP_SessionMessage> DHCP_Stream_t;

#endif
