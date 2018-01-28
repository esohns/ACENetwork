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

#include "ace/Synch.h"
#include "test_i_module_htmlparser.h"

#include <regex>
#include <sstream>
#include <string>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"

#include "stream_tools.h"

#include "net_macros.h"

void
errorCallback (void* userData_in,
               const char* message_in,
               ...)
{
  NETWORK_TRACE (ACE_TEXT ("::errorCallback"));

  int result = -1;

  struct Test_I_URLStreamLoad_SAXParserContext* data_p =
      static_cast<struct Test_I_URLStreamLoad_SAXParserContext*> (userData_in);

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
              ACE_TEXT ("errorCallback[%s:%d:%d] (%d,%d): %s"),
              data_p->parserContext->lastError.file, data_p->parserContext->lastError.line, data_p->parserContext->lastError.int2,
              data_p->parserContext->lastError.domain, data_p->parserContext->lastError.code,
              buffer));
}

void
structuredErrorCallback (void* userData_in,
                         xmlErrorPtr error_in)
{
  NETWORK_TRACE (ACE_TEXT ("::structuredErrorCallback"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("structuredErrorCallback: %s\n"),
              ACE_TEXT (error_in->message)));
}

Test_I_Module_HTMLParser::Test_I_Module_HTMLParser (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTMLParser::Test_I_Module_HTMLParser"));

}

Test_I_Module_HTMLParser::~Test_I_Module_HTMLParser ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTMLParser::~Test_I_Module_HTMLParser"));

}

