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
#include "stdafx.h"

#include "net_wlan_profile_xml_handler.h"

#include <sstream>

#include <wlantypes.h>

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "net_wlan_common.h"
#include "net_wlan_defines.h"

//Net_WLAN_Profile_XML_Handler::Net_WLAN_Profile_XML_Handler (struct Net_WLAN_Profile_ParserContext* parserContextHandle_in)
// : inherited ()
// , inHex_ (false)
// , inSSID_ (false)
// , inSSIDConfig_ (false)
// , parserContextHandle_ (parserContextHandle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::Net_WLAN_Profile_XML_Handler"));
//
//}
//
//void
//Net_WLAN_Profile_XML_Handler::characters (const ACEXML_Char* characters_in,
//                                          size_t start_in,
//                                          size_t length_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::characters"));
//
//  ACE_UNUSED_ARG (start_in);
//  ACE_UNUSED_ARG (length_in);
//
//  if (likely (!inHex_))
//    return;
//
//  // sanity check(s)
//  ACE_ASSERT (parserContextHandle_);
//
//  parserContextHandle_->SSIDs.push_back (hexToString (ACE_TEXT_ALWAYS_CHAR (characters_in)));
//}
//
//void
//Net_WLAN_Profile_XML_Handler::endElement (const ACEXML_Char* namespaceURI_in,
//                                          const ACEXML_Char* localName_in,
//                                          const ACEXML_Char* qName_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::endElement"));
//
//  ACE_UNUSED_ARG (namespaceURI_in);
//  ACE_UNUSED_ARG (qName_in);
//
//  if (inSSIDConfig_ && !inSSID_)
//    goto in_ssid;
//  else if (inSSID_)
//    goto in_hex;
//
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSIDCONFIG_ELEMENT_STRING))))
//    inSSIDConfig_ = false;
//
//  return;
//
//in_ssid:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSID_ELEMENT_STRING))))
//    inSSID_ = false;
//
//  return;
//
//in_hex:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_HEX_ELEMENT_STRING))))
//    inHex_ = false;
//}
//
//void
//Net_WLAN_Profile_XML_Handler::startElement (const ACEXML_Char* namespaceURI_in,
//                                            const ACEXML_Char* localName_in,
//                                            const ACEXML_Char* qName_in,
//                                            ACEXML_Attributes* attributes_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::startElement"));
//
//  ACE_UNUSED_ARG (namespaceURI_in);
//  ACE_UNUSED_ARG (qName_in);
//  ACE_UNUSED_ARG (attributes_in);
//
//  if (inSSIDConfig_ && !inSSID_)
//    goto in_ssid;
//  else if (inSSID_)
//    goto in_hex;
//
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSIDCONFIG_ELEMENT_STRING))))
//    inSSIDConfig_ = true;
//
//  return;
//
//in_ssid:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSID_ELEMENT_STRING))))
//    inSSID_ = true;
//
//  return;
//
//in_hex:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_HEX_ELEMENT_STRING))))
//    inHex_ = true;
//}
//
////////////////////////////////////////////
//
//void
//Net_WLAN_Profile_XML_Handler::error (ACEXML_SAXParseException& exception_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::error"));
//
//  //char* message = XMLString::transcode(exception_in.getMessage());
//  //ACE_ASSERT(message);
//
//  //ACE_DEBUG((LM_ERROR,
//  //           ACE_TEXT("Net_WLAN_Profile_XML_Handler::error(file: \"%s\", line: %d, column: %d): \"%s\"\n"),
//  //           ACE_TEXT(exception_in.getSystemId()),
//  //           exception_in.getLineNumber(),
//  //           exception_in.getColumnNumber(),
//  //           ACE_TEXT(message)));
//
//  //// clean up
//  //XMLString::release(&message);
//}
//
//void
//Net_WLAN_Profile_XML_Handler::fatalError (ACEXML_SAXParseException& exception_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::fatalError"));
//
//  //char* message = XMLString::transcode(exception_in.getMessage());
//  //ACE_ASSERT(message);
//
//  //ACE_DEBUG((LM_CRITICAL,
//  //           ACE_TEXT("Net_WLAN_Profile_XML_Handler::fatalError(file: \"%s\", line: %d, column: %d): \"%s\"\n"),
//  //           ACE_TEXT(exception_in.getSystemId()),
//  //           exception_in.getLineNumber(),
//  //           exception_in.getColumnNumber(),
//  //           ACE_TEXT(message)));
//
//  //// clean up
//  //XMLString::release(&message);
//}
//
//void
//Net_WLAN_Profile_XML_Handler::warning (ACEXML_SAXParseException& exception_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::warning"));
//
//  //char* message = XMLString::transcode(exception_in.getMessage());
//  //ACE_ASSERT(message);
//
//  //ACE_DEBUG((LM_WARNING,
//  //           ACE_TEXT("Net_WLAN_Profile_XML_Handler::warning(file: \"%s\", line: %d, column: %d): \"%s\"\n"),
//  //           ACE_TEXT(exception_in.getSystemId()),
//  //           exception_in.getLineNumber(),
//  //           exception_in.getColumnNumber(),
//  //           ACE_TEXT(message)));
//
//  //// clean up
//  //XMLString::release(&message);
//}
//
////Net_WLAN_Profile_XML_Handler::XMLElementType
////Net_WLAN_Profile_XML_Handler::stringToXMLElementType(const std::string& elementString_in) const
////{
////  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_Handler::stringToXMLElementType"));
////
////  if (elementString_in == ACE_TEXT_ALWAYS_CHAR("schema"))
////    return XML_SCHEMA;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("annotation"))
////    return XML_ANNOTATION;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("choice"))
////    return XML_CHOICE;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("documentation"))
////    return XML_DOCUMENTATION;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("simpleType"))
////    return XML_SIMPLETYPE;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("restriction"))
////    return XML_RESTRICTION;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("enumeration"))
////    return XML_ENUMERATION;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("complexType"))
////    return XML_COMPLEXTYPE;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("complexContent"))
////    return XML_COMPLEXCONTENT;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("sequence"))
////    return XML_SEQUENCE;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("attribute"))
////    return XML_ATTRIBUTE;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("element"))
////    return XML_ELEMENT;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("include"))
////    return XML_INCLUDE;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("extension"))
////    return XML_EXTENSION;
////  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("union"))
////    return XML_UNION;
////  else
////  {
////    ACE_DEBUG((LM_WARNING,
////               ACE_TEXT("unknown XML element type: \"%s\", aborting\n"),
////               ACE_TEXT(elementString_in.c_str())));
////  } // end ELSE
////
////  return XML_INVALID;
////}
////
////void
////Net_WLAN_Profile_XML_Handler::insertPreamble(std::ofstream& targetStream_inout)
////{
////  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_Handler::insertPreamble"));
////
////  // step1: insert file contents as-is
////  targetStream_inout << myPreamble;
////  targetStream_inout << std::endl;
////
////  // step2: insert edit warning
////  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("// -------------------------------- * * * ----------------------------------- //") << std::endl;
////  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("// PLEASE NOTE: this file was/is generated by ");
////#if defined _MSC_VER
////	targetStream_inout << XML2CPPCODE_PACKAGE_STRING << std::endl;
////#else
//////  // *TODO*: leave as-is for now (see Yarp/configure.ac)
//////  targetStream_inout << PACKAGE_STRING << std::endl;
////  targetStream_inout << XML2CPPCODE_PACKAGE_STRING << std::endl;
////#endif
////  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("// -------------------------------- * * * ----------------------------------- //") << std::endl;
////  targetStream_inout << std::endl;
////}
////
////void
////Net_WLAN_Profile_XML_Handler::insertMultipleIncludeProtection(const bool& usePragmaOnce_in,
////                                             const std::string& filename_in,
////                                             std::ofstream& targetStream_inout)
////{
////  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_Handler::insertMultipleIncludeProtection"));
////
////  if (usePragmaOnce_in)
////    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#pragma once") << std::endl;
////
////  // generate a "unique" identifier
////  std::string definition = filename_in;
////  // replace '.' with '_'s
////  std::string dot = ACE_TEXT_ALWAYS_CHAR(".");
////  std::string::size_type position = std::string::npos;
////  while ((position = definition.find(dot)) != std::string::npos)
////  {
////    definition.replace(position, 1, ACE_TEXT_ALWAYS_CHAR("_"));
////  } // end WHILE
////  // transform to uppercase
////  std::transform(definition.begin(),
////                 definition.end(),
////                 definition.begin(),
////                 std::bind2nd(std::ptr_fun(&std::toupper<char>),
////                              std::locale("")));
////
////  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#ifndef ");
////  targetStream_inout << definition << std::endl;
////  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#define ");
////  targetStream_inout << definition << std::endl;
////  targetStream_inout << std::endl;
////}
////
////void
////Net_WLAN_Profile_XML_Handler::insertPostscript(std::ofstream& targetStream_inout)
////{
////  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_Handler::insertPostscript"));
////
////  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#endif");
////  targetStream_inout << std::endl;
////}
////
////void
////Net_WLAN_Profile_XML_Handler::insertIncludeHeaders(const XML2CppCode_Headers_t& headers_in,
////                                  const bool& includeStdVector_in,
////                                  std::ofstream& targetStream_inout)
////{
////  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_Handler::insertIncludeHeaders"));
////
////  if (includeStdVector_in)
////    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#include <vector>")
////                       << std::endl;
////
////  for (XML2CppCode_HeadersIterator_t iterator = headers_in.begin();
////       iterator != headers_in.end();
////       iterator++)
////  {
////    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#include \"");
////    targetStream_inout << *iterator;
////    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("\"");
////    targetStream_inout << std::endl;
////  } // end FOR
////  targetStream_inout << std::endl;
////}
//
//unsigned char
//Net_WLAN_Profile_XML_Handler::hexToValue (unsigned char hexValue_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::hexToValue"));
//
//  // C++98 guarantees that '0', '1', ... '9' are consecutive.
//  // It only guarantees that 'a' ... 'f' and 'A' ... 'F' are
//  // in increasing order, but the only two alternative encodings
//  // of the basic source character set that are still used by
//  // anyone today (ASCII and EBCDIC) make them consecutive.
//  if ('0' <= hexValue_in && hexValue_in <= '9')
//    return hexValue_in - '0';
//  else if ('a' <= hexValue_in && hexValue_in <= 'f')
//    return hexValue_in - 'a' + 10;
//  else if ('A' <= hexValue_in && hexValue_in <= 'F')
//    return hexValue_in - 'A' + 10;
//
//  return 0;
//}
//std::string
//Net_WLAN_Profile_XML_Handler::hexToString (const std::string& hexString_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::hexToString"));
//
//  std::string result;
//  result.reserve (hexString_in.size () / 2);
//
//  // sanity check(s)
//  ACE_ASSERT (!(hexString_in.size () % 2));
//  ACE_ASSERT (!(hexString_in.size () > (DOT11_SSID_MAX_LENGTH * 2)));
//
//  unsigned char character_c = 0, temp = 0;
//  std::istringstream string_stream (hexString_in);
//  string_stream.flags (std::ios::hex);
//  do
//  {
//    string_stream >> std::dec >> character_c;
//    if (unlikely (string_stream.eof ()))
//      break;
//    string_stream >> std::dec >> temp;
//    character_c = (hexToValue (character_c) << 4) + hexToValue (temp);
//    result.push_back (character_c);
//  } while (true);
//
//  return result;
//}
//
////////////////////////////////////////////
//
//Net_WLAN_Profile_XML_ListHandler::Net_WLAN_Profile_XML_ListHandler (struct Net_WLAN_Profile_ParserContext* parserContextHandle_in)
// : inherited ()
// , inHex_ (false)
// , inSSID_ (false)
// , inSSIDConfig_ (false)
// , parserContextHandle_ (parserContextHandle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::Net_WLAN_Profile_XML_ListHandler"));
//
//}
//
//void
//Net_WLAN_Profile_XML_ListHandler::characters (const ACEXML_Char* characters_in,
//                                              size_t start_in,
//                                              size_t length_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::characters"));
//
//  ACE_UNUSED_ARG (start_in);
//  ACE_UNUSED_ARG (length_in);
//
//  if (likely (!inHex_))
//    return;
//
//  // sanity check(s)
//  ACE_ASSERT (parserContextHandle_);
//
//  parserContextHandle_->SSIDs.push_back (hexToString (ACE_TEXT_ALWAYS_CHAR (characters_in)));
//}
//
//void
//Net_WLAN_Profile_XML_ListHandler::endElement (const ACEXML_Char* namespaceURI_in,
//                                              const ACEXML_Char* localName_in,
//                                              const ACEXML_Char* qName_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::endElement"));
//
//  ACE_UNUSED_ARG (namespaceURI_in);
//  ACE_UNUSED_ARG (qName_in);
//
//  if (inSSIDConfig_ && !inSSID_)
//    goto in_ssid;
//  else if (inSSID_)
//    goto in_hex;
//
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSIDCONFIG_ELEMENT_STRING))))
//    inSSIDConfig_ = false;
//
//  return;
//
//in_ssid:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSID_ELEMENT_STRING))))
//    inSSID_ = false;
//
//  return;
//
//in_hex:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_HEX_ELEMENT_STRING))))
//    inHex_ = false;
//}
//
//void
//Net_WLAN_Profile_XML_ListHandler::startElement (const ACEXML_Char* namespaceURI_in,
//                                                const ACEXML_Char* localName_in,
//                                                const ACEXML_Char* qName_in,
//                                                ACEXML_Attributes* attributes_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::startElement"));
//
//  ACE_UNUSED_ARG (namespaceURI_in);
//  ACE_UNUSED_ARG (qName_in);
//  ACE_UNUSED_ARG (attributes_in);
//
//  if (inSSIDConfig_ && !inSSID_)
//    goto in_ssid;
//  else if (inSSID_)
//    goto in_hex;
//
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSIDCONFIG_ELEMENT_STRING))))
//    inSSIDConfig_ = true;
//
//  return;
//
//in_ssid:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_SSID_ELEMENT_STRING))))
//    inSSID_ = true;
//
//  return;
//
//in_hex:
//  if (unlikely (!ACE_OS::strcmp (ACE_TEXT (localName_in),
//                                 ACE_TEXT (NET_WLAN_PROFILE_HEX_ELEMENT_STRING))))
//    inHex_ = true;
//}

