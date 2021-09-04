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

typedef Stream_ISessionDataNotify_T<struct PCP_Stream_SessionData,
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

/////////////////////////////////////////

//struct PCPOpcodeAnnounceData
//{};

struct PCPOpcodeMapData
{
  ACE_UINT64     nonce;
  ACE_UINT8      protocol;
  ACE_UINT32     reserved;
  ACE_UINT16     internal_port;
  ACE_UINT16     external_port;
  ACE_INET_Addr* external_address;
};

struct PCPOpcodePeerData
{
  ACE_UINT64     nonce;
  ACE_UINT8      protocol;
  ACE_UINT32     reserved;
  ACE_UINT16     internal_port;
  ACE_UINT16     external_port;
  ACE_INET_Addr* external_address;
  ACE_UINT16     remote_peer_port;
  ACE_UINT16     reserved_2;
  ACE_INET_Addr* remote_peer_address;
};

// rfc7652
struct PCPOpcodeAuthenticationData
{
  ACE_UINT32 session_id;
  ACE_UINT32 sequence_number;
};

/////////////////////////////////////////

struct PCPOptionThirdParty
{
  ACE_INET_Addr* address;
};

//struct PCPOptionPreferFailure
//{};

struct PCPOptionFilter
{
  ACE_UINT8      reserved;
  ACE_UINT8      prefix_length; // IPv4: 96-128, IPv6: 0-128, 0: delete set filters
  ACE_UINT16     remote_peer_port; // 0: all ports
  ACE_INET_Addr* remote_peer_address;
};

// rfc7652
struct PCPOptionNonce
{
  ACE_UINT32 nonce;
};

struct PCPOptionAuthenticationTag
{
  ACE_UINT32 session_id;
  ACE_UINT32 sequence_number;
  ACE_UINT32 key_id;
  ACE_UINT8* data; // variable-length
};

struct PCPOptionPAAuthenticationTag
{
  ACE_UINT32 key_id;
  ACE_UINT8* data; // variable-length
};

struct PCPOptionEAPPayload
{
  ACE_UINT8* data; // variable-length
};

struct PCPOptionPseudoRandomFunction
{
  ACE_UINT32 id; // see rfc7296, rfc4868
};

struct PCPOptionMACAlgorithm
{
  ACE_UINT32 id; // see rfc7296, rfc4868
};

struct PCPOptionSessionLifetime
{
  ACE_UINT32 lifetime;
};

struct PCPOptionReceivedPAK
{
  ACE_UINT32 sequence_number;
};

struct PCPOptionIdIndicator
{
  ACE_UINT8* data; // utf8, rfc7613 // variable-length
};

