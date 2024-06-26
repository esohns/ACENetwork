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

#include "ppspp_tools.h"

#include <locale>
#include <regex>
#include <sstream>

#include <ace/Log_Msg.h>

#include "net_macros.h"

#include "ppspp_defines.h"

std::string
HTTP_Tools::dump (const HTTP_Record& record_in)
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
    buffer += HTTP_Tools::Method2String (record_in.method);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Request-URI: ");
    buffer += record_in.URI;
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Version: ");
    buffer += HTTP_Tools::Version2String (record_in.version);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
  } // end IF
  else
  {
    buffer = ACE_TEXT_ALWAYS_CHAR ("Version: ");
    buffer += HTTP_Tools::Version2String (record_in.version);
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Status: ");
    converter << record_in.status;
    buffer += converter.str ();
    buffer += ACE_TEXT_ALWAYS_CHAR ("\n");
    buffer += ACE_TEXT_ALWAYS_CHAR ("Reason: \"");
    buffer += HTTP_Tools::Status2Reason (record_in.status);
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
HTTP_Tools::Method2String (const HTTP_Method_t& method_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::Method2String"));

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
HTTP_Tools::Version2String (const HTTP_Version_t& version_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::Version2String"));

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
HTTP_Tools::Status2Reason (const HTTP_Status_t& status_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::Status2Reason"));

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
HTTP_Tools::Method2Type (const std::string& method_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::Method2Type"));

  if (method_in == ACE_TEXT_ALWAYS_CHAR ("GET"))
    return HTTP_Codes::HTTP_METHOD_GET;
  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("POST"))
    return HTTP_Codes::HTTP_METHOD_POST;
  else if (method_in == ACE_TEXT_ALWAYS_CHAR ("HEAD"))
    return HTTP_Codes::HTTP_METHOD_HEAD;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown method (was: \"%s\"), aborting\n"),
                ACE_TEXT (method_in.c_str ())));
  } // end IF

  return HTTP_Codes::HTTP_METHOD_INVALID;
}

HTTP_Version_t
HTTP_Tools::Version2Type (const std::string& version_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::Version2Type"));

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
HTTP_Tools::isRequest (const HTTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::isRequest"));

  return (record_in.method < HTTP_Codes::HTTP_METHOD_MAX);
}

enum Stream_Decoder_CompressionFormatType
HTTP_Tools::Encoding2CompressionFormat (const std::string& encoding_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::Encoding2CompressionFormat"));

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
                      std::string& URI_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Tools::parseURL"));

  bool use_SSL = false;
  std::string hostname;
  unsigned short port = HTTP_DEFAULT_SERVER_PORT;
  std::istringstream converter;
  //std::string dotted_decimal_string;
  int result = -1;

  // step1: split protocol/hostname/port
  std::string regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(?:http(s)?://)?([[:alnum:]-.]+)(?:\\:([[:digit:]]{1,5}))?(.+)?$");
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
  ACE_ASSERT (match_results.ready () && !match_results.empty ());

  if (match_results[1].matched)
    use_SSL = true;
  ACE_UNUSED_ARG (use_SSL);
  ACE_ASSERT (match_results[2].matched);
  hostname = match_results[2];
  if (match_results[3].matched)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.str (match_results[3].str ());
    converter >> port;
  } // end IF
  ACE_ASSERT (match_results[4].matched);
  URI_out = match_results[4];

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
  result = address_out.set (port,
                            hostname.c_str (),
                            1,
                            ACE_ADDRESS_FAMILY_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set (): \"%m\", aborting\n")));
    return false;
  } // end IF

  // step3: validate URI
  regex_string =
    ACE_TEXT_ALWAYS_CHAR ("^(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
  //regex_string =
  //    ACE_TEXT_ALWAYS_CHAR ("^(?:http(?:s)?://)?((.+\\.)+([^\\/]+))(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
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
  ACE_ASSERT (match_results_3.ready () && !match_results_3.empty ());

  return true;
}
