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

#ifndef NET_SERVER_COMMON_H
#define NET_SERVER_COMMON_H

#include "common_istatistic.h"

#include "stream_common.h"
#include "stream_iallocator.h"

#include "net_configuration.h"
#include "net_connection_manager_common.h"
#include "net_ilistener.h"

typedef Common_IStatistic_T<Stream_Statistic> Net_Server_StatisticReportingHandler_t;

struct Net_Server_SignalHandlerConfiguration
{
  Net_IListener_t*                        listener;
  Net_Server_StatisticReportingHandler_t* statisticReportingHandler;
  long                                    statisticReportingTimerID;
};

#endif
