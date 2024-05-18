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

#include "common_parser_bencoding_tools.h"

#include "common_timer_manager_common.h"

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"
#include "bittorrent_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               UserDataType>::BitTorrent_Module_PeerParser_T (ISTREAM_T* stream_in)
#else
                               UserDataType>::BitTorrent_Module_PeerParser_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , headFragment_ (NULL)
 , parserConfiguration ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::BitTorrent_Module_PeerParser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               UserDataType>::~BitTorrent_Module_PeerParser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::~BitTorrent_Module_PeerParser_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
bool
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  { ACE_ASSERT (inherited::msg_queue_);
    result = inherited::msg_queue_->activate ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::activate(): \"%m\", continuing\n")));

    if (unlikely (headFragment_))
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF
  } // end IF

  parserConfiguration = *configuration_in.parserConfiguration;
  parserConfiguration.messageQueue = inherited::msg_queue_;
  if (unlikely (!inherited2::initialize(parserConfiguration)))
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
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
int
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               UserDataType>::put (ACE_Message_Block* messageBlock_in,
                                                   ACE_Time_Value* timeout_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::put"));

  int result = -1;
  bool stop = false;

  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_USER:
    {
      // *NOTE*: currently, all of these are 'session' messages
      SessionMessageType* session_message_p =
//        dynamic_cast<SessionMessageType*> (messageBlock_in); // *TODO*: enable this (why ? it's safe(r))
        static_cast<SessionMessageType*> (messageBlock_in);
      if (unlikely (!session_message_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: static_cast<Stream_SessionMessageBase_T>(0x%@) failed (type was: %d), aborting\n"),
                    inherited::mod_->name (),
                    messageBlock_in,
                    messageBlock_in->msg_type ()));
        messageBlock_in->release ();
        return -1;
      } // end IF
      if (unlikely (session_message_p->type () == STREAM_SESSION_MESSAGE_END))
        stop = true;
      break;
    }
    default:
      break;
  } // end SWITCH

  // drop the message into the queue
  result = inherited::put (messageBlock_in,
                           timeout_in);

  if (unlikely (stop))
   inherited::stop (false,  // don't wait
                    false); // high priority ?

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::handleDataMessage"));

  DataMessageType* message_p = NULL;
  bool release_inbound_message = true; // message_inout

  // initialize return value(s)
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!headFragment_)
  {
    headFragment_ = message_inout;

    DATA_T* data_p = NULL;
    ACE_NEW_NORETURN (data_p,
                      DATA_T ());
    if (!data_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, returning\n")));
      goto error;
    } // end IF
    DATA_CONTAINER_T* data_container_p = NULL;
    ACE_NEW_NORETURN (data_container_p,
                      DATA_CONTAINER_T (data_p,
                                        true));
    if (!data_container_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
      goto error;
    } // end IF
    headFragment_->initialize (data_container_p,
                               headFragment_->sessionId (),
                               NULL);
  } // end IF
  else
  {
    for (message_p = headFragment_;
         message_p->cont ();
         message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
    message_p->cont (message_inout);
  } // end ELSE
  message_inout = NULL;
  release_inbound_message = false;
next:
  message_p = headFragment_;

  while (message_p)
  {
    if (!this->parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_IParser::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
      goto error;
    } // end IF
    goto next;
  };

error:
  if (release_inbound_message)
  { ACE_ASSERT (message_inout);
    message_inout->release (); message_inout = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::handleSessionMessage"));

//  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_END:
    {
      if (headFragment_)
      {
        headFragment_->release (); headFragment_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               UserDataType>::record (struct BitTorrent_PeerRecord*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (headFragment_);

  DATA_CONTAINER_T& data_container_r =
      const_cast<DATA_CONTAINER_T&> (headFragment_->getR ());
  DATA_T& data_r = const_cast<DATA_T&> (data_container_r.getR ());
  ACE_ASSERT (!data_r.peerRecord);
  data_r.peerRecord = record_inout;
  record_inout = NULL;

#if defined (_DEBUG)
  if (inherited2::configuration_->debugParser)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s\n"),
                ACE_TEXT (BitTorrent_Tools::RecordToString (*data_r.peerRecord).c_str ())));
#endif // _DEBUG

  // set new head fragment
  unsigned int message_bytes = 0;
  if (!data_r.peerRecord->length)
    message_bytes = 4;
  else
  {
    switch (data_r.peerRecord->type)
    {
      case BITTORRENT_MESSAGETYPE_CHOKE:
      case BITTORRENT_MESSAGETYPE_UNCHOKE:
      case BITTORRENT_MESSAGETYPE_INTERESTED:
      case BITTORRENT_MESSAGETYPE_NOT_INTERESTED:
        message_bytes = 4 + 1; break;
      case BITTORRENT_MESSAGETYPE_HAVE:
        message_bytes = 4 + 1 + 4; break;
      case BITTORRENT_MESSAGETYPE_BITFIELD:
        message_bytes = 4 + 1 + (data_r.peerRecord->length - 1); break;
      case BITTORRENT_MESSAGETYPE_REQUEST:
        message_bytes = 4 + 1 + 4 + 4 + 4; break;
      case BITTORRENT_MESSAGETYPE_PIECE:
        message_bytes = 4 + 1 + 4 + 4 + (data_r.peerRecord->length - 9); break;
      case BITTORRENT_MESSAGETYPE_CANCEL:
        message_bytes = 4 + 1 + 4 + 4 + 4; break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), aborting\n"),
                    data_r.peerRecord->type));
        break;
      }
    } // end SWITCH
  } // end ELSE
  ACE_ASSERT (message_bytes);

  ACE_Message_Block* message_block_p = headFragment_;
  unsigned int skipped_bytes = 0;
  if (data_r.peerRecord->type == BITTORRENT_MESSAGETYPE_PIECE)
  { // keep only block data
    unsigned int header_bytes = 4 + 1 + 4 + 4;
    do
    { ACE_ASSERT (message_block_p);
      if (message_block_p->length () >= header_bytes)
      {
        message_block_p->rd_ptr (header_bytes);
        break;
      } // end IF

      header_bytes -= static_cast<unsigned int> (message_block_p->length ());
      message_block_p->rd_ptr (message_block_p->length ());
      message_block_p = message_block_p->cont ();
    } while (true);
    ACE_ASSERT (message_block_p);
    message_bytes -= 4 + 1 + 4 + 4;
    do
    { ACE_ASSERT (message_block_p);
      skipped_bytes += static_cast<unsigned int> (message_block_p->length ());
      if (skipped_bytes < message_bytes)
      {
        message_block_p = message_block_p->cont ();
        continue;
      } // end IF

      // skipped_bytes >= message_bytes
      if (skipped_bytes == message_bytes)
      {
        if (message_block_p->cont ())
        {
          ACE_Message_Block* message_block_2 = headFragment_;
          headFragment_ =
              dynamic_cast<DataMessageType*> (message_block_p->cont ());
          ACE_ASSERT (headFragment_);
          message_block_p->cont (NULL);
          message_block_p = message_block_2;
        } // end IF
        else
        {
          message_block_p = headFragment_;
          headFragment_ = NULL;
        } // end ELSE
        break;
      } // end IF

      // skipped_bytes > message_bytes
      unsigned int remainder = skipped_bytes - message_bytes;
      ACE_Message_Block* message_block_2 = message_block_p->duplicate ();
      ACE_ASSERT (message_block_2);
      ACE_Message_Block* message_block_3 = message_block_p->cont ();
      if (message_block_3)
        message_block_2->cont (message_block_3);
      message_block_p->cont (NULL);
      message_block_p->wr_ptr (message_block_p->rd_ptr () + (message_block_p->length () - remainder));
      message_block_2->rd_ptr (message_block_2->length () - remainder);
      message_block_p = headFragment_;
      headFragment_ = dynamic_cast<DataMessageType*> (message_block_2);
      ACE_ASSERT (headFragment_);
      break;
    } while (true);
    ACE_ASSERT (message_block_p->total_length () == (data_r.peerRecord->length - 9));
  } // end IF
  else
  {
    do
    {
      skipped_bytes += static_cast<unsigned int> (message_block_p->length ());
      if (skipped_bytes >= message_bytes)
      {
        unsigned int remainder = skipped_bytes - message_bytes;
        if (remainder)
        {
          ACE_Message_Block* message_block_2 = message_block_p->duplicate ();
          ACE_ASSERT (message_block_2);
          ACE_Message_Block* message_block_3 = message_block_p->cont ();
          if (message_block_3)
            message_block_2->cont (message_block_3);
          message_block_p->cont (NULL);
          message_block_p->wr_ptr (message_block_p->rd_ptr () + (message_block_p->length () - remainder));
          message_block_2->rd_ptr (message_block_2->length () - remainder);
          message_block_p = headFragment_;
          headFragment_ = dynamic_cast<DataMessageType*> (message_block_2);
          ACE_ASSERT (headFragment_);
        } // end IF
        else
        {
          if (message_block_p->cont ())
          {
            ACE_Message_Block* message_block_2 = headFragment_;
            headFragment_ = dynamic_cast<DataMessageType*> (message_block_p->cont ());
            ACE_ASSERT (headFragment_);
            message_block_p->cont (NULL);
            message_block_p = message_block_2;
          } // end IF
          else
          {
            message_block_p = headFragment_;
            headFragment_ = NULL;
          } // end ELSE
        } // end ELSE
        break;
      } // end IF
      message_block_p = message_block_p->cont ();
      ACE_ASSERT (message_block_p);
    } while (true);
    ACE_ASSERT (message_block_p->total_length () == message_bytes);
  } // end ELSE

  if (headFragment_)
  {
    DATA_T* data_p = NULL;
    ACE_NEW_NORETURN (data_p,
                      DATA_T ());
    if (!data_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing\n")));
      return;
    } // end IF
    DATA_CONTAINER_T* data_container_p = NULL;
    ACE_NEW_NORETURN (data_container_p,
                      DATA_CONTAINER_T (data_p,
                                        true));
    if (!data_container_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", continuing\n")));
      return;
    } // end IF
    headFragment_->initialize (data_container_p,
                               headFragment_->sessionId (),
                               NULL);
  } // end IF

  // make sure the whole fragment chain references the same data record
  DataMessageType* message_p =
      dynamic_cast<DataMessageType*> (message_block_p->cont ());
  while (message_p)
  {
    data_container_r.increase ();
    DATA_CONTAINER_T* data_container_2 = &data_container_r;
    message_p->initialize (data_container_2,
                           message_p->sessionId (),
                           NULL);
    message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  int result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

    // clean up
    message_block_p->release ();
  } // end IF
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
BitTorrent_Module_PeerParser_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               UserDataType>::handshake (struct BitTorrent_PeerHandShake*& handshake_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParser_T::handshake"));

  // sanity check(s)
  ACE_ASSERT (handshake_inout);
  ACE_ASSERT (inherited::sessionData_);

#if defined (_DEBUG)
  if (inherited2::configuration_->debugParser)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s"),
                ACE_TEXT (BitTorrent_Tools::HandShakeToString (*handshake_inout).c_str ())));
