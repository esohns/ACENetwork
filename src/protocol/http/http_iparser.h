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

#ifndef HTTP_IPARSER_H
#define HTTP_IPARSER_H

#include <string>

#include "common.h"

#include "common_iscanner.h"
#include "common_iparser.h"

#include "http_common.h"

class HTTP_IParser
 : public Common_IYaccRecordParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct HTTP_Record>
 , public Common_ILexScanner_T<struct Common_FlexScannerState,
                               HTTP_IParser>
{
 public:
  // convenient types
  typedef Common_IYaccRecordParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct HTTP_Record> IPARSER_T;

  using IPARSER_T::error;

  virtual unsigned int currentChunkSize () = 0; // current chunk size

  ////////////////////////////////////////
  // callbacks
  virtual void encoding (const std::string&) = 0; // encoding
  virtual void chunk (unsigned int) = 0; // size
};

#endif
