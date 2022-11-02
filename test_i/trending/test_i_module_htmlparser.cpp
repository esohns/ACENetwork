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

#include "test_i_module_htmlparser.h"

#include <regex>
#include <sstream>
#include <string>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"

#include "stream_macros.h"
#include "stream_tools.h"

void
test_i_libxml2_sax_error_cb (void* userData_in,
                             const char* message_in,
                             ...)
{
  //STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_error_cb"));

  int result = -1;

  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_TCHAR buffer[BUFSIZ];
  va_list arguments;

  va_start (arguments, message_in);
  result = ACE_OS::vsnprintf (buffer,
                              sizeof (buffer),
//                            sizeof (buffer) / sizeof (buffer[0]),
                              message_in, arguments);
  ACE_UNUSED_ARG (result);
  va_end (arguments);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("test_i_libxml2_sax_error_cb[%s:%d:%d] (%d,%d): %s"),
              data_p->parserContext->lastError.file, data_p->parserContext->lastError.line, data_p->parserContext->lastError.int2,
              data_p->parserContext->lastError.domain, data_p->parserContext->lastError.code,
              buffer));
}

void
test_i_libxml2_sax_structured_error_cb (void* userData_in,
                                        xmlErrorPtr error_in)
{
  //STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_structured_error_cb"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("test_i_libxml2_sax_structured_error_cb: %s\n"),
              ACE_TEXT (error_in->message)));
}

//////////////////////////////////////////

Test_I_Stream_HTMLParser::Test_I_Stream_HTMLParser (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , iterator_ ()
 , iterator_2_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::Test_I_Stream_HTMLParser"));

}

void
Test_I_Stream_HTMLParser::handleDataMessage (Test_I_Stream_Message*& message_inout,
                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::sessionData_);

  //std::string filename = Common_File_Tools::getTempDirectory ();
  //filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //filename += ACE_TEXT_ALWAYS_CHAR ("output.html");
  //Stream_Tools::dump (message_inout,
  //                    filename);

  // insert target record
  struct Test_I_Trending_SessionData& session_data_r =
    const_cast<Test_I_Trending_SessionData&> (inherited::sessionData_->getR ());

  // sanity check(s)
  const Test_I_Stream_MessageData& message_data_container_r =
    message_inout->getR ();
  Test_I_Trending_MessageData& message_data_r =
    const_cast<Test_I_Trending_MessageData&> (message_data_container_r.getR ());
  Test_I_StockItemsIterator_t iterator =
    inherited::configuration_->stockItems.find (message_data_r.stockItem);
  ACE_ASSERT (iterator != inherited::configuration_->stockItems.end ());
  ACE_ASSERT (!inherited::parserContext_.record);

  struct Test_I_StockRecord stock_record;
  stock_record.item = &const_cast<Test_I_StockItem&> (*iterator);
  session_data_r.data.push_back (stock_record);

  iterator_2_ = session_data_r.data.begin ();
  for (;
       iterator_2_ != session_data_r.data.end ();
       ++iterator_2_)
    if ((*iterator_2_).item->ISIN == message_data_r.stockItem.ISIN)
      break;
  ACE_ASSERT (iterator_2_ != session_data_r.data.end ());
  inherited::parserContext_.record =
    &const_cast<struct Test_I_StockRecord&> (*iterator_2_);

  inherited::handleDataMessage (message_inout,
                                passMessageDownstream_out);
}

void
Test_I_Stream_HTMLParser::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!inherited::parserContext_.sessionData);

      // *TODO*: remove type inference
      inherited::parserContext_.sessionData =
        &const_cast<Test_I_Trending_SessionData&> (inherited::sessionData_->getR ());

      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (parserContext_.sessionData);

      // *TODO*: remove type inference
      inherited::parserContext_.sessionData =
        &const_cast<Test_I_Trending_SessionData&> (inherited::sessionData_->getR ());

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
      inherited::handleSessionMessage (message_inout,
                                       passMessageDownstream_out);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: parsed HTML document (symbol: \"%s\")\n"),
                  inherited::mod_->name (),
                  ACE_TEXT ((*iterator_2_).item->symbol.c_str ())));
      inherited::parserContext_.record = NULL;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      inherited::parserContext_.sessionData = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}

bool
Test_I_Stream_HTMLParser::initialize (const Test_I_Trending_ModuleHandlerConfiguration& configuration_in,
                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.mode == STREAM_MODULE_HTMLPARSER_MODE_SAX);

//  initGenericErrorDefaultFunc ((xmlGenericErrorFunc*)&::errorCallback);
//  xmlSetGenericErrorFunc (inherited::parserContext_, &::errorCallback);
//  xmlSetStructuredErrorFunc (inherited::parserContext_, &::structuredErrorCallback);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

