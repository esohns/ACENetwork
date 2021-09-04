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

#ifndef DHCP_COMMON_H
#define DHCP_COMMON_H

#include <map>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/OS.h"
#include "ace/Time_Value.h"

#include "common_inotify.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"

#include "net_common.h"

#include "dhcp_codes.h"
#include "dhcp_defines.h"

// forward declarations
struct DHCP_Configuration;
struct DHCP_Record;
class DHCP_SessionMessage;
struct DHCP_Stream_SessionData;

typedef Stream_ISessionDataNotify_T<struct DHCP_Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    DHCP_Record,
                                    DHCP_SessionMessage> DHCP_ISessionNotify_t;

typedef Net_StreamStatistic_t DHCP_Statistic_t;
typedef Common_IStatistic_T<DHCP_Statistic_t> DHCP_StatisticReportingHandler_t;
typedef Common_StatisticHandler_T<DHCP_Statistic_t> DHCP_StatisticHandler_t;

// convenient type definitions
typedef DHCP_Codes::OpType DHCP_Op_t;
typedef DHCP_Codes::OptionFieldType DHCP_OptionFieldType_t;
typedef DHCP_Codes::OptionType DHCP_Option_t;
typedef DHCP_Codes::MessageType DHCP_MessageType_t;

//typedef std::list<std::pair<ACE_UINT8, std::string> > DHCP_Options_t;
typedef std::map<ACE_UINT8, std::string> DHCP_Options_t;
typedef DHCP_Options_t::const_iterator DHCP_OptionsIterator_t;
struct DHCP_Record
{
  DHCP_Record ()
   : op (DHCP_Codes::DHCP_OP_INVALID)
   , htype (0)
   , hlen (0)
   , hops (0)
   , xid (0)
   , secs (0)
   , flags (0)
   , ciaddr (0)
   , yiaddr (0)
   , siaddr (0)
   , giaddr (0)
//   , chaddr ()
   , sname ()
   , file ()
   , cookie (0)
   , options ()
  {
    ACE_OS::memset (chaddr, 0, sizeof (ACE_UINT8[DHCP_CHADDR_SIZE]));
//    ACE_OS::memset (options, 0, 312);
  }

  DHCP_Codes::OpType op;
  ACE_UINT8          htype;
  ACE_UINT8          hlen;
  ACE_UINT8          hops;
  ACE_UINT32         xid;
  ACE_UINT16         secs;
  ACE_UINT16         flags;
  ACE_UINT32         ciaddr;
  ACE_UINT32         yiaddr;
  ACE_UINT32         siaddr;
  ACE_UINT32         giaddr;
  ACE_UINT8          chaddr[DHCP_CHADDR_SIZE];
  std::string        sname;   // max. 64 bytes
  std::string        file;    // max. 128 bytes
  ACE_UINT32         cookie;
  DHCP_Options_t     options; // max. 312 bytes (negotiable)
//  unsigned char      options[DHCP_OPTIONS_SIZE];
};
struct DHCP_MessageData
{
  DHCP_MessageData ()
   : DHCPRecord (NULL)
  {}
  ~DHCP_MessageData ()
  {
    if (DHCPRecord)
      delete DHCPRecord;
  }

  DHCP_Record* DHCPRecord;
};
typedef Stream_DataBase_T<DHCP_MessageData> DHCP_MessageData_t;

struct DHCP_ConnectionState
 : Net_StreamConnectionState
{
  DHCP_ConnectionState ()
   : Net_StreamConnectionState ()
   , serverAddress (static_cast<u_short> (0),
                    static_cast<ACE_UINT32> (INADDR_ANY))
   , timeStamp (ACE_Time_Value::zero)
   , xid (0)
  {}

  ACE_INET_Addr  serverAddress;
  ACE_Time_Value timeStamp; // lease timeout
  ACE_UINT32     xid;       // session id
};

#endif
