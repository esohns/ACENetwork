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
#include <string>

#include "ace/OS.h"

#include "common_timer_manager_common.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "dhcp_common.h"
#include "dhcp_defines.h"
#include "dhcp_tools.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename ConnectionManagerType,
          typename ConnectorType>
DHCP_Module_Discover_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType,
                       ConfigurationType,
                       ConnectionManagerType,
                       ConnectorType>::DHCP_Module_Discover_T ()
 : inherited ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , initialized_ (false)
 , sendRequestOnOffer_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::DHCP_Module_Discover_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename ConnectionManagerType,
          typename ConnectorType>
DHCP_Module_Discover_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType,
                       ConfigurationType,
                       ConnectionManagerType,
                       ConnectorType>::~DHCP_Module_Discover_T ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::~DHCP_Module_Discover_T"));

  if (sessionData_)
    sessionData_->decrease ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename ConnectionManagerType,
          typename ConnectorType>
const ConfigurationType&
DHCP_Module_Discover_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType,
                       ConfigurationType,
                       ConnectionManagerType,
                       ConnectorType>::get () const
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::get"));

  ACE_ASSERT (configuration_);

  return *configuration_;
}
template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
DHCP_Module_Discover_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType,
                       ConfigurationType,
                       ConnectionManagerType,
                       ConnectorType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.protocolConfiguration);

  if (initialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // clean up
    configuration_ = NULL;
    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    initialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  // *TODO*: remove type inference
  sendRequestOnOffer_ =
      configuration_in.protocolConfiguration->sendRequestOnOffer;
  initialized_ = true;

  return initialized_;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
DHCP_Module_Discover_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType,
                       ConfigurationType,
                       ConnectionManagerType,
                       ConnectorType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout->isInitialized ());
  ACE_ASSERT (sessionData_);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->connection);
  ACE_ASSERT (configuration_->protocolConfiguration);
  ACE_ASSERT (configuration_->socketConfiguration);
  ACE_ASSERT (configuration_->socketHandlerConfiguration);

  const typename ProtocolMessageType::DATA_T& data_r = message_inout->get ();
  if  (!sendRequestOnOffer_ ||
       (DHCP_Tools::type (data_r) != DHCP_Codes::DHCP_MESSAGE_OFFER))
    return; // done

  // sanity check(s)
  const typename SessionMessageType::SESSION_DATA_T::DATA_T& data_2 =
      sessionData_->get ();
  if (data_r.xid != data_2.xid)
    return; // done

  ProtocolMessageType* message_p =
      allocateMessage (configuration_->socketHandlerConfiguration->PDUSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to DHCP_Module_Discover_T::allocateMessage(%u): \"%m\", returning\n"),
                configuration_->socketHandlerConfiguration->PDUSize));
    return;
  } // end IF
  DHCP_Record DHCP_record;
  DHCP_record.op = DHCP_Codes::DHCP_OP_REQUEST;
  DHCP_record.htype = DHCP_FRAME_HTYPE;
  DHCP_record.hlen = DHCP_FRAME_HLEN;
  DHCP_record.xid = data_r.xid;
  DHCP_record.secs = data_r.secs;
  if (configuration_->protocolConfiguration->requestBroadcastReplies)
    DHCP_record.flags = DHCP_FLAGS_BROADCAST;
  if (!Net_Common_Tools::interface2MACAddress (configuration_->socketConfiguration->networkInterface,
                                               DHCP_record.chaddr))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interface2MACAddress(), returning\n")));

    // clean up
    message_p->release ();

    return;
  } // end IF
  std::string buffer;
  buffer.append (reinterpret_cast<const char*> (&data_r.yiaddr), 4);
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_REQUESTEDIPADDRESS,
                                              buffer));
  //     *TODO*: support optional options:
  //             - 'overload'            (52)
  DHCP_OptionsIterator_t iterator =
      data_r.options.find (DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME);
  ACE_ASSERT (iterator != data_r.options.end ());
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_IPADDRESSLEASETIME,
                                              (*iterator).second));
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE,
                                              std::string (1,
                                                           static_cast<char> (DHCP_Codes::DHCP_MESSAGE_REQUEST))));
  iterator =
      data_r.options.find (DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER);
  ACE_ASSERT (iterator != data_r.options.end ());
  DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_SERVERIDENTIFIER,
                                              (*iterator).second));
  //         - 'parameter request list'  (55) [include in all subsequent messages]
  //         - 'message'                 (56)
  //         - 'maximum message size'    (57)
  //         - 'vendor class identifier' (60)
  //         - 'client identifier'       (61)
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  //  message_p->initialize (message_data_container_p,
  message_p->initialize (DHCP_record,
                         NULL);

  typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p =
    dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (configuration_->connection);
  if (!isocket_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                configuration_->connection));

    // clean up
    message_p->release ();

    return;
  } // end IF
  isocket_connection_p->send (message_p);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
