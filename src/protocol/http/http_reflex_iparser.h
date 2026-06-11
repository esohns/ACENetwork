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

#ifndef HTTP_REFLEX_IPARSER_T_H
#define HTTP_REFLEX_IPARSER_T_H

#include <string>

#include "ace/Basic_Types.h"

#include "common.h"

#include "common_iscanner.h"
#include "common_iparser.h"

#include "http_common.h"

#if !defined HTTP_LTYPE && !defined HTTP_LTYPE_IS_DECLARED
typedef struct HTTP_LTYPE HTTP_LTYPE;
struct HTTP_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
#define HTTP_LTYPE_IS_DECLARED 1
#define HTTP_LTYPE_IS_TRIVIAL 1
#endif

template <typename StateType,     // implements struct Common_ScannerState
          typename ExtraDataType> // 'extra' data type
class Common_IReflexScanner_T
 : public Common_IScanner_T<struct Common_FlexBisonParserConfiguration>
 , public Common_IGetR_2_T<StateType>
//, public Common_IGetSetP_T<ExtraDataType>
//, public Common_IGetP_2_T<ExtraDataType>
{
 public:
  virtual void setDebug (yyscan_t,  // state handle
                         bool) = 0; // toggle
  virtual void reset () = 0; // resets the offsets (line/column to 1,1)

  virtual bool initialize (yyscan_t&,                  // return value: state handle
                           ExtraDataType* = NULL) = 0; // 'extra' data handle
  virtual void finalize (yyscan_t&) = 0; // state handle

  virtual void* create (yyscan_t,    // state handle
                        char*,       // buffer handle
                        size_t) = 0; // buffer size
  virtual void destroy (yyscan_t,    // state handle
                        void*&) = 0; // buffer handle

  virtual bool lex (yyscan_t) = 0; // state handle
};

class HTTP_Reflex_IParser
 : public Common_IYaccRecordParser_T<struct HTTP_ParserConfiguration,
                                     struct HTTP_LTYPE,
                                     struct HTTP_Record>
 , public Common_IReflexScanner_T<struct Common_FlexScannerState,
                                  HTTP_Reflex_IParser>
{
 public:
  // convenient types
  typedef Common_IYaccRecordParser_T<struct HTTP_ParserConfiguration,
                                     struct HTTP_LTYPE,
                                     struct HTTP_Record> IPARSER_T;

  using IPARSER_T::error;

  virtual bool headerOnly () = 0; // returns: parse HTTP header only ?
  virtual ACE_UINT32 currentChunkSize () = 0; // returns: current chunk size
  virtual ACE_UINT64 contentLengthOrChunkSize () = 0;
  virtual ACE_UINT64 bodyOrChunkBytesToSkip () = 0;

  ////////////////////////////////////////
  // callbacks
  virtual void encoding (const std::string&) = 0; // encoding
  virtual void chunk (ACE_UINT32) = 0; // size
  virtual void contentLengthOrChunkSize (ACE_UINT64) = 0; // size
  virtual void bodyOrChunkBytesSkipped (ACE_UINT64) = 0; // content bytes skipped
};

#endif
