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

#ifndef TEST_I_EVENTHANDLER_H
#define TEST_I_EVENTHANDLER_H

#include "ace/FILE_IO.h"
#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "test_i_stream_common.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

// forward declarations
#if defined (GUI_SUPPORT)
struct FTP_Client_UI_CBData;
#endif // GUI_SUPPORT

class Test_I_EventHandler
 : public FTP_Client_ISessionNotify_t
{
 public:
#if defined (GUI_SUPPORT)
  Test_I_EventHandler (struct FTP_Client_UI_CBData*, // UI callback data handle
                       FTP_IControl*);               // FTP control handle
#else
  Test_I_EventHandler (FTP_IControl*);               // FTP control handle
#endif // GUI_SUPPORT
  inline virtual ~Test_I_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                    // session id
                      const struct FTP_Client_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);                // session id
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_I_Message&);          // (protocol) message
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_I_SessionMessage&);   // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler (const Test_I_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler& operator= (const Test_I_EventHandler&))

#if defined (GUI_SUPPORT)
  struct FTP_Client_UI_CBData*   CBData_;
#endif // GUI_SUPPORT
  FTP_IControl*                  control_;
  struct FTP_Client_SessionData* sessionData_;
};

//////////////////////////////////////////

class Test_I_EventHandler_2
 : public FTP_Client_ISessionNotify_t
{
 public:
#if defined (GUI_SUPPORT)
  Test_I_EventHandler_2 (struct FTP_Client_UI_CBData*); // UI callback data handle
#else
  Test_I_EventHandler_2 ();
#endif // GUI_SUPPORT
  inline virtual ~Test_I_EventHandler_2 () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                    // session id
                      const struct FTP_Client_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);                // session id
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_I_Message&);          // (protocol) message
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_I_SessionMessage&);   // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_2 ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_2 (const Test_I_EventHandler_2&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_2& operator= (const Test_I_EventHandler_2&))

#if defined (GUI_SUPPORT)
  struct FTP_Client_UI_CBData*   CBData_;
#endif // GUI_SUPPORT
  ACE_FILE_IO                    stream_;
  struct FTP_Client_SessionData* sessionData_;
};

#endif
