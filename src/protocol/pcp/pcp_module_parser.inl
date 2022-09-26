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

#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "net_defines.h"
#include "net_macros.h"

#include "pcp_common.h"
#include "pcp_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
PCP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     SessionMessageType>::PCP_Module_Parser_T (ISTREAM_T* stream_in)
#else
                     SessionMessageType>::PCP_Module_Parser_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , driver_ (COMMON_PARSER_DEFAULT_LEX_TRACE,  // trace scanning ?
            COMMON_PARSER_DEFAULT_YACC_TRACE) // trace parsing ?
 , isDriverInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_Parser_T::PCP_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
PCP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_Parser_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    isDriverInitialized_ = false;
  } // end IF

  // initialize driver
  isDriverInitialized_ =
      driver_.initialize (*configuration_in.parserConfiguration);
  if (!isDriverInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize parser driver, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
PCP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_Parser_T::handleDataMessage"));

  PCP_Record* record_p = NULL;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () -
               message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

//  DATA_CONTAINER_T& data_container_r =
//    const_cast<DATA_CONTAINER_T&> (message_inout->get ());
//  DATA_T& data_r = const_cast<DATA_T&> (data_container_r.get ());
  DATA_T& data_r = const_cast<DATA_T&> (message_inout->getR ());
//  if (!data_r.DHCPRecord)
//  {
//    ACE_NEW_NORETURN (data_r.DHCPRecord,
//                      PCP_Record ());
//    if (!data_r.DHCPRecord)
//    {
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate memory, returning\n")));
//      return;
//    } // end IF
//  } // end IF
//  record_p = data_r.DHCPRecord;
  record_p = &data_r;

  // set target record
  driver_.record_ = record_p;

  // OK: parse this message
  ACE_ASSERT (isDriverInitialized_);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
//              message_p->id (),
//              message_p->length ()));

  if (!driver_.parse (message_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to PCP_ParserDriver::parse() (message ID: %d), returning\n"),
                message_inout->id ()));
    return;
  } // end IF
  message_inout->rd_ptr (message_inout->base ());
  message_inout->initialize (*record_p,
                             message_inout->id (),
                             NULL);
  //isDriverInitialized_ = false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
PCP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_Parser_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  //switch (message_inout->type ())
  //{
  //  default:
  //    break;
  //} // end SWITCH
}

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
PCP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      StatisticHandlerType>::PCP_Module_ParserH_T (ISTREAM_T* stream_in,
#else
                      StatisticHandlerType>::PCP_Module_ParserH_T (typename inherited::ISTREAM_T* stream_in,
#endif
                                                                    bool autoStart_in,
                                                                    bool generateSessionMessages_in)
 : inherited (stream_in,
              autoStart_in,
              STREAM_HEADMODULECONCURRENCY_PASSIVE,
              generateSessionMessages_in)
 , driver_ (COMMON_PARSER_DEFAULT_LEX_TRACE,  // trace scanning ?
            COMMON_PARSER_DEFAULT_YACC_TRACE) // trace parsing ?
 , isDriverInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_ParserH_T::PCP_Module_ParserH_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
PCP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      StatisticHandlerType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_ParserH_T::initialize"));

//  bool result = false;

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::initialized_)
  {
    isDriverInitialized_ = false;
  } // end IF

  isDriverInitialized_ =
      driver_.initialize (configuration_in.parserConfiguration);
  if (!isDriverInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize parser driver: \"%m\", aborting\n")));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
void
PCP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      StatisticHandlerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_ParserH_T::handleDataMessage"));

//  int result = -1;
  struct PCP_Record* record_p = NULL;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () -
               message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

//  DATA_CONTAINER_T& data_container_r =
//    const_cast<DATA_CONTAINER_T&> (message_inout->get ());
//  DATA_T& data_r = const_cast<DATA_T&> (data_container_r.get ());
  DATA_T& data_r = const_cast<DATA_T&> (message_inout->get ());
  if (!data_r.DHCPRecord)
  {
    ACE_NEW_NORETURN (data_r.DHCPRecord,
                      struct PCP_Record ());
    if (!data_r.DHCPRecord)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, returning\n")));
      return;
    } // end IF
  } // end IF
  record_p = data_r.DHCPRecord;

  // set target record
  driver_.record_ = record_p;

  // OK: parse this message
  ACE_ASSERT (isDriverInitialized_);

  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
  //              message_p->id (),
  //              message_p->length ()));

  if (!driver_.parse (message_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to PCP_ParserDriver::parse() (message ID: %d), returning\n"),
                message_inout->id ()));
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
void
PCP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      StatisticHandlerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_ParserH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
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
    case STREAM_SESSION_MESSAGE_END:
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
PCP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      StatisticHandlerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Module_ParserH_T::collect"));

  // step1: initialize info container POD
  data_out.bytes = 0;
  data_out.dataMessages = 0;
  data_out.droppedMessages = 0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(SESSION_STATISTICS), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <ACE_SYNCH_USE,
//          typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//PCP_Module_ParserH_T<ACE_SYNCH_USE,
//                     TaskSynchType,
//                     TimePolicyType,
//                     SessionMessageType,
//                     ProtocolMessageType,
//                     ConfigurationType,
//                     StreamStateType,
//                     SessionDataType,
//                     SessionDataContainerType,
//                     StatisticContainerType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("PCP_Module_ParserH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return);
//}