//////////////////////////////////////////

void
Net_WLAN_Profile_XML_ListHandler::error (ACEXML_SAXParseException& exception_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::error"));

  //char* message = XMLString::transcode(exception_in.getMessage());
  //ACE_ASSERT(message);

  //ACE_DEBUG((LM_ERROR,
  //           ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::error(file: \"%s\", line: %d, column: %d): \"%s\"\n"),
  //           ACE_TEXT(exception_in.getSystemId()),
  //           exception_in.getLineNumber(),
  //           exception_in.getColumnNumber(),
  //           ACE_TEXT(message)));

  //// clean up
  //XMLString::release(&message);
}

void
Net_WLAN_Profile_XML_ListHandler::fatalError (ACEXML_SAXParseException& exception_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::fatalError"));

  //char* message = XMLString::transcode(exception_in.getMessage());
  //ACE_ASSERT(message);

  //ACE_DEBUG((LM_CRITICAL,
  //           ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::fatalError(file: \"%s\", line: %d, column: %d): \"%s\"\n"),
  //           ACE_TEXT(exception_in.getSystemId()),
  //           exception_in.getLineNumber(),
  //           exception_in.getColumnNumber(),
  //           ACE_TEXT(message)));

  //// clean up
  //XMLString::release(&message);
}

void
Net_WLAN_Profile_XML_ListHandler::warning (ACEXML_SAXParseException& exception_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::warning"));

  //char* message = XMLString::transcode(exception_in.getMessage());
  //ACE_ASSERT(message);

  //ACE_DEBUG((LM_WARNING,
  //           ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::warning(file: \"%s\", line: %d, column: %d): \"%s\"\n"),
  //           ACE_TEXT(exception_in.getSystemId()),
  //           exception_in.getLineNumber(),
  //           exception_in.getColumnNumber(),
  //           ACE_TEXT(message)));

  //// clean up
  //XMLString::release(&message);
}

