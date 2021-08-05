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

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "stream_dec_common.h"

#include "http_codes.h"
#include "http_common.h"

class HTTP_Tools
{
 public:
  inline virtual ~HTTP_Tools () {};

  // debug info
  static std::string dump (const struct HTTP_Record&);

  static std::string MethodToString (const HTTP_Method_t&);
  static std::string VersionToString (const HTTP_Version_t&);
  static std::string StatusToReason (const HTTP_Status_t&);

  static HTTP_Method_t MethodToType (const std::string&);
  static HTTP_Version_t VersionToType (const std::string&);

  static bool isHeaderType (const std::string&,      // field name
                            HTTP_Codes::HeaderType); // header type
  static bool isRequest (const struct HTTP_Record&);

  static enum Stream_Decoder_CompressionFormatType EncodingToCompressionFormat (const std::string&);

  static bool parseURL (const std::string&, // URL
                        std::string&,       // return value: host name
//                        ACE_INET_Addr&,     // return value: host address
                        std::string&,       // return value: URI
                        bool&);             // return value: use SSL ?
  static bool URLRequiresSSL (const std::string&); // URL

  // *NOTE*: this 'escapes' the input so it can be sent in an URL
  //         (see also: RFC 1738)
  static std::string URLEncode (const std::string&);
  // *NOTE*: removes any parameters (i.e. ?name=value&name=value...)
  static std::string stripURI (const std::string&);

  static std::string IPAddressToHostName (const ACE_INET_Addr&); // host address

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_Tools ())
  //ACE_UNIMPLEMENTED_FUNC (~HTTP_Tools ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_Tools (const HTTP_Tools&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Tools& operator= (const HTTP_Tools&))
};

#endif
