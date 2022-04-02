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
#include "stdafx.h"

#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//#include "ace/Netlink_Addr.h"
#endif
#include "ace/Synch_Traits.h"
#include "ace/Time_Policy.h"

#include "stream_common.h"
#include "stream_session_data_base.h"
#include "stream_statemachine_control.h"

#include "stream_module_tcpio_stream.h"

#include "net_connection_manager.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

typedef Stream_Module_TCPIO_Stream_T<ACE_MT_SYNCH,
                                     ACE_HR_Time_Policy,
                                     Stream_StateMachine_ControlState,
                                     Test_I_Stream_State,
                                     Test_I_Stream_Configuration,
                                     Stream_Statistic,
                                     Stream_ModuleConfiguration,
                                     Test_I_Stream_ModuleHandlerConfiguration,
                                     Test_I_Stream_SessionData,
                                     Stream_SessionDataBase_T<Test_I_Stream_SessionData>,
                                     Stream_SessionMessage,
                                     Stream_Message,
                                     Net_Connection_Manager_T<ACE_INET_Addr,
                                                              Test_I_Configuration,
                                                              Test_I_ConnectionState,
                                                              Stream_Statistic,
                                                              Test_I_Stream_UserData> > Stream_Module_TCPIO_Stream_t;

template <>
ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long>
Stream_Module_TCPIO_Stream_t::currentSessionID = 0;
