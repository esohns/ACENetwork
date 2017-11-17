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

#ifndef TEST_U_CONNECTION_STREAM_H
#define TEST_U_CONNECTION_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"

#include "stream_net_io_stream.h"

#include "test_u_common.h"
#include "test_u_connection_manager_common.h"

#include "test_u_dhcp_client_common.h"

// forward declarations
class Stream_IAllocator;
class Test_U_Message;
class Test_U_SessionMessage;

extern const char stream_name_string_[];

class Test_U_InboundConnectionStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_StreamState,
                                        struct Test_U_StreamConfiguration,
                                        DHCP_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_U_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_StreamModuleHandlerConfiguration,
                                        struct Test_U_DHCPClient_SessionData, // session data
                                        Test_U_DHCPClient_SessionData_t,      // session data container (reference counted)
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_U_ConnectionManager_t,
                                        struct Test_U_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_StreamState,
                                        struct Test_U_StreamConfiguration,
                                        DHCP_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_U_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_StreamModuleHandlerConfiguration,
                                        struct Test_U_DHCPClient_SessionData,
                                        Test_U_DHCPClient_SessionData_t,
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_U_ConnectionManager_t,
                                        struct Test_U_UserData> inherited;

 public:
  Test_U_InboundConnectionStream ();
  virtual ~Test_U_InboundConnectionStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);                                // socket handle

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (DHCP_Statistic_t&); // return value: statistic data
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_InboundConnectionStream (const Test_U_InboundConnectionStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_InboundConnectionStream& operator= (const Test_U_InboundConnectionStream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

////////////////////////////////////////////////////////////////////////////////

class Test_U_OutboundConnectionStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_StreamState,
                                        struct Test_U_StreamConfiguration,
                                        DHCP_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_U_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_StreamModuleHandlerConfiguration,
                                        struct Test_U_DHCPClient_SessionData, // session data
                                        Test_U_DHCPClient_SessionData_t,      // session data container (reference counted)
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_U_ConnectionManager_t,
                                        struct Test_U_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_U_StreamState,
                                        struct Test_U_StreamConfiguration,
                                        DHCP_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_U_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_U_StreamModuleHandlerConfiguration,
                                        struct Test_U_DHCPClient_SessionData,
                                        Test_U_DHCPClient_SessionData_t,
                                        Test_U_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_U_ConnectionManager_t,
                                        struct Test_U_UserData> inherited;

 public:
  Test_U_OutboundConnectionStream ();
  virtual ~Test_U_OutboundConnectionStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);                                // socket handle

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (DHCP_Statistic_t&); // return value: statistic data
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_OutboundConnectionStream (const Test_U_OutboundConnectionStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_OutboundConnectionStream& operator= (const Test_U_OutboundConnectionStream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

#endif
