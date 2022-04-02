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

#ifndef TEST_I_COMMON_H
#define TEST_I_COMMON_H

#include "ace/OS.h"

#include "common.h"
#include "common_configuration.h"
#include "common_statistic_handler.h"
#include "common_time_common.h"

#include "common_parser_common.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_inotify.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_defines.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (QT_SUPPORT)
#include "test_i_qt_common.h"
#endif // QT_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

struct Test_I_ProgressData
{
  Test_I_ProgressData ()
   : sessionId (0)
   , statistic ()
  {
    ACE_OS::memset (&statistic, 0, sizeof (struct Stream_Statistic));
  }

  Stream_SessionId_t      sessionId;
  struct Stream_Statistic statistic;
};

struct Test_I_CBData
{
  Test_I_CBData ()
   : allowUserRuntimeStatistic (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
  {}

  bool                            allowUserRuntimeStatistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_I_ProgressData      progressData;
};

struct Test_I_ThreadData
{
  Test_I_ThreadData ()
   : CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_I_CBData*           CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
};

#if defined (GUI_SUPPORT)
struct Test_I_UI_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (QT_USE)
 : Test_I_Qt_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#else
 : Test_I_ProgressData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#elif defined (QT_USE)
   : Test_I_Qt_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
#else
   : Test_I_ProgressData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , sessionId (0)
  {}

  Stream_SessionId_t sessionId;
};

struct Test_I_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (QT_USE)
 : Test_I_Qt_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#else
 : Test_I_CBData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
#elif defined (QT_USE)
   : Test_I_Qt_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
#else
   : Test_I_CBData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , allowUserRuntimeStatistic (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
  {
#if defined (GTK_USE) || defined (QT_USE) || defined (WXWIDGETS_USE)
    progressData.state = UIState;
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
  }

  bool                            allowUserRuntimeStatistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_I_UI_ProgressData   progressData;
};

struct Test_I_UI_ThreadData
#if defined (GTK_USE)
 : Test_I_GTK_ThreadData
#elif defined (QT_USE)
 : Test_I_Qt_ThreadData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ThreadData
#else
 : Test_I_ThreadData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_I_GTK_ThreadData ()
#elif defined (QT_USE)
   : Test_I_Qt_ThreadData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ThreadData ()
#else
   : Test_I_ThreadData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , CBData (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_I_UI_CBData*        CBData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
};
#endif // GUI_SUPPORT

#endif
