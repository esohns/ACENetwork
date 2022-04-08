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

#ifndef M3U_IPARSER_H
#define M3U_IPARSER_H

#include <string>
#include <vector>

#include "common_configuration.h"

#include "common_iscanner.h"
#include "common_iparser.h"
#include "common_parser_common.h"

typedef std::vector<std::pair<std::string, std::string> > M3U_KeyValues_t;
typedef M3U_KeyValues_t::const_iterator M3U_KeyValuesIterator_t;

struct M3U_Element
{
  std::string Artist;
  std::string Album;
  ACE_INT32 Length;
  std::string Title;
  std::string URL;

  std::string key; // temp
  M3U_KeyValues_t keyValues;
};
typedef std::vector<struct M3U_Element> M3U_Playlist_t;
typedef M3U_Playlist_t::const_iterator M3U_PlaylistIterator_t;

class M3U_IParser
 : public Common_IYaccRecordParser_T<struct Common_FlexBisonParserConfiguration,
                                     M3U_Playlist_t>
 , virtual public Common_ILexScanner_T<struct Common_FlexScannerState,
                                       M3U_IParser>
 , public Common_ISetP_T<M3U_Playlist_t>
 , public Common_ISetP_2_T<struct M3U_Element>
{
 public:
  // convenient types
  typedef Common_IYaccRecordParser_T<struct Common_FlexBisonParserConfiguration,
                                     M3U_Playlist_t> IPARSER_T;
  typedef Common_ILexScanner_T<struct Common_FlexScannerState,
                               M3U_IParser> ISCANNER_T;

  using IPARSER_T::error;
//  using Common_IScanner::error;

  virtual struct M3U_Element& current_2 () = 0; // element handle

  virtual void record (M3U_Playlist_t*&) = 0; // data record
};

////////////////////////////////////////////

typedef M3U_IParser M3U_IParser_t;
typedef Common_ILexScanner_T<struct Common_FlexScannerState,
                             M3U_IParser_t> M3U_IScanner_t;

#endif
