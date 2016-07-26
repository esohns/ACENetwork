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

#ifndef TEST_U_HTTP_DECODER_COMMON_H
#define TEST_U_HTTP_DECODER_COMMON_H

#include "stream_common.h"
#include "stream_control_message.h"

#include "test_u_common.h"

//#include "test_u_message.h"
//#include "test_u_session_message.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;

typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Test_U_AllocatorConfiguration,
                                Test_U_Message,
                                Test_U_SessionMessage> Test_U_ControlMessage_t;

//typedef Stream_IModuleHandler_T<Test_U_ModuleHandlerConfiguration> Test_U_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<Test_U_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

#endif
