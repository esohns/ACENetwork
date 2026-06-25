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
  virtual bool hasFinished () const = 0;
  virtual bool headerOnly () = 0; // returns: parse HTTP header only ?

  virtual void record (struct HTTP_Record*&) = 0; // data record
  virtual void chunk (ACE_UINT32) = 0; // chunk size

  virtual ACE_Message_Block* head () = 0; // return value: head buffer handle
  virtual bool switchBuffer (bool = true) = 0; // begin() current fragment ?

  // virtual void chunkOffset (size_t) = 0;
  // virtual size_t chunkOffset () = 0;
};

#endif
