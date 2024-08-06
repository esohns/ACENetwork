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

#ifndef NET_STATISTIC_H
#define NET_STATISTIC_H

#include "ace/Basic_Types.h"
#include "ace/Time_Value.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"

struct Net_Statistic
{
  Net_Statistic ()
   : sentBytes (0)
   , receivedBytes (0)
   //, timeStamp (ACE_Time_Value::zero)
   , previousBytes (0)
   , previousTimeStamp (ACE_Time_Value::zero)
  {}

  struct Net_Statistic operator+= (const struct Net_Statistic& rhs_in)
  {
    sentBytes += rhs_in.sentBytes;
    receivedBytes += rhs_in.receivedBytes;

    //timeStamp = rhs_in.timeStamp;
    previousBytes += rhs_in.previousBytes;
    previousTimeStamp = rhs_in.previousTimeStamp;

    return *this;
  }

  ACE_UINT64     sentBytes;
  ACE_UINT64     receivedBytes;

  // statistic and speed calculations
  //ACE_Time_Value timeStamp;
  ACE_UINT64     previousBytes; // total-
  ACE_Time_Value previousTimeStamp;
};
typedef struct Net_Statistic Net_Statistic_t;
typedef Common_IStatistic_T<Net_Statistic_t> Net_IStatisticHandler_t;
typedef Common_StatisticHandler_T<Net_Statistic_t> Net_StatisticHandler_t;

template <typename StreamStatisticType>
class Net_StreamStatistic_T
 : public Net_Statistic
 , public StreamStatisticType
{
 public:
  Net_StreamStatistic_T ()
   : Net_Statistic ()
   , StreamStatisticType ()
  {}

  Net_StreamStatistic_T<StreamStatisticType>
  operator+= (const Net_StreamStatistic_T<StreamStatisticType>& rhs_in)
  {
    Net_Statistic::operator+= (rhs_in);
    StreamStatisticType::operator+= (rhs_in);

    return *this;
  }

  Net_StreamStatistic_T<StreamStatisticType>
  operator+= (const StreamStatisticType& rhs_in)
  {
    StreamStatisticType::operator+= (rhs_in);

    return *this;
  }
};
typedef Net_StreamStatistic_T<struct Stream_Statistic> Net_StreamStatistic_t;
typedef Common_IStatistic_T<Net_StreamStatistic_t> Net_IStreamStatisticHandler_t;
typedef Common_StatisticHandler_T<Net_StreamStatistic_t> Net_StreamStatisticHandler_t;

#endif
