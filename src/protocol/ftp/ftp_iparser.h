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

#ifndef FTP_IPARSER_H
#define FTP_IPARSER_H

#include "common_parser_common.h"

#include "common_iscanner.h"
#include "common_iparser.h"

#include "ftp_common.h"

class FTP_IParser
 : public Common_IYaccStreamParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct FTP_Record>
 , public Common_ILexScanner_T<struct Common_FlexScannerState,
                               FTP_IParser>
{
 public:
  // convenient types
  typedef Common_IYaccStreamParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct FTP_Record> IPARSER_T;

  using IPARSER_T::error;
};

//////////////////////////////////////////

struct FTP_IParserDataDummy
{};
class FTP_IParserData
 : public Common_IYaccStreamParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct FTP_IParserDataDummy>
 , public Common_ILexScanner_T<struct Common_FlexScannerState,
                               FTP_IParserData>
{
 public:
  // convenient types
  typedef Common_IYaccStreamParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct FTP_IParserDataDummy> IPARSER_T;

  using IPARSER_T::error;

  virtual void directory (const std::string&) = 0;
  virtual void file () = 0;
  virtual void data () = 0;

  virtual enum FTP_ProtocolDataState state () const = 0;
  virtual void state (enum FTP_ProtocolDataState) = 0;

  virtual void resetOffset () = 0;
  virtual ACE_UINT64 availableBytes () = 0;
};

#endif