void
Test_I_Module_HTMLParser::handleDataMessage (Test_I_Message*& message_inout,
                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTMLParser::handleDataMessage"));

  // sanity check(s)
  //ACE_ASSERT (inherited::configuration_);
  //ACE_ASSERT (inherited::sessionData_);

  //// insert target record
  //struct Test_I_URLStreamLoad_SessionData& session_data_r =
  //  const_cast<struct Test_I_URLStreamLoad_SessionData&> (inherited::sessionData_->get ());

  //// sanity check(s)
  //const typename Test_I_Message::DATA_T& message_data_container_r =
  //  message_inout->get ();
  //typename Test_I_Message::DATA_T::DATA_T& message_data_r =
  //  const_cast<typename Test_I_Message::DATA_T::DATA_T&> (message_data_container_r.get ());
  //Test_I_StockItemsIterator_t iterator =
  //  inherited::configuration_->stockItems.find (message_data_r.stockItem);
  //ACE_ASSERT (iterator != inherited::configuration_->stockItems.end ());
  //ACE_ASSERT (!inherited::parserContext_.record);

  //Test_I_StockRecord stock_record;
  //stock_record.item = &const_cast<Test_I_StockItem&> (*iterator);
  //session_data_r.data.push_back (stock_record);

  //Test_I_StockRecordsIterator_t iterator_2 = session_data_r.data.begin ();
  //for (;
  //     iterator_2 != session_data_r.data.end ();
  //     ++iterator_2)
  //  if ((*iterator_2).item->ISIN == message_data_r.stockItem.ISIN)
  //    break;
  //ACE_ASSERT (iterator_2 != session_data_r.data.end ());
  //inherited::parserContext_.record =
  //  &const_cast<Test_I_StockRecord&> (*iterator_2);

  inherited::handleDataMessage (message_inout,
                                passMessageDownstream_out);

  if (inherited::complete_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: parsed HTML document (stream URL: \"%s\")\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::parserContext_.URL.c_str ())));

    inherited::complete_ = false;
    inherited::parserContext_.URL.clear ();
  } // end IF
}

//void
//Test_I_Module_HTMLParser::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
//                                                bool& passMessageDownstream_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTMLParser::handleSessionMessage"));
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  switch (message_inout->type ())
//  {
//    case STREAM_SESSION_MESSAGE_BEGIN:
//    {
//      // sanity check(s)
//      ACE_ASSERT (inherited::sessionData_);
//      ACE_ASSERT (!inherited::parserContext_.sessionData);
//
////      if (parserContext_)
////        htmlCtxtReset (parserContext_);
//
//      // *TODO*: remove type inference
//      inherited::parserContext_.sessionData =
//        &const_cast<Test_I_HTTPGet_SessionData&> (inherited::sessionData_->get ());
//
//      break;
//    }
//    case STREAM_SESSION_MESSAGE_LINK:
//    {
//      // sanity check(s)
//      ACE_ASSERT (inherited::parserContext_.sessionData);
//
//      // *TODO*: remove type inference
//      inherited::parserContext_.sessionData =
//        &const_cast<Test_I_HTTPGet_SessionData&> (inherited::sessionData_->get ());
//
//      break;
//    }
//    case STREAM_SESSION_MESSAGE_STEP:
//    {
//      if (inherited::parserContext_.parserContext)
//        htmlCtxtReset (inherited::parserContext_.parserContext);
//
//      //++iterator_;
//      //ACE_ASSERT (iterator_ != session_data_r.data.end ());
//      //inherited::parserContext_.data =
//      //  &const_cast<Test_I_StockRecord&> (*iterator_);
//
//      break;
//    }
//    case STREAM_SESSION_MESSAGE_END:
//    {
//      inherited::parserContext_.sessionData = NULL;
//
//      break;
//    }
//    default:
//      break;
//  } // end SWITCH
//}

bool
Test_I_Module_HTMLParser::initialize (const struct Test_I_URLStreamLoad_ModuleHandlerConfiguration& configuration_in,
                                      Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTMLParser::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.mode == STREAM_MODULE_HTMLPARSER_MODE_SAX);

//  initGenericErrorDefaultFunc ((xmlGenericErrorFunc*)&::errorCallback);
//  xmlSetGenericErrorFunc (inherited::parserContext_, &::errorCallback);
//  xmlSetStructuredErrorFunc (inherited::parserContext_, &::structuredErrorCallback);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

bool
Test_I_Module_HTMLParser::initializeSAXParser ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Module_HTMLParser::initializeSAXParser"));

  // sanity check(s)
  ACE_ASSERT (inherited::SAXHandler_.initialized);

  // set necessary SAX parser callbacks
  // *IMPORTANT NOTE*: the default SAX callbacks expect xmlParserCtxtPtr as user
  //                   data; this implementation uses Test_I_URLStreamLoad_SAXParserContext*
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
  inherited::SAXHandler_.startElement = startElement;
  inherited::SAXHandler_.endElement = endElement;
  inherited::SAXHandler_.characters = characters;
  ////////////////////////////////////////
  inherited::SAXHandler_.warning = errorCallback;
  inherited::SAXHandler_.error = errorCallback;
  inherited::SAXHandler_.fatalError = errorCallback;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

//void
//startDocument (void* userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::startDocument"));
//
//  ACE_UNUSED_ARG (userData_in);
//}
//void
//endDocument (void* userData_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::endDocument"));
//
//  ACE_UNUSED_ARG (userData_in);
//}
void
characters (void* userData_in,
            const xmlChar* string_in,
            int length_in)
{
  NETWORK_TRACE (ACE_TEXT ("::characters"));

  ACE_UNUSED_ARG (length_in);

  struct Test_I_URLStreamLoad_SAXParserContext* data_p =
      static_cast<struct Test_I_URLStreamLoad_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  if (!data_p->accumulate)
    return; // --> nothing to do

  data_p->characters += reinterpret_cast<const char*> (string_in);
}
void
startElement (void* userData_in,
              const xmlChar* name_in,
              const xmlChar** attributes_in)
{
  NETWORK_TRACE (ACE_TEXT ("::startElement"));

  struct Test_I_URLStreamLoad_SAXParserContext* data_p =
      static_cast<struct Test_I_URLStreamLoad_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  const xmlChar** attributes_p = attributes_in;

  // ------------------------------- html --------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == TEST_I_SAXPARSER_STATE_INVALID);

    data_p->state = TEST_I_SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF

  // ------------------------------- head/body ---------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == TEST_I_SAXPARSER_STATE_IN_HTML);

    data_p->state = TEST_I_SAXPARSER_STATE_IN_HEAD;

    return;
  } // end IF
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == TEST_I_SAXPARSER_STATE_IN_HTML);

    data_p->state = TEST_I_SAXPARSER_STATE_IN_BODY;

    return;
  } // end IF

  switch (data_p->state)
  {
    case TEST_I_SAXPARSER_STATE_IN_HTML:
      goto html;
    case TEST_I_SAXPARSER_STATE_IN_HEAD:
      goto head;
    case TEST_I_SAXPARSER_STATE_IN_BODY:
    //////////////////////////////////////
    //case TEST_I_SAXPARSER_STATE_IN_BODY_DIV_CONTENT:
    //case TEST_I_SAXPARSER_STATE_IN_SYMBOL_H1_CONTENT:
    ////////////////////////////////////////
    //case TEST_I_SAXPARSER_STATE_READ_CHANGE:
    //case TEST_I_SAXPARSER_STATE_READ_DATE:
    //case TEST_I_SAXPARSER_STATE_READ_ISIN_WKN:
    //case TEST_I_SAXPARSER_STATE_READ_SYMBOL:
    //case TEST_I_SAXPARSER_STATE_READ_VALUE:
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
  if (!(data_p->state == TEST_I_SAXPARSER_STATE_IN_HTML))
    return;

  return;

head:
  // -------------------------------- head -------------------------------------
  if (!(data_p->state == TEST_I_SAXPARSER_STATE_IN_HEAD))
    return;

  //if (xmlStrEqual (name_in,
  //                 BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("title"))))
  //{
  //  // sanity check(s)
  //  data_p->state = TEST_I_SAXPARSER_STATE_READ_SYMBOL_WKN_ISIN;

  //  return;
  //} // end IF

  return;

body:
  // -------------------------------- body -------------------------------------
  //if (!(data_p->state == TEST_I_SAXPARSER_STATE_IN_BODY))
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
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("vwd_content"))))
        {
          //data_p->state = TEST_I_SAXPARSER_STATE_IN_BODY_DIV_CONTENT;
          break;
        } // end IF
      } // end IF
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        ACE_ASSERT (attributes_p[1]);

        if (xmlStrEqual (attributes_p[1],
                          BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("einzelkurs_header"))))
        {
          //data_p->state = TEST_I_SAXPARSER_STATE_IN_SYMBOL_H1_CONTENT;
          break;
        } // end IF
        else if (xmlStrEqual (attributes_p[1],
                              BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("vwd_infobox vertical_gap_2"))))
        {
          data_p->accumulate = true;
          //data_p->state = TEST_I_SAXPARSER_STATE_READ_ISIN_WKN;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("h1"))))
  {
    //if (data_p->state == TEST_I_SAXPARSER_STATE_IN_SYMBOL_H1_CONTENT)
    //{
    //  data_p->accumulate = true;
    //  data_p->state = TEST_I_SAXPARSER_STATE_READ_SYMBOL;
    //  return;
    //} // end IF
  } // end ELSE IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("span"))))
  {
    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("title"))))
      {
        ACE_ASSERT (attributes_p[1]);

        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("aktueller Wert"))))
        {
          data_p->accumulate = true;
          //data_p->state = TEST_I_SAXPARSER_STATE_READ_VALUE;
          break;
        } // end IF
        //else if (xmlStrEqual (attributes_p[1],
        //                      BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("prozentuale Veränderung zum Vortag"))))
        //{
        //  data_p->state = TEST_I_SAXPARSER_STATE_READ_CHANGE;
        //  break;
        //} // end IF
      } // end IF
      else if (xmlStrEqual (attributes_p[0],
                            BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        ACE_ASSERT (attributes_p[1]);

        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("rightfloat big positive vertical_gap_1"))))
        {
          data_p->accumulate = true;
          //data_p->state = TEST_I_SAXPARSER_STATE_READ_CHANGE;
          break;
        } // end IF
        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("rightfloat big negative vertical_gap_1"))))
        {
          data_p->accumulate = true;
          //data_p->state = TEST_I_SAXPARSER_STATE_READ_CHANGE;
          break;
        } // end IF
        else if (xmlStrEqual (attributes_p[1],
                              BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("leftfloat bottom_aligned"))))
        {
          data_p->accumulate = true;
          //data_p->state = TEST_I_SAXPARSER_STATE_READ_DATE;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end ELSE IF
}
void
endElement (void* userData_in,
            const xmlChar* name_in)
{
  NETWORK_TRACE (ACE_TEXT ("::endElement"));

  struct Test_I_URLStreamLoad_SAXParserContext* data_p =
    static_cast<struct Test_I_URLStreamLoad_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->URL.empty ());

  bool done = true;
