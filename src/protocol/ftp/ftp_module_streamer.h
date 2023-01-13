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

#ifndef FTP_MODULE_STREAMER_H
#define FTP_MODULE_STREAMER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_task_base_synch.h"

//extern NET_PROTOCOL_FTP_Export const char libacenetwork_protocol_default_ftp_streamer_module_name_string[];
extern const char libacenetwork_protocol_default_ftp_streamer_module_name_string[];

template <typename SynchStrategyType,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class FTP_Module_Streamer_T
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
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  FTP_Module_Streamer_T (ISTREAM_T*);                     // stream handle
#else
  FTP_Module_Streamer_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~FTP_Module_Streamer_T () {}

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_Streamer_T ())
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_Streamer_T (const FTP_Module_Streamer_T&))
  ACE_UNIMPLEMENTED_FUNC (FTP_Module_Streamer_T& operator= (const FTP_Module_Streamer_T&))
};

// include template definition
#include "ftp_module_streamer.inl"

#endif
