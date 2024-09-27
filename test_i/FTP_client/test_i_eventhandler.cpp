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
#include "stdafx.h"

#include "test_i_eventhandler.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include <regex>

#include "ace/FILE_Addr.h"
#include "ace/FILE_Connector.h"
#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_file_tools.h"
#include "common_string_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "net_macros.h"

#include "ftp_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "test_i_defines.h"

#include "test_i_ftp_client_defines.h"

#if defined (GUI_SUPPORT)
Test_I_EventHandler::Test_I_EventHandler (struct FTP_Client_UI_CBData* CBData_in,
                                          FTP_IControl* control_in)
#else
Test_I_EventHandler::Test_I_EventHandler (FTP_IControl* control_in)
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , control_ (control_in)
#else
 : control_(control_in)
#endif // GUI_SUPPORT
 , sessionData_(NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::Test_I_EventHandler"));

  // sanity check(s)
  ACE_ASSERT (control_);
}

void
Test_I_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct FTP_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = &const_cast<struct FTP_Client_SessionData&> (sessionData_in);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

//  CBData_->progressData.transferred = 0;
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Stream_SessionMessageType& sessionEvent_in,
                             bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = NULL;
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  FTP_Client_MessageData_t& data_r =
    const_cast<FTP_Client_MessageData_t&> (message_in.getR ());
  struct FTP_Client_MessageData& record_r =
    const_cast<struct FTP_Client_MessageData&> (data_r.getR ());

  control_->responseCB (record_r);

#if defined (GUI_SUPPORT)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    CBData_->records.push_back (record_r);
  } // end lock scope
#if defined (GTK_USE)
  guint event_source_id = g_idle_add (idle_response_received_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_response_received_cb): \"%m\", returning\n")));
    return;
  } // end IF
  state_r.eventSourceIds.insert (event_source_id);

  if (record_r.code == FTP_Codes::FTP_CODE_USER_LOGGED_IN)
  {
    event_source_id = g_idle_add (idle_login_complete_cb,
                                  CBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_login_complete_cb): \"%m\", returning\n")));
      return;
    } // end IF
    state_r.eventSourceIds.insert (event_source_id);
  } // end IF
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    CBData_->progressData.statistic.bytes += message_in.total_length ();
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    { ACE_ASSERT (sessionData_);
      if (sessionData_->lock)
      {
        result = sessionData_->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

      CBData_->progressData.statistic =
        sessionData_->statistic;

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    {
      event_e = COMMON_UI_EVENT_STEP;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
  state_r.eventStack.push (event_e);
#endif // GUI_SUPPORT
}

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
Test_I_EventHandler_2::Test_I_EventHandler_2 (struct FTP_Client_UI_CBData* CBData_in)
#else
Test_I_EventHandler_2::Test_I_EventHandler_2 ()
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , stream_ ()
#else
 : stream_ ()
#endif // GUI_SUPPORT
 , sessionData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::Test_I_EventHandler_2"));

}

void
Test_I_EventHandler_2::start (Stream_SessionId_t sessionId_in,
                              const struct FTP_Client_SessionData& sessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::start"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = &const_cast<struct FTP_Client_SessionData&> (sessionData_in);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

//  CBData_->progressData.transferred = 0;
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Stream_SessionMessageType& sessionEvent_in,
                               bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_EventHandler_2::end (Stream_SessionId_t sessionId_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::end"));

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE

#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
#endif // GTK_USE
#endif // GUI_SUPPORT

  sessionData_ = NULL;

  if (stream_.get_handle () != ACE_INVALID_HANDLE)
  {
    int result = stream_.close ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n")));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed file...\n")));

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    guint event_source_id = g_idle_add (idle_data_received_cb,
                                        CBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_data_received_cb): \"%m\", returning\n")));
      return;
    } // end IF
    state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
#endif // GUI_SUPPORT
  } // end IF
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_Message& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
  CBData_->entries.clear ();
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

  FTP_Client_MessageData_t& data_container_r =
    const_cast<FTP_Client_MessageData_t&> (message_in.getR ());
  struct FTP_Client_MessageData& record_r =
    const_cast<struct FTP_Client_MessageData&> (data_container_r.getR ());

  switch (record_r.type)
  {
    case FTP_Codes::FTP_RECORD_DIRECTORY:
    {
      std::string buffer_string =
        Net_Common_Tools::bufferToString (&const_cast<Test_I_Message&> (message_in));
      std::istringstream converter (buffer_string);
      char buffer_a[BUFSIZ];
      struct Common_File_Entry file_entry_s;

#if defined (GUI_SUPPORT)
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GUI_SUPPORT

      do
      {
        converter.getline (buffer_a, sizeof (char[BUFSIZ]));
        std::string buffer_string_2 = buffer_a;
        buffer_string_2 = Common_String_Tools::strip (buffer_string_2);
        if (unlikely (buffer_string_2.empty ()))
          continue;
        file_entry_s = Common_File_Tools::parseFileEntry (buffer_string_2);
        if (unlikely (file_entry_s.type == Common_File_Entry::INVALID))
          continue;
#if defined (GUI_SUPPORT)
        CBData_->entries.push_back (file_entry_s);
#endif // GUI_SUPPORT
      } while (!converter.fail ());

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      guint event_source_id = g_idle_add (idle_list_received_cb,
                                          CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_list_received_cb): \"%m\", returning\n")));
        return;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
