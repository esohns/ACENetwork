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

#ifndef POP_MODULE_STREAMER_H
#define POP_MODULE_STREAMER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_task_base_synch.h"

//extern NET_PROTOCOL_POP_Export const char libacenetwork_protocol_default_POP_streamer_module_name_string[];
extern const char libacenetwork_protocol_default_pop_streamer_module_name_string[];

template <typename SynchStrategyType,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class POP_Module_Streamer_T
 : public Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<SynchStrategyType,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  POP_Module_Streamer_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~POP_Module_Streamer_T () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (POP_Module_Streamer_T ())
  ACE_UNIMPLEMENTED_FUNC (POP_Module_Streamer_T (const POP_Module_Streamer_T&))
  ACE_UNIMPLEMENTED_FUNC (POP_Module_Streamer_T& operator= (const POP_Module_Streamer_T&))
};

// include template definition
#include "pop_module_streamer.inl"

#endif