bool
Test_I_Stream_HTMLParser::initializeSAXParser ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::initializeSAXParser"));

  // sanity check(s)
  ACE_ASSERT (inherited::SAXHandler_.initialized);

  // set necessary SAX parser callbacks
  // *IMPORTANT NOTE*: the default SAX callbacks expect xmlParserCtxtPtr as user
  //                   data; this implementation uses Test_I_SAXParserContext*
  //                   --> all default callbacks will crash on invocation, so
  //                       disable them here
  inherited::SAXHandler_.cdataBlock = NULL;
  inherited::SAXHandler_.comment = NULL;
  inherited::SAXHandler_.getEntity = NULL;
  inherited::SAXHandler_.ignorableWhitespace = NULL;
  inherited::SAXHandler_.internalSubset = NULL;
  inherited::SAXHandler_.processingInstruction = NULL;
  inherited::SAXHandler_.setDocumentLocator = NULL;
  inherited::SAXHandler_.startDocument = NULL;
  inherited::SAXHandler_.endDocument = NULL;

//  inherited::SAXHandler_.getEntity = getEntity;
//  inherited::SAXHandler_.startDocument = startDocument;
//  inherited::SAXHandler_.endDocument = endDocument;
  inherited::SAXHandler_.startElement = test_i_libxml2_sax_start_element_cb;
  inherited::SAXHandler_.endElement = test_i_libxml2_sax_end_element_cb;
  inherited::SAXHandler_.characters = test_i_libxml2_sax_characters_cb;
  ////////////////////////////////////////
  inherited::SAXHandler_.warning = test_i_libxml2_sax_error_cb;
  inherited::SAXHandler_.error = test_i_libxml2_sax_error_cb;
  inherited::SAXHandler_.fatalError = test_i_libxml2_sax_error_cb;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

//void
//test_i_libxml2_sax_start_document_cb (void* userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_start_document_cb"));
//
//  ACE_UNUSED_ARG (userData_in);
//}

//void
//test_i_libxml2_sax_end_document_cb (void* userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_end_document_cb"));
//
//  ACE_UNUSED_ARG (userData_in);
//}

void
test_i_libxml2_sax_characters_cb (void* userData_in,
                                  const xmlChar* string_in,
                                  int length_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_characters_cb"));

  ACE_UNUSED_ARG (length_in);

  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  //if (!data_p->accumulate) return; // --> nothing to do

  data_p->characters = reinterpret_cast<const char*> (string_in);
}

void
test_i_libxml2_sax_start_element_cb (void* userData_in,
                                     const xmlChar* name_in,
                                     const xmlChar** attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_start_element_cb"));

  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  const xmlChar** attributes_p = attributes_in;

  // ------------------------------- html --------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_INVALID);

    data_p->state = SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF

  // ------------------------------- head/body ---------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_IN_HEAD;

    return;
  } // end IF
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_IN_BODY;

    return;
  } // end IF

  switch (data_p->state)
  {
    case SAXPARSER_STATE_IN_HTML:
      goto html;
    case SAXPARSER_STATE_IN_HEAD:
      goto head;
    case SAXPARSER_STATE_IN_BODY:
    //////////////////////////////////////
    case SAXPARSER_STATE_IN_BODY_DIV_CONTENT:
    case SAXPARSER_STATE_IN_TBODY_CONTENT:
    case SAXPARSER_STATE_IN_TR_CONTENT:
    //////////////////////////////////////
    case SAXPARSER_STATE_READ_SYMBOL:
    case SAXPARSER_STATE_READ_WKN:
    case SAXPARSER_STATE_READ_ISIN:
    case SAXPARSER_STATE_READ_CHANGE:
    case SAXPARSER_STATE_READ_CHANGE_2:
    case SAXPARSER_STATE_READ_DATE:
    case SAXPARSER_STATE_READ_VALUE:
      goto body;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown parser state (was: %d), returning\n"),
                  data_p->state));
      return;
    }
  } // end SWITCH

html:
  // -------------------------------- html -------------------------------------
  if (!(data_p->state == SAXPARSER_STATE_IN_HTML))
    return;

  return;

head:
  // -------------------------------- head -------------------------------------
  if (!(data_p->state == SAXPARSER_STATE_IN_HEAD))
    return;

  //if (xmlStrEqual (name_in,
  //                 BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("title"))))
  //{
  //  // sanity check(s)
  //  data_p->state = SAXPARSER_STATE_READ_SYMBOL_WKN_ISIN;

  //  return;
  //} // end IF

  return;

