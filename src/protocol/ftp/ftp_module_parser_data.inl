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

#include "common_file_tools.h"

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
FTP_Module_Parser_Data_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::FTP_Module_Parser_Data_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , forwardImmediately_ (false)
 , record_ ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_Data_T::FTP_Module_Parser_Data_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
FTP_Module_Parser_Data_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::directory (const std::string& directoryEntry_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_Data_T::directory"));

  // sanity check(s)
  ACE_ASSERT (inherited::headFragment_);

  forwardImmediately_ = false;

  struct Common_File_Entry file_entry_s =
    Common_File_Tools::parseFileEntry (directoryEntry_in);
  if (likely (file_entry_s.type != Common_File_Entry::INVALID))
    record_.entries.push_back (file_entry_s);

  DataMessageType* message_p = inherited::allocateMessage (1);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate message: \"%m\", returning\n")));
    return;
  } // end IF

  push (FTP_Codes::FTP_RECORD_DIRECTORY,
        inherited::headFragment_->sessionId (),
        message_p);
  record_.entries.clear ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
FTP_Module_Parser_Data_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::file (const std::string& fileEntry_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_Data_T::file"));

  // sanity check(s)
  ACE_ASSERT (inherited::headFragment_);

  forwardImmediately_ = false;

  struct Common_File_Entry file_entry_s =
    Common_File_Tools::parseFileEntry (fileEntry_in);
  if (likely (file_entry_s.type != Common_File_Entry::INVALID))
    record_.entries.push_back (file_entry_s);

  DataMessageType* message_p = inherited::allocateMessage (1);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
               ACE_TEXT ("failed to allocate message: \"%m\", returning\n")));
    return;
  } // end IF

  push (FTP_Codes::FTP_RECORD_FILE,
        inherited::headFragment_->sessionId (),
        message_p);
  record_.entries.clear ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
FTP_Module_Parser_Data_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::data ()
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_Data_T::data"));

  // sanity check(s)
  ACE_ASSERT (inherited::headFragment_);

  // bypass parser from now on
  forwardImmediately_ = true;

  push (FTP_Codes::FTP_RECORD_DATA,
        inherited::headFragment_->sessionId (),
        inherited::headFragment_);
}

template <ACE_SYNCH_DECL,
         typename TimePolicyType,
         typename ConfigurationType,
         typename ControlMessageType,
         typename DataMessageType,
         typename SessionMessageType>
void
FTP_Module_Parser_Data_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::push (FTP_Codes::RecordType type_in,
                                                    Stream_SessionId_t sessionId_in,
                                                    DataMessageType*& message_inout)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_Data_T::push"));

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (!message_inout->isInitialized ());

  typename DataMessageType::DATA_T::DATA_T* data_p = NULL;
  typename DataMessageType::DATA_T* data_container_p = NULL, *data_container_2 = NULL;
  int result;

  ACE_NEW_NORETURN (data_p,
                   typename DataMessageType::DATA_T::DATA_T ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF
  *data_p = record_;
  data_p->type = type_in;

  ACE_NEW_NORETURN (data_container_p,
                    typename DataMessageType::DATA_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF
  data_container_p->setPR (data_p);
  ACE_ASSERT (!data_p);
  data_container_2 = data_container_p;
  message_inout->initialize (data_container_2,
                             sessionId_in,
                             NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  //message_p =
  //  static_cast<DataMessageType*> (inherited::headFragment_->cont ());
  //while (message_p)
  //{
  //  data_container_p->increase ();
  //  data_container_2 = data_container_p;
  //  message_p->initialize (data_container_2,
  //                         inherited::headFragment_->sessionId (),
  //                         NULL);
  //  message_p = static_cast<DataMessageType*> (message_p->cont ());
  //} // end WHILE

  // push message downstream
  result = inherited::put_next (message_inout);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to put_next(): \"%m\", returning\n")));
    message_inout->release (); message_inout = NULL;
  } // end IF
  message_inout = NULL;
}

template <ACE_SYNCH_DECL,
         typename TimePolicyType,
         typename ConfigurationType,
         typename ControlMessageType,
         typename DataMessageType,
         typename SessionMessageType>
