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

#if defined (GUI_SUPPORT)
#include "ace/OS.h"
#endif // GUI_SUPPORT

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "common_parser_common.h"

#include "common_signal_common.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#include "net_common.h"

//typedef Common_IStatistic_T<Net_Statistic_t> Test_U_IStatistic_t;
//typedef Common_StatisticHandler_T<Net_Statistic_t> Test_U_StatisticHandler_t;

struct Test_U_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_U_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
  {}
};

//////////////////////////////////////////

struct Test_U_Configuration
{
  Test_U_Configuration ()
   : allocatorConfiguration ()
   , dispatchConfiguration ()
   , signalHandlerConfiguration ()
   , userData ()
  {}

  struct Common_AllocatorConfiguration     allocatorConfiguration;
  struct Common_EventDispatchConfiguration dispatchConfiguration;
  struct Common_SignalHandlerConfiguration signalHandlerConfiguration;

  struct Net_UserData                      userData;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_U_UI_ProgressData
{
  Test_U_UI_ProgressData ()
   : state (NULL)
   //, statistic ()
  {
    //ACE_OS::memset (&statistic, 0, sizeof (Net_Statistic_t));
  }

  struct Common_UI_State* state;
  //Net_Statistic_t      statistic;
};

struct Test_U_UI_CBData
 : Common_UI_CBData
{
  Test_U_UI_CBData ()
   : Common_UI_CBData ()
   , allowUserRuntimeStatistic (true)
   , progressData ()
   , UIState (NULL)
  {
    progressData.state = UIState;
  }

  bool                          allowUserRuntimeStatistic;
  struct Test_U_UI_ProgressData progressData;
  struct Common_UI_State*       UIState;
};

struct Test_U_UI_ThreadData
{
  Test_U_UI_ThreadData ()
   : CBData (NULL)
   , sessionId (0)
  {}

  struct Test_U_UI_CBData* CBData;
  size_t                   sessionId;
};
#endif // GUI_SUPPORT

#endif
