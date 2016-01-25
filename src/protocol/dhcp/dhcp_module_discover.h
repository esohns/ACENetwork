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

#ifndef DHCP_MODULE_DISCOVER_H
#define DHCP_MODULE_DISCOVER_H

#include "ace/Global_Macros.h"

#include "stream_task_base_synch.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
class DHCP_Module_Discover_T
 : public Stream_TaskBaseSynch_T<TimePolicyType,
                                 SessionMessageType,
                                 ProtocolMessageType>
{
 public:
  DHCP_Module_Discover_T ();
  virtual ~DHCP_Module_Discover_T ();

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?

 private:
  typedef Stream_TaskBaseSynch_T<TimePolicyType,
                                 SessionMessageType,
                                 ProtocolMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_Discover_T (const DHCP_Module_Discover_T&))
  ACE_UNIMPLEMENTED_FUNC (DHCP_Module_Discover_T& operator= (const DHCP_Module_Discover_T&))
};

// include template implementation
#include "dhcp_module_discover.inl"

#endif
