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

#ifndef TEST_U_SESSION_MESSAGE_H
#define TEST_U_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#include "stream_session_message_base.h"

#include "test_u_pcp_client_common.h"

// forward declaration(s)
class ACE_Allocator;
class Test_U_Message;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

struct PCP_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          //PCPClient_ConnectionConfiguration,
                          struct PCP_ConnectionState,
                          PCP_Statistic_t> PCPClient_IConnection_t;
struct PCPClient_SessionData
 : Test_U_StreamSessionData
{
  PCPClient_SessionData ()
   : Test_U_StreamSessionData ()
   , connection (NULL) // outbound
   //, userData (NULL)
  {}
  struct PCPClient_SessionData& operator= (const struct PCPClient_SessionData& rhs_in)
  {
    Test_U_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    return *this;
  }

  PCPClient_IConnection_t* connection;          // RELEASE
  std::string              targetFileName;      // file writer module

  //struct Stream_UserData*   userData;
};
typedef Stream_SessionData_T<struct PCPClient_SessionData> PCPClient_SessionData_t;

class Test_U_SessionMessage
 : public Stream_SessionMessageBase_T<//struct PCP_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      PCPClient_SessionData_t,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<//struct PCP_AllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      PCPClient_SessionData_t,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct PCP_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_U_Message,
                                                 Test_U_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  Test_U_SessionMessage (Stream_SessionId_t,
                         enum Stream_SessionMessageType,
                         PCPClient_SessionData_t*&, // session data container handle
                         struct Stream_UserData*,
                         bool); // expedited ?
  // copy ctor to be used by duplicate()
  Test_U_SessionMessage (const Test_U_SessionMessage&);
  inline virtual ~Test_U_SessionMessage () {}

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_U_SessionMessage (Stream_SessionId_t,
                         ACE_Allocator*);    // message allocator
  Test_U_SessionMessage (Stream_SessionId_t,
                         ACE_Data_Block*,    // data block to use
                         ACE_Allocator*);    // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_U_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_SessionMessage& operator= (const Test_U_SessionMessage&))
};

#endif
