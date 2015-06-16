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

#ifndef NET_MODULE_HEADERPARSER_H
#define NET_MODULE_HEADERPARSER_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

#include "net_exports.h"

// forward declaration(s)
class Net_Message;
class Net_SessionMessage;

class Net_Export Net_Module_HeaderParser
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 Net_SessionMessage,
                                 Net_Message>
{
 public:
  Net_Module_HeaderParser ();
  virtual ~Net_Module_HeaderParser ();

  // initialization
  bool initialize ();

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Net_Message*&, // data message handle
                                  bool&);        // return value: pass message downstream ?

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 Net_SessionMessage,
                                 Net_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Module_HeaderParser (const Net_Module_HeaderParser&));
  ACE_UNIMPLEMENTED_FUNC (Net_Module_HeaderParser& operator= (const Net_Module_HeaderParser&));

  bool isInitialized_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,               // task synch type
                              Common_TimePolicy_t,        // time policy type
                              Stream_ModuleConfiguration, // configuration type
                              Net_Module_HeaderParser);   // writer type

#endif
