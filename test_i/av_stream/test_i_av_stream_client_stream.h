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

#ifndef TEST_I_AVSTREAM_CLIENT_STREAM_H
#define TEST_I_AVSTREAM_CLIENT_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_mediafoundation_callback.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_net_io_stream.h"
#include "stream_net_target.h"

#include "test_i_network.h"
#include "test_i_connection_manager_common.h"
#include "test_i_av_stream_client_common.h"
#include "test_i_av_stream_client_message.h"
#include "test_i_av_stream_client_session_message.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMFMediaSession;
#endif // ACE_WIN32 || ACE_WIN64
class Stream_IAllocator;

extern const char stream_name_string_[];
extern const char stream_name_string_2[];

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_AVStream_Client_DirectShow_Audio_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_DirectShow_StreamState,
                        struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                        Test_I_Client_DirectShow_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_DirectShow_Message,
                        Test_I_AVStream_Client_DirectShow_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_DirectShow_StreamState,
                        struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                        Test_I_Client_DirectShow_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_DirectShow_Message,
                        Test_I_AVStream_Client_DirectShow_SessionMessage> inherited;

public:
  Test_I_AVStream_Client_DirectShow_Audio_Stream ();
  inline virtual ~Test_I_AVStream_Client_DirectShow_Audio_Stream () { inherited::shutdown (); }

     // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                     bool&);          // return value: delete modules ?

     // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_DirectShow_Audio_Stream (const Test_I_AVStream_Client_DirectShow_Audio_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_DirectShow_Audio_Stream& operator= (const Test_I_AVStream_Client_DirectShow_Audio_Stream&))
};

template <typename ConnectionManagerType,
          typename ConnectorType>
class Test_I_AVStream_Client_DirectShow_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_DirectShow_StreamState,
                        struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                        Test_I_Client_DirectShow_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_DirectShow_Message,
                        Test_I_AVStream_Client_DirectShow_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_DirectShow_StreamState,
                        struct Test_I_AVStream_Client_DirectShow_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                        Test_I_Client_DirectShow_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_DirectShow_Message,
                        Test_I_AVStream_Client_DirectShow_SessionMessage> inherited;

 public:
  Test_I_AVStream_Client_DirectShow_Stream_T ();
  inline virtual ~Test_I_AVStream_Client_DirectShow_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&);

 private:
  typedef Test_I_AVStream_Client_DirectShow_Stream_T<ConnectionManagerType,
                                                     ConnectorType> OWN_TYPE_T;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_DirectShow_Message,
                                     Test_I_AVStream_Client_DirectShow_SessionMessage,
                                     Test_I_AVStream_Client_DirectShow_StreamSessionData_t,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,               // task synch type
                                         Common_TimePolicy_t,        // time policy
                                         Test_I_AVStream_Client_DirectShow_StreamSessionData, // session data type
                                         Stream_SessionMessageType,  // session event type
                                         Stream_ModuleConfiguration, // module configuration type
                                         struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                                         libacestream_default_net_target_module_name_string,
                                         Stream_INotify_t,           // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;  // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_DirectShow_Stream_T (const Test_I_AVStream_Client_DirectShow_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_DirectShow_Stream_T& operator= (const Test_I_AVStream_Client_DirectShow_Stream_T&))
};

//////////////////////////////////////////

template <typename ConnectionManagerType,
          typename ConnectorType>
