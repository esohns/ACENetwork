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

#include "stream_dec_common.h"

#include "http_tools.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <locale>
#include <regex>
#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"
#include "common_tools.h"

#include "net_macros.h"

#include "http_defines.h"

std::string
HTTP_Tools::dump (const struct HTTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::dump"));

  std::ostringstream converter;
  std::string buffer;
  bool is_request = false;
  HTTP_HeadersConstIterator_t iterator;

  if (HTTP_Tools::isRequest (record_in))
  {
    is_request = true;

    buffer = ACE_TEXT_ALWAYS_CHAR ("Method: ");
    buffer += HTTP_Tools::MethodToString (record_in.method);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Request-URI: ");
    buffer += record_in.URI;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Version: ");
    buffer += HTTP_Tools::VersionToString (record_in.version);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end IF
  else
  {
    buffer = ACE_TEXT_ALWAYS_CHAR ("Version: ");
    buffer += HTTP_Tools::VersionToString (record_in.version);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Status: ");
    converter << record_in.status;
    buffer += converter.str ();
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Reason: \"");
    buffer += HTTP_Tools::StatusToReason (record_in.status);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  } // end ELSE

  buffer += ACE_TEXT_ALWAYS_CHAR ("Headers (");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << record_in.headers.size ();
  buffer += converter.str ();
  buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");
  // general headers
  for (iterator = record_in.headers.begin ();
       iterator != record_in.headers.end ();
       ++iterator)
  {
    if (!HTTP_Tools::isHeaderType ((*iterator).first,
                                   HTTP_Codes::HTTP_HEADER_GENERAL))
      continue;

    buffer += (*iterator).first;
    buffer += ACE_TEXT_ALWAYS_CHAR (": \"");
    buffer += (*iterator).second;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  } // end FOR
  buffer += ACE_TEXT_ALWAYS_CHAR ("-------------------------------\n");

  // general headers
  for (iterator = record_in.headers.begin ();
       iterator != record_in.headers.end ();
       ++iterator)
  {
    if (!HTTP_Tools::isHeaderType ((*iterator).first,
                                   (is_request ? HTTP_Codes::HTTP_HEADER_REQUEST
                                               : HTTP_Codes::HTTP_HEADER_RESPONSE)))
      continue;

    buffer += (*iterator).first;
    buffer += ACE_TEXT_ALWAYS_CHAR (": \"");
    buffer += (*iterator).second;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  } // end FOR
  buffer += ACE_TEXT_ALWAYS_CHAR ("-------------------------------\n");

  // general headers
  for (iterator = record_in.headers.begin ();
       iterator != record_in.headers.end ();
       ++iterator)
  {
    if (!HTTP_Tools::isHeaderType ((*iterator).first,
                                   HTTP_Codes::HTTP_HEADER_ENTITY))
      continue;

    buffer += (*iterator).first;
    buffer += ACE_TEXT_ALWAYS_CHAR (": \"");
    buffer += (*iterator).second;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
  } // end FOR
  buffer += ACE_TEXT_ALWAYS_CHAR ("Headers /END\n");

  if (!record_in.form.empty ())
  {
    buffer += ACE_TEXT_ALWAYS_CHAR ("Form (");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << record_in.form.size ();
    buffer += converter.str ();
    buffer += ACE_TEXT_ALWAYS_CHAR ("):\n");

    for (HTTP_FormIterator_t iterator_2 = record_in.form.begin ();
         iterator_2 != record_in.form.end ();
         ++iterator_2)
    {
      buffer += ACE_TEXT_ALWAYS_CHAR ("\"");
      buffer += (*iterator_2).first;
      buffer += ACE_TEXT_ALWAYS_CHAR ("\"=\"");
      buffer += (*iterator_2).second;
      buffer += ACE_TEXT_ALWAYS_CHAR ("\"\n");
    } // end FOR
    buffer += ACE_TEXT_ALWAYS_CHAR ("Form /END\n");
  } // end IF

  return buffer;
}