body:
  // -------------------------------- body -------------------------------------
  //if (!(data_p->state == SAXPARSER_STATE_IN_BODY))
  //  return;

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("div"))))
  {
    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("id"))))
      {
        ACE_ASSERT (attributes_p[1]);

        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("desktopSearchResult"))))
        {
          data_p->state = SAXPARSER_STATE_IN_BODY_DIV_CONTENT;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("tbody"))))
  {
    if (data_p->state == SAXPARSER_STATE_IN_BODY_DIV_CONTENT)
      data_p->state = SAXPARSER_STATE_IN_TBODY_CONTENT;
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("tr"))))
  {
    if (data_p->state == SAXPARSER_STATE_IN_TBODY_CONTENT)
      data_p->state = SAXPARSER_STATE_IN_TR_CONTENT;
  } // end ELSE IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("td"))))
  {
    if (data_p->state == SAXPARSER_STATE_IN_TR_CONTENT)
      data_p->state = SAXPARSER_STATE_READ_SYMBOL;
    else if (data_p->state >= SAXPARSER_STATE_READ_SYMBOL)
      data_p->state = (enum Test_I_SAXParserState)(((int)data_p->state) + 1);
  } // end ELSE IF
}

void
test_i_libxml2_sax_end_element_cb (void* userData_in,
                                   const xmlChar* name_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_end_element_cb"));

  struct Test_I_SAXParserContext* data_p =
    static_cast<struct Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->record);

  bool done = true;
  std::string regex_string;
  std::regex::flag_type flags = std::regex_constants::ECMAScript;
  std::regex regex;
  std::smatch match_results;
  std::istringstream converter;

  switch (data_p->state)
  {
    case SAXPARSER_STATE_READ_SYMBOL:
    { ACE_ASSERT (data_p->record->item);
      regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)([[:print:]]+)(?:.*)$");
      //try {
      regex.assign (regex_string, flags);
      //} catch (std::regex_error exception_in) {
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
      //              ACE_TEXT (exception_in.what ())));
      //  return;
      //}
      if (!std::regex_match (data_p->characters,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid symbol string (was: \"%s\"), returning\n"),
                    ACE_TEXT (data_p->characters.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());
      ACE_ASSERT (match_results[1].matched);

      data_p->record->item->symbol = match_results[1].str ();
      // strip trailing whitespace
      std::string::size_type position =
        data_p->record->item->symbol.find_last_of (' ',
                                                   std::string::npos);
      if (position != std::string::npos)
        data_p->record->item->symbol.erase (position,
                                            std::string::npos);
      break;
    }
    case SAXPARSER_STATE_READ_WKN:
    { ACE_ASSERT (data_p->record->item);
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)([[:print:]]+)(?:.*)$");
      //try
      //{
      regex.assign (regex_string, flags);
      //}
      //catch (std::regex_error exception_in)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
      //              ACE_TEXT (exception_in.what ())));
      //  return;
      //}
      if (!std::regex_match (data_p->characters,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid wkn string (was: \"%s\"), returning\n"),
                    ACE_TEXT (data_p->characters.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());
      ACE_ASSERT (match_results[1].matched);
      data_p->record->item->WKN  = match_results[1].str (); 

      // match WKN
      regex_string = ACE_TEXT_ALWAYS_CHAR ("^([[:alnum:]]{6})$");
      regex.assign (regex_string, flags);
      if (std::regex_match (data_p->record->item->WKN,
                            match_results,
                            regex,
                            std::regex_constants::match_default))
      {
//        ACE_ASSERT (match_results.ready () && !match_results.empty ());
        ACE_ASSERT (!match_results.empty ());
        ACE_ASSERT (match_results[1].matched);
      } // end IF
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid wkn string (was: \"%s\"), continuing\n"),
                    ACE_TEXT (data_p->record->item->WKN.c_str ())));
      break;
    }
    case SAXPARSER_STATE_READ_ISIN:
    { ACE_ASSERT (data_p->record->item);
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)([[:print:]]+)(?:.*)$");
      //try
      //{
      regex.assign (regex_string, flags);
      //}
      //catch (std::regex_error exception_in)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
      //              ACE_TEXT (exception_in.what ())));
      //  return;
      //}
      if (!std::regex_match (data_p->characters,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid isin string (was: \"%s\"), returning\n"),
                    ACE_TEXT (data_p->characters.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());
      ACE_ASSERT (match_results[1].matched);
      data_p->record->item->ISIN = match_results[1].str ();

      // match ISIN
      regex_string = ACE_TEXT_ALWAYS_CHAR ("^([[:alpha:]]{2}[[:digit:]]{10})$");
      regex.assign (regex_string, flags);
      if (std::regex_match (data_p->record->item->ISIN,
                            match_results,
                            regex,
                            std::regex_constants::match_default))
      {
//        ACE_ASSERT (match_results.ready () && !match_results.empty ());
        ACE_ASSERT (!match_results.empty ());
        ACE_ASSERT (match_results[1].matched);
      } // end IF
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid isin string (was: \"%s\"), continuing\n"),
                    ACE_TEXT (data_p->record->item->ISIN.c_str ())));
      break;
    }
    case SAXPARSER_STATE_READ_CHANGE:
    {
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([+\\-]{1})([[:digit:]]+),([[:digit:]]+)(.*)$");
      //try
      //{
      regex.assign (regex_string, flags);
      //}
      //catch (std::regex_error exception_in)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
      //              ACE_TEXT (exception_in.what ())));
      //  return;
      //}
      if (!std::regex_match (data_p->characters,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid change string (was: \"%s\"), returning\n"),
                    ACE_TEXT (data_p->characters.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());
      ACE_ASSERT (match_results[1].matched);
      ACE_ASSERT (match_results[2].matched);
      ACE_ASSERT (match_results[3].matched);

      std::string value_string_2 = match_results[2].str ();
      value_string_2 += '.';
      value_string_2 += match_results[3].str ();
      converter.str (value_string_2);
      converter >> data_p->record->change;
      value_string_2 = match_results[1].str ();
      if (value_string_2[0] == '-')
        data_p->record->change = -data_p->record->change;
      break;
    }
    case SAXPARSER_STATE_READ_CHANGE_2:
    {
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([+\\-]{1})([[:digit:]]+),([[:digit:]]+)(.*)$");
      //try
      //{
      regex.assign (regex_string, flags);
      //}
      //catch (std::regex_error exception_in)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
      //              ACE_TEXT (exception_in.what ())));
      //  return;
      //}
      if (!std::regex_match (data_p->characters,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid change_2 string (was: \"%s\"), returning\n"),
                    ACE_TEXT (data_p->characters.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());
      ACE_ASSERT (match_results[1].matched);
      ACE_ASSERT (match_results[2].matched);
      ACE_ASSERT (match_results[3].matched);

      std::string value_string_2 = match_results[2].str ();
      value_string_2 += '.';
      value_string_2 += match_results[3].str ();
      converter.str (value_string_2);
      converter >> data_p->record->change;
      value_string_2 = match_results[1].str ();
      if (value_string_2[0] == '-')
        data_p->record->change = -data_p->record->change;
      break;
    }
    case SAXPARSER_STATE_READ_DATE:
    {
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{2}):([[:digit:]]{2})$");
      regex.assign (regex_string, flags);
      if (!std::regex_match (data_p->characters,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid date string (was: \"%s\"), returning\n"),
                    ACE_TEXT (data_p->characters.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());
      ACE_ASSERT (match_results[1].matched);
      ACE_ASSERT (match_results[2].matched);

      long value;
      struct tm tm_time;
      ACE_OS::memset (&tm_time, 0, sizeof (struct tm));

      time_t today = COMMON_TIME_NOW.sec ();
      ACE_OS::gmtime_r (&today, &tm_time);
      converter.clear ();
      converter.str (match_results[1].str ());
      converter >> value;
      tm_time.tm_hour = value;
      converter.clear ();
      converter.str (match_results[2].str ());
      converter >> value;
      tm_time.tm_min = value;

      time_t time_seconds = ACE_OS::mktime (&tm_time);
      data_p->record->timeStamp.set (time_seconds, 0);
      break;
    }
    case SAXPARSER_STATE_READ_VALUE:
    {
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^(?:[^[:digit:]]*)([[:digit:]]+\\.)?([[:digit:]]+)(,[[:digit:]]+)(?:.*)$");
      regex.assign (regex_string, flags);
      if (!std::regex_match (data_p->characters,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid value string (was: \"%s\"), returning\n"),
                    ACE_TEXT (data_p->characters.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());
      //ACE_ASSERT (match_results[1].matched);
      ACE_ASSERT (match_results[2].matched);
      ACE_ASSERT (match_results[3].matched);

      // *TODO*: this isn't quite right yet, i.e. make sure to set a locale that
      //         allows thousands separators
      std::locale locale (ACE_TEXT_ALWAYS_CHAR (""));
      std::string value_string_2;
      if (match_results[1].matched)
        value_string_2 = match_results[1].str ();
      value_string_2 += match_results[2].str ();
      value_string_2 += match_results[3].str ();

      std::istringstream converter (value_string_2);
      converter.imbue (locale);
      converter >> data_p->record->value;

      data_p->state = SAXPARSER_STATE_IN_HTML;

      break;
    }
    default:
      done = false;
      break;
  } // end SWITCH

  // clean up
  //data_p->accumulate = false;
  data_p->characters.clear ();

  if (done) return;

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  { ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);
    data_p->state = SAXPARSER_STATE_INVALID;
    return;
  } // end IF

  // ------------------------------- head --------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  { ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HEAD);
    data_p->state = SAXPARSER_STATE_IN_HTML;
    return;
  } // end IF

  // -------------------------------- body -------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    data_p->state = SAXPARSER_STATE_IN_HTML;
    return;
  } // end IF
}
