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

#ifndef NET_IPARSER_H
#define NET_IPARSER_H

#include <string>

#include "location.hh"

#include "common_idumpstate.h"
#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_iscanner.h"

#include "net_defines.h"

// forward declarations
struct yy_buffer_state;
class ACE_Message_Block;
class ACE_Message_Queue_Base;
typedef void* yyscan_t;

//////////////////////////////////////////

// *NOTE*: the template parameter ought to derive from Net_IParser_T
template <typename ParserInterfaceType>
class Net_IScanner_T
 : public Common_IScanner
 , public Common_ISetP_T<ParserInterfaceType>
{
 public:
  // *NOTE*: this is the C-ish interface (not needed by C++ scanners)
  virtual void debug (yyscan_t,  // state handle
                      bool) = 0; // toggle

  virtual bool initialize (yyscan_t&) = 0; // return value: state handle
  virtual void finalize (yyscan_t&) = 0; // state handle

  virtual struct yy_buffer_state* create (yyscan_t,    // state handle
                                          char*,       // buffer handle
                                          size_t) = 0; // buffer size
  virtual void destroy (yyscan_t,                      // state handle
                        struct yy_buffer_state*&) = 0; // buffer handle
};

//////////////////////////////////////////

template <typename ConfigurationType>
class Net_IParser_T
 : public Common_IInitialize_T<ConfigurationType>
 , public Common_IDumpState
{
 public:
  inline virtual ~Net_IParser_T () {};

  //virtual ACE_Message_Block* buffer () = 0;
  //virtual bool debugScanner () const = 0;
  //virtual bool isBlocking () const = 0;

//  virtual void error (const struct YYLTYPE&,
  virtual void error (const yy::location&,
                      const std::string&) = 0;

  // *NOTE*: to be invoked by the scanner (ONLY !)
  //virtual void offset (unsigned int) = 0; // offset (increment)
  //virtual unsigned int offset () const = 0;

  virtual bool parse (ACE_Message_Block*) = 0; // data buffer handle
  // *IMPORTANT NOTE*: when the parser detects a frame end, it inserts a new
  //                   buffer to the continuation and passes 'true'
  //                   --> separate the current frame from the next
  //virtual bool switchBuffer (bool = false) = 0; // unlink current buffer ?
  //virtual void wait () = 0;
};

//////////////////////////////////////////

template <typename ConfigurationType,
          typename RecordType>
class Net_IRecordParser_T
 : public Net_IParser_T<ConfigurationType>
{
 public:
  // convenient types
  typedef Net_IParser_T<ConfigurationType> IPARSER_T;

  virtual RecordType& current () = 0;

  virtual bool hasFinished () const = 0;

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (RecordType*&) = 0; // data record
};

//////////////////////////////////////////

template <typename ConfigurationType,
          typename RecordType>
class Net_IStreamParser_T
 : public Net_IParser_T<ConfigurationType>
{
 public:
  virtual RecordType& current () = 0;

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (RecordType*&) = 0; // data record
};

#endif