/////////////////////////////////////////

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
      case PCP_Codes::PCP_OPTION_NONCE: // rfc7652
        nonce = option_in.nonce;  break;
      case PCP_Codes::PCP_OPTION_AUTHENTICATION_TAG: // rfc7652
        authentication_tag = option_in.authentication_tag;  break;
      case PCP_Codes::PCP_OPTION_PA_AUTHENTICATION_TAG: // rfc7652
        pa_authentication_tag = option_in.pa_authentication_tag;  break;
      case PCP_Codes::PCP_OPTION_EAP_PAYLOAD: // rfc7652
        eap_payload = option_in.eap_payload;  break;
      case PCP_Codes::PCP_OPTION_PSEUDO_RANDOM_FUNCTION: // rfc7652
        pseudo_random_function = option_in.pseudo_random_function;  break;
      case PCP_Codes::PCP_OPTION_MAC_ALGORITHM: // rfc7652
        mac_algorithm = option_in.mac_algorithm;  break;
      case PCP_Codes::PCP_OPTION_SESSION_LIFETIME: // rfc7652
        session_lifetime = option_in.session_lifetime;  break;
      case PCP_Codes::PCP_OPTION_RECEIVED_PAK: // rfc7652
        received_pak = option_in.received_pak;  break;
      case PCP_Codes::PCP_OPTION_ID_INDICATOR: // rfc7652
        id_indicator = option_in.id_indicator;  break;
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
  struct PCPOption& operator= (const struct PCPOption& rhs_in)
  {
    code = rhs_in.code;
    reserved = rhs_in.reserved;
    length = rhs_in.length;
    switch (rhs_in.code)
    {
      case PCP_Codes::PCP_OPTION_THIRD_PARTY:
        third_party = rhs_in.third_party; break;
      case PCP_Codes::PCP_OPTION_PREFER_FAILURE:
        break;
      case PCP_Codes::PCP_OPTION_FILTER:
        filter = rhs_in.filter;  break;
      case PCP_Codes::PCP_OPTION_NONCE: // rfc7652
        nonce = rhs_in.nonce;  break;
      case PCP_Codes::PCP_OPTION_AUTHENTICATION_TAG: // rfc7652
        authentication_tag = rhs_in.authentication_tag;  break;
      case PCP_Codes::PCP_OPTION_PA_AUTHENTICATION_TAG: // rfc7652
        pa_authentication_tag = rhs_in.pa_authentication_tag;  break;
      case PCP_Codes::PCP_OPTION_EAP_PAYLOAD: // rfc7652
        eap_payload = rhs_in.eap_payload;  break;
      case PCP_Codes::PCP_OPTION_PSEUDO_RANDOM_FUNCTION: // rfc7652
        pseudo_random_function = rhs_in.pseudo_random_function;  break;
      case PCP_Codes::PCP_OPTION_MAC_ALGORITHM: // rfc7652
        mac_algorithm = rhs_in.mac_algorithm;  break;
      case PCP_Codes::PCP_OPTION_SESSION_LIFETIME: // rfc7652
        session_lifetime = rhs_in.session_lifetime;  break;
      case PCP_Codes::PCP_OPTION_RECEIVED_PAK: // rfc7652
        received_pak = rhs_in.received_pak;  break;
      case PCP_Codes::PCP_OPTION_ID_INDICATOR: // rfc7652
        id_indicator = rhs_in.id_indicator;  break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown option code (was: %d), continuing\n"),
                    rhs_in.code));
        break; // *TODO*: throw ?
      }
    } // end SWITCH  

    return *this;
  }

  enum PCP_Codes::OptionType             code;
  ACE_UINT8                              reserved;
  ACE_UINT16                             length;
  union
  {
    struct PCPOptionThirdParty           third_party;
//    struct PCPOptionPreferFailure        prefer_failure;
    struct PCPOptionFilter               filter;
    struct PCPOptionNonce                nonce; // rfc7652
    struct PCPOptionAuthenticationTag    authentication_tag; // rfc7652
    struct PCPOptionPAAuthenticationTag  pa_authentication_tag; // rfc7652
    struct PCPOptionEAPPayload           eap_payload; // rfc7652
    struct PCPOptionPseudoRandomFunction pseudo_random_function; // rfc7652
    struct PCPOptionMACAlgorithm         mac_algorithm; // rfc7652
    struct PCPOptionSessionLifetime      session_lifetime; // rfc7652
    struct PCPOptionReceivedPAK          received_pak; // rfc7652
    struct PCPOptionIdIndicator          id_indicator; // rfc7652
  };
};

typedef std::vector<struct PCPOption> PCP_Options_t;
typedef PCP_Options_t::iterator PCP_OptionsIterator_t;
typedef PCP_Options_t::const_iterator PCP_OptionsConstIterator_t;
struct PCP_Record
{
  PCP_Record ()
   : version (PCP_Codes::PCP_VERSION_INVALID)
   , opcode (0)
   , reserved (0)
   , result_code (PCP_Codes::PCP_RESULTCODE_INVALID)
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
        break;
      case PCP_Codes::PCP_OPCODE_AUTHENTICATION: // rfc7652
        authentication = record_in.authentication; break;
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
  struct PCP_Record& operator= (const struct PCP_Record& rhs_in)
  {
    version = rhs_in.version;
    opcode = rhs_in.opcode;
    reserved = rhs_in.reserved;
    result_code = rhs_in.result_code;
    lifetime = rhs_in.lifetime;
    epoch_time = rhs_in.epoch_time;
    reserved_2 = rhs_in.reserved_2;
    client_address = rhs_in.client_address;
    switch (rhs_in.opcode)
    {
      case PCP_Codes::PCP_OPCODE_MAP:
        map = rhs_in.map; break;
      case PCP_Codes::PCP_OPCODE_PEER:
        peer = rhs_in.peer; break;
      case PCP_Codes::PCP_OPCODE_ANNOUNCE:
        break;
      case PCP_Codes::PCP_OPCODE_AUTHENTICATION: // rfc7652
        authentication = rhs_in.authentication; break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown opcode (was: %d), continuing\n"),
                    rhs_in.opcode));
        break; // *TODO*: throw ?
      }
    } // end SWITCH
    options = rhs_in.options;

    return *this;
  }

  enum PCP_Codes::VersionType          version;
  // *NOTE*: this contains the (MSB) 'R'-bit
  ACE_UINT8                            opcode;
  ACE_UINT8                            reserved; // 16 bits in request
  enum PCP_Codes::ResultCodeType       result_code; // response only // // rfc7652: request/response
  ACE_UINT32                           lifetime; // second(s): 0: remove mapping
  ACE_UINT32                           epoch_time; // response only
  ACE_UINT32                           reserved_2; // response only
  ACE_INET_Addr                        client_address; // request only
  union
  {
    struct PCPOpcodeMapData            map;
    struct PCPOpcodePeerData           peer;
//    struct PCPOpcodeAnnounceData       announce;
    struct PCPOpcodeAuthenticationData authentication;
  };
  PCP_Options_t                        options;
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
  {}
};

#endif
