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

#include "common_timer_manager_common.h"

#include "stream_session_data.h"

#include "irc_common.h"
#include "irc_configuration.h"
#include "irc_message.h"
#include "irc_stream.h"
#include "irc_stream_common.h"

#include "IRC_client_network.h"
//#include "IRC_client_stream_common.h"

// forward declarations
struct IRC_Client_StreamState;
struct IRC_Client_StreamConfiguration;
struct IRC_Client_ModuleHandlerConfiguration;
struct IRC_Client_SessionData;
typedef Stream_SessionData_T<struct IRC_Client_SessionData> IRC_Client_SessionData_t;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct IRC_AllocatorConfiguration> IRC_Client_ControlMessage_t;
class IRC_Client_SessionMessage;
struct IRC_Client_UserData;

template <typename TimerManagerType> // implements Common_ITimer
class IRC_Client_Stream_T
 : public IRC_Stream_T<struct IRC_Client_StreamState,
                       struct IRC_Client_StreamConfiguration,
                       IRC_Statistic_t,
                       TimerManagerType,
                       struct IRC_Client_ModuleHandlerConfiguration,
                       struct IRC_Client_SessionData,
                       IRC_Client_SessionData_t,
                       IRC_Client_ControlMessage_t,
                       IRC_Message,
                       IRC_Client_SessionMessage,
                       IRC_Client_Connection_Manager_t,
                       struct IRC_Client_UserData>
{
  typedef IRC_Stream_T<struct IRC_Client_StreamState,
                       struct IRC_Client_StreamConfiguration,
                       IRC_Statistic_t,
                       TimerManagerType,
                       struct IRC_Client_ModuleHandlerConfiguration,
                       struct IRC_Client_SessionData,
                       IRC_Client_SessionData_t,
                       IRC_Client_ControlMessage_t,
                       IRC_Message,
                       IRC_Client_SessionMessage,
                       IRC_Client_Connection_Manager_t,
                       struct IRC_Client_UserData> inherited;

 public:
  IRC_Client_Stream_T (); // name
  inline virtual ~IRC_Client_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  virtual bool initialize (const IRC_Client_StreamConfiguration_t&,
                           ACE_HANDLE);

  // *TODO*: remove this API
  void ping ();

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Stream_T (const IRC_Client_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Stream_T& operator= (const IRC_Client_Stream_T&))
};

// include template definition
#include "IRC_client_stream.inl"

#endif
