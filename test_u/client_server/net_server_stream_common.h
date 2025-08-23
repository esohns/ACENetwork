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

#ifndef NET_SERVER_STREAM_COMMON_H
#define NET_SERVER_STREAM_COMMON_H

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_messageallocatorheap_base.h"

#include "net_server_listener_common.h"
#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_stream_2.h"

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_Parser_FlexAllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

// server_2-specific
typedef Test_U_Stream_2<Server_Asynch_TCP_Listener_t> Test_U_Server_Asynch_TCP_Stream_2;
typedef Test_U_Stream_2<Server_TCP_Listener_t> Test_U_Server_TCP_Stream_2;
#if defined (SSL_SUPPORT)
typedef Test_U_Stream_2<Server_SSL_Listener_t> Test_U_Server_SSL_Stream_2;
#endif // SSL_SUPPORT

// *TODO*: Server_UDP_AsynchConnector_t && Server_UDP_Connector_t must define
//          SINGLETON_T typedefs first
//typedef Test_U_Stream_2<Server_UDP_AsynchConnector_t> Test_U_Server_Asynch_UDP_Stream_2;
//typedef Test_U_Stream_2<Server_UDP_Connector_t> Test_U_Server_UDP_Stream_2;

#endif
