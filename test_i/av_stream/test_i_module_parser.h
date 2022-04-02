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

#ifndef TEST_I_AVSTREAM_MODULE_PARSER_H
#define TEST_I_AVSTREAM_MODULE_PARSER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_misc_parser.h"

#include "test_i_av_stream_parser_driver.h"

// forward declaration(s)
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Test_I_AVStream_Parser_T
 : public Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 AVStream_ParserDriver,
                                 struct Stream_UserData>
{
  typedef Stream_Module_Parser_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 AVStream_ParserDriver,
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Parser_T (ISTREAM_T*);                     // stream handle
#else
  Test_I_AVStream_Parser_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Test_I_AVStream_Parser_T () {}

  virtual void message_cb ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Parser_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Parser_T (const Test_I_AVStream_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Parser_T& operator= (const Test_I_AVStream_Parser_T&))
};

// include template definition
#include "test_i_module_parser.inl"

#endif
