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

#ifndef IRC_CLIENT_STREAM_H
#define IRC_CLIENT_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_statemachine_control.h"

#include "IRC_client_common_modules.h"
#include "IRC_client_configuration.h"
#include "IRC_client_message.h"
#include "IRC_client_module_IRCparser.h"
#include "IRC_client_module_IRCsplitter.h"
#include "IRC_client_module_IRCstreamer.h"
#include "IRC_client_sessionmessage.h"

class IRC_Client_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        IRC_Client_StreamState,
                        /////////////////
                        IRC_Client_StreamConfiguration,
                        /////////////////
                        IRC_Client_RuntimeStatistic_t,
                        /////////////////
                        Stream_ModuleConfiguration,
                        IRC_Client_ModuleHandlerConfiguration,
                        /////////////////
                        IRC_Client_StreamSessionData,
                        IRC_Client_StreamSessionData_t,
                        IRC_Client_SessionMessage,
                        IRC_Client_Message>
{
 public:
  IRC_Client_Stream ();
  virtual ~IRC_Client_Stream ();

  // implement Common_IInitialize_T
  virtual bool initialize (const IRC_Client_StreamConfiguration&); // configuration

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (IRC_Client_RuntimeStatistic_t&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

  // *TODO*: remove this API
  void ping ();

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        /////////////////
                        Stream_StateMachine_ControlState,
                        IRC_Client_StreamState,
                        /////////////////
                        IRC_Client_StreamConfiguration,
                        /////////////////
                        IRC_Client_RuntimeStatistic_t,
                        /////////////////
                        Stream_ModuleConfiguration,
                        IRC_Client_ModuleHandlerConfiguration,
                        /////////////////
                        IRC_Client_StreamSessionData,
                        IRC_Client_StreamSessionData_t,
                        IRC_Client_SessionMessage,
                        IRC_Client_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Stream (const IRC_Client_Stream&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Stream& operator= (const IRC_Client_Stream&))

  // finalize stream
  // *NOTE*: need this to clean up queued modules if something goes wrong
  //         during initialize () !
  bool finalize (const IRC_Client_Configuration&); // configuration

  // modules
  IRC_Client_Module_IRCMarshal_Module       IRCMarshal_;
  IRC_Client_Module_IRCParser_Module        IRCParser_;
  IRC_Client_Module_RuntimeStatistic_Module runtimeStatistic_;
  // *NOTE*: the final module needs to be supplied to the stream from outside,
  //         otherwise data might be lost if event dispatch runs in (a) separate
  //         thread(s)
  //   IRC_Client_Module_IRCHandler_Module IRCHandler_;
};

#endif
