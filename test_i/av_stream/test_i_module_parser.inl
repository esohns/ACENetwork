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

#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Test_I_AVStream_Parser_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                         SessionMessageType>::Test_I_AVStream_Parser_T (ISTREAM_T* stream_in)
#else
                         SessionMessageType>::Test_I_AVStream_Parser_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Parser_T::Test_I_AVStream_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Test_I_AVStream_Parser_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::message_cb ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Parser_T::Test_I_AVStream_Parser_T"));

  // sanity check(s)
  ACE_ASSERT (inherited::headFragment_);
  ACE_ASSERT (inherited::sessionData_);

  // frame message
  ACE_Message_Block* message_block_p = inherited::headFragment_;
  ACE_Message_Block* message_block_2 = NULL;
  ACE_UINT32 bytes_to_skip_i =
    sizeof (struct acestream_av_stream_header) + inherited::header_.length;
  ACE_UINT32 bytes_available_i = 0;
  while (bytes_to_skip_i)
  {
    bytes_available_i = message_block_p->length ();
    if (bytes_to_skip_i > bytes_available_i)
    {
      bytes_to_skip_i -= bytes_available_i;
      message_block_p = message_block_p->cont ();
      ACE_ASSERT (message_block_p);
    } // end IF
    else if (bytes_to_skip_i == bytes_available_i)
    {
      message_block_2 = message_block_p->cont ();
      if (message_block_2)
        message_block_p->cont (NULL);
      message_block_p = inherited::headFragment_;
      inherited::headFragment_ =
        static_cast<DataMessageType*> (message_block_2);
      break;
    } // end ELSE IF
    else
    {
      message_block_2 = message_block_p->duplicate ();
      ACE_ASSERT (message_block_2);
      message_block_p->length (bytes_to_skip_i);
      message_block_p->cont (NULL);
      message_block_2->rd_ptr (bytes_to_skip_i);
      message_block_p = inherited::headFragment_;
      inherited::headFragment_ =
        static_cast<DataMessageType*> (message_block_2);
      break;
    } // end ELSE
  } // end WHILE

  // frame data
  bytes_to_skip_i = sizeof (struct acestream_av_stream_header);
  message_block_2 = message_block_p;
  while (bytes_to_skip_i)
  {
    bytes_available_i = message_block_2->length ();
    if (bytes_to_skip_i >= bytes_available_i)
    {
      bytes_to_skip_i -= bytes_available_i;
      message_block_2->rd_ptr (bytes_available_i);
      message_block_2 = message_block_2->cont ();
      ACE_ASSERT (message_block_2);
    } // end IF
    else
    {
      message_block_2->rd_ptr (bytes_to_skip_i);
      break;
    } // end ELSE
  } // end WHILE

  // initialize message
  DataMessageType* message_p = static_cast<DataMessageType*> (message_block_p);
  typename DataMessageType::DATA_T& message_data_r =
    const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
  message_data_r.header = inherited::header_;

  // push downstream
  int result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
}
