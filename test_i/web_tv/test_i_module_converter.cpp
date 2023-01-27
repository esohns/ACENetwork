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
#include "stdafx.h"

#include "test_i_module_converter.h"

#include "ace/Log_Msg.h"

#include "common_string_tools.h"

#include "net_macros.h"

Test_I_LibAVConverter::Test_I_LibAVConverter (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_LibAVConverter::Test_I_LibAVConverter"));

}

void
Test_I_LibAVConverter::handleDataMessage (Test_I_Message*& message_inout,
                                          bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_LibAVConverter::handleDataMessage"));

  switch (message_inout->getMediaType())
  {
    case STREAM_MEDIATYPE_AUDIO:
      break; // do nothing
    case STREAM_MEDIATYPE_VIDEO:
    {
      // *NOTE*: does not set media type :-(
      //inherited::handleDataMessage (message_inout,
      //                              passMessageDownstream_out);
      handleDataMessage_2 (message_inout,
                           passMessageDownstream_out);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown media type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  message_inout->getMediaType ()));
      passMessageDownstream_out = false;
      message_inout->release (); message_inout = NULL;
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
      break;
    }
  } // end SWITCH
}

void
Test_I_LibAVConverter::handleDataMessage_2 (Test_I_Message*& message_inout,
                                            bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_LibAVConverter::handleDataMessage_2"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::frame_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  int line_sizes_a[AV_NUM_DATA_POINTERS];
  uint8_t* data_a[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&line_sizes_a, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (&data_a, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));

  // sanity check(s)
  ACE_ASSERT (inherited::buffer_);
//  ACE_ASSERT (buffer_->capacity () >= frameSize_);

  result = av_image_fill_linesizes (line_sizes_a,
                                    inherited::inputFormat_,
                                    static_cast<int> (inherited::frame_->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (data_a,
                              inherited::inputFormat_,
                              static_cast<int> (inherited::frame_->height),
                              reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
                              line_sizes_a);
  ACE_ASSERT (result >= 0);
  if (unlikely (!Stream_Module_Decoder_Tools::convert (context_,
                                                       inherited::frame_->width, inherited::frame_->height, inputFormat_,
                                                       data_a,
                                                       inherited::frame_->width, inherited::frame_->height, static_cast<AVPixelFormat> (inherited::frame_->format),
                                                       inherited::frame_->data,
                                                       inherited::configuration_->flipImage)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  inherited::buffer_->wr_ptr (inherited::frameSize_);
  inherited::buffer_->initialize (message_inout->sessionId (),
                                  NULL);
  inherited::buffer_->set (message_inout->type ());
  inherited::buffer_->setMediaType (STREAM_MEDIATYPE_VIDEO);
  message_inout->release (); message_inout = NULL;

  // forward the converted frame
  result = inherited::put_next (inherited::buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  inherited::buffer_ = NULL;

  // allocate a message buffer for the next frame
  inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
  if (unlikely (!inherited::buffer_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                inherited::frameSize_));
    goto error;
  } // end IF
//  av_frame_unref (frame_);
  result = av_image_fill_linesizes (inherited::frame_->linesize,
                                    static_cast<AVPixelFormat> (inherited::frame_->format),
                                    static_cast<int> (inherited::frame_->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (inherited::frame_->data,
                              static_cast<AVPixelFormat> (inherited::frame_->format),
                              static_cast<int> (inherited::frame_->height),
                              reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                              inherited::frame_->linesize);
  ACE_ASSERT (result >= 0);

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}
