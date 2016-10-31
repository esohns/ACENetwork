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

#ifndef TEST_I_GTK_COMMON_H
#define TEST_I_GTK_COMMON_H

#include <map>
#include <set>

#include <ace/OS_NS_Thread.h>

#include <gtk/gtk.h>

#include "common_ui_gtk_common.h"

#include "stream_common.h"

// forward declarations
struct Test_I_Configuration;

enum Test_I_GTK_Event
{
  TEST_I_GKTEVENT_INVALID = -1,
  // ------------------------------------
  TEST_I_GTKEVENT_START = 0,
  TEST_I_GTKEVENT_DATA,
  TEST_I_GTKEVENT_END,
  TEST_I_GTKEVENT_STATISTIC,
  // ------------------------------------
  TEST_I_GTKEVENT_MAX
};
typedef std::deque<Test_I_GTK_Event> Test_I_GTK_Events_t;
typedef Test_I_GTK_Events_t::const_iterator Test_I_GTK_EventsIterator_t;

typedef std::map<guint, ACE_Thread_ID> Test_I_PendingActions_t;
typedef Test_I_PendingActions_t::iterator Test_I_PendingActionsIterator_t;
typedef std::set<guint> Test_I_CompletedActions_t;
typedef Test_I_CompletedActions_t::iterator Test_I_CompletedActionsIterator_t;

struct Test_I_GTK_ProgressData
{
  inline Test_I_GTK_ProgressData ()
   : completedActions ()
   //   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
   , statistic ()
  {};

  Test_I_CompletedActions_t completedActions;
  //  GdkCursorType                      cursorType;
  Common_UI_GTKState*       GTKState;
  Test_I_PendingActions_t   pendingActions;
  Stream_Statistic          statistic;
};

struct Test_I_GTK_CBData
 : Common_UI_GTKState
{
  inline Test_I_GTK_CBData ()
   : Common_UI_GTKState ()
   , configuration (NULL)
   , eventStack ()
   , progressData ()
   , progressEventSourceID (0)
  {};

  Test_I_Configuration*   configuration;
  Test_I_GTK_Events_t     eventStack;
  Test_I_GTK_ProgressData progressData;
  guint                   progressEventSourceID;
};

struct Test_I_ThreadData
{
  inline Test_I_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
  {};

  Test_I_GTK_CBData* CBData;
  guint              eventSourceID;
};

#endif
