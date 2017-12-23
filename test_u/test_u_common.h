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

#include "ace/config-lite.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_configuration.h"

// forward declarations
class Stream_IAllocator;
struct Net_Server_Configuration;

typedef Net_Statistic_t Test_U_Statistic_t;
typedef Common_IStatistic_T<Test_U_Statistic_t> Test_U_IStatisticHandler_t;
typedef Common_StatisticHandler_T<Test_U_Statistic_t> Test_U_StatisticHandler_t;

struct Test_U_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_U_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
  {};
};

struct Test_U_ConnectionConfiguration;
struct Test_U_UserData
 : Net_UserData
{
  Test_U_UserData ()
   : Net_UserData ()
//   , connectionConfiguration (NULL)
  {};

//  struct Test_U_ConnectionConfiguration* connectionConfiguration;
};

//////////////////////////////////////////

struct Test_U_Configuration
{
  Test_U_Configuration ()
   : signalHandlerConfiguration ()
   , userData ()
  {};

  struct Test_U_SignalHandlerConfiguration signalHandlerConfiguration;

  struct Test_U_UserData                   userData;
};

#endif
