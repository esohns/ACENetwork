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

#ifndef TEST_U_STREAM_COMMON_H
#define TEST_U_STREAM_COMMON_H

#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_control_message.h"
//#include "stream_inotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "net_common.h"
#include "net_configuration.h"

#include "test_u_common.h"

 // forward declarations
class Test_U_Message;
class Test_U_SessionMessage;

struct Test_U_StreamSessionData
 : Stream_SessionData
{
  Test_U_StreamSessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   //, statistic ()
   , userData (NULL)
  {};

  struct Net_StreamConnectionState* connectionState;
  //Test_U_Statistic_t          statistic;

  struct Net_UserData*              userData;
};
typedef Stream_SessionData_T<struct Test_U_StreamSessionData> Test_U_StreamSessionData_t;

struct Test_U_StreamState
 : Stream_State
{
  Test_U_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {};

  struct Test_U_StreamSessionData* sessionData;

  struct Net_UserData*             userData;
};

//////////////////////////////////////////

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Common_FlexParserAllocatorConfiguration> Test_U_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_FlexParserAllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

#endif