#endif // _DEBUG

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, const_cast<ACE_MT_SYNCH::MUTEX&> (inherited::sessionData_->getR_2 ()));
    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

    // sanity check(s)
    ACE_ASSERT (!session_data_r.handshake);

    // *TODO*: remove type inference
    session_data_r.handshake = handshake_inout;
  } // end lock scope

  handshake_inout = NULL;

  unsigned int handshake_bytes = inherited2::offset ();
  ACE_Message_Block* message_block_p = headFragment_;
  do
  {
    ACE_ASSERT (message_block_p);
    if (message_block_p->length () >= handshake_bytes)
    {
      if (message_block_p->length () == handshake_bytes)
      {
        message_block_p->rd_ptr (handshake_bytes);
        message_block_p = message_block_p->cont ();
      } // end IF
      else
        message_block_p->rd_ptr (handshake_bytes);
      break;
    } // end IF
    else
    {
      handshake_bytes -= static_cast<unsigned int> (message_block_p->length ());
      message_block_p->rd_ptr (message_block_p->length ());
    } // end ELSE
    message_block_p = message_block_p->cont ();
  } while (true);
}

////////////////////////////////////////////////////////////////////////////////

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//                            UserDataType>::BitTorrent_Module_PeerParserH_T (ISTREAM_T* stream_in)
//#else
//                            UserDataType>::BitTorrent_Module_PeerParserH_T (typename inherited::ISTREAM_T* stream_in)
//#endif
// : inherited (stream_in,
//              false,
//              STREAM_HEADMODULECONCURRENCY_CONCURRENT,
//              true)
// , inherited2 (COMMON_PARSER_DEFAULT_LEX_TRACE,  // trace scanning ?
//               COMMON_PARSER_DEFAULT_YACC_TRACE) // trace parsing ?
// , headFragment_ (NULL)
// , crunch_ (true) // strip protocol data ?
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::BitTorrent_Module_PeerParserH_T"));
//
//}
//
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//                            UserDataType>::~BitTorrent_Module_PeerParserH_T ()
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::~BitTorrent_Module_PeerParserH_T"));
//
//  if (headFragment_)
//    headFragment_->release ();
//}
//
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//bool
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//                            UserDataType>::initialize (const ConfigurationType& configuration_in,
//                                                      Stream_IAllocator* allocator_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::initialize"));
//
//  // sanity check(s)
//  ACE_ASSERT (configuration_in.parserConfiguration);
//
//  if (inherited::isInitialized_)
//  {
//    crunch_ = true;
//
//    if (headFragment_)
//    {
//      headFragment_->release ();
//      headFragment_ = NULL;
//    } // end IF
//  } // end IF
//
////  crunch_ = configuration_in.crunchMessages;
//  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
//      inherited::msg_queue_;
//  if (!inherited2::initialize (*configuration_in.parserConfiguration))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to initialize parser driver: \"%m\", aborting\n"),
//                inherited::mod_->name ()));
//    return false;
//  } // end IF
//
//  return inherited::initialize (configuration_in,
//                                allocator_in);
//}
//
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//void
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//                            UserDataType>::handleDataMessage (DataMessageType*& message_inout,
//                                                              bool& passMessageDownstream_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::handleDataMessage"));
//
//  DataMessageType* message_p = NULL;
//  int result = -1;
//  bool release_inbound_message = true; // message_inout
//  bool release_message = false; // message_p
//
//  // initialize return value(s)
//  passMessageDownstream_out = false;
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::mod_);
//
//  // append the "\0\0"-sequence, as required by flex
//  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
//  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
//  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
//  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was
//
//  {
//    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
//
//    if (!headFragment_)
//      headFragment_ = message_inout;
//    else
//    {
//      for (message_p = headFragment_;
//           message_p->cont ();
//           message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
//      message_p->cont (message_inout);
//
//      //// just signal the parser (see below for an explanation)
//      //result = condition_.broadcast ();
//      //if (result == -1)
//      //  ACE_DEBUG ((LM_ERROR,
//      //              ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%s\", continuing\n"),
//      //              inherited::mod_->name ()));
//    } // end ELSE
//
//    message_p = headFragment_;
//  } // end lock scope
//  ACE_ASSERT (message_p);
//  message_inout = NULL;
//  release_inbound_message = false;
//
//  { // *NOTE*: protect scanner/parser state
//    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
//
//    // OK: parse the message (fragment)
//
//    //  ACE_DEBUG ((LM_DEBUG,
//    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
//    //              message_p->id (),
//    //              message_p->length ()));
//
//    if (!this->parse (message_p))
//    { // *NOTE*: most probable reason: connection
//      //         has been closed --> session end
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: failed to HTTP_ParserDriver::parse() (message ID was: %d), returning\n"),
//                  inherited::mod_->name (),
//                  message_p->id ()));
//      goto error;
//    } // end IF
//    // the message fragment has been parsed successfully
//
////    if (!this->hasFinished ())
////      goto continue_; // --> wait for more data to arrive
//
////    // set session data format
////    // sanity check(s)
////    ACE_ASSERT (inherited::sessionData_);
////    ACE_ASSERT (inherited2::record_);
//
////    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
////      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());
//
////    HTTP_HeadersIterator_t iterator =
////      inherited2::record_->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
////    if (iterator != inherited2::record_->headers.end ())
////    {
////      session_data_r.format =
////        HTTP_Tools::Encoding2CompressionFormat ((*iterator).second);
////      ACE_DEBUG ((LM_DEBUG,
////                  ACE_TEXT ("%s: set compression format: \"%s\"\n"),
////                  inherited::mod_->name (),
////                  ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (session_data_r.format).c_str ())));
////    } // end IF
//
//    // make sure the whole fragment chain references the same data record
//    // sanity check(s)
//    ACE_ASSERT (headFragment_->isInitialized ());
////    DATA_CONTAINER_T* data_container_p =
////        &const_cast<DATA_CONTAINER_T&> (headFragment_->get ());
////    DATA_CONTAINER_T* data_container_2 = NULL;
////    message_p = dynamic_cast<DataMessageType*> (headFragment_->cont ());
////    while (message_p)
////    {
////      data_container_p->increase ();
////      data_container_2 = data_container_p;
////      message_p->initialize (data_container_2,
////                             NULL);
////      message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
////    } // end WHILE
//  } // end lock scope
//
//  // *NOTE*: the message has been parsed successfully
//  //         --> pass the data (chain) downstream
//  {
//    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
//
//    //// *NOTE*: new data fragments may have arrived by now
//    ////         --> set the next head fragment ?
//    //message_2 = dynamic_cast<DataMessageType*> (message_p->cont ());
//    //if (message_2)
//    //  message_p->cont (NULL);
//
//    result = inherited::put_next (headFragment_, NULL);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
//                  inherited::mod_->name ()));
//
//      // clean up
//      headFragment_->release ();
//
//      goto error;
//    } // end IF
//    headFragment_ = NULL;
//  } // end lock scope
//
////continue_:
//error:
//  if (release_inbound_message)
//  {
//    ACE_ASSERT (message_inout);
//    message_inout->release ();
//    message_inout = NULL;
//  } // end IF
//  if (release_message)
//  {
//    ACE_ASSERT (message_p);
//    message_p->release ();
//  } // end IF
//}
//
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//void
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//                            UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
//                                                                 bool& passMessageDownstream_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::handleSessionMessage"));
//
////  int result = -1;
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  switch (message_inout->type ())
//  {
//    case STREAM_SESSION_MESSAGE_BEGIN:
//    {
//      // sanity check(s)
//      ACE_ASSERT (inherited::sessionData_);
//
//      const SessionDataType& session_data_r = inherited::sessionData_->getR ();
//
//      // retain session ID for reporting
//      ACE_ASSERT (inherited::streamState_);
//      ACE_ASSERT (inherited::streamState_->sessionData);
//      ACE_ASSERT (inherited::streamState_->sessionData->lock);
//      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *(inherited::streamState_->sessionData->lock));
//        inherited::streamState_->sessionData->sessionId =
//          session_data_r.sessionId;
//      } // end lock scope
//
//      //// start profile timer
//      //profile_.start ();
//
//      break;
//    }
//    //// *NOTE*: the stream has been link()ed, the message contains the (merged)
//    ////         upstream session data --> retain a reference
//    //case STREAM_SESSION_MESSAGE_LINK:
//    //{
//    //  // *NOTE*: relax the concurrency policy in this case
//    //  // *TODO*: this isn't very reliable
//    //  inherited::concurrent_ = true;
//
//    //  break;
//    //}
//    case STREAM_SESSION_MESSAGE_END:
//    {
//      // *NOTE*: only process the first 'session end' message (see above: 2566)
//      { ACE_GUARD (ACE_SYNCH_RECURSIVE_MUTEX, aGuard, inherited::lock_);
//        if (inherited::sessionEndProcessed_)
//          break; // done
//        inherited::sessionEndProcessed_ = true;
//      } // end lock scope
//
//      if (headFragment_)
//      {
//        headFragment_->release (); headFragment_ = NULL;
//      } // end IF
//
//      // *NOTE*: in passive 'concurrent' scenarios, there is no 'worker' thread
//      //         running svc()
//      //         --> do not signal completion in this case
//      // *TODO*: remove type inference
//      if (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
//        this->inherited::TASK_BASE_T::stop (false,  // wait for completion ?
//                                            false); // N/A
//
//      break;
//    }
//    default:
//      break;
//  } // end SWITCH
//}
//
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//bool
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//                            UserDataType>::collect (StatisticContainerType& data_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::collect"));
//
//  // step1: initialize info container POD
//  data_out.capturedFrames = 0;
//  data_out.droppedFrames = 0;
//  data_out.bytes = 0;
//  data_out.dataMessages = 0;
//  data_out.timeStamp = COMMON_TIME_NOW;
//
//  // *NOTE*: information is collected by the statistic module (if any)
//
//  // step1: send the container downstream
//  if (!inherited::putStatisticMessage (data_out)) // data container
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
//    return false;
//  } // end IF
//
//  return true;
//}
//
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//void
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//                            UserDataType>::record (struct BitTorrent_PeerRecord*& record_inout)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::record"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::sessionData_);
//  ACE_ASSERT (headFragment_);
//  ACE_ASSERT (record_inout);
//
//  DATA_CONTAINER_T& data_container_r =
//      const_cast<DATA_CONTAINER_T&> (headFragment_->getR ());
//  DATA_T& data_r = const_cast<DATA_T&> (data_container_r.getR ());
//  ACE_ASSERT (!data_r.peerRecord);
//  data_r.peerRecord = record_inout;
//  record_inout = NULL;
//
//  // debug info
//  if (inherited2::trace_)
//    ACE_DEBUG ((LM_INFO,
//                ACE_TEXT ("%s"),
//                ACE_TEXT (BitTorrent_Tools::RecordToString (*data_r.peerRecord).c_str ())));
//
//  // step1: set session data
//  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
//      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
//
//  // set new head fragment ?
//  ACE_Message_Block* message_block_p = headFragment_;
//  do
//  {
//    if (!message_block_p->cont ()) break;
//    message_block_p = message_block_p->cont ();
//  } while (true);
//  if (message_block_p != inherited2::fragment_)
//  {
//    message_block_p = headFragment_;
//    // *IMPORTANT NOTE*: the fragment has already been unlinked in the previous
//    //                   call to switchBuffer()
//    headFragment_ = dynamic_cast<DataMessageType*> (inherited2::fragment_);
//    ACE_ASSERT (headFragment_);
//  } // end IF
//  else
//  {
//    message_block_p = headFragment_;
//    headFragment_ = NULL;
//  } // end ELSE
//
//  // make sure the whole fragment chain references the same data record
//  DataMessageType* message_p =
//      dynamic_cast<DataMessageType*> (headFragment_->cont ());
//  while (message_p)
//  {
//    data_container_r.increase ();
//    DATA_CONTAINER_T* data_container_2 = &data_container_r;
//    message_p->initialize (data_container_2,
//                           session_data_r.sessionId,
//                           NULL);
//    message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
//  } // end WHILE
//
//  int result = inherited::put_next (message_block_p, NULL);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", continuing\n"),
//                inherited::mod_->name ()));
//
//    // clean up
//    message_block_p->release ();
//  } // end IF
//}
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename UserDataType>
//void
//BitTorrent_Module_PeerParserH_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ConfigurationType,
//                            StreamControlType,
//                            StreamNotificationType,
//                            StreamStateType,
//                            SessionDataType,
//                            SessionDataContainerType,
//                            StatisticContainerType,
//                            TimerManagerType,
//                            UserDataType>::handshake (struct BitTorrent_PeerHandShake*& handShake_inout)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_PeerParserH_T::handshake"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::sessionData_);
//  ACE_ASSERT (headFragment_);
//  ACE_ASSERT (handShake_inout);
//
//  DATA_CONTAINER_T& data_container_r =
//      const_cast<DATA_CONTAINER_T&> (headFragment_->getR ());
//  DATA_T& data_r = const_cast<DATA_T&> (data_container_r.getR ());
//  ACE_ASSERT (!data_r.handShakeRecord);
//  data_r.handShakeRecord = handShake_inout;
//  handShake_inout = NULL;
//
//  // debug info
//  if (inherited2::trace_)
//    ACE_DEBUG ((LM_INFO,
//                ACE_TEXT ("%s"),
//                ACE_TEXT (BitTorrent_Tools::HandShakeToString (*data_r.handShakeRecord).c_str ())));
//
//  // step1: set session data
//  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
//      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
//
//  // sanity check(s)
//  // *TODO*: remove type inference
//  ACE_ASSERT (!session_data_r.handShake);
//
//  ACE_NEW_NORETURN (session_data_r.handShake,
//                    struct BitTorrent_PeerHandShake ());
//  if (!session_data_r.handShake)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
//                inherited::mod_->name ()));
//    return;
//  } // end IF
//  *session_data_r.handShake = *data_r.handShakeRecord;
//
//  // step2: set new head fragment ?
//  ACE_Message_Block* message_block_p = headFragment_;
//  do
//  {
//    if (!message_block_p->cont ()) break;
//    message_block_p = message_block_p->cont ();
//  } while (true);
//  if (message_block_p != inherited2::fragment_)
//  {
//    message_block_p = headFragment_;
//    // *IMPORTANT NOTE*: the fragment has already been unlinked in the previous
//    //                   call to switchBuffer()
//    headFragment_ = dynamic_cast<DataMessageType*> (inherited2::fragment_);
//    ACE_ASSERT (headFragment_);
//  } // end IF
//  else
//  {
//    message_block_p = headFragment_;
//    headFragment_ = NULL;
//  } // end ELSE
//
//  // make sure the whole fragment chain references the same data record
//  DataMessageType* message_p =
//      dynamic_cast<DataMessageType*> (headFragment_->cont ());
//  while (message_p)
//  {
//    data_container_r.increase ();
//    DATA_CONTAINER_T* data_container_2 = &data_container_r;
//    message_p->initialize (data_container_2,
//                           session_data_r.sessionId,
//                           NULL);
//    message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
//  } // end WHILE
//
//  // step3: push message (chain)
//  int result = inherited::put_next (message_block_p, NULL);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", continuing\n"),
//                inherited::mod_->name ()));
//
//    // clean up
//    message_block_p->release ();
//  } // end IF
//}

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
BitTorrent_Module_TrackerParser_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  UserDataType>::BitTorrent_Module_TrackerParser_T (ISTREAM_T* stream_in)
#else
                                  UserDataType>::BitTorrent_Module_TrackerParser_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 (COMMON_PARSER_DEFAULT_LEX_TRACE,  // trace scanning ?
               COMMON_PARSER_DEFAULT_YACC_TRACE) // trace parsing ?
 , headFragment_ (NULL)
 , parserConfiguration ()
 , crunch_ (true) // strip protocol data ?
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerParser_T::BitTorrent_Module_TrackerParser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
BitTorrent_Module_TrackerParser_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  UserDataType>::~BitTorrent_Module_TrackerParser_T ()
      {
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerParser_T::~BitTorrent_Module_TrackerParser_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
bool
BitTorrent_Module_TrackerParser_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                             Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerParser_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    ACE_ASSERT (inherited::msg_queue_);
    result = inherited::msg_queue_->activate ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::activate(): \"%m\", continuing\n")));

    crunch_ = true;

    if (unlikely (headFragment_))
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF
  } // end IF

  parserConfiguration = *configuration_in.parserConfiguration;
  parserConfiguration.messageQueue = inherited::msg_queue_;
  if (unlikely (!inherited2::initialize(parserConfiguration)))
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
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
BitTorrent_Module_TrackerParser_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerParser_T::handleDataMessage"));

  DataMessageType* message_p = NULL;
