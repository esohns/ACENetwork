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

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data_base.h"

#include "IRC_client_configuration.h"
#include "IRC_client_IRCmessage.h"
//#include "IRC_client_sessionmessage.h"

// forward declarations
struct IRC_Client_ConnectionState;
class IRC_Client_SessionMessage;

typedef Stream_IModuleHandler_T<IRC_Client_ModuleHandlerConfiguration> IRC_Client_IModuleHandler_t;
typedef Stream_IModule_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t,
                         Stream_ModuleConfiguration,
                         IRC_Client_ModuleHandlerConfiguration> IRC_Client_IModule_t;

struct IRC_Client_StreamSessionData
 : Stream_SessionData
{
  inline IRC_Client_StreamSessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , currentStatistic ()
  {};

  IRC_Client_ConnectionState*   connectionState;

  IRC_Client_RuntimeStatistic_t currentStatistic;
};

struct IRC_Client_StreamUserData
{
  inline IRC_Client_StreamUserData ()
   : configuration (NULL)
   , moduleConfiguration (NULL)
   , moduleHandlerConfiguration (NULL)
  {};

  IRC_Client_Configuration*              configuration;

  // *TODO*: remove these ASAP
  Stream_ModuleConfiguration*            moduleConfiguration;
  IRC_Client_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct IRC_Client_StreamState
 : Stream_State
{
  inline IRC_Client_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  IRC_Client_StreamSessionData* currentSessionData;
  IRC_Client_StreamUserData*    userData;
};

typedef Stream_SessionDataBase_T<IRC_Client_StreamSessionData> IRC_Client_StreamSessionData_t;

typedef Common_INotify_T<IRC_Client_StreamSessionData,
                         IRC_Client_IRCMessage,
                         IRC_Client_SessionMessage> IRC_Client_IStreamNotify_t;

#endif
