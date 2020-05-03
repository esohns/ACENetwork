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

#ifndef DHCP_CODES_H
#define DHCP_CODES_H

#include "ace/Global_Macros.h"

class DHCP_Codes
{
 public:
  enum OpType
  {
    DHCP_OP_REQUEST = 1,
    DHCP_OP_REPLY,
    /////////////////////////////////////
    DHCP_OP_MAX,
    DHCP_OP_INVALID
  };

  enum OptionFieldType
  {
    DHCP_OPTION_FIELDTYPE_ADDRESS = 0,
    // *TODO*: extract this idea from ACEStream  (i.e. Stream_MessageBase_T)
    //         and add it ACENetwork
    DHCP_OPTION_FIELDTYPE_COMMAND, // reserved: option DHCP_MESSAGETYPE (53)
    DHCP_OPTION_FIELDTYPE_INTEGER,
    DHCP_OPTION_FIELDTYPE_STRING,
    DHCP_OPTION_FIELDTYPE_ADDRESSES, // see e.g. RFC-7291
    /////////////////////////////////////
    DHCP_OPTION_FIELDTYPE_MAX,
    DHCP_OPTION_FIELDTYPE_INVALID
  };
  //enum OptionValueType
  //{
  //  DHCP_OPTION_VALUETYPE_FIXED = 0,
  //  DHCP_OPTION_VALUETYPE_VARIABLE,
  //  /////////////////////////////////////
  //  DHCP_OPTION_VALUETYPE_MAX,
  //  DHCP_OPTION_VALUETYPE_INVALID
  //};
  enum OptionType
  { // *NOTE*: see RFC-951
    DHCP_OPTION_PAD = 0,    // "fixed length"
    DHCP_OPTION_SUBNETMASK, // "fixed length"
    DHCP_OPTION_TIMEOFFSET, // "fixed length"
    DHCP_OPTION_GATEWAY,
    DHCP_OPTION_TIMESERVER,
    DHCP_OPTION_IEN116NAMESERVER,
    DHCP_OPTION_DOMAINNAMESERVER,
    DHCP_OPTION_LOGSERVER,
    DHCP_OPTION_COOKIESERVER,
    DHCP_OPTION_LPRSERVER,
    DHCP_OPTION_IMPRESSSERVER,
    DHCP_OPTION_RLPSERVER,
    DHCP_OPTION_HOSTNAME,
    //////////////////////////////////////
    // *NOTE*: see RFC-1395
    DHCP_OPTION_BOOTFILESIZE,
    DHCP_OPTION_MERITDUMPFILE,
    DHCP_OPTION_DOMAINNAME,
    DHCP_OPTION_SWAPSERVER,
    DHCP_OPTION_ROOTPATH,
    //////////////////////////////////////
    // *NOTE*: see RFC-1497
    DHCP_OPTION_EXTENSIONSPATH,
    //////////////////////////////////////
    // *NOTE*: see RFC-1533
    DHCP_OPTION_IP_FORWARDING,
    DHCP_OPTION_IP_NONLOCALSOURCEROUTING,
    DHCP_OPTION_IP_POLICYFILTER,
    DHCP_OPTION_IP_MAXIMUMDATAGRAMREASSEMBLYSIZE,
    DHCP_OPTION_IP_TIMETOLIVE,
    DHCP_OPTION_IP_PATHMTUAGINGTIMEOUT,
    DHCP_OPTION_IP_PATHMTUPLATEAUTABLE,
    DHCP_OPTION_IP_MTU,
    DHCP_OPTION_IP_LOCALSUBNETS,
    DHCP_OPTION_IP_BROADCASTADDRESS,
    DHCP_OPTION_IP_DISCOVERSUBNETMASKS,
    DHCP_OPTION_IP_SUPPLYSUBNETMASKS,
    DHCP_OPTION_IP_DISCOVERROUTERS,
    DHCP_OPTION_IP_ROUTERSOLICITATIONADDRESS,
    DHCP_OPTION_IP_STATICROUTE,
    //////////////////////////////////////
    DHCP_OPTION_ARP_TRAILERENCAPSULATION,
    DHCP_OPTION_ARP_CACHETIMEOUT,
    DHCP_OPTION_ARP_ETHERNETENCAPSULATION,
    //////////////////////////////////////
    DHCP_OPTION_TCP_DEFAULTTIMETOLIVE,
    DHCP_OPTION_TCP_KEEPALIVEINTERVAL,
    DHCP_OPTION_TCP_KEEPALIVEGARBAGE,
    //////////////////////////////////////
    DHCP_OPTION_NIS_DOMAIN,
    DHCP_OPTION_NIS_SERVER,
    DHCP_OPTION_NTP_SERVER,
    DHCP_OPTION_VENDORSPECIFICINFORMATION,
    DHCP_OPTION_NB_NAMESERVER,
    DHCP_OPTION_NB_DATAGRAMDISTRIBUTIONSERVER,
    DHCP_OPTION_NB_NODETYPE,
    DHCP_OPTION_NB_SCOPE,
    DHCP_OPTION_X_FONTSERVER,
    DHCP_OPTION_X_DISPLAYMANAGER,
    //////////////////////////////////////
    DHCP_OPTION_DHCP_REQUESTEDIPADDRESS,
    DHCP_OPTION_DHCP_IPADDRESSLEASETIME,
    DHCP_OPTION_DHCP_OVERLOAD,
    DHCP_OPTION_DHCP_MESSAGETYPE,
    DHCP_OPTION_DHCP_SERVERIDENTIFIER,
    DHCP_OPTION_DHCP_PARAMETERREQUESTLIST,
    DHCP_OPTION_DHCP_MESSAGE,
    DHCP_OPTION_DHCP_MAXIMUMDHCPMESSAGESIZE,
    DHCP_OPTION_DHCP_RENEWALT1TIME,
    DHCP_OPTION_DHCP_REBINDINGT2TIME,
    DHCP_OPTION_DHCP_VENDORCLASSIDENTIFIER,
    DHCP_OPTION_DHCP_CLIENTIDENTIFIER,
    //////////////////////////////////////
    // *NOTE*: see RFC-2242
    DHCP_OPTION_NETWAREIP_DOMAINNAME,
    DHCP_OPTION_NETWAREIP_INFORMATION,
    //////////////////////////////////////
    DHCP_OPTION_NISP_DOMAIN,
    DHCP_OPTION_NISP_SERVER,
    //////////////////////////////////////
    DHCP_OPTION_TFTP_SERVER,
    //////////////////////////////////////
    DHCP_OPTION_BOOTFILE,
    //////////////////////////////////////
    DHCP_OPTION_MIP_HOMEAGENT,
    //////////////////////////////////////
    DHCP_OPTION_SMTP_SERVER,
    //////////////////////////////////////
    DHCP_OPTION_POP3_SERVER,
    //////////////////////////////////////
    DHCP_OPTION_NNTP_SERVER,
    //////////////////////////////////////
    DHCP_OPTION_WWW_DEFAULTSERVER,
    //////////////////////////////////////
    DHCP_OPTION_FINGER_DEFAULTSERVER,
    //////////////////////////////////////
    DHCP_OPTION_IRC_DEFAULTSERVER,
    //////////////////////////////////////
    DHCP_OPTION_STREETTALK_SERVER,
    DHCP_OPTION_STREETTALK_DIRECTORYASSISTANCESERVER,
    //////////////////////////////////////
    // *NOTE*: see RFC-3004
    DHCP_OPTION_USERCLASS,
    //////////////////////////////////////
    // *NOTE*: see RFC-2610
    DHCP_OPTION_SLP_DIRECTORYAGENT,
    DHCP_OPTION_SLP_SERVICESCOPE,
    //////////////////////////////////////
    // *NOTE*: see RFC-4039
    DHCP_OPTION_RAPIDCOMMIT,
    //////////////////////////////////////
    // *NOTE*: see RFC-4702
    DHCP_OPTION_CLIENTFQDN,
    //////////////////////////////////////
    // *NOTE*: see RFC-3046
    DHCP_OPTION_RELAYAGENTINFORMATION,
    //////////////////////////////////////
    // *NOTE*: see RFC-4174
    DHCP_OPTION_ISNS_SERVER,
    //////////////////////////////////////
    // *NOTE*: see RFC-2241
    DHCP_OPTION_NDS_SERVER = 85,
    DHCP_OPTION_NDS_TREE,
    DHCP_OPTION_NDS_CONTEXT,
    //////////////////////////////////////
    // *NOTE*: see RFC-4280
    DHCP_OPTION_BCMCCS_DOMAINNAMELIST,
    DHCP_OPTION_BCMCCS_IPADDRESS,
    //////////////////////////////////////
    // *NOTE*: see RFC-3118
    DHCP_OPTION_AUTHENTICATION,
    //////////////////////////////////////
    // *NOTE*: see RFC-4388
    DHCP_OPTION_CLIENTLASTTRANSACTIONTIME,
    DHCP_OPTION_CLIENTASSOCIATEDIP,
    //////////////////////////////////////
    // *NOTE*: see RFC-4578
    DHCP_OPTION_IPXE_CLIENTSYSTEMARCHITECTURETYPE,
    DHCP_OPTION_IPXE_CLIENTNETWORKINTERFACEIDENTIFIER,
    DHCP_OPTION_IPXE_CLIENTMACHINEIDENTIFIER = 97,
    //////////////////////////////////////
    // *TODO*: see also: http://www.iana.org/assignments/bootp-dhcp-parameters/bootp-dhcp-parameters.xhtml
    //////////////////////////////////////
    // *NOTE*: see RFC-3442
    DHCP_OPTION_CLASSLESSROUTE = 121,
    //////////////////////////////////////
    //DHCP_OPTION_RESERVED_START = 128,
    // *NOTE*: see RFC-7291
    DHCP_OPTION_PCP_SERVER = 158,
    //------------------------------------
    //DHCP_OPTION_RESERVED_END = 254,
    DHCP_OPTION_END = 255,  // "fixed length"
    /////////////////////////////////////
    DHCP_OPTION_MAX,
    DHCP_OPTION_INVALID
  };