class Test_I_AVStream_Client_MediaFoundation_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                        struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_Client_MediaFoundation_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_MediaFoundation_Message,
                        Test_I_AVStream_Client_MediaFoundation_SessionMessage>
 , public Stream_MediaFramework_MediaFoundation_Callback_T<struct Test_I_MediaFoundationConfiguration>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_MediaFoundation_StreamState,
                        struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                        Test_I_Client_MediaFoundation_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_MediaFoundation_Message,
                        Test_I_AVStream_Client_MediaFoundation_SessionMessage> inherited;
  typedef Stream_MediaFramework_MediaFoundation_Callback_T<struct Test_I_MediaFoundationConfiguration> inherited2;

 public:
  Test_I_AVStream_Client_MediaFoundation_Stream_T ();
  virtual ~Test_I_AVStream_Client_MediaFoundation_Stream_T ();

  // override (part of) Stream_IStreamControl_T
  //virtual Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,   // wait for completion ?
                     bool = true,   // recurse upstream (if any) ?
                     bool = false); // high priority ?

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&);

 private:
  typedef Test_I_AVStream_Client_MediaFoundation_Stream_T<ConnectionManagerType,
                                                          ConnectorType> OWN_TYPE_T;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_MediaFoundation_Message,
                                     Test_I_AVStream_Client_MediaFoundation_SessionMessage,
                                     Test_I_AVStream_Client_MediaFoundation_StreamSessionData_t,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,               // task synch type
                                         Common_TimePolicy_t,        // time policy
                                         Test_I_AVStream_Client_MediaFoundation_StreamSessionData, // session data type
                                         enum Stream_SessionMessageType,  // session event type
                                         struct Stream_ModuleConfiguration, // module configuration type
                                         struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                                         libacestream_default_net_target_module_name_string,
                                         Stream_INotify_t,     // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;  // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_MediaFoundation_Stream_T (const Test_I_AVStream_Client_MediaFoundation_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_MediaFoundation_Stream_T& operator= (const Test_I_AVStream_Client_MediaFoundation_Stream_T&))

  //// *TODO*: re-consider this API
  //inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // media session
  IMFMediaSession* mediaSession_;
};
#else
class Test_I_AVStream_Client_ALSA_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                        Test_I_Client_ALSA_V4L_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_ALSA_V4L_Message,
                        Test_I_AVStream_Client_ALSA_V4L_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                        Test_I_Client_ALSA_V4L_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_ALSA_V4L_Message,
                        Test_I_AVStream_Client_ALSA_V4L_SessionMessage> inherited;

public:
  Test_I_AVStream_Client_ALSA_Stream ();
  inline virtual ~Test_I_AVStream_Client_ALSA_Stream () { inherited::shutdown (); }

     // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                    bool&);          // return value: delete modules ?

     // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_ALSA_Stream (const Test_I_AVStream_Client_ALSA_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_ALSA_Stream& operator= (const Test_I_AVStream_Client_ALSA_Stream&))
};

template <typename ConnectionManagerType,
          typename ConnectorType>
class Test_I_AVStream_Client_V4L_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                        Test_I_Client_ALSA_V4L_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_ALSA_V4L_Message,
                        Test_I_AVStream_Client_ALSA_V4L_SessionMessage>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamState,
                        struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                        Test_I_Client_ALSA_V4L_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_AVStream_Client_ALSA_V4L_Message,
                        Test_I_AVStream_Client_ALSA_V4L_SessionMessage> inherited;

 public:
  Test_I_AVStream_Client_V4L_Stream_T ();
  inline virtual ~Test_I_AVStream_Client_V4L_Stream_T () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: module list
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  typedef Test_I_AVStream_Client_V4L_Stream_T<ConnectionManagerType,
                                              ConnectorType> OWN_TYPE_T;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_AVStream_Client_ALSA_V4L_Message,
                                     Test_I_AVStream_Client_ALSA_V4L_SessionMessage,
                                     Test_I_AVStream_Client_ALSA_V4L_StreamSessionData_t,
                                     ConnectionManagerType,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                      // task synch type
                                         Common_TimePolicy_t,               // time policy
                                         Test_I_AVStream_Client_ALSA_V4L_StreamSessionData, // session data type
                                         enum Stream_SessionMessageType,    // session event type
                                         struct Stream_ModuleConfiguration, // module configuration type
                                         struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                                         libacestream_default_net_target_module_name_string,
                                         Stream_INotify_t,            // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;         // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_V4L_Stream_T (const Test_I_AVStream_Client_V4L_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_V4L_Stream_T& operator= (const Test_I_AVStream_Client_V4L_Stream_T&))
};
#endif // ACE_WIN32 || ACE_WIN64

// include template definition
#include "test_i_av_stream_client_stream.inl"

#endif
