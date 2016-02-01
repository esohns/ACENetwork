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
#include <string>

#include "ace/OS.h"

#include "net_macros.h"

#include "dhcp_common.h"
#include "dhcp_defines.h"
#include "dhcp_tools.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
DHCP_Module_Streamer_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType>::DHCP_Module_Streamer_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Streamer_T::DHCP_Module_Streamer_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
DHCP_Module_Streamer_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType>::~DHCP_Module_Streamer_T ()
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Streamer_T::~DHCP_Module_Streamer_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
DHCP_Module_Streamer_T<TaskSynchType,
                       TimePolicyType,
                       SessionMessageType,
                       ProtocolMessageType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("DHCP_Module_Streamer_T::handleDataMessage"));

  int result = -1;
  char buffer[DHCP_SNAME_SIZE];
  char buffer_2[DHCP_FILE_SIZE];
  unsigned int magic_cookie = DHCP_MAGIC_COOKIE;
  unsigned char field_length;
  unsigned int offset = 0;
  unsigned char tag = DHCP_OPTION_TAG_END;
  char buffer_3[DHCP_OPTIONS_SIZE];

  // don't care (implies yes per default, if part of a stream)
  // *NOTE*: as this is an "upstream" module, the "wording" is wrong
  //         --> the logic remains the same, though
  passMessageDownstream_out = true;

  // sanity check(s)
  ACE_ASSERT (message_inout->length () == 0);
  ACE_ASSERT (message_inout->capacity () >= DHCP_MESSAGE_SIZE);

  // serialize structured data
  // --> create the appropriate bytestream corresponding to its elements
//  const typename ProtocolMessageType::DATA_T& data_container_r =
//      message_inout->get ();
//  const typename ProtocolMessageType::DATA_T::DATA_T& data_r =
//        data_container_r.get ();
  const typename ProtocolMessageType::DATA_T& data_r =
      message_inout->get ();
  ACE_ASSERT (data_r.DHCPRecord);

  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->op),
                         1);
  if (result == -1)
    goto error;
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->htype),
                         1);
  if (result == -1)
    goto error;
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->hlen),
                         1);
  if (result == -1)
    goto error;
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->hops),
                         1);
  if (result == -1)
    goto error;
  //
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->xid),
                         4);
  if (result == -1)
    goto error;
  //
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->secs),
                         2);
  if (result == -1)
    goto error;
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->flags),
                         2);
  if (result == -1)
    goto error;
  //
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->ciaddr),
                         4);
  if (result == -1)
    goto error;
  //
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->yiaddr),
                         4);
  if (result == -1)
    goto error;
  //
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->siaddr),
                         4);
  if (result == -1)
    goto error;
  //
  result =
    message_inout->copy (reinterpret_cast<char*> (&data_r.DHCPRecord->giaddr),
                         4);
  if (result == -1)
    goto error;
  ////////
  result =
    message_inout->copy (reinterpret_cast<char*> (data_r.DHCPRecord->chaddr),
                         16);
  if (result == -1)
    goto error;
  ////////////////////////////////
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  ACE_OS::memcpy (buffer, data_r.DHCPRecord->sname.c_str (),
                  data_r.DHCPRecord->sname.size ());
  result = message_inout->copy (buffer, DHCP_SNAME_SIZE);
  if (result == -1)
    goto error;
  ////////////////////////////////////////////////////////////////
  ACE_OS::memset (buffer_2, 0, sizeof (buffer_2));
  ACE_OS::memcpy (buffer_2, data_r.DHCPRecord->file.c_str (),
                  data_r.DHCPRecord->file.size ());
  result = message_inout->copy (buffer_2, DHCP_FILE_SIZE);
  if (result == -1)
    goto error;
  ///////////////////////////////////////////////////////////////////////////...
  ACE_OS::memset (buffer_3, 0, sizeof (buffer_3));
  if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
    magic_cookie = ACE_SWAP_LONG (magic_cookie);
  ACE_OS::memcpy (buffer_3, &magic_cookie, 4);
  offset += 4;
  for (DHCP_OptionsIterator_t iterator = data_r.DHCPRecord->options.begin ();
       iterator != data_r.DHCPRecord->options.end ();
       ++iterator)
  {
    ACE_OS::memcpy (buffer_3 + offset, &(*iterator).first, 1);
    ++offset;
    ACE_ASSERT ((*iterator).second.size () <=
                std::numeric_limits<unsigned char>::max ());
    field_length = static_cast<unsigned char> ((*iterator).second.size ());
    ACE_OS::memcpy (buffer_3 + offset, &field_length, 1);
    ++offset;
    ACE_OS::memcpy (buffer_3 + offset, (*iterator).second.c_str (),
                    (*iterator).second.size ());
    offset += (*iterator).second.size ();
  } // end FOR
  ACE_OS::memcpy (buffer_3 + offset, &tag, 1);
  result = message_inout->copy (buffer_3, DHCP_OPTIONS_SIZE);
  if (result == -1)
    goto error;

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("[%u]: streamed [%u byte(s)]...\n"),
  //            message_inout->getID (),
  //            message_inout->length ()));

  return;

error:
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", continuing\n")));
}
