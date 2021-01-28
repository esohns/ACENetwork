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
#include "ace/Synch_Traits.h"

#include "common_timer_manager_common.h"

#include "stream_session_data.h"

#include "net_configuration.h"
#include "net_connection_manager.h"

#include "irc_common.h"
#include "irc_configuration.h"
#include "irc_message.h"
#include "irc_stream.h"
#include "irc_stream_common.h"

#include "IRC_client_sessionmessage.h"

// forward declarations
struct IRC_Client_StreamState;
struct IRC_Client_StreamConfiguration;
struct IRC_Client_ModuleHandlerConfiguration;
struct IRC_Client_SessionData;
typedef Stream_SessionData_T<struct IRC_Client_SessionData> IRC_Client_SessionData_t;
//class IRC_Client_SessionMessage;
struct Net_UserData;

//extern const char stream_name_string_[];
struct IRC_Client_StreamConfiguration;
struct IRC_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct IRC_Client_StreamConfiguration,
                               struct IRC_Client_ModuleHandlerConfiguration> IRC_Client_StreamConfiguration_t;
class IRC_Client_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<IRC_Client_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_TCP>
{
 public:
  IRC_Client_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T ()
   ///////////////////////////////////////
   , cursesState (NULL)
   , protocolConfiguration (NULL)
   , logToFile (IRC_CLIENT_SESSION_DEFAULT_LOG)
  {
    //PDUSize = IRC_MAXIMUM_FRAME_SIZE;
  }

  struct IRC_Client_CursesState*    cursesState;
  struct IRC_ProtocolConfiguration* protocolConfiguration;
  bool                              logToFile;
};
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 IRC_Client_ConnectionConfiguration,
                                 struct IRC_SessionState,
                                 IRC_Statistic_t,
                                 struct Net_UserData> IRC_Client_Connection_Manager_t;

template <typename TimerManagerType> // implements Common_ITimer
class IRC_Client_Stream_T
 : public IRC_Stream_T<struct IRC_Client_StreamState,
                       struct IRC_Client_StreamConfiguration,
                       struct Stream_Statistic,
                       TimerManagerType,
                       struct IRC_Client_ModuleHandlerConfiguration,
                       struct IRC_Client_SessionData,
                       IRC_Client_SessionData_t,
                       Stream_ControlMessage_t,
                       IRC_Message,
                       IRC_Client_SessionMessage,
                       IRC_Client_Connection_Manager_t,
                       struct Stream_UserData>
{
  typedef IRC_Stream_T<struct IRC_Client_StreamState,
                       struct IRC_Client_StreamConfiguration,
                       struct Stream_Statistic,
                       TimerManagerType,
                       struct IRC_Client_ModuleHandlerConfiguration,
                       struct IRC_Client_SessionData,
                       IRC_Client_SessionData_t,
                       Stream_ControlMessage_t,
                       IRC_Message,
                       IRC_Client_SessionMessage,
                       IRC_Client_Connection_Manager_t,
                       struct Stream_UserData> inherited;

 public:
  IRC_Client_Stream_T (); // name
  inline virtual ~IRC_Client_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

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

//////////////////////////////////////////

typedef IRC_Client_Stream_T<Common_Timer_Manager_t> IRC_Client_Stream_t;

#endif
