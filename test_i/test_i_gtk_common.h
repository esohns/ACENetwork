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

#include <deque>
#include <map>
#include <set>

#include "ace/OS_NS_Thread.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "common_ui_common.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"

#include "stream_common.h"

#include "test_i_common.h"

// forward declarations
struct Test_I_Configuration;

struct Test_I_GTK_ProgressData
 : Common_UI_GTK_ProgressData
{
  Test_I_GTK_ProgressData ()
   : Common_UI_GTK_ProgressData ()
   , statistic ()
  {};

  Test_I_Statistic_t statistic;
};

struct Test_I_GTK_CBData
 : Common_UI_GTK_State
{
  Test_I_GTK_CBData ()
   : Common_UI_GTK_State ()
   , configuration (NULL)
//   , cursorType (GDK_LAST_CURSOR)
   , progressData ()
   , progressEventSourceId (0)
  {};

  struct Test_I_Configuration*   configuration;
//  GdkCursorType                      cursorType;
  struct Test_I_GTK_ProgressData progressData;
  guint                          progressEventSourceId;
};

struct Test_I_ThreadData
{
  Test_I_ThreadData ()
   : CBData (NULL)
   , eventSourceId (0)
  {};

  struct Test_I_GTK_CBData* CBData;
  guint                     eventSourceId;
};

typedef Common_UI_GtkBuilderDefinition_T<struct Test_I_GTK_CBData> Test_I_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                struct Test_I_GTK_CBData> Test_I_GTK_Manager_t;
typedef ACE_Singleton<Test_I_GTK_Manager_t,
                      typename ACE_MT_SYNCH::MUTEX> TEST_I_UI_GTK_MANAGER_SINGLETON;

#endif
