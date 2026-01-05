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

#ifndef TEST_I_MODULE_ENCODER_H
#define TEST_I_MODULE_ENCODER_H

#include "ace/Global_Macros.h"

#include "stream_streammodule_base.h"

#include "stream_dec_libav_encoder.h"

#include "test_i_message.h"
#include "test_i_web_tv_common.h"

// forward declaration(s)
struct AVFrame;
struct SwsContext;
class ACE_Message_Block;
class Stream_IAllocator;

class Test_I_Encoder
 : public Stream_Decoder_LibAVEncoder_2<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Test_I_WebTV_ModuleHandlerConfiguration_3,
                                        Stream_ControlMessage_t,
                                        Test_I_Message,
                                        Test_I_SessionMessage_3,
                                        struct Stream_MediaFramework_FFMPEG_MediaType>
{
  typedef Stream_Decoder_LibAVEncoder_2<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Test_I_WebTV_ModuleHandlerConfiguration_3,
                                        Stream_ControlMessage_t,
                                        Test_I_Message,
                                        Test_I_SessionMessage_3,
                                        struct Stream_MediaFramework_FFMPEG_MediaType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Encoder (ISTREAM_T*); // stream handle
#else
  Test_I_Encoder (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_Encoder () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (Test_I_Message*&, // data message handle
                                  bool&);           // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_I_SessionMessage_3*&, // session message handle
                                     bool&);                    // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Encoder ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Encoder (const Test_I_Encoder&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Encoder& operator= (const Test_I_Encoder&))
};

//////////////////////////////////////////

DATASTREAM_MODULE_INPUT_ONLY (Test_I_WebTV_SessionData_3,                                // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_WebTV_ModuleHandlerConfiguration_3,          // module handler configuration type
                              libacestream_default_dec_libav_encoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_Encoder);                                           // writer task

#endif // TEST_I_MODULE_ENCODER_H
