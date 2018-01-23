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

#ifndef NET_WLAN_PROFILE_XML_HANDLER_H
#define NET_WLAN_PROFILE_XML_HANDLER_H

#include <string>

#include "ace/Global_Macros.h"

#include "common_xml_handler_base.h"

// forward declarations
struct Net_WLAN_Profile_ParserContext;

class Net_WLAN_Profile_XML_Handler
 : public Common_XML_Handler_Base_T<struct Net_WLAN_Profile_ParserContext>
{
  typedef Common_XML_Handler_Base_T<struct Net_WLAN_Profile_ParserContext> inherited;

 public:
  Net_WLAN_Profile_XML_Handler (struct Net_WLAN_Profile_ParserContext*); // parser context handle
  inline virtual ~Net_WLAN_Profile_XML_Handler () {}

  // implement (part of) ACEXML_ContentHandler
  virtual void characters (const ACEXML_Char*, // chars
                           size_t,             // start
                           size_t);            // length
  virtual void endElement (const ACEXML_Char*,  // namespace uri
                           const ACEXML_Char*,  // localname
                           const ACEXML_Char*); // qname
  virtual void startElement (const ACEXML_Char*,  // namespace uri
                             const ACEXML_Char*,  // localname
                             const ACEXML_Char*,  // qname
                             ACEXML_Attributes*); // attrs

  // implement ACEXML_ErrorHandler
  virtual void error (ACEXML_SAXParseException&); // exception
  virtual void fatalError (ACEXML_SAXParseException&); // exception
  virtual void warning (ACEXML_SAXParseException&); // exception

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Profile_XML_Handler ());
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Profile_XML_Handler (const Net_WLAN_Profile_XML_Handler&));
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Profile_XML_Handler& operator= (const Net_WLAN_Profile_XML_Handler&));

  // helper methods
  unsigned char hexToValue (unsigned char);
  std::string hexToString (const std::string&);

  bool                                   inHex_;
  bool                                   inSSID_;
  bool                                   inSSIDConfig_;
  struct Net_WLAN_Profile_ParserContext* parserContextHandle_;
};

#endif
