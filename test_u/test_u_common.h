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

#ifndef TEST_U_COMMON_H
#define TEST_U_COMMON_H

#include <deque>
#include <list>

#include "ace/Synch_Traits.h"

#include "common.h"

#include "common_ui_common.h"

#include "net_common.h"

//#include "test_u_configuration.h"
#include "test_u_stream_common.h"

//#include "net_server_common.h"

// forward declarations
class Stream_IAllocator;
struct Net_Client_Configuration;
class Net_Client_TimeoutHandler;
struct Net_Server_Configuration;

extern unsigned int random_seed;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
extern struct random_data random_data;
extern char random_state_buffer[];
#endif

enum Net_GTK_Event
{
  NET_GKTEVENT_INVALID = -1,
  NET_GTKEVENT_CONNECT = 0,
  NET_GTKEVENT_DATA,
  NET_GTKEVENT_DISCONNECT,
  NET_GTKEVENT_STATISTIC,
  // -------------------------------------
  NET_GTKEVENT_MAX
};
typedef std::deque<Net_GTK_Event> Net_GTK_Events_t;
typedef Net_GTK_Events_t::const_iterator Net_GTK_EventsIterator_t;

struct Net_GTK_ProgressData
{
  inline Net_GTK_ProgressData ()
   : /*cursorType (GDK_LAST_CURSOR)
   ,*/ GTKState (NULL)
   , statistic ()
   , transferred (0)
   , size (0)
  {};

  //GdkCursorType       cursorType;
  Common_UI_GTKState* GTKState;
  Stream_Statistic    statistic;
  size_t              transferred; // bytes
  size_t              size; // bytes
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
   , progressData ()
   , progressEventSourceID (0)
   , serverConfiguration (NULL)
   , stackLock ()
  {};

  bool                       allowUserRuntimeStatistic;
  Net_Client_Configuration*  clientConfiguration;
  Net_GTK_Events_t           eventStack;
  Common_MessageStack_t      logStack;
  Net_GTK_ProgressData       progressData;
  guint                      progressEventSourceID;
  Net_Server_Configuration*  serverConfiguration;
  ACE_SYNCH_RECURSIVE_MUTEX  stackLock;
};

#endif
