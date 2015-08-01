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

#ifndef TEST_U_NET_COMMON_H
#define TEST_U_NET_COMMON_H

#include <deque>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common.h"

#include "common_ui_common.h"

#include "net_configuration.h"
#include "net_stream_common.h"

#include "net_client_common.h"
#include "net_client_connector_common.h"
//#include "net_client_timeouthandler.h"

#include "net_server_common.h"

// forward declarations
class Stream_IAllocator;
class Net_Client_TimeoutHandler;

extern unsigned int random_seed;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
extern struct random_data random_data;
extern char random_state_buffer[];
#endif

enum Net_GTK_Event
{
  NET_GTKEVENT_CONNECT = 0,
  NET_GTKEVENT_DATA,
  NET_GTKEVENT_DISCONNECT,
  NET_GTKEVENT_STATISTICS,
  // ----------------------
  NET_GTKEVENT_MAX,
  NET_GKTEVENT_INVALID
};
typedef std::deque<Net_GTK_Event> Net_GTK_Events_t;
typedef Net_GTK_Events_t::const_iterator Net_GTK_EventsIterator_t;

struct Net_Client_SignalHandlerConfiguration
{
  inline Net_Client_SignalHandlerConfiguration ()
   : actionTimerId (-1)
   , connector (NULL)
   , messageAllocator (NULL)
   , peerAddress ()
   , socketHandlerConfiguration (NULL)
   , statisticCollectionInterval (0)
  {};

  long                            actionTimerId;
  Net_IConnector_t*               connector;
  Stream_IAllocator*              messageAllocator;
  ACE_INET_Addr                   peerAddress;
  Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  unsigned int                    statisticCollectionInterval; // statistics collecting interval (second(s))
                                                               // 0 --> DON'T collect statistics
};

struct Net_Client_Configuration
 : Net_Configuration
{
  inline Net_Client_Configuration ()
   : Net_Configuration ()
   , signalHandlerConfiguration ()
   , timeoutHandler (NULL)
  {};

  Net_Client_SignalHandlerConfiguration signalHandlerConfiguration;
  Net_Client_TimeoutHandler*            timeoutHandler;
};

struct Net_Server_Configuration
 : Net_Configuration
{
  inline Net_Server_Configuration ()
   : Net_Configuration ()
   , listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
  {};

  Net_IListener_t*                      listener;
  Net_ListenerConfiguration             listenerConfiguration;
  Net_Server_SignalHandlerConfiguration signalHandlerConfiguration;
};

struct Net_GTK_CBData
 : Common_UI_GTKState
{
  inline Net_GTK_CBData ()
   : Common_UI_GTKState ()
   , allowUserRuntimeStatistic (true)
   , clientConfiguration (NULL)
   , eventStack ()
   , logStack ()
   , serverConfiguration (NULL)
   , stackLock ()
   , subscribers ()
   , subscribersLock ()
  {};

  bool                       allowUserRuntimeStatistic;
  Net_Client_Configuration*  clientConfiguration;
  Net_GTK_Events_t           eventStack;
  Common_MessageStack_t      logStack;
  Net_Server_Configuration*  serverConfiguration;
  ACE_SYNCH_RECURSIVE_MUTEX  stackLock;
  Net_Subscribers_t          subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX  subscribersLock;
};

#endif
