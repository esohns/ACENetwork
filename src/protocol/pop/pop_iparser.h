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

#ifndef POP_IPARSER_H
#define POP_IPARSER_H

#include "common_parser_common.h"

#include "common_iscanner.h"
#include "common_iparser.h"

#include "pop_common.h"

class POP_IParser
 : public Common_IYaccStreamParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct POP_Record>
 , public Common_ILexScanner_T<struct Common_FlexScannerState,
                               POP_IParser>
{
 public:
  // convenient types
  typedef Common_IYaccStreamParser_T<struct Common_FlexBisonParserConfiguration,
                                     struct POP_Record> IPARSER_T;

  using IPARSER_T::error;
};

#endif
