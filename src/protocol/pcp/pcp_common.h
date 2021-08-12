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

#ifndef PCP_COMMON_H
#define PCP_COMMON_H

#include <vector>

#include "ace/Basic_Types.h"
#include "ace/INET_Addr.h"
#include "ace/OS.h"
#include "ace/Time_Value.h"

#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_isessionnotify.h"

#include "net_common.h"

#include "pcp_codes.h"
#include "pcp_defines.h"

// forward declarations
struct PCP_Configuration;
struct PCP_Record;
class PCP_SessionMessage;
struct PCP_Stream_SessionData;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct PCP_Stream_SessionData,
                                    enum Stream_SessionMessageType,
                                    struct PCP_Record,
                                    PCP_SessionMessage> PCP_ISessionNotify_t;

typedef Net_StreamStatistic_t PCP_Statistic_t;
typedef Common_IStatistic_T<PCP_Statistic_t> PCP_IStatistic_t;
typedef Common_StatisticHandler_T<PCP_Statistic_t> PCP_StatisticHandler_t;

// convenient type definitions
typedef PCP_Codes::VersionType PCP_Version_t;
typedef PCP_Codes::OpcodeType PCP_Opcode_t;
typedef PCP_Codes::ResultCodeType PCP_ResultCode_t;
typedef PCP_Codes::OptionType PCP_Option_t;

struct PCPOpcodeMapData
{
  ACE_UINT64    nonce;
  ACE_UINT8     protocol;
  ACE_UINT32    reserved;
  ACE_UINT16    internal_port;
  ACE_UINT16    external_port;
  ACE_INET_Addr external_address;
};

struct PCPOpcodePeerData
{
  ACE_UINT64    nonce;
  ACE_UINT8     protocol;
  ACE_UINT32    reserved;
  ACE_UINT16    internal_port;
  ACE_UINT16    external_port;
  ACE_INET_Addr external_address;
  ACE_UINT16    remote_peer_port;
  ACE_UINT16    reserved_2;
  ACE_INET_Addr remote_peer_address;
};

struct PCPOptionThirdParty
{
  ACE_INET_Addr address;
};

struct PCPOptionPreferFailure
{
};

struct PCPOptionFilter
{
  ACE_UINT8     reserved;
  ACE_UINT8     prefix_length; // IPv4: 96-128, IPv6: 0-128, 0: delete set filters
  ACE_UINT16    remote_peer_port; // 0: all ports
  ACE_INET_Addr remote_peer_address;
};

struct PCPOption
{
  PCPOption ()
   : code (PCP_Codes::PCP_OPTION_INVALID)
   , reserved (0)
   , length (0)
  {}
  PCPOption (const PCPOption& option_in)
   : code (option_in.code)
   , reserved (option_in.reserved)
   , length (option_in.length)
  {
    switch (option_in.code)
    {
      case PCP_Codes::PCP_OPTION_THIRD_PARTY:
        third_party = option_in.third_party; break;
      case PCP_Codes::PCP_OPTION_PREFER_FAILURE:
        break;
      case PCP_Codes::PCP_OPTION_FILTER:
        filter = option_in.filter;  break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown option code (was: %d), continuing\n"),
                    option_in.code));
        break; // *TODO*: throw ?
      }
    } // end SWITCH  
  }
  ~PCPOption () {}

  ACE_UINT8                       code;
  ACE_UINT8                       reserved;
  ACE_UINT16                      length;
  union
  {
    struct PCPOptionThirdParty    third_party;
    struct PCPOptionPreferFailure prefer_failure;
    struct PCPOptionFilter        filter;
  };
};

typedef std::vector<struct PCPOption> PCP_Options_t;
typedef PCP_Options_t::const_iterator PCP_OptionsIterator_t;
struct PCP_Record
{
  PCP_Record ()
   : version (PCP_Codes::PCP_VERSION_INVALID)
   , opcode (0)
   , reserved (0)
   , result_code (0)
   , lifetime (0)
   , epoch_time (0)
   , reserved_2 (0)
   , options ()
  {}
  PCP_Record (const struct PCP_Record& record_in)
   : version (record_in.version)
   , opcode (record_in.opcode)
   , reserved (record_in.reserved)
   , result_code (record_in.result_code)
   , lifetime (record_in.lifetime)
   , epoch_time (record_in.epoch_time)
   , reserved_2 (record_in.reserved_2)
   , options (record_in.options)
  {
    switch (record_in.opcode)
    {
      case PCP_Codes::PCP_OPCODE_MAP:
        map = record_in.map; break;
      case PCP_Codes::PCP_OPCODE_PEER:
        peer = record_in.peer; break;
      case PCP_Codes::PCP_OPCODE_ANNOUNCE:
        break; // *TODO*
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown opcode (was: %d), continuing\n"),
                    record_in.opcode));
        break; // *TODO*: throw ?
      }
    } // end SWITCH
  }
  ~PCP_Record () {}
  inline struct PCP_Record& operator= (const struct PCP_Record& rhs_in) { *this = rhs_in; return *this; }

  PCP_Codes::VersionType     version;
  // *NOTE*: this contains the (MSB) 'R'-bit
  ACE_UINT8                  opcode;
  ACE_UINT8                  reserved; // 16 bits in request
  ACE_UINT8                  result_code; // response only !
  ACE_UINT32                 lifetime; // second(s): 0: remove mapping
  ACE_UINT32                 epoch_time; // response only !
  ACE_UINT32                 reserved_2; // response only !
  ACE_INET_Addr              client_address; // request only !
  union
  {
    struct PCPOpcodeMapData  map;
    struct PCPOpcodePeerData peer;
  };
  PCP_Options_t              options;
};

struct PCP_MessageData
 : PCP_Record
{
  //PCP_MessageData ()
  //  : PCP_Record ()
  //{}
  //~PCP_MessageData () {}
};
typedef Stream_DataBase_T<struct PCP_MessageData> PCP_MessageData_t;

struct PCP_ConnectionState
 : Net_StreamConnectionState
{
  PCP_ConnectionState ()
   : Net_StreamConnectionState ()
   , nonce (0)
   , serverAddress (static_cast<u_short> (0),
                    static_cast<ACE_UINT32> (INADDR_ANY))
   , timeStamp (ACE_Time_Value::zero)
  {}

  ACE_UINT64     nonce;         // session nonce
  ACE_INET_Addr  serverAddress;
  ACE_Time_Value timeStamp;     // (mapping-) timeout
};

#endif