DHCP_Module_Discover_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType,
                       ConfigurationType,
                       ConnectionManagerType,
                       ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      ACE_ASSERT (!sessionData_);
      sessionData_ =
        &const_cast<typename SessionMessageType::SESSION_DATA_T&> (message_inout->get ());
      sessionData_->increase ();
      break;
    }
    case STREAM_SESSION_END:
    {
      // clean up
      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename ConnectionManagerType,
          typename ConnectorType>
ProtocolMessageType*
DHCP_Module_Discover_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType,
                       ConfigurationType,
                       ConnectionManagerType,
                       ConnectorType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("DHCP_Module_Discover_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_p = NULL;

  // *TODO*: remove type inference
  if (configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      // *TODO*: remove type inference
      message_p =
          static_cast<ProtocolMessageType*> (configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      ProtocolMessageType (requestedSize_in));
  if (!message_p)
  {
    if (configuration_->streamConfiguration->messageAllocator)
    {
      if (configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                    requestedSize_in));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                  requestedSize_in));
  } // end IF

  return message_p;
}

////////////////////////////////////////////////////////////////////////////////

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::DHCP_Module_DiscoverH_T ()
 : inherited (NULL,  // lock handle
              // *NOTE*: the current (pull-)parser needs to be active because
              //         yyparse() will not return until the entity has been
              //         received and processed completely; otherwise, it would
              //         tie one dispatch thread during this time (deadlock for
              //         single-threaded reactors/proactor scenarios)
              true,  // active by default
              true,  // auto-start !
              false, // do not run the svc() routine on start (passive mode)
              false) // do not push session messages
 , statisticCollectHandler_ (ACTION_COLLECT,
                             this,
                             false)
 , statisticCollectHandlerID_ (-1)
 , initialized_ (false)
 , sendRequestOnOffer_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::DHCP_Module_DiscoverH_T"));

}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::~DHCP_Module_DiscoverH_T ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::~DHCP_Module_DiscoverH_T"));

  // clean up timer if necessary
  if (statisticCollectHandlerID_ != -1)
  {
    int result =
     COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_);
    if (result <= 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  statisticCollectHandlerID_));
  } // end IF
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (initialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    // clean up
    if (statisticCollectHandlerID_ != -1)
    {
      int result =
       COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (statisticCollectHandlerID_);
      if (result <= 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    statisticCollectHandlerID_));
      statisticCollectHandlerID_ = -1;
    } // end IF

    initialized_ = false;
  } // end IF

  if (configuration_in.streamConfiguration->statisticReportingInterval)
  {
    // schedule regular statistic collection
    ACE_Time_Value interval (STREAM_STATISTIC_COLLECTION_INTERVAL, 0);
    ACE_ASSERT (statisticCollectHandlerID_ == -1);
    ACE_Event_Handler* handler_p = &statisticCollectHandler_;
    statisticCollectHandlerID_ =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule (handler_p,                        // event handler
                                                            NULL,                             // act
                                                            COMMON_TIME_POLICY () + interval, // first wakeup time
                                                            interval);                        // interval
    if (statisticCollectHandlerID_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Timer_Manager::schedule(): \"%m\", aborting\n")));
      return false;
    } // end IF
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for intervals of %u second(s)...\n"),
//                 statisticCollectHandlerID_,
//                 statisticCollectionInterval_in));
  } // end IF

  // *NOTE*: need to clean up timer beyond this point !

  // OK: all's well...
  initialized_ = inherited::initialize (configuration_in);
  if (!initialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // retain session ID for reporting
      const SessionDataContainerType& session_data_container_r =
          message_inout->get ();
      const SessionDataType& session_data_r = session_data_container_r.get ();
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->currentSessionData);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*(inherited::streamState_->currentSessionData->lock));
      inherited::streamState_->currentSessionData->sessionID =
          session_data_r.sessionID;

      // start profile timer...
      //profile_.start ();

      break;
    }
    case STREAM_SESSION_END:
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::collect"));

  // step1: initialize info container POD
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::report () const
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::report"));

  // *TODO*: support (local) reporting here as well ?
  //         --> leave this to any downstream modules...
  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
DHCP_Module_DiscoverH_T<LockType,
                        TaskSynchType,
                        TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType,
                        ConfigurationType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_DiscoverH_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

//  // step1: initialize session data
//  IRC_StreamSessionData* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    IRC_StreamSessionData ());
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  //ACE_OS::memset (data_p, 0, sizeof (IRC_SessionData));
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  session_data_r.currentStatistic = statisticData_in;

//  // step2: allocate session data container
//  IRC_StreamSessionData_t* session_data_container_p = NULL;
//  // *NOTE*: fire-and-forget stream_session_data_p
//  ACE_NEW_NORETURN (session_data_container_p,
//                    IRC_StreamSessionData_t (stream_session_data_p,
//                                                    true));
//  if (!session_data_container_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

//    // clean up
//    delete stream_session_data_p;

//    return false;
//  } // end IF

  // step3: send the data downstream...
  // *NOTE*: fire-and-forget session_data_container_p
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       *inherited::sessionData_,
                                       inherited::configuration_->streamConfiguration->messageAllocator);
}