std::string
HTTP_Tools::MethodToString (const HTTP_Method_t& method_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::MethodToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (method_in)
  {
    case HTTP_Codes::HTTP_METHOD_GET:
      result = ACE_TEXT_ALWAYS_CHAR ("GET"); break;
    case HTTP_Codes::HTTP_METHOD_POST:
      result = ACE_TEXT_ALWAYS_CHAR ("POST"); break;
    case HTTP_Codes::HTTP_METHOD_HEAD:
      result = ACE_TEXT_ALWAYS_CHAR ("HEAD"); break;
    case HTTP_Codes::HTTP_METHOD_M_SEARCH:
      result = ACE_TEXT_ALWAYS_CHAR ("M-SEARCH"); break;
    case HTTP_Codes::HTTP_METHOD_NOTIFY:
      result = ACE_TEXT_ALWAYS_CHAR ("NOTIFY"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown method (was: %d), aborting\n"),
                  method_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
HTTP_Tools::VersionToString (const HTTP_Version_t& version_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::VersionToString"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (version_in)
  {
    case HTTP_Codes::HTTP_VERSION_0_9:
      result = ACE_TEXT_ALWAYS_CHAR ("0.9"); break;
    case HTTP_Codes::HTTP_VERSION_1_0:
      result = ACE_TEXT_ALWAYS_CHAR ("1.0"); break;
    case HTTP_Codes::HTTP_VERSION_1_1:
      result = ACE_TEXT_ALWAYS_CHAR ("1.1"); break;
    case HTTP_Codes::HTTP_VERSION_2:
      result = ACE_TEXT_ALWAYS_CHAR ("2"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown version (was: %d), aborting\n"),
                  version_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
HTTP_Tools::StatusToReason (const HTTP_Status_t& status_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::StatusToReason"));

  // initialize result
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID/UNKNOWN");

  switch (status_in)
  {
    case HTTP_Codes::HTTP_STATUS_OK:
      result = ACE_TEXT_ALWAYS_CHAR ("OK"); break;
    case HTTP_Codes::HTTP_STATUS_CREATED:
      result = ACE_TEXT_ALWAYS_CHAR ("Created"); break;
    case HTTP_Codes::HTTP_STATUS_ACCEPTED:
      result = ACE_TEXT_ALWAYS_CHAR ("Accepted"); break;
    case HTTP_Codes::HTTP_STATUS_NOCONTENT:
      result = ACE_TEXT_ALWAYS_CHAR ("No Content"); break;
    case HTTP_Codes::HTTP_STATUS_MULTIPLECHOICES:
      result = ACE_TEXT_ALWAYS_CHAR ("Multiple Choices"); break;
    case HTTP_Codes::HTTP_STATUS_MOVEDPERMANENTLY:
      result = ACE_TEXT_ALWAYS_CHAR ("Moved Permanently"); break;
    case HTTP_Codes::HTTP_STATUS_MOVEDTEMPORARILY:
      result = ACE_TEXT_ALWAYS_CHAR ("Moved Temporarily"); break;
    case HTTP_Codes::HTTP_STATUS_NOTMODIFIED:
      result = ACE_TEXT_ALWAYS_CHAR ("Not Modified"); break;
    case HTTP_Codes::HTTP_STATUS_BADREQUEST:
      result = ACE_TEXT_ALWAYS_CHAR ("Bad Request"); break;
    case HTTP_Codes::HTTP_STATUS_UNAUTHORIZED:
      result = ACE_TEXT_ALWAYS_CHAR ("Unauthorized"); break;
    case HTTP_Codes::HTTP_STATUS_FORBIDDEN:
      result = ACE_TEXT_ALWAYS_CHAR ("Forbidden"); break;
    case HTTP_Codes::HTTP_STATUS_NOTFOUND:
      result = ACE_TEXT_ALWAYS_CHAR ("Not Found"); break;
    case HTTP_Codes::HTTP_STATUS_INTERNALSERVERERROR:
      result = ACE_TEXT_ALWAYS_CHAR ("Internal Server Error"); break;
    case HTTP_Codes::HTTP_STATUS_NOTIMPLEMENTED:
      result = ACE_TEXT_ALWAYS_CHAR ("Not Implemented"); break;
    case HTTP_Codes::HTTP_STATUS_BADGATEWAY:
      result = ACE_TEXT_ALWAYS_CHAR ("Bad Gateway"); break;
    case HTTP_Codes::HTTP_STATUS_SERVICEUNAVAILABLE:
      result = ACE_TEXT_ALWAYS_CHAR ("Service Unavailable"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown status (was: %d), aborting\n"),
                  status_in));
      break;
    }
  } // end SWITCH

  return result;
}

HTTP_Method_t
HTTP_Tools::MethodToType (const std::string& method_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::MethodToType"));

  if (method_in == ACE_TEXT_ALWAYS_CHAR ("GET"))
    return HTTP_Codes::HTTP_METHOD_GET;
  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("POST"))
    return HTTP_Codes::HTTP_METHOD_POST;
  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("HEAD"))
    return HTTP_Codes::HTTP_METHOD_HEAD;
  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("M-SEARCH"))
    return HTTP_Codes::HTTP_METHOD_M_SEARCH;
  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("NOTIFY"))
    return HTTP_Codes::HTTP_METHOD_NOTIFY;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown method (was: \"%s\"), aborting\n"),
                ACE_TEXT (method_in.c_str ())));
  } // end IF

  return HTTP_Codes::HTTP_METHOD_INVALID;
}

