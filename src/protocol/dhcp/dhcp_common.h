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

#include "ace/OS.h"
#include "ace/Time_Value.h"

#include "common_inotify.h"

#include "stream_common.h"
#include "stream_data_base.h"

#include "net_common.h"

#include "dhcp_codes.h"
#include "dhcp_defines.h"

// forward declarations
struct DHCP_Configuration;
struct DHCP_Record;
class DHCP_SessionMessage;
struct DHCP_Stream_SessionData;
struct DHCP_Stream_UserData;
typedef Common_INotify_T<DHCP_Stream_SessionData,
                         DHCP_Record,
                         DHCP_SessionMessage> DHCP_IStreamNotify_t;
typedef Stream_Statistic DHCP_RuntimeStatistic_t;

//typedef std::list<std::pair<unsigned char, std::string> > DHCP_Options_t;
typedef std::map<unsigned char, std::string> DHCP_Options_t;
typedef DHCP_Options_t::const_iterator DHCP_OptionsIterator_t;
struct DHCP_Record
{
  inline DHCP_Record ()
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
    ACE_OS::memset (chaddr, 0, 16);
//    ACE_OS::memset (options, 0, 312);
  };

  DHCP_Codes::OpType op;
  unsigned char      htype;
  unsigned char      hlen;
  unsigned char      hops;
  unsigned int       xid;
  unsigned short     secs;
  unsigned short     flags;
  unsigned int       ciaddr;
  unsigned int       yiaddr;
  unsigned int       siaddr;
  unsigned int       giaddr;
  unsigned char      chaddr[DHCP_CHADDR_SIZE];
  std::string        sname;   // max. 64 bytes
  std::string        file;    // max. 128 bytes
  unsigned int       cookie;
  DHCP_Options_t     options; // max. 312 bytes (negotiable)
//  unsigned char      options[DHCP_OPTIONS_SIZE];
};
struct DHCP_MessageData
{
  inline DHCP_MessageData ()
   : DHCPRecord (NULL)
  {};
  inline ~DHCP_MessageData ()
  {
    if (DHCPRecord)
      delete DHCPRecord;
  };

  DHCP_Record* DHCPRecord;
};
typedef Stream_DataBase_T<DHCP_MessageData> DHCP_MessageData_t;

struct DHCP_ConnectionState
 : Net_ConnectionState
{
  inline DHCP_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
   , timeStamp (ACE_Time_Value::zero)
   , xid (0)
  {};

  DHCP_Configuration*   configuration;
  DHCP_Stream_UserData* userData;

  ACE_Time_Value        timeStamp; // lease timeout
  unsigned int          xid;       // session ID
};

#endif
