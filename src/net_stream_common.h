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

#ifndef Net_STREAM_COMMON_H
#define Net_STREAM_COMMON_H

#include "ace/Time_Value.h"
#include "ace/Notification_Strategy.h"
#include "ace/Stream.h"
#include "ace/Synch_Traits.h"

#include "common.h"

#include "stream_iallocator.h"
#include "stream_imodule.h"
#include "stream_statistichandler.h"

#include "net_common.h"

//typedef Common_Module_t Net_Module_t;
typedef Stream_IModule<ACE_MT_SYNCH,
                       Common_TimePolicy_t> Net_IModule_t;
typedef ACE_Stream_Iterator<ACE_MT_SYNCH,
                            Common_TimePolicy_t> Net_StreamIterator_t;

struct Net_RuntimeStatistic_t
{
  unsigned int numDataMessages; // (protocol) messages
  double       numBytes;        // amount of processed data

  // convenience
  inline Net_RuntimeStatistic_t operator+= (const Net_RuntimeStatistic_t& rhs)
  {
    numDataMessages += rhs.numDataMessages;
    numBytes += rhs.numBytes;

    return *this;
  };
};

struct Net_StreamSocketConfiguration_t
{
  // ************************** connection data ********************************
  int                        socketBufferSize;
  Stream_IAllocator*         messageAllocator;
  unsigned int               bufferSize;
  // **************************** stream data **********************************
  bool                       useThreadPerConnection;
  // *IMPORTANT NOTE*: in a threaded environment, workers MAY be
  // dispatching the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
  bool                       serializeOutput;
  ACE_Notification_Strategy* notificationStrategy;
  Common_Module_t*           module;
  bool                       deleteModule;
  unsigned int               statisticsReportingInterval;
  bool                       printFinalReport;
};

struct Net_StreamProtocolConfigurationState_t
{
  // *********************** stream / socket data ******************************
  Net_StreamSocketConfiguration_t configuration;
  // *************************** protocol data *********************************
  unsigned int                    peerPingInterval; // ms {0 --> OFF}
  bool                            pingAutoAnswer;
  bool                            printPongMessages;
  // **************************** runtime data *********************************
  unsigned int                    sessionID; // (== socket handle !)
  Net_RuntimeStatistic_t          currentStatistics;
  ACE_Time_Value                  lastCollectionTimestamp;
};

typedef Stream_StatisticHandler_Reactor_T<Net_RuntimeStatistic_t> Net_StatisticHandler_Reactor_t;
typedef Stream_StatisticHandler_Proactor_T<Net_RuntimeStatistic_t> Net_StatisticHandler_Proactor_t;

#endif