#endif // GTK_USE
#endif // GUI_SUPPORT

      break;
    }
    case FTP_Codes::FTP_RECORD_FILE:
    {
      std::string buffer_string =
        Net_Common_Tools::bufferToString (&const_cast<Test_I_Message&> (message_in));
      std::istringstream converter (buffer_string);
      char buffer_a[BUFSIZ];
      struct Common_File_Entry file_entry_s;

#if defined (GUI_SUPPORT)
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GUI_SUPPORT

      do
      {
        converter.getline (buffer_a, sizeof (char[BUFSIZ]));
        std::string buffer_string_2 = buffer_a;
        buffer_string_2 = Common_String_Tools::strip (buffer_string_2);
        if (unlikely (buffer_string_2.empty ()))
          continue;
        file_entry_s = Common_File_Tools::parseFileEntry (buffer_string_2);
        if (unlikely (file_entry_s.type == Common_File_Entry::INVALID))
          continue;
#if defined (GUI_SUPPORT)
        CBData_->entries.push_back (file_entry_s);
#endif // GUI_SUPPORT
      } while (!converter.fail ());

      break;
    }
    case FTP_Codes::FTP_RECORD_DATA:
    {
      int result = -1; 
      if (unlikely (stream_.get_handle () == ACE_INVALID_HANDLE))
      {
        std::string filename_string = Common_File_Tools::getWorkingDirectory ();
        filename_string += ACE_TEXT_ALWAYS_CHAR (ACE_DIRECTORY_SEPARATOR_STR);
#if defined (GUI_SUPPORT)
        filename_string += CBData_->fileName;
#endif // GUI_SUPPORT
        ACE_FILE_Addr file_address (ACE_TEXT (filename_string.c_str ()));

        ACE_FILE_Connector file_connector;
        result = file_connector.connect (stream_,
                                         file_address,
                                         NULL,
                                         ACE_Addr::sap_any,
                                         0,
                                         O_WRONLY | O_CREAT | O_BINARY,
                                         ACE_DEFAULT_FILE_PERMS);
        if (unlikely ((result == -1) ||
                      (stream_.get_handle () == ACE_INVALID_HANDLE)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_Connector::connect(%s): \"%m\", returning\n"),
                      ACE_TEXT (filename_string.c_str ())));
          return;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("opened file \"%s\"...\n"),
                    ACE_TEXT (filename_string.c_str ())));
      } // end IF

      ACE_Message_Block* message_block_p = &const_cast<Test_I_Message&> (message_in);
      ssize_t result_2 = -1;
      while (message_block_p)
      {
        result_2 = stream_.send (message_block_p->rd_ptr (),
                                 message_block_p->length ());
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::send(%B): \"%m\", returning\n"),
                      message_block_p->length ()));
          return;
        } // end IF
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("wrote %b byte(s)...\n"),
                      message_block_p->length ()));
        message_block_p = message_block_p->cont ();
      } // end WHILE

#if defined (GUI_SUPPORT)
      { ACE_GUARD(ACE_SYNCH_MUTEX, aGuard, state_r.lock);
        CBData_->progressData.transferred += message_in.total_length ();
      } // end lock scope
#endif // GUI_SUPPORT

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown FTP data record type: %d, returning\n"),
                  record_r.type));
      return;
    }
  } // end SWITCH

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
  CBData_->progressData.statistic.bytes += message_in.total_length ();
#if defined (GTK_USE)
  state_r.eventStack.push (COMMON_UI_EVENT_DATA);
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Test_I_EventHandler_2::notify (Stream_SessionId_t sessionId_in,
                               const Test_I_SessionMessage& sessionMessage_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_EventHandler_2::notify"));

  int result = -1;

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      event_e = COMMON_UI_EVENT_CONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      event_e = COMMON_UI_EVENT_DISCONNECT;
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    { ACE_ASSERT (sessionData_);
      if (sessionData_->lock)
      {
        result = sessionData_->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

      CBData_->progressData.statistic = sessionData_->statistic;

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    case STREAM_SESSION_MESSAGE_STEP_DATA:
    {
      event_e = COMMON_UI_EVENT_STEP;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH
  state_r.eventStack.push (event_e);
#endif // GUI_SUPPORT
}
