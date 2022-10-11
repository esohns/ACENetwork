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

#ifndef Test_I_LibAVConverter_H
#define Test_I_LibAVConverter_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_dec_libav_converter.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_WebTV_ModuleHandlerConfiguration_3,
                               Stream_ControlMessage_t,
                               Test_I_Message,
                               Test_I_SessionMessage_3,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_TaskBaseSynch_t;

class Test_I_LibAVConverter
 : public Stream_Decoder_LibAVConverter_T<Test_I_TaskBaseSynch_t,
                                          struct Stream_MediaFramework_FFMPEG_MediaType>
{
  typedef Stream_Decoder_LibAVConverter_T<Test_I_TaskBaseSynch_t,
                                          struct Stream_MediaFramework_FFMPEG_MediaType> inherited;

 public:
  Test_I_LibAVConverter (ISTREAM_T*); // stream handle
  inline virtual ~Test_I_LibAVConverter () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_I_Message*&, // data message handle
                                  bool&);           // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_LibAVConverter ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_LibAVConverter (const Test_I_LibAVConverter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_LibAVConverter& operator= (const Test_I_LibAVConverter&))

  void handleDataMessage_2 (Test_I_Message*&, // data message handle
                            bool&);           // return value: pass message downstream ?
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_WebTV_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_WebTV_ModuleHandlerConfiguration_3,            // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_LibAVConverter);                                      // writer type

#endif
