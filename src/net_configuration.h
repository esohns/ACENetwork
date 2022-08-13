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

#ifndef NET_CONFIGURATION_H
#define NET_CONFIGURATION_H

#include "common_event_common.h"
#include "common_event_defines.h"

#include "common_parser_common.h"

// forward declarations
//class Stream_IAllocator;
//struct Net_UserData;

//struct Net_AllocatorConfiguration
// : Common_AllocatorConfiguration
//{
//  Net_AllocatorConfiguration ()
//   : Common_AllocatorConfiguration ()
//  {
//    defaultBufferSize = NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
//    // *NOTE*: this facilitates (message block) data buffers to be scanned with
//    //         'flex's yy_scan_buffer() method
//    paddingBytes = COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE;
//  }
//};

//typedef std::deque<ACE_INET_Addr> Net_InetAddressStack_t;
//typedef Net_InetAddressStack_t::iterator Net_InetAddressStackIterator_t;

struct Common_FlexBisonParserConfiguration;
struct Net_SessionConfiguration
{
  Net_SessionConfiguration ()
   : dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   , parserConfiguration (NULL)
  {}

  enum Common_EventDispatchType               dispatch;
  struct Common_FlexBisonParserConfiguration* parserConfiguration;
};

//template <typename ConnectionConfigurationType,
//          enum Net_TransportLayerType TransportLayerType_e>
//class Net_ListenerConfiguration_T
// //: public Net_ConnectionConfigurationBase_T<TransportLayerType_e>
//{
// public:
//  Net_ListenerConfiguration_T ()
////   : Net_ConnectionConfigurationBase_T ()
//   : addressFamily (ACE_ADDRESS_FAMILY_INET)
//   , connectionConfiguration (NULL)
//  {}
//
//  int                          addressFamily;
//  ConnectionConfigurationType* connectionConfiguration;
//};

#endif