//  int result = -1;
  bool release_inbound_message = true; // message_inout
  bool release_message = false; // message_p

  // initialize return value(s)
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  { //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    if (!headFragment_)
      headFragment_ = message_inout;
    else
    {
      for (message_p = headFragment_;
           message_p->cont ();
           message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
      message_p->cont (message_inout);

      //// just signal the parser (see below for an explanation)
      //result = condition_.broadcast ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%s\", continuing\n"),
      //              inherited::mod_->name ()));
    } // end ELSE

    message_p = headFragment_;
  } // end lock scope
  ACE_ASSERT (message_p);
  message_inout = NULL;
  release_inbound_message = false;

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!this->parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_IParser::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully
  } // end lock scope

//  // *NOTE*: the message has been parsed successfully
//  //         --> pass the data (chain) downstream
//  {
//    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

//    //// *NOTE*: new data fragments may have arrived by now
//    ////         --> set the next head fragment ?
//    //message_2 = dynamic_cast<DataMessageType*> (message_p->cont ());
//    //if (message_2)
//    //  message_p->cont (NULL);

//    result = inherited::put_next (headFragment_, NULL);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
//                  inherited::mod_->name ()));

//      // clean up
//      headFragment_->release ();

//      goto error;
//    } // end IF
//    headFragment_ = NULL;
//  } // end lock scope

