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

#ifndef HTTP_ANTLR_IPARSER_T_H
#define HTTP_ANTLR_IPARSER_T_H

#include "ace/Basic_Types.h"

// forward declarations
class ACE_Message_Block;
struct HTTP_Record;

class HTTP_ANTLR_IParser
{
 public:
  virtual bool initialize (const struct HTTP_ParserConfiguration&) = 0;
  virtual ACE_Message_Block* buffer () = 0;
  virtual bool isBlocking () const = 0;
  virtual size_t offset () const = 0;
  virtual bool begin (const char*, // buffer handle
                      size_t) = 0; // buffer size
  //virtual void end () = 0;
  virtual bool parse (ACE_Message_Block*) = 0; // data buffer handle
  virtual bool switchBuffer (bool = true) = 0; // begin() current fragment ?
  // *NOTE*: (waits for and) appends the next data chunk to fragment_;
  virtual void waitBuffer () = 0;
   
  virtual bool hasFinished () = 0;
  virtual bool headerOnly () = 0; // returns: parse HTTP header only ?
  virtual const struct HTTP_Record& current () = 0; // current record
  virtual void chunk_2 (ACE_UINT64, ACE_UINT32) = 0; // chunk offset, chunk size

  // *NOTE*: the implementation needs to frame the 'body' based on this data
  virtual void record (struct HTTP_Record*&) = 0; // record
  virtual void chunk (ACE_UINT32) = 0;            // chunk size
  virtual ACE_Message_Block* head () = 0; // return value: head buffer handle
};

#endif