//  std::string regex_string;
//  std::regex::flag_type flags = std::regex_constants::ECMAScript;
//  std::regex regex;
//  std::smatch match_results;
//  std::istringstream converter;

  switch (data_p->state)
  {
    //case TEST_I_SAXPARSER_STATE_READ_CHANGE:
    //{
    //  regex_string =
    //    ACE_TEXT_ALWAYS_CHAR ("^([+\\-]{1})([[:digit:]]+),([[:digit:]]+)%$");
    //  //try
    //  //{
    //  regex.assign (regex_string, flags);
    //  //}
    //  //catch (std::regex_error exception_in)
    //  //{
    //  //  ACE_DEBUG ((LM_ERROR,
    //  //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
    //  //              ACE_TEXT (exception_in.what ())));
    //  //  return;
    //  //}
    //  if (!std::regex_match (data_p->characters,
    //                         match_results,
    //                         regex,
    //                         std::regex_constants::match_default))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("invalid change string (was: \"%s\"), returning\n"),
    //                ACE_TEXT (data_p->characters.c_str ())));
    //    return;
    //  } // end IF
    //  ACE_ASSERT (match_results.ready () && !match_results.empty ());
    //  ACE_ASSERT (match_results[1].matched);
    //  ACE_ASSERT (match_results[2].matched);
    //  ACE_ASSERT (match_results[3].matched);

    //  std::string value_string_2 = match_results[2].str ();
    //  value_string_2 += '.';
    //  value_string_2 += match_results[3].str ();
    //  converter.str (value_string_2);
    //  converter >> data_p->record->change;
    //  value_string_2 = match_results[1].str ();
    //  if (value_string_2[0] == '-')
    //    data_p->record->change = -data_p->record->change;

    //  data_p->state = TEST_I_SAXPARSER_STATE_IN_BODY_DIV_CONTENT;

    //  break;
    //}
    //case TEST_I_SAXPARSER_STATE_READ_DATE:
    //{
    //  regex_string =
    //    ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{2})\\.([[:digit:]]{2})\\.([[:digit:]]{4}).{4}([[:digit:]]{2}):([[:digit:]]{2})$");
    //  regex.assign (regex_string, flags);
    //  if (!std::regex_match (data_p->characters,
    //                         match_results,
    //                         regex,
    //                         std::regex_constants::match_default))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("invalid date string (was: \"%s\"), returning\n"),
    //                ACE_TEXT (data_p->characters.c_str ())));
    //    return;
    //  } // end IF
    //  ACE_ASSERT (match_results.ready () && !match_results.empty ());
    //  ACE_ASSERT (match_results[1].matched);
    //  ACE_ASSERT (match_results[2].matched);
    //  ACE_ASSERT (match_results[3].matched);
    //  ACE_ASSERT (match_results[4].matched);
    //  ACE_ASSERT (match_results[5].matched);

    //  long value;
    //  struct tm tm_time;
    //  ACE_OS::memset (&tm_time, 0, sizeof (struct tm));

    //  converter.str (match_results[1].str ());
    //  converter >> value;
    //  tm_time.tm_mday = value;
    //  converter.clear ();
    //  converter.str (match_results[2].str ());
    //  converter >> value;
    //  tm_time.tm_mon = value - 1; // months are 0-11
    //  converter.clear ();
    //  converter.str (match_results[3].str ());
    //  converter >> value;
    //  tm_time.tm_year = value - 1900; // years are since 1900

    //  converter.clear ();
    //  converter.str (match_results[4].str ());
    //  converter >> value;
    //  tm_time.tm_hour = value;
    //  converter.clear ();
    //  converter.str (match_results[5].str ());
    //  converter >> value;
    //  tm_time.tm_min = value;

    //  time_t time_seconds = ACE_OS::mktime (&tm_time);
    //  data_p->record->timeStamp.set (time_seconds, 0);

    //  data_p->state = TEST_I_SAXPARSER_STATE_IN_BODY_DIV_CONTENT;

    //  break;
    //}
    //case TEST_I_SAXPARSER_STATE_READ_ISIN_WKN:
    //{
    //  // sanity check(s)
    //  ACE_ASSERT (data_p->record->item);

    //  regex_string =
    //    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)ISIN ([[:alnum:]]+) \\| WKN ([[:alnum:]]+)(?:[[:space:]]*)$");
    //  //try
    //  //{
    //  regex.assign (regex_string, flags);
    //  //}
    //  //catch (std::regex_error exception_in)
    //  //{
    //  //  ACE_DEBUG ((LM_ERROR,
    //  //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
    //  //              ACE_TEXT (exception_in.what ())));
    //  //  return;
    //  //}
    //  if (!std::regex_match (data_p->characters,
    //                         match_results,
    //                         regex,
    //                         std::regex_constants::match_default))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("invalid isin/wkn string (was: \"%s\"), returning\n"),
    //                ACE_TEXT (data_p->characters.c_str ())));
    //    return;
    //  } // end IF
    //  ACE_ASSERT (match_results.ready () && !match_results.empty ());
    //  ACE_ASSERT (match_results[1].matched);
    //  ACE_ASSERT (match_results[2].matched);
    //  data_p->record->item->ISIN = match_results[1].str ();
    //  data_p->record->item->WKN  = match_results[2].str ();

    //  // match ISIN
    //  regex_string = ACE_TEXT_ALWAYS_CHAR ("^([[:alpha:]]{2}[[:digit:]]{10})$");
    //  regex.assign (regex_string, flags);
    //  if (std::regex_match (data_p->record->item->ISIN,
    //                        match_results,
    //                        regex,
    //                        std::regex_constants::match_default))
    //  {
    //    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    //    ACE_ASSERT (match_results[1].matched);
    //  } // end IF
    //  else
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("invalid isin string (was: \"%s\"), continuing\n"),
    //                ACE_TEXT (data_p->record->item->ISIN.c_str ())));

    //  // match WKN
    //  regex_string = ACE_TEXT_ALWAYS_CHAR ("^([[:alnum:]]{6})$");
    //  regex.assign (regex_string, flags);
    //  if (std::regex_match (data_p->record->item->WKN,
    //                        match_results,
    //                        regex,
    //                        std::regex_constants::match_default))
    //  {
    //    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    //    ACE_ASSERT (match_results[1].matched);
    //  } // end IF
    //  else
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("invalid wkn string (was: \"%s\"), continuing\n"),
    //                ACE_TEXT (data_p->record->item->WKN.c_str ())));

    //  data_p->state = TEST_I_SAXPARSER_STATE_IN_BODY_DIV_CONTENT;

    //  break;
    //}
    //case TEST_I_SAXPARSER_STATE_READ_SYMBOL:
    //{
    //  // sanity check(s)
    //  ACE_ASSERT (data_p->record->item);

    //  regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)([[:print:]]+)(?:.*)$");
    //  //try {
    //  regex.assign (regex_string, flags);
    //  //} catch (std::regex_error exception_in) {
    //  //  ACE_DEBUG ((LM_ERROR,
    //  //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
    //  //              ACE_TEXT (exception_in.what ())));
    //  //  return;
    //  //}
    //  if (!std::regex_match (data_p->characters,
    //                         match_results,
    //                         regex,
    //                         std::regex_constants::match_default))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("invalid symbol string (was: \"%s\"), returning\n"),
    //                ACE_TEXT (data_p->characters.c_str ())));
    //    return;
    //  } // end IF
    //  ACE_ASSERT (match_results.ready () && !match_results.empty ());
    //  ACE_ASSERT (match_results[1].matched);

    //  data_p->record->item->symbol = match_results[1].str ();
    //  // strip trailing whitespace
    //  std::string::size_type position =
    //    data_p->record->item->symbol.find_last_of (' ',
    //                                               std::string::npos);
    //  if (position != std::string::npos)
    //    data_p->record->item->symbol.erase (position,
    //                                        std::string::npos);

    //  // *TODO*: move this to endElement ()
    //  data_p->state = TEST_I_SAXPARSER_STATE_IN_BODY_DIV_CONTENT;

    //  break;
    //}
    //case TEST_I_SAXPARSER_STATE_READ_VALUE:
    //{
    //  regex_string =
    //    ACE_TEXT_ALWAYS_CHAR ("^(?:[^[:digit:]]*)([[:digit:]]+\\.)?([[:digit:]]+)(,[[:digit:]]+)(?:.*)$");
    //  regex.assign (regex_string, flags);
    //  if (!std::regex_match (data_p->characters,
    //                         match_results,
    //                         regex,
    //                         std::regex_constants::match_default))
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("invalid value string (was: \"%s\"), returning\n"),
    //                ACE_TEXT (data_p->characters.c_str ())));
    //    return;
    //  } // end IF
    //  ACE_ASSERT (match_results.ready () && !match_results.empty ());
    //  //ACE_ASSERT (match_results[1].matched);
    //  ACE_ASSERT (match_results[2].matched);
    //  ACE_ASSERT (match_results[3].matched);

    //  // *TODO*: this isn't quite right yet, i.e. make sure to set a locale that
    //  //         allows thousands separators
    //  std::locale locale (ACE_TEXT_ALWAYS_CHAR (""));
    //  std::string value_string_2;
    //  if (match_results[1].matched)
    //    value_string_2 = match_results[1].str ();
    //  value_string_2 += match_results[2].str ();
    //  value_string_2 += match_results[3].str ();
    //  std::istringstream converter (value_string_2);
    //  converter.imbue (locale);
    //  converter >> data_p->record->value;

    //  data_p->state = TEST_I_SAXPARSER_STATE_IN_BODY_DIV_CONTENT;

    //  break;
    //}
    default:
      done = false;
      break;
  } // end SWITCH

  // clean up
  data_p->accumulate = false;
  data_p->characters.clear ();

  if (done)
    return;

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  { // sanity check(s)
    ACE_ASSERT (data_p->state == TEST_I_SAXPARSER_STATE_IN_HTML);
    data_p->state = TEST_I_SAXPARSER_STATE_INVALID;
    return;
  } // end IF

  // ------------------------------- head --------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  { // sanity check(s)
    ACE_ASSERT (data_p->state == TEST_I_SAXPARSER_STATE_IN_HEAD);
    data_p->state = TEST_I_SAXPARSER_STATE_IN_HTML;
    return;
  } // end IF

  // -------------------------------- body -------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    data_p->state = TEST_I_SAXPARSER_STATE_IN_HTML;
    return;
  } // end IF
}
