/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#include "ace/OS.h"

#include "net_macros.h"

template <typename ConfigurationType>
Net_SocketHandlerBase_T<ConfigurationType>::Net_SocketHandlerBase_T ()
 : configuration_ (NULL)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketHandlerBase_T::Net_SocketHandlerBase_T"));

}

template <typename ConfigurationType>
bool
Net_SocketHandlerBase_T<ConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketHandlerBase_T::initialize"));

  if (unlikely (isInitialized_))
  {
    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isInitialized_ = true;

  return true;
}

template <typename ConfigurationType>
ACE_Message_Block*
Net_SocketHandlerBase_T<ConfigurationType>::allocateMessage (unsigned int size_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketHandlerBase_T::allocateMessage"));

  ACE_Message_Block* result_p = NULL;

  ACE_NEW_NORETURN (result_p,
                    ACE_Message_Block (size_in,                            // size
                                       ACE_Message_Block::MB_DATA,         // type
                                       NULL,                               // cont
                                       NULL,                               // data
                                       NULL,                               // allocator_strategy
                                       NULL,                               // locking_strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution_time
                                       ACE_Time_Value::max_time,           // deadline_time
                                       NULL,                               // data_block_allocator
                                       NULL));                             // message_block_allocator
  if (unlikely (!result_p))
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return result_p;
}