HTTP_Version_t
HTTP_Tools::VersionToType (const std::string& version_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::VersionToType"));

  if (version_in == ACE_TEXT_ALWAYS_CHAR ("0.9"))
    return HTTP_Codes::HTTP_VERSION_0_9;
  else if (version_in == ACE_TEXT_ALWAYS_CHAR ("1.0"))
    return HTTP_Codes::HTTP_VERSION_1_0;
  else if (version_in == ACE_TEXT_ALWAYS_CHAR ("1.1"))
    return HTTP_Codes::HTTP_VERSION_1_1;
  else if (version_in == ACE_TEXT_ALWAYS_CHAR ("2"))
      return HTTP_Codes::HTTP_VERSION_2;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown version (was: \"%s\"), aborting\n"),
                ACE_TEXT (version_in.c_str ())));
  } // end IF

  return HTTP_Codes::HTTP_VERSION_INVALID;
}

////////////////////////////////////////////////////////////////////////////////

bool
HTTP_Tools::isHeaderType (const std::string& fieldName_in,
                          HTTP_Codes::HeaderType headerType_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::isHeaderType"));

  switch (headerType_in)
  {
    case HTTP_Codes::HTTP_HEADER_GENERAL:
      return ((fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Date")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Pragma")));
    case HTTP_Codes::HTTP_HEADER_REQUEST:
      return ((fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Authorization")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("From")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("If-Modified-Since")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Referer")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("User-Agent")));
    case HTTP_Codes::HTTP_HEADER_RESPONSE:
      return ((fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Location")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Server")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("WWW-Authenticate")));
    case HTTP_Codes::HTTP_HEADER_ENTITY:
      return ((fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Allow")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Content-Encoding")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Content-Length")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Content-Type")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Expires")) ||
              (fieldName_in == ACE_TEXT_ALWAYS_CHAR ("Last-Modified")) ||
              (!HTTP_Tools::isHeaderType (fieldName_in,
                                          HTTP_Codes::HTTP_HEADER_GENERAL) &&
               !HTTP_Tools::isHeaderType (fieldName_in,
                                          HTTP_Codes::HTTP_HEADER_REQUEST) &&
               !HTTP_Tools::isHeaderType (fieldName_in,
                                          HTTP_Codes::HTTP_HEADER_RESPONSE)));
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown header type (was: %d), aborting\n"),
                  headerType_in));
      break;
    }
  } // end SWITCH

  return false;
}

bool
HTTP_Tools::isRequest (const struct HTTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::isRequest"));

  return (record_in.method < HTTP_Codes::HTTP_METHOD_MAX);
}

enum Stream_Decoder_CompressionFormatType
HTTP_Tools::EncodingToCompressionFormat (const std::string& encoding_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::EncodingToCompressionFormat"));

  std::locale locale;
  std::string lowercase_string;
  for (std::string::size_type i = 0; i < encoding_in.length (); ++i)
    lowercase_string += std::tolower (encoding_in[i], locale);
  //std::transform (encoding_in.begin (), encoding_in.end (),
  //                lowercase_string.begin (), ::tolower);

  if ((lowercase_string == ACE_TEXT_ALWAYS_CHAR ("deflate")) ||
      (lowercase_string == ACE_TEXT_ALWAYS_CHAR ("gzip")))
    return STREAM_COMPRESSION_FORMAT_GZIP;
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown encoding (was: \"%s\"), aborting\n"),
                ACE_TEXT (encoding_in.c_str ())));

  return STREAM_COMPRESSION_FORMAT_INVALID;
}