//Net_WLAN_Profile_XML_ListHandler::XMLElementType
//Net_WLAN_Profile_XML_ListHandler::stringToXMLElementType(const std::string& elementString_in) const
//{
//  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::stringToXMLElementType"));
//
//  if (elementString_in == ACE_TEXT_ALWAYS_CHAR("schema"))
//    return XML_SCHEMA;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("annotation"))
//    return XML_ANNOTATION;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("choice"))
//    return XML_CHOICE;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("documentation"))
//    return XML_DOCUMENTATION;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("simpleType"))
//    return XML_SIMPLETYPE;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("restriction"))
//    return XML_RESTRICTION;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("enumeration"))
//    return XML_ENUMERATION;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("complexType"))
//    return XML_COMPLEXTYPE;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("complexContent"))
//    return XML_COMPLEXCONTENT;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("sequence"))
//    return XML_SEQUENCE;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("attribute"))
//    return XML_ATTRIBUTE;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("element"))
//    return XML_ELEMENT;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("include"))
//    return XML_INCLUDE;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("extension"))
//    return XML_EXTENSION;
//  else if (elementString_in == ACE_TEXT_ALWAYS_CHAR("union"))
//    return XML_UNION;
//  else
//  {
//    ACE_DEBUG((LM_WARNING,
//               ACE_TEXT("unknown XML element type: \"%s\", aborting\n"),
//               ACE_TEXT(elementString_in.c_str())));
//  } // end ELSE
//
//  return XML_INVALID;
//}
//
//void
//Net_WLAN_Profile_XML_ListHandler::insertPreamble(std::ofstream& targetStream_inout)
//{
//  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::insertPreamble"));
//
//  // step1: insert file contents as-is
//  targetStream_inout << myPreamble;
//  targetStream_inout << std::endl;
//
//  // step2: insert edit warning
//  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("// -------------------------------- * * * ----------------------------------- //") << std::endl;
//  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("// PLEASE NOTE: this file was/is generated by ");
//#if defined _MSC_VER
//	targetStream_inout << XML2CPPCODE_PACKAGE_STRING << std::endl;
//#else
////  // *TODO*: leave as-is for now (see Yarp/configure.ac)
////  targetStream_inout << PACKAGE_STRING << std::endl;
//  targetStream_inout << XML2CPPCODE_PACKAGE_STRING << std::endl;
//#endif
//  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("// -------------------------------- * * * ----------------------------------- //") << std::endl;
//  targetStream_inout << std::endl;
//}
//
//void
//Net_WLAN_Profile_XML_ListHandler::insertMultipleIncludeProtection(const bool& usePragmaOnce_in,
//                                             const std::string& filename_in,
//                                             std::ofstream& targetStream_inout)
//{
//  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::insertMultipleIncludeProtection"));
//
//  if (usePragmaOnce_in)
//    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#pragma once") << std::endl;
//
//  // generate a "unique" identifier
//  std::string definition = filename_in;
//  // replace '.' with '_'s
//  std::string dot = ACE_TEXT_ALWAYS_CHAR(".");
//  std::string::size_type position = std::string::npos;
//  while ((position = definition.find(dot)) != std::string::npos)
//  {
//    definition.replace(position, 1, ACE_TEXT_ALWAYS_CHAR("_"));
//  } // end WHILE
//  // transform to uppercase
//  std::transform(definition.begin(),
//                 definition.end(),
//                 definition.begin(),
//                 std::bind2nd(std::ptr_fun(&std::toupper<char>),
//                              std::locale("")));
//
//  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#ifndef ");
//  targetStream_inout << definition << std::endl;
//  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#define ");
//  targetStream_inout << definition << std::endl;
//  targetStream_inout << std::endl;
//}
//
//void
//Net_WLAN_Profile_XML_ListHandler::insertPostscript(std::ofstream& targetStream_inout)
//{
//  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::insertPostscript"));
//
//  targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#endif");
//  targetStream_inout << std::endl;
//}
//
//void
//Net_WLAN_Profile_XML_ListHandler::insertIncludeHeaders(const XML2CppCode_Headers_t& headers_in,
//                                  const bool& includeStdVector_in,
//                                  std::ofstream& targetStream_inout)
//{
//  NETWORK_TRACE(ACE_TEXT("Net_WLAN_Profile_XML_ListHandler::insertIncludeHeaders"));
//
//  if (includeStdVector_in)
//    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#include <vector>")
//                       << std::endl;
//
//  for (XML2CppCode_HeadersIterator_t iterator = headers_in.begin();
//       iterator != headers_in.end();
//       iterator++)
//  {
//    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("#include \"");
//    targetStream_inout << *iterator;
//    targetStream_inout << ACE_TEXT_ALWAYS_CHAR("\"");
//    targetStream_inout << std::endl;
//  } // end FOR
//  targetStream_inout << std::endl;
//}