  enum OverloadType
  {
    DHCP_OVERLOAD_FILE = 1,
    DHCP_OVERLOAD_SNAME,
    DHCP_OVERLOAD_BOTH,
    /////////////////////////////////////
    DHCP_OVERLOAD_MAX,
    DHCP_OVERLOAD_INVALID
  };

  enum MessageType
  {
    DHCP_MESSAGE_DISCOVER = 1,
    DHCP_MESSAGE_OFFER,
    DHCP_MESSAGE_REQUEST,
    DHCP_MESSAGE_DECLINE,
    DHCP_MESSAGE_ACK,
    DHCP_MESSAGE_NAK,
    DHCP_MESSAGE_RELEASE,
    DHCP_MESSAGE_INFORM,
    /////////////////////////////////////
    // *NOTE*: see RFC-3203
    DHCP_MESSAGE_FORCERENEW,
    /////////////////////////////////////
    // *NOTE*: see RFC-4388
    DHCP_MESSAGE_LEASEQUERY,
    DHCP_MESSAGE_LEASEUNASSIGNED,
    DHCP_MESSAGE_LEASEUNKNOWN,
    DHCP_MESSAGE_LEASEACTIVE,
    /////////////////////////////////////
    // *NOTE*: see RFC-6926
    DHCP_MESSAGE_BULKLEASEQUERY,
    DHCP_MESSAGE_LEASEQUERYDONE,
    /////////////////////////////////////
    // *NOTE*: see RFC-7724
    DHCP_MESSAGE_ACTIVELEASEQUERY,
    DHCP_MESSAGE_LEASEQUERYSTATUS,
    DHCP_MESSAGE_TLS,
    /////////////////////////////////////
    DHCP_MESSAGE_MAX,
    DHCP_MESSAGE_INVALID
  };

 private:
  ACE_UNIMPLEMENTED_FUNC (DHCP_Codes ())
  ACE_UNIMPLEMENTED_FUNC (DHCP_Codes (const DHCP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_Codes& operator= (const DHCP_Codes&))
  ACE_UNIMPLEMENTED_FUNC (virtual ~DHCP_Codes ())
};

#endif
