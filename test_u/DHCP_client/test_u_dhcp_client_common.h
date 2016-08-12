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

#ifndef TEST_U_DHCPCLIENT_COMMON_H
#define TEST_U_DHCPCLIENT_COMMON_H

#include <list>

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"
//#include "test_u_message.h"
//#include "test_u_session_message.h"
//#include "test_u_stream.h"

// forward declarations
class Test_U_Message;
class Test_U_SessionMessage;

//typedef Test_U_Stream_T<Test_U_OutboundConnector_t> Test_U_Stream_t;
//typedef Test_U_Stream_T<Test_U_OutboundAsynchConnector_t> Test_U_AsynchStream_t;

struct Test_U_DHCPClient_Configuration
 : Test_U_Configuration
{
  inline Test_U_DHCPClient_Configuration ()
   : Test_U_Configuration ()
   , broadcastHandle (ACE_INVALID_HANDLE)
   , handle (ACE_INVALID_HANDLE)
  {};

  ACE_HANDLE broadcastHandle; // listen handle (broadcast)
  ACE_HANDLE handle;          // listen handle (unicast)
};

typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Test_U_AllocatorConfiguration,
                                Test_U_Message,
                                Test_U_SessionMessage> Test_U_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<Test_U_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_U_StreamSessionData,
                                    Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_U_ISessionNotify_t> Test_U_ISubscribe_t;

struct Test_U_DHCPClient_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Test_U_DHCPClient_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , subscribers ()
   , subscribersLock ()
  {};

  Test_U_DHCPClient_Configuration* configuration;
  Test_U_Subscribers_t             subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX        subscribersLock;
};

//struct Test_U_DHCPClient_ThreadData
//{
//  inline Test_U_DHCPClient_ThreadData ()
//   : CBData (NULL)
//   , eventSourceID (0)
//  {};

//  Test_U_DHCPClient_GTK_CBData* CBData;
//  guint                         eventSourceID;
//};

#endif
