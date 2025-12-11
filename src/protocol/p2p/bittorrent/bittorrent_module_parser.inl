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

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"

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
                               UserDataType>::BitTorrent_Module_PeerParser_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , parserConfiguration_ ()
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

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  parserConfiguration_ = *configuration_in.parserConfiguration;
  parserConfiguration_.messageQueue = NULL;
  const_cast<ConfigurationType&> (configuration_in).parserConfiguration =
    &parserConfiguration_;

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
  ACE_ASSERT (inherited::headFragment_);
  ACE_ASSERT (record_inout);

  // allocate message data and -container
  DATA_T* data_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    DATA_T ());
  if (unlikely (!data_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                inherited::mod_->name ()));
    delete record_inout;
    return;
  } // end IF
  DATA_CONTAINER_T* data_container_p = NULL;
  ACE_NEW_NORETURN (data_container_p,
                    DATA_CONTAINER_T (data_p,
                                      true));
  if (unlikely (!data_container_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                inherited::mod_->name ()));
    delete data_p;
    delete record_inout;
    return;
  } // end IF
  Stream_SessionId_t session_id = inherited::headFragment_->sessionId ();
  inherited::headFragment_->initialize (data_container_p,
                                        session_id,
                                        NULL);
  data_container_p = NULL;

  DATA_CONTAINER_T& data_container_r =
    const_cast<DATA_CONTAINER_T&> (inherited::headFragment_->getR ());
  DATA_T& data_r = const_cast<DATA_T&> (data_container_r.getR ());
  ACE_ASSERT (!data_r.peerRecord);
  data_r.peerRecord = record_inout;
  record_inout = NULL;

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
        message_bytes = 4 + 1;
        break;
      case BITTORRENT_MESSAGETYPE_HAVE:
        message_bytes = 4 + 1 + 4;
        break;
      case BITTORRENT_MESSAGETYPE_BITFIELD:
        message_bytes = 4 + 1 + (data_r.peerRecord->length - 1);
        break;
      case BITTORRENT_MESSAGETYPE_REQUEST:
        message_bytes = 4 + 1 + 4 + 4 + 4;
        break;
      case BITTORRENT_MESSAGETYPE_PIECE:
        message_bytes = 4 + 1 + 4 + 4 + (data_r.peerRecord->length - 9);
        break;
      case BITTORRENT_MESSAGETYPE_CANCEL:
        message_bytes = 4 + 1 + 4 + 4 + 4;
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown record type (was: %d), returning\n"),
                    inherited::mod_->name (),
                    data_r.peerRecord->type));
        inherited::headFragment_->release (); inherited::headFragment_ = NULL;
        return;
      }
    } // end SWITCH
  } // end ELSE
  ACE_ASSERT (message_bytes);
  if (inherited::headFragment_->total_length () < message_bytes)
  { // *TODO*: find out why this happens
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: received only %Q/%u record byte(s), returning\n"),
                inherited::mod_->name (),
                inherited::headFragment_->total_length (), message_bytes));
    inherited::headFragment_->release (); inherited::headFragment_ = NULL;
    return;
  } // end IF

  ACE_Message_Block* message_block_p = inherited::headFragment_;
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
          ACE_Message_Block* message_block_2 = inherited::headFragment_;
          inherited::headFragment_ =
            static_cast<DataMessageType*> (message_block_p->cont ());
          message_block_p->cont (NULL);
          message_block_p = message_block_2;
        } // end IF
        else
        {
          message_block_p = inherited::headFragment_;
          inherited::headFragment_ = NULL;
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
      message_block_p = inherited::headFragment_;
      inherited::headFragment_ =
        static_cast<DataMessageType*> (message_block_2);
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
          message_block_p = inherited::headFragment_;
          inherited::headFragment_ =
            static_cast<DataMessageType*> (message_block_2);
        } // end IF
        else
        {
          if (message_block_p->cont ())
          {
            ACE_Message_Block* message_block_2 = inherited::headFragment_;
            inherited::headFragment_ =
              static_cast<DataMessageType*> (message_block_p->cont ());
            message_block_p->cont (NULL);
            message_block_p = message_block_2;
          } // end IF
          else
          {
            message_block_p = inherited::headFragment_;
            inherited::headFragment_ = NULL;
          } // end ELSE
        } // end ELSE
        break;
      } // end IF
      message_block_p = message_block_p->cont ();
      ACE_ASSERT (message_block_p);
    } while (true);
    ACE_ASSERT (message_block_p->total_length () == message_bytes);
  } // end ELSE

  // make sure the whole fragment chain references the same data record
  DataMessageType* message_p =
    static_cast<DataMessageType*> (message_block_p->cont ());
  while (message_p)
  {
    data_container_r.increase ();
    DATA_CONTAINER_T* data_container_2 = &data_container_r;
    message_p->initialize (data_container_2,
                           session_id,
                           NULL);
    message_p = static_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  int result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
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

//#if defined (_DEBUG)
//  if (inherited2::configuration_->debugParser)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s"),
//                ACE_TEXT (BitTorrent_Tools::HandShakeToString (*handshake_inout).c_str ())));
//#endif // _DEBUG

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, const_cast<ACE_MT_SYNCH::MUTEX&> (inherited::sessionData_->getR_2 ()));
    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

    // sanity check(s)
    ACE_ASSERT (!session_data_r.handshake);

    // *TODO*: remove type inference
    session_data_r.handshake = handshake_inout;
  } // end lock scope

  handshake_inout = NULL;

  unsigned int handshake_bytes = inherited::offset ();
  ACE_Message_Block* message_block_p = inherited::headFragment_;
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
                                  UserDataType>::BitTorrent_Module_TrackerParser_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
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
  ACE_ASSERT (inherited::headFragment_);
  ACE_ASSERT (bencoding_inout);

  DATA_CONTAINER_T& data_container_r =
    const_cast<DATA_CONTAINER_T&> (inherited::headFragment_->getR ());
  DATA_T& data_r =
    const_cast<DATA_T&> (data_container_r.getR ());

// #if defined (_DEBUG)
//  if (inherited::configuration_->debugParser)
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s\n"),
//                ACE_TEXT (Common_Parser_Bencoding_Tools::DictionaryToString (*bencoding_inout).c_str ())));
//#endif // _DEBUG

  // set new head fragment ?
  ACE_Message_Block* message_block_p = inherited::headFragment_;
  do
  {
    if (!message_block_p->cont ())
      break;
    message_block_p = message_block_p->cont ();
  } while (true);
  if (message_block_p != inherited::fragment_)
  {
    message_block_p = inherited::headFragment_;
    // *IMPORTANT NOTE*: the fragment has already been unlinked in the previous
    //                   call to switchBuffer()
    inherited::headFragment_ =
      static_cast<DataMessageType*> (inherited::fragment_);
    ACE_ASSERT (inherited::headFragment_);
  } // end IF
  else
  {
    message_block_p = inherited::headFragment_;
    inherited::headFragment_ = NULL;
  } // end ELSE

  // make sure the whole fragment chain references the same data record
  DataMessageType* message_p =
    static_cast<DataMessageType*> (message_block_p->cont ());
  while (message_p)
  {
    data_container_r.increase ();
    DATA_CONTAINER_T* data_container_2 = &data_container_r;
    message_p->initialize (data_container_2,
                           NULL);
    message_p = static_cast<DataMessageType*> (message_p->cont ());
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