bool
HTTP_Tools::parseURL (const std::string& URL_in,
                      ACE_INET_Addr& address_out,
                      std::string& hostName_out,
                      std::string& URI_out,
                      bool& useSSL_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::parseURL"));

  // intialize return value(s)
  //address_out.reset ();
  hostName_out.clear ();
  URI_out.clear ();
  useSSL_out = false;

  unsigned short port = 0;
  std::istringstream converter;
  int result = -1;
  std::string hostname_no_port_string;

  // step1: split protocol/hostname/port/URI
  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(?:http|ftp)(s){0,1}(?:\\://)([^/]*)(.*)$");
  std::regex regex (regex_string,
                    std::regex::ECMAScript);
  std::smatch match_results;
  if (!std::regex_match (URL_in,
                         match_results,
                         regex,
                         std::regex_constants::match_default))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid URL string (was: \"%s\"), aborting\n"),
                ACE_TEXT (URL_in.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());

  //ACE_ASSERT (match_results[1].matched);
  std::string match_string = match_results[1];
  if (!match_string.empty ())
    useSSL_out = true;
  ACE_ASSERT (match_results[2].matched);
  match_string = match_results[2];
  if (!match_string.empty ())
  {
    regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([[:alnum:].-]+)(?:\\:([[:digit:]]{1,5})){0,1}$");
    regex.assign (regex_string,
                  std::regex::ECMAScript);
    std::smatch match_results_2;
    if (!std::regex_match (match_string,
                           match_results_2,
                           regex,
                           std::regex_constants::match_default))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("invalid hostname string (was: \"%s\"), aborting\n"),
                 ACE_TEXT (match_string.c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (match_results_2.ready () && !match_results_2.empty ());
    ACE_ASSERT (match_results_2[1].matched);
    hostName_out = match_results_2[1];
    hostname_no_port_string = hostName_out;
    if (match_results_2[2].matched)
    {
      match_string = match_results_2[2];
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.str (match_string);
      converter >> port;

      hostName_out += ':';
      hostName_out += match_string;
    } // end IF
  } // end IF
  ACE_ASSERT (match_results[3].matched);
  match_string = match_results[3];
  if (!match_string.empty ())
    URI_out = match_string;
  else
    URI_out = ACE_TEXT_ALWAYS_CHAR ("/");

  // step2: validate address/verify host name exists
  //        --> resolve
  // *TODO*: support IPv6 as well
  //regex_string =
  //  ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{1,3}\\.){4}$");
  //regex = regex_string;
  //std::smatch match_results_2;
  //if (std::regex_match (hostname,
  //                      match_results_2,
  //                      regex,
  //                      std::regex_constants::match_default))
  //{
  //  ACE_ASSERT (match_results_2.ready () &&
  //              !match_results_2.empty () &&
  //              match_results_2[1].matched);
  //  dotted_decimal_string = hostname;
  //} // end IF
//  result = address_out.set (port,
//                            hostname.c_str (),
//                            1,
//                            AF_INET);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_INET_Addr::set (): \"%m\", aborting\n")));
//    return false;
//  } // end IF

  // step3: validate URI
  regex_string = ACE_TEXT_ALWAYS_CHAR ("^\\/([^\\/]+\\/)*(.*?)$");
  regex.assign (regex_string,
                (std::regex_constants::ECMAScript |
                 std::regex_constants::icase));
  std::smatch match_results_3;
  if (!std::regex_match (URI_out,
                         match_results_3,
                         regex,
                         std::regex_constants::match_default))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid URI (was: \"%s\"), aborting\n"),
                ACE_TEXT (URI_out.c_str ())));
    return false;
  } // end IF
//  ACE_ASSERT (match_results_3.ready () && !match_results_3.empty ());
  ACE_ASSERT (!match_results_3.empty ());

  if (!hostname_no_port_string.empty ())
    result =
      address_out.set ((port ? port : (useSSL_out ? HTTPS_DEFAULT_SERVER_PORT : HTTP_DEFAULT_SERVER_PORT)),
                       hostname_no_port_string.c_str (),
                       1, // encode port number
                       AF_INET);
  else
    result =
      address_out.set ((port ? static_cast<u_short> (port) 
                             : (useSSL_out ? static_cast<u_short> (HTTPS_DEFAULT_SERVER_PORT) 
                                           : static_cast<u_short> (HTTP_DEFAULT_SERVER_PORT))),
                       static_cast<ACE_UINT32> (INADDR_ANY),
                       1,  // encode port number
                       0); // do not map
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

bool
HTTP_Tools::URLIsURI (const std::string& URL_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::URLIsURI"));

  // sanity check(s)
  ACE_ASSERT (!URL_in.empty ());

  std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^[^/]+(?:/(?:[^/])+)*(?:/)?$");
  std::regex regex (regex_string);
  std::smatch match_results;
  return std::regex_match (URL_in,
                           match_results,
                           regex,
                           std::regex_constants::match_default);
}

