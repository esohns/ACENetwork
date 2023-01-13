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

#include "ftp_common.h"
#include "ftp_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
FTP_Module_Parser_T<ACE_SYNCH_USE,
                    TimePolicyType,
                    ConfigurationType,
                    ControlMessageType,
                    DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                    SessionMessageType>::FTP_Module_Parser_T (ISTREAM_T* stream_in)
#else
                    SessionMessageType>::FTP_Module_Parser_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_T::FTP_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
FTP_Module_Parser_T<ACE_SYNCH_USE,
                    TimePolicyType,
                    ConfigurationType,
                    ControlMessageType,
                    DataMessageType,
                    SessionMessageType>::record (struct FTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == inherited::record_);
  ACE_ASSERT (inherited::headFragment_);
  ACE_ASSERT (!inherited::headFragment_->isInitialized ());

  typename DataMessageType::DATA_T* data_container_p = NULL, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;
  int result = -1;

  ACE_NEW_NORETURN (data_container_p,
                    typename DataMessageType::DATA_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    delete record_inout; record_inout = NULL;
    return;
  } // end IF
  data_container_p->setPR (record_inout);
  ACE_ASSERT (!record_inout);
  data_container_2 = data_container_p;
  inherited::headFragment_->initialize (data_container_2,
                                        inherited::headFragment_->sessionId (),
                                        NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  message_p =
      static_cast<DataMessageType*> (inherited::headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           inherited::headFragment_->sessionId (),
                           NULL);
    message_p = static_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  // push message downstream
  result = inherited::put_next (inherited::headFragment_);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to put_next(): \"%m\", returning\n")));
    message_p->release (); message_p = NULL;
  } // end IF
  inherited::headFragment_ = NULL;
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
          typename StatisticContainerType>
FTP_Module_ParserH_T<ACE_SYNCH_USE,
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     StatisticContainerType>::FTP_Module_ParserH_T (ISTREAM_T* stream_in)
#else
                     StatisticContainerType>::FTP_Module_ParserH_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_ParserH_T::FTP_Module_ParserH_T"));

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
          typename StatisticContainerType>
void
FTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                     StatisticContainerType>::record (struct FTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_ParserH_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == inherited::record_);
  ACE_ASSERT (inherited::headFragment_);
  ACE_ASSERT (!inherited::headFragment_->isInitialized ());

  typename DataMessageType::DATA_T* data_container_p = NULL, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;

  ACE_NEW_NORETURN (data_container_p,
                    typename DataMessageType::DATA_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    delete record_inout; record_inout = NULL;
    return;
  } // end IF
  data_container_p->setPR (record_inout);
  ACE_ASSERT (!record_inout);
  data_container_2 = data_container_p;
  inherited::headFragment_->initialize (data_container_2,
                                        inherited::headFragment_->sessionId (),
                                        NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  message_p = static_cast<DataMessageType*> (inherited::headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           inherited::headFragment_->sessionId (),
                           NULL);
    message_p = static_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE
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
          typename StatisticContainerType>
bool
FTP_Module_ParserH_T<ACE_SYNCH_USE,
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
                     StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_ParserH_T::collect"));

  // step1: initialize info container POD
  data_out.bytes = 0;
  data_out.dataMessages = 0;
  data_out.sessionMessages = 0;
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
//          typename SessionDataContainerType>
//void
//FTP_Module_ParserH_T<ACE_SYNCH_USE,
//                     TaskSynchType,
//                     TimePolicyType,
//                     SessionMessageType,
//                     ProtocolMessageType,
//                     ConfigurationType,
//                     StreamStateType,
//                     SessionDataType,
//                     SessionDataContainerType>::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("FTP_Module_ParserH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return);
//}
