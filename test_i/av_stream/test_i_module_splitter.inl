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

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType>
Test_I_AVStream_Splitter_T<ACE_SYNCH_USE,
                           ConfigurationType,
                           ControlMessageType,
                           MessageType,
                           SessionMessageType,
                           SessionDataType>::Test_I_AVStream_Splitter_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Splitter_T::Test_I_AVStream_Splitter_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Test_I_AVStream_Splitter_T<ACE_SYNCH_USE,
                           ConfigurationType,
                           ControlMessageType,
                           MessageType,
                           SessionMessageType,
                           SessionDataType>::forward (MessageType* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_AVStream_Splitter_T::forward"));

  // sanity check(s)
  ACE_ASSERT (message_in);

  int result = -1;
  const typename MessageType::DATA_T& message_data_r = message_in->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  const typename MessageType::DATA_T::DATA_T& message_data_2 =
    message_data_r.getR ();
#endif // ACE_WIN32 || ACE_WIN64
  std::string branch_name_string;
  ACE_Message_Block* message_block_p = message_in->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // map message type to branch name
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (message_data_r.header.type)
#else
  switch (message_data_2.header.type)
#endif // ACE_WIN32 || ACE_WIN64
  {
    case AVSTREAM_MESSAGE_AUDIO:
    {
      branch_name_string =
        ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_AUDIO_NAME_STRING);
      break;
    }
    case AVSTREAM_MESSAGE_VIDEO:
    {
      branch_name_string =
        ACE_TEXT_ALWAYS_CHAR (TEST_I_AVSTREAM_MODULE_SPLITTER_BRANCH_VIDEO_NAME_STRING);
      break;
    }
    default:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message type (was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_data_r.header.type));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message type (was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_data_2.header.type));
#endif // ACE_WIN32 || ACE_WIN64
      message_block_p->release (); message_block_p = NULL;
      return;
    }
  } // end SWITCH

  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    typename inherited::BRANCH_TO_HEAD_CONST_ITERATOR_T iterator =
      inherited::heads_.find (branch_name_string);
    ACE_ASSERT (iterator != inherited::heads_.end ());
    ACE_ASSERT ((*iterator).second);
    typename inherited::QUEUE_TO_MODULE_CONST_ITERATOR_T iterator_2 =
      std::find_if (inherited::modules_.begin (), inherited::modules_.end (),
                    std::bind (typename inherited::QUEUE_TO_MODULE_MAP_FIND_S (),
                               std::placeholders::_1,
                               (*iterator).second));
    ACE_ASSERT (iterator_2 != inherited::modules_.end ());
    ACE_ASSERT ((*iterator_2).first);
    result = (*iterator_2).first->enqueue_tail (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Base::enqueue_tail(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      return;
    } // end IF
  } // end lock scope
}
