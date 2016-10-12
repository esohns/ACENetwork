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

#ifndef HTTP_TOOLS_H
#define HTTP_TOOLS_H

#include <string>

#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>

#include "stream_dec_common.h"

#include "http_codes.h"
#include "http_common.h"
#include "http_exports.h"

class HTTP_Export HTTP_Tools
{
 public:
  inline virtual ~HTTP_Tools () {};

  // debug info
  static std::string dump (const HTTP_Record&);

  static std::string Method2String (const HTTP_Method_t&);
  static std::string Version2String (const HTTP_Version_t&);
  static std::string Status2Reason (const HTTP_Status_t&);

  static HTTP_Method_t Method2Type (const std::string&);
  static HTTP_Version_t Version2Type (const std::string&);

  static bool isHeaderType (const std::string&,      // field name
                            HTTP_Codes::HeaderType); // header type
  static bool isRequest (const HTTP_Record&);

  static enum Stream_Decoder_CompressionFormatType Encoding2CompressionFormat (const std::string&);

  static bool parseURL (const std::string&, // URL
                        ACE_INET_Addr&,     // return value: host address
                        std::string&);      // return value: URI

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_Tools ())
  //ACE_UNIMPLEMENTED_FUNC (~HTTP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_Tools (const HTTP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Tools& operator= (const HTTP_Tools&))
};

#endif
