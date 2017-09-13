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

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_macros.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"
#include "bittorrent_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
BitTorrent_Module_Streamer_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             SessionMessageType>::BitTorrent_Module_Streamer_T (ISTREAM_T* stream_in)
#else
                             SessionMessageType>::BitTorrent_Module_Streamer_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_Streamer_T::BitTorrent_Module_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
BitTorrent_Module_Streamer_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Module_Streamer_T::handleDataMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  // *NOTE*: as this is an "upstream" module, the "wording" is wrong
  //         --> the logic remains the same, though
  passMessageDownstream_out = true;

  // sanity check(s)
  ACE_ASSERT (message_inout->length () == 0);

  // serialize structured data
  // --> create the appropriate bytestream corresponding to its elements
  const typename DataMessageType::DATA_T& data_container_r =
      message_inout->getR ();
  const typename DataMessageType::DATA_T::DATA_T& record_r =
        data_container_r.getR ();

  // sanity check(s)
  ACE_ASSERT ((record_r.handShakeRecord || record_r.peerRecord) &&
              !(record_r.handShakeRecord && record_r.peerRecord));

  std::ostringstream converter;

  if (record_r.handShakeRecord)
  {
//    // sanity check
//    if (message_inout->space () < (49 + record_r.handShakeRecord->pstr.size ()))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("[%u]: not enough buffer space (was: %d/%d), aborting\n"),
//                  message_inout->id (),
//                  message_inout->space (),
//                  (49 + record_r.handShakeRecord->pstr.size ())));
//      goto error;
//    } // end IF

    unsigned char pstrlen =
        static_cast<unsigned char> (record_r.handShakeRecord->pstr.size ());
    result = message_inout->copy (reinterpret_cast<char*> (&pstrlen),
                                  1);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    result =
        message_inout->copy (record_r.handShakeRecord->pstr.c_str (),
                             pstrlen);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    result =
        message_inout->copy (reinterpret_cast<char*> (record_r.handShakeRecord->reserved),
                             BITTORRENT_PEER_HANDSHAKE_RESERVED_SIZE);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    result =
        message_inout->copy (record_r.handShakeRecord->info_hash.c_str (),
                             BITTORRENT_PRT_INFO_HASH_SIZE);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    result =
        message_inout->copy (record_r.handShakeRecord->peer_id.c_str (),
                             BITTORRENT_PRT_PEER_ID_LENGTH);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
      goto error;
    } // end IF
  } // end IF
  else
  {
    ACE_ASSERT (false);
    ACE_NOTSUP;

    ACE_NOTREACHED (return;)
  } // end ELSE

//continue_:
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("[%u]: streamed [%u byte(s)]...\n"),
//               message_inout->id (),
//               message_inout->length ()));

  return;

error:
  passMessageDownstream_out = false;
  message_inout->release ();
  message_inout = NULL;
}
