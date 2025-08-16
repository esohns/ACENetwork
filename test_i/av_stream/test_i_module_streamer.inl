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

#include "ace/Assert.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_macros.h"

#include "test_i_av_stream_common.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
Test_I_AVStream_Streamer_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           UserDataType>::Test_I_AVStream_Streamer_T (ISTREAM_T* stream_in)
#else
                           UserDataType>::Test_I_AVStream_Streamer_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , sessionId_ (-1)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Streamer_T::Test_I_AVStream_Streamer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
Test_I_AVStream_Streamer_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Streamer_T::handleDataMessage"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // prepend message header
  message_block_p =
    inherited::allocateMessage (sizeof (struct acestream_av_stream_header));
  ACE_ASSERT (message_block_p);
  struct acestream_av_stream_header header_s;
  ACE_OS::memset (&header_s, 0, sizeof (struct acestream_av_stream_header));
  ACE_ASSERT (!message_inout->cont ());
  // *TODO*: support lengths of more than std::numeric_limits<ACE_UINT32>::max() bytes
  //         --> change the header_s.length to ACE_UINT64, update scanner.l and
  //             regenerate the scanner
  header_s.length = static_cast<ACE_UINT32> (message_inout->length ());
  switch (message_inout->getMediaType ())
  {
    case STREAM_MEDIATYPE_AUDIO:
      header_s.type = AVSTREAM_MESSAGE_AUDIO;
      break;
    case STREAM_MEDIATYPE_VIDEO:
      header_s.type = AVSTREAM_MESSAGE_VIDEO;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type (was: %d), aborting\n"),
                  message_inout->getMediaType ()));
      passMessageDownstream_out = false;
      message_inout->release (); message_inout = NULL;
      return;
    }
  } // end SWITCH

  result = message_block_p->copy (reinterpret_cast<char*> (&header_s),
                                  sizeof (struct acestream_av_stream_header));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%u): \"%m\", aborting\n"),
                inherited::mod_->name (),
                sizeof (struct acestream_av_stream_header)));
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;
    message_block_p->release ();
    return;
  } // end IF

  passMessageDownstream_out = false;
  message_block_p->cont (message_inout);
  message_inout = NULL;
  DataMessageType* message_p = static_cast<DataMessageType*> (message_block_p);
  message_p->initialize (sessionId_,
                         NULL);

  result = inherited::put_next (message_block_p,
                                NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_block_p->release ();
    return;
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
Test_I_AVStream_Streamer_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVStream_Streamer_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->getR ();

      sessionId_ = session_data_r.sessionId;

      break;

// error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}
