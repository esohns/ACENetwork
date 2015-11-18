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

#include "common_inotify.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data_base.h"

#include "irc_icontrol.h"
#include "irc_record.h"
#include "irc_stream_common.h"

#include "IRC_client_common.h"

// forward declarations
struct IRC_Client_ModuleHandlerConfiguration;
class IRC_Client_SessionMessage;

typedef Stream_IModuleHandler_T<IRC_Client_ModuleHandlerConfiguration> IRC_Client_IModuleHandler_t;
typedef Stream_IModule_T<ACE_MT_SYNCH,
                         Common_TimePolicy_t,
                         Stream_ModuleConfiguration,
                         IRC_Client_ModuleHandlerConfiguration> IRC_Client_IModule_t;

struct IRC_Client_StreamState
 : IRC_StreamState
{
  inline IRC_Client_StreamState ()
   : IRC_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  IRC_Client_SessionData* currentSessionData;
  IRC_Client_UserData*    userData;
};

typedef Stream_SessionDataBase_T<IRC_Client_SessionData> IRC_Client_SessionData_t;

typedef Common_INotify_T<IRC_Client_SessionData,
                         IRC_Record,
                         IRC_Client_SessionMessage> IRC_Client_IStreamNotify_t;

typedef IRC_IControl_T<IRC_Client_IStreamNotify_t> IRC_Client_IControl_t;

#endif
