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

#ifndef TEST_I_AVSTREAM_SERVER_STREAM_H
#define TEST_I_AVSTREAM_SERVER_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_statemachine_control.h"

#include "stream_net_io_stream.h"

#include "net_connection_manager.h"

#include "test_i_av_stream_server_common.h"
#include "test_i_av_stream_server_message.h"

// forward declarations
class Stream_IAllocator;

extern const char stream_name_string_[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_DirectShow_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_DirectShow_UDPConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_MediaFoundation_UDPConnectionManager_t;
#else
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_AVStream_Server_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_AVStream_Server_UDPConnectionManager_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Server_DirectShow_TCPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_DirectShow_Message,
                                        Test_I_AVStream_Server_DirectShow_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_DirectShow_TCPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_DirectShow_Message,
                                        Test_I_AVStream_Server_DirectShow_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_DirectShow_TCPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_AVStream_Server_DirectShow_TCPStream ();
  virtual ~Test_I_AVStream_Server_DirectShow_TCPStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&,
                           ACE_HANDLE);            // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_TCPStream (const Test_I_AVStream_Server_DirectShow_TCPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_TCPStream& operator= (const Test_I_AVStream_Server_DirectShow_TCPStream&))

  // helper methods
  void setFormat (IGraphBuilder*,              // builder handle
                  const std::wstring&,         // filter name
                  const struct _AMMediaType&); // media type

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};
class Test_I_AVStream_Server_DirectShow_UDPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_DirectShow_Message,
                                        Test_I_AVStream_Server_DirectShow_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_DirectShow_UDPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamState,
                                        struct Test_I_AVStream_Server_DirectShow_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                        Test_I_AVStream_Server_DirectShow_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_DirectShow_Message,
                                        Test_I_AVStream_Server_DirectShow_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_DirectShow_UDPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_AVStream_Server_DirectShow_UDPStream ();
  virtual ~Test_I_AVStream_Server_DirectShow_UDPStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&,
                           ACE_HANDLE);            // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_UDPStream (const Test_I_AVStream_Server_DirectShow_UDPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_DirectShow_UDPStream& operator= (const Test_I_AVStream_Server_DirectShow_UDPStream&))

  // helper methods
  void setFormat (IGraphBuilder*,              // builder handle
                  const std::wstring&,         // filter name
                  const struct _AMMediaType&); // media type

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

class Test_I_AVStream_Server_MediaFoundation_TCPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_MediaFoundation_Message,
                                        Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_MediaFoundation_TCPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_MediaFoundation_Message,
                                        Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_MediaFoundation_TCPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_AVStream_Server_MediaFoundation_TCPStream ();
  virtual ~Test_I_AVStream_Server_MediaFoundation_TCPStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&,
                           ACE_HANDLE); // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_TCPStream (const Test_I_AVStream_Server_MediaFoundation_TCPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_TCPStream& operator= (const Test_I_AVStream_Server_MediaFoundation_TCPStream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // media session
  IMFMediaSession* mediaSession_;
  ULONG            referenceCount_;
};
class Test_I_AVStream_Server_MediaFoundation_UDPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_MediaFoundation_Message,
                                        Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_MediaFoundation_UDPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamState,
                                        struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
                                        Test_I_AVStream_Server_MediaFoundation_StreamSessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_MediaFoundation_Message,
                                        Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_MediaFoundation_UDPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_AVStream_Server_MediaFoundation_UDPStream ();
  virtual ~Test_I_AVStream_Server_MediaFoundation_UDPStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&,
                           ACE_HANDLE); // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_UDPStream (const Test_I_AVStream_Server_MediaFoundation_UDPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_MediaFoundation_UDPStream& operator= (const Test_I_AVStream_Server_MediaFoundation_UDPStream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // media session
  IMFMediaSession* mediaSession_;
  ULONG            referenceCount_;
};
#else
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_I_AVStream_Server_StreamSessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_I_Server_SessionManager_t;

class Test_I_AVStream_Server_TCPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_StreamState,
                                        struct Test_I_AVStream_Server_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                        Test_I_Server_SessionManager_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_Message,
                                        Test_I_AVStream_Server_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_TCPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_StreamState,
                                        struct Test_I_AVStream_Server_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                        Test_I_Server_SessionManager_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_Message,
                                        Test_I_AVStream_Server_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_TCPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_AVStream_Server_TCPStream ();
  inline virtual ~Test_I_AVStream_Server_TCPStream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE);                                // socket handle

  // *TODO*: re-consider this API
  void ping ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_TCPStream (const Test_I_AVStream_Server_TCPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_TCPStream& operator= (const Test_I_AVStream_Server_TCPStream&))
};

class Test_I_AVStream_Server_UDPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_StreamState,
                                        struct Test_I_AVStream_Server_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                        Test_I_Server_SessionManager_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_Message,
                                        Test_I_AVStream_Server_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_UDPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_AVStream_Server_StreamState,
                                        struct Test_I_AVStream_Server_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AVStream_Server_ModuleHandlerConfiguration,
                                        Test_I_Server_SessionManager_t,
                                        Stream_ControlMessage_t,
                                        Test_I_AVStream_Server_Message,
                                        Test_I_AVStream_Server_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_AVStream_Server_UDPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_AVStream_Server_UDPStream ();
  inline virtual ~Test_I_AVStream_Server_UDPStream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE);                                // socket handle

  // *TODO*: re-consider this API
  void ping ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_UDPStream (const Test_I_AVStream_Server_UDPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Server_UDPStream& operator= (const Test_I_AVStream_Server_UDPStream&))
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
