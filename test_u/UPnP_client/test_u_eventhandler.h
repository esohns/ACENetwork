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

#ifndef TEST_U_EVENTHANDLER_H
#define TEST_U_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "test_u_stream_common.h"

#include "test_u_message.h"
#include "test_u_session_message.h"

// forward declarations
#if defined (GUI_SUPPORT)
struct UPnP_Client_UI_CBData;
#endif // GUI_SUPPORT

class Test_U_EventHandler
 : public UPnP_Client_ISessionNotify_t
{
 public:
#if defined (GUI_SUPPORT)
  Test_U_EventHandler (struct UPnP_Client_UI_CBData*); // UI callback data handle
#else
  Test_U_EventHandler ();
#endif // GUI_SUPPORT
  inline virtual ~Test_U_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                    // session id
                      const struct UPnP_Client_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);                // session id
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_U_Message&);          // (protocol) message
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_U_SessionMessage&);   // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler (const Test_U_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler& operator= (const Test_U_EventHandler&))

#if defined (GUI_SUPPORT)
  struct UPnP_Client_UI_CBData*   CBData_;
#endif // GUI_SUPPORT
  struct UPnP_Client_SessionData* sessionData_;
};

//////////////////////////////////////////

enum Test_U_EventHandlerState
{
  EVENT_HANDLER_STATE_DEVICE = 0, // got device description --> retrieve service description
  EVENT_HANDLER_STATE_SERVICE, // got service description --> send SOAP commands
  EVENT_HANDLER_STATE_EXTERNAL_ADDRESS_CONTROL, // receive SOAP reply
  EVENT_HANDLER_STATE_MAP_CONTROL, // receive SOAP reply
  ////////////////////////////////////////
  EVENT_HANDLER_STATE_MAX,
  EVENT_HANDLER_STATE_INVALID
};

class Test_U_EventHandler_2
 : public UPnP_Client_ISessionNotify_t
{
 public:
#if defined (GUI_SUPPORT)
  Test_U_EventHandler_2 (struct UPnP_Client_UI_CBData*); // UI callback data handle
#else
  Test_U_EventHandler_2 ();
#endif // GUI_SUPPORT
  inline virtual ~Test_U_EventHandler_2 () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                    // session id
                      const struct UPnP_Client_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);                // session id
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_U_Message&);          // (protocol) message
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_U_SessionMessage&);   // session message

  enum Test_U_EventHandlerState   state_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler_2 ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler_2 (const Test_U_EventHandler_2&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler_2& operator= (const Test_U_EventHandler_2&))

  // helper methods
  std::string retrievePresentationURL (xmlDocPtr);

#if defined (GUI_SUPPORT)
  struct UPnP_Client_UI_CBData*   CBData_;
#endif // GUI_SUPPORT
  struct UPnP_Client_SessionData* sessionData_;
};

#endif