bool
HTTP_Tools::URLRequiresSSL (const std::string& URL_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::URLRequiresSSL"));

  std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:http(s)?://)?(.+)?$");
  std::regex regex (regex_string);
  std::smatch match_results;
  if (!std::regex_match (URL_in,
                         match_results,
                         regex,
                         std::regex_constants::match_default))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid URL string (was: \"%s\"), aborting\n"),
                ACE_TEXT (URL_in.c_str ())));
    return false;
  } // end IF
//  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  ACE_ASSERT (!match_results.empty ());

  return match_results[1].matched;
}

std::string
HTTP_Tools::URLEncode (const std::string& string_in,
                       bool escapeAll_in,
                       bool upperCaseHexDigits_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::URLEncode"));

  std::string result;

  // *NOTE*: use the US-ASCII locale for isdigit,isalpha
  std::locale c_locale = std::locale::classic ();
  std::ostringstream converter;
  converter << std::hex << std::setfill ('0');
  std::string converted_string;
  for (std::string::const_iterator iterator = string_in.begin ();
       iterator != string_in.end ();
       ++iterator)
  {
    if (escapeAll_in                                ||
        // 'unreserved' characters (see also: RFC 3986)
        (!std::isdigit (*iterator, c_locale) &&
         !std::isalpha (*iterator, c_locale) &&
         !((*iterator == '.') || (*iterator == '-') || (*iterator == '_') ||
           (*iterator == '~'))))//                     ||
        //// 'unsafe' characters (see also: RFC 1738)
        //((*iterator == '<') || (*iterator == '>') || (*iterator == '"') ||
        // (*iterator == '#') || (*iterator == '%') || (*iterator == '{') ||
        // (*iterator == '}') || (*iterator == '|') || (*iterator == '\\') ||
        // (*iterator == '^') || (*iterator == '~') || (*iterator == '[') ||
        // (*iterator == ']') || (*iterator == '`'))  ||
        //// 'reserved' characters (see also: RFC 1738)
        //((*iterator == ';') || (*iterator == '/') || (*iterator == '?') ||
        // (*iterator == ':') || (*iterator == '@') || (*iterator == '=') ||
        // (*iterator == '&')))
    {
      result += '%';
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      // *TODO*: there is probably a better way to do this...
      converter << std::setw (2) << static_cast<ACE_UINT16> (static_cast<unsigned char> (*iterator));
      converted_string = converter.str ();
      // "...For consistency, URI producers and normalizers should use
      //  uppercase hexadecimal digits for all percent-encodings. ..."
      if (upperCaseHexDigits_in)
        std::transform (converted_string.begin (), converted_string.end (),
                        converted_string.begin (),
                        [](unsigned char c) { return std::toupper (c); });
      result +=
        ((converted_string.size () == 2) ? converted_string
                                         : converted_string.substr (2, std::string::npos));
    } // end IF
    else
      result += *iterator;
  } // end FOR

  return result;
}

std::string
HTTP_Tools::stripURI (const std::string& URI_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::stripURI"));

  std::string result;

  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(\\/(?:[^\\/]+\\/)*(?:[^?]*))(?:\\?(.*))?$");
  std::regex regex (regex_string,
                    std::regex_constants::ECMAScript);
  std::smatch match_results;
  if (unlikely (!std::regex_match (URI_in,
                                   match_results,
                                   regex,
                                   std::regex_constants::match_default)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid URI string (was: \"%s\"), aborting\n"),
                ACE_TEXT (URI_in.c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (match_results.ready () && !match_results.empty ());
  ACE_ASSERT (match_results[1].matched);
  result = match_results[1].str ();

  return result;
}

std::string
HTTP_Tools::IPAddressToHostName (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::IPAddressToHostName"));

  std::string result;

  int result_2 = -1;
  ACE_TCHAR buffer_a[BUFSIZ];
  ACE_OS::memset (&buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
  result_2 = address_in.addr_to_string (buffer_a,
                                        sizeof (ACE_TCHAR[BUFSIZ]),
                                        0); // want hostname !
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Inet_Addr::addr_to_string(): \"%m\", aborting\n")));
    return result;
  } // end IF
  result = ACE_TEXT_ALWAYS_CHAR (buffer_a);

  return result;
}