unsigned char
Net_WLAN_Profile_XML_ListHandler::hexToValue (unsigned char hexValue_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_Handler::hexToValue"));

  // C++98 guarantees that '0', '1', ... '9' are consecutive.
  // It only guarantees that 'a' ... 'f' and 'A' ... 'F' are
  // in increasing order, but the only two alternative encodings
  // of the basic source character set that are still used by
  // anyone today (ASCII and EBCDIC) make them consecutive.
  if ('0' <= hexValue_in && hexValue_in <= '9')
    return hexValue_in - '0';
  else if ('a' <= hexValue_in && hexValue_in <= 'f')
    return hexValue_in - 'a' + 10;
  else if ('A' <= hexValue_in && hexValue_in <= 'F')
    return hexValue_in - 'A' + 10;

  return 0;
}
std::string
Net_WLAN_Profile_XML_ListHandler::hexToString (const std::string& hexString_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Profile_XML_ListHandler::hexToString"));

  std::string result;
  result.reserve (hexString_in.size () / 2);

  // sanity check(s)
  ACE_ASSERT (!(hexString_in.size () % 2));
  ACE_ASSERT (!(hexString_in.size () > (DOT11_SSID_MAX_LENGTH * 2)));

  unsigned char character_c = 0, temp = 0;
  std::istringstream string_stream (hexString_in);
  string_stream.flags (std::ios::hex);
  do
  {
    string_stream >> std::dec >> character_c;
    if (unlikely (string_stream.eof ()))
      break;
    string_stream >> std::dec >> temp;
    character_c = (hexToValue (character_c) << 4) + hexToValue (temp);
    result.push_back (character_c);
  } while (true);

  return result;
}
