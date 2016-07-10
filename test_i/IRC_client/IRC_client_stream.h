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

#include <string>

#include "ace/Global_Macros.h"

#include "irc_common.h"
#include "irc_common_modules.h"
#include "irc_configuration.h"
#include "irc_message.h"
#include "irc_stream.h"
#include "irc_stream_common.h"

#include "IRC_client_configuration.h"
#include "IRC_client_sessionmessage.h"
#include "IRC_client_stream_common.h"

class IRC_Client_Stream
 : public IRC_Stream_T<IRC_Client_StreamState,
                       ///////////////////
                       IRC_Client_StreamConfiguration,
                       ///////////////////
                       IRC_RuntimeStatistic_t,
                       ///////////////////
                       IRC_Client_ModuleHandlerConfiguration,
                       ///////////////////
                       IRC_Client_SessionData,
                       IRC_Client_SessionData_t,
                       ///////////////////
                       ACE_Message_Block,
                       IRC_Message,
                       IRC_Client_SessionMessage>
{
 public:
  IRC_Client_Stream (const std::string&); // name
  virtual ~IRC_Client_Stream ();

  // implement Common_IInitialize_T
  virtual bool initialize (const IRC_Client_StreamConfiguration&, // configuration
                           bool = true,                           // setup pipeline ?
                           bool = true);                          // reset session data ?

  //// implement Common_IStatistic_T
  //// *NOTE*: delegate this to rntimeStatistic_
  //virtual bool collect (IRC_RuntimeStatistic_t&); // return value: statistic data
  //// this is just a dummy (use statisticsReportingInterval instead)
  //virtual void report () const;

  // *TODO*: remove this API
  void ping ();

 private:
  typedef IRC_Stream_T<IRC_Client_StreamState,
                       ///////////////////
                       IRC_Client_StreamConfiguration,
                       ///////////////////
                       IRC_RuntimeStatistic_t,
                       ///////////////////
                       IRC_Client_ModuleHandlerConfiguration,
                       ///////////////////
                       IRC_Client_SessionData,
                       IRC_Client_SessionData_t,
                       ///////////////////
                       ACE_Message_Block,
                       IRC_Message,
                       IRC_Client_SessionMessage> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Stream ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Stream (const IRC_Client_Stream&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Stream& operator= (const IRC_Client_Stream&))

  // modules
  //IRC_Client_Module_Marshal_Module   marshal_;
  //IRC_Module_Parser_Module           parser_;
  //IRC_Module_RuntimeStatistic_Module runtimeStatistic_;
  // *NOTE*: the final module needs to be supplied to the stream from outside,
  //         otherwise data might be lost if event dispatch runs in (a) separate
  //         thread(s)
  //   IRC_Client_Module_Handler_Module handler_;
};

#endif