//continue_:
error:
  if (release_inbound_message)
  {
    ACE_ASSERT (message_inout);
    message_inout->release (); message_inout = NULL;
  } // end IF
  if (release_message)
  {
    ACE_ASSERT (message_p);
    message_p->release ();
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
BitTorrent_Module_TrackerParser_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerParser_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: (in a 'passive' scenario,) a parser thread may be waiting for
      //         additional (entity) fragments to arrive
      //         --> tell it to return
      ACE_ASSERT (inherited::msg_queue_);
      result = inherited::msg_queue_->pulse ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue_Base::pulse(): \"%m\", continuing\n")));

      if (unlikely (headFragment_))
      {
        headFragment_->release (); headFragment_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
BitTorrent_Module_TrackerParser_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  UserDataType>::record (Bencoding_Dictionary_t*& bencoding_inout)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_TrackerParser_T::record"));

  // sanity check(s)
  ACE_ASSERT (bencoding_inout);
  ACE_ASSERT (headFragment_);

  DATA_CONTAINER_T& data_container_r =
      const_cast<DATA_CONTAINER_T&> (headFragment_->getR ());
  DATA_T& data_r =
      const_cast<DATA_T&> (data_container_r.getR ());

  // debug info
  if (inherited2::trace_)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s"),
                ACE_TEXT (Common_Parser_Bencoding_Tools::DictionaryToString (*bencoding_inout).c_str ())));

  // set new head fragment ?
  ACE_Message_Block* message_block_p = headFragment_;
  do
  {
    if (!message_block_p->cont ()) break;
    message_block_p = message_block_p->cont ();
  } while (true);
  if (message_block_p != inherited2::fragment_)
  {
    message_block_p = headFragment_;
    // *IMPORTANT NOTE*: the fragment has already been unlinked in the previous
    //                   call to switchBuffer()
    headFragment_ = dynamic_cast<DataMessageType*> (inherited2::fragment_);
    ACE_ASSERT (headFragment_);
  } // end IF
  else
  {
    message_block_p = headFragment_;
    headFragment_ = NULL;
  } // end ELSE

  // make sure the whole fragment chain references the same data record
  DataMessageType* message_p =
      dynamic_cast<DataMessageType*> (message_block_p->cont ());
  while (message_p)
  {
    data_container_r.increase ();
    DATA_CONTAINER_T* data_container_2 = &data_container_r;
    message_p->initialize (data_container_2,
                           NULL);
    message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  int result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

    // clean up
    message_block_p->release ();
  } // end IF
}
