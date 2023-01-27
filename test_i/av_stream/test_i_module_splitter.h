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

#ifndef TEST_I_MODULE_SPLITTER_H
#define TEST_I_MODULE_SPLITTER_H

#include "ace/Global_Macros.h"
#include "ace/Module.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_distributor.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType> // reference counted-
class Test_I_AVStream_Splitter_T
 : public Stream_Miscellaneous_Distributor_WriterTask_T<ACE_SYNCH_USE,
                                                        Common_TimePolicy_t,
                                                        ConfigurationType,
                                                        ControlMessageType,
                                                        MessageType,
                                                        SessionMessageType,
                                                        SessionDataType>
{
  typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_SYNCH_USE,
                                                        Common_TimePolicy_t,
                                                        ConfigurationType,
                                                        ControlMessageType,
                                                        MessageType,
                                                        SessionMessageType,
                                                        SessionDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Splitter_T (ISTREAM_T*);                     // stream handle
#else
  Test_I_AVStream_Splitter_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  inline virtual ~Test_I_AVStream_Splitter_T () {}

  //virtual void handleControlMessage (ControlMessageType&); // control message handle
  inline virtual void handleDataMessage (MessageType*& message_inout, bool& passMessageDownstream_out) { passMessageDownstream_out = false; forward (message_inout); }
  //virtual void handleSessionMessage (SessionMessageType*&, // session message handle
  //                                   bool&);               // return value: pass message downstream ?
  //inline virtual void handleUserMessage (ACE_Message_Block*& message_inout, bool& passMessageDownstream_out) { ACE_UNUSED_ARG (passMessageDownstream_out); forward (message_inout, false); }

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Splitter_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Splitter_T (const Test_I_AVStream_Splitter_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Splitter_T& operator= (const Test_I_AVStream_Splitter_T&))

  // helper methods
  void forward (MessageType*); // message handle
};

// include template definition
#include "test_i_module_splitter.inl"

#endif