int
FTP_Module_Parser_Data_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_Data_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  ACE_Message_Block* message_block_p;
  int result, error = 0;
  bool result_2;
  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    inherited::sessionData_->getR ();

  do
  {
deque:
    message_block_p = NULL;
    result = inherited::parserQueue_.dequeue_head (message_block_p,
                                                   NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
        ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Message_Queue::dequeue_head(): \"%m\", aborting\n"),
                   inherited::mod_->name ()));
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
        //         not have been set at this stage

        message_block_p->release (); message_block_p = NULL;

        result = 0;

        goto done;
      }
      default:
      {
        if (forwardImmediately_)
        {
          DataMessageType* message_p =
            static_cast<DataMessageType*> (message_block_p);
          push (FTP_Codes::FTP_RECORD_DATA,
                session_data_r.sessionId,
                message_p);
          goto deque;
        } // end IF

        // initialize message ?
        // message_p = static_cast<DataMessageType*> (message_block_p);

        //{ ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
        if (inherited::headFragment_)
          Stream_Tools::append (inherited::headFragment_,
                                message_block_p);
        else
          inherited::headFragment_ = static_cast<DataMessageType*> (message_block_p);

        // free 0-length fragments from the head
        // ACE_Message_Block* message_block_2 = headFragment_;
        // while (message_block_2 &&
        //        !message_block_2->length ())
        //   message_block_2 = message_block_2->cont ();
        // if (!message_block_2)
        // { // there is no data at all
        //   headFragment_->release (); headFragment_ = NULL;
        //   continue; // wait for more data
        // } // end IF
        // ACE_ASSERT (message_block_2 && message_block_2->length ());
        // if (message_block_2 != headFragment_)
        // {
        //   ACE_Message_Block* message_block_3 = headFragment_;
        //   while (message_block_3->cont () != message_block_2)
        //     message_block_3 = message_block_3->cont ();
        //   ACE_ASSERT (message_block_3 && (message_block_3->cont () == message_block_2));
        //   message_block_3->cont (NULL);
        //   headFragment_->release (); headFragment_ = NULL;
        //   headFragment_ = static_cast<DataMessageType*> (message_block_2);
        // } // end IF
        // ACE_ASSERT (headFragment_ && headFragment_->length ());
        //} // end lock scope
continue_:
        // parse next data fragment(s)
        try {
          result_2 = inherited::parse (inherited::headFragment_);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                     ACE_TEXT ("%s: caught exception in Common_IParser_T::parse(), continuing\n"),
                     inherited::mod_->name ()));
          result_2 = false;
        }
        if (unlikely (!result_2))
        {
          // *NOTE*: most probable reason: connection has been closed
          //         --> received end-of-session
          ACE_DEBUG (((inherited::error_ ? LM_ERROR : LM_DEBUG),
                     ACE_TEXT ("%s: failed to Common_IParser_T::parse(), aborting\n"),
                     inherited::mod_->name ()));
          if (unlikely (inherited::error_))
            goto error;
          break;
        } // end IF
        // *NOTE*: handle cases where all (available) data has been parsed
        if (inherited::PARSER_DRIVER_T::finished_)
        {
          inherited::PARSER_DRIVER_T::finished_ = false;
          break;
        } // end IF
        if (inherited::headFragment_ &&
            inherited::headFragment_->length ())
          goto continue_;

        break;

error:
        if (inherited::headFragment_)
        {
          inherited::headFragment_->release (); inherited::headFragment_ = NULL;
        } // end IF
        result = -1;

        goto error_2;
      }
    } // end SWITCH
  } while (true);
  result = -1;

done:
  return result;

error_2:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

  return result;
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
          typename StatisticContainerType,
          typename SessionManagerType>
FTP_Module_Parser_DataH_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          StatisticContainerType,
                          SessionManagerType>::FTP_Module_Parser_DataH_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_DataH_T::FTP_Module_Parser_DataH_T"));

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
          typename StatisticContainerType,
          typename SessionManagerType>
bool
FTP_Module_Parser_DataH_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          StatisticContainerType,
                          SessionManagerType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("FTP_Module_Parser_DataH_T::collect"));

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
