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

#include "stream_tools.h"

#include "net_macros.h"

void
test_i_libxml2_sax_error_cb (void* userData_in,
                             const char* message_in,
                             ...)
{
  //NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_error_cb"));

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
  //NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_structured_error_cb"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("test_i_libxml2_sax_structured_error_cb: %s\n"),
              ACE_TEXT (error_in->message)));
}

//////////////////////////////////////////

Test_I_Stream_HTMLParser::Test_I_Stream_HTMLParser (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , isInitial_ (true)
 , iterator_ ()
 , iterator_2_ ()
 , context_ ()
 , context_2 ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::Test_I_Stream_HTMLParser"));

}

void
Test_I_Stream_HTMLParser::handleDataMessage (Test_I_Stream_Message*& message_inout,
                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  if (isInitial_)
  {
    isInitial_ = false;

    // extract 'retrieve URL' and send control with URL back upstream to the
    // HTTP-GET module
    ACE_ASSERT (inherited::parserContext_.parserContext);
    context_.parserContext = inherited::parserContext_.parserContext;
    inherited::parserContext_ = context_;

    inherited::handleDataMessage (message_inout,
                                  passMessageDownstream_out);

    // clean up
    passMessageDownstream_out = false;
    message_inout->release (); message_inout = NULL;

    ACE_ASSERT (!inherited::parserContext_.URL.empty ());
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: parsed HTML document; stock value retrieve URL is: \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::parserContext_.URL.c_str ())));

    inherited::configuration_->URL = inherited::parserContext_.URL;

    // notify upstream
    notify (STREAM_SESSION_MESSAGE_STEP);

    return;
  } // end IF

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

  inherited::parserContext_ = context_2;
  initializeSAXParser ();
  inherited::resetParser ();
  inherited::parserContext_.record =
    &const_cast<struct Test_I_StockRecord&> (*iterator_2_);

  inherited::handleDataMessage (message_inout,
                                passMessageDownstream_out);

  // done
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, *session_data_r.lock);
    if (session_data_r.connection)
      session_data_r.connection->abort ();
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (headFragment_);

  const typename Test_I_Stream_Message::DATA_T& data_container_r =
    headFragment_->getR ();
  typename Test_I_Stream_Message::DATA_T::DATA_T& data_r =
    const_cast<typename Test_I_Stream_Message::DATA_T::DATA_T&> (data_container_r.getR ());
  int result = -1;

  // *IMPORTANT NOTE*: no more data will arrive for this document
  result = htmlParseChunk (inherited::parserContext_.parserContext,
                           ACE_TEXT_ALWAYS_CHAR (""),
                           0,
                           1); // terminate
  xmlErrorPtr error_p = xmlGetLastError ();
  if (result)
  {
    xmlParserErrors parse_errors = static_cast<xmlParserErrors> (result);
    ACE_DEBUG ((Stream_HTML_Tools::errorLevelToLogPriority (error_p ? error_p->level : XML_ERR_ERROR),
                ACE_TEXT ("%s: failed to htmlParseChunk() (result was: %d): \"%s\", continuing\n"),
                inherited::mod_->name (),
                result,
                error_p ? ACE_TEXT (error_p->message) : ACE_TEXT ("")));
  } // end IF
  if (!inherited::parserContext_.parserContext->wellFormed)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: document not well-formed, continuing\n"),
                inherited::mod_->name ()));
  if (error_p &&
      error_p->code)
    ACE_DEBUG ((Stream_HTML_Tools::errorLevelToLogPriority (error_p->level),
                ACE_TEXT ("%s: document had errors (last error was: %d: \"%s\"), continuing\n"),
                inherited::mod_->name (),
                error_p->code, ACE_TEXT (error_p->message)));
  xmlCtxtResetLastError (inherited::parserContext_.parserContext);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("parsing HTML...DONE\n")));

  //// *TODO*: remove type inference
  //ACE_ASSERT (!data_r.HTMLDocument);
  //data_r.HTMLDocument = inherited::parserContext_.parserContext->myDoc;
  //inherited::parserContext_.parserContext->myDoc = NULL;

  //result = inherited::put_next (inherited::headFragment_, NULL);
  //if (unlikely (result == -1))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
  //              inherited::mod_->name ()));
  //  inherited::headFragment_->release ();
  //} // end IF
  //inherited::headFragment_ = NULL;
}

void
Test_I_Stream_HTMLParser::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::handleSessionMessage"));

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
      //inherited::handleSessionMessage (message_inout,
      //                                 passMessageDownstream_out);
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: parsed HTML document (symbol: \"%s\")\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT ((*iterator_2_).item->symbol.c_str ())));
      //inherited::parserContext_.record = NULL;

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
  NETWORK_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.mode == STREAM_MODULE_HTMLPARSER_MODE_SAX);

//  initGenericErrorDefaultFunc ((xmlGenericErrorFunc*)&::errorCallback);
//  xmlSetGenericErrorFunc (inherited::parserContext_, &::errorCallback);
//  xmlSetStructuredErrorFunc (inherited::parserContext_, &::structuredErrorCallback);
  context_2.accumulate = true;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

bool
Test_I_Stream_HTMLParser::initializeSAXParser ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::initializeSAXParser"));

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

  if (isInitial_)
  {
    inherited::SAXHandler_.startElement = test_i_libxml2_sax_start_element_cb;
    inherited::SAXHandler_.endElement = test_i_libxml2_sax_end_element_cb;
    inherited::SAXHandler_.characters = test_i_libxml2_sax_characters_cb;
  } // end IF
  else
  {
    inherited::SAXHandler_.startElement = test_i_libxml2_sax_start_element_2;
    inherited::SAXHandler_.endElement = test_i_libxml2_sax_end_element_2;
    inherited::SAXHandler_.characters = test_i_libxml2_sax_characters_2;
  } // end ELSE

  ////////////////////////////////////////
  inherited::SAXHandler_.warning = test_i_libxml2_sax_error_cb;
  inherited::SAXHandler_.error = test_i_libxml2_sax_error_cb;
  inherited::SAXHandler_.fatalError = test_i_libxml2_sax_error_cb;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
test_i_libxml2_sax_characters_cb (void* userData_in,
                                  const xmlChar* string_in,
                                  int length_in)
{
  NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_characters_cb"));

  ACE_UNUSED_ARG (length_in);

  // sanity check(s)
  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);
  ACE_ASSERT (data_p);

  data_p->characters = reinterpret_cast<const char*> (string_in);
}

void
test_i_libxml2_sax_start_element_cb (void* userData_in,
                                     const xmlChar* name_in,
                                     const xmlChar** attributes_in)
{
  NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_start_element_cb"));

  // sanity check(s)
  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);
  ACE_ASSERT (data_p);

  const xmlChar** attributes_p = attributes_in;

  // ------------------------------- html --------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("div"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_INVALID);

    data_p->state = SAXPARSER_STATE_IN_BODY;

    //return;
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
    case SAXPARSER_STATE_IN_UL_CONTENT:
    case SAXPARSER_STATE_IN_LI_CONTENT:
    //////////////////////////////////////
    case SAXPARSER_STATE_READ_ONCLICK:
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
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        ACE_ASSERT (attributes_p[1]);

        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("list_searchsuggest"))))
        {
          data_p->state = SAXPARSER_STATE_IN_BODY_DIV_CONTENT;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("ul"))))
  {
    if (data_p->state == SAXPARSER_STATE_IN_BODY_DIV_CONTENT)
      data_p->state = SAXPARSER_STATE_IN_UL_CONTENT;
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("li"))))
  {
    if (data_p->state == SAXPARSER_STATE_IN_UL_CONTENT)
      data_p->state = SAXPARSER_STATE_IN_LI_CONTENT;

    if (data_p->state != SAXPARSER_STATE_IN_LI_CONTENT)
      return;
    data_p->state = SAXPARSER_STATE_READ_ONCLICK;

    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("onclick"))))
      { ACE_ASSERT (attributes_p[1]);
        ACE_ASSERT (data_p->URL.empty ());
        std::string regex_string;
        std::regex::flag_type flags = std::regex_constants::ECMAScript;
        std::regex regex;
        std::cmatch match_results;

        regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:top.location.href=')(.+)(?:';)$");
        //try {
        regex.assign (regex_string, flags);
        //} catch (std::regex_error exception_in) {
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("caught regex exception (was: \"%s\"), returning\n"),
        //              ACE_TEXT (exception_in.what ())));
        //  return;
        //}
        if (!std::regex_match (reinterpret_cast<const char*> (attributes_p[1]),
                               match_results,
                               regex,
                               std::regex_constants::match_default))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid symbol string (was: \"%s\"), returning\n"),
                      ACE_TEXT (reinterpret_cast<const char*> (attributes_p[1]))));
          return;
        } // end IF
  //      ACE_ASSERT (match_results.ready () && !match_results.empty ());
        ACE_ASSERT (!match_results.empty ());
        ACE_ASSERT (match_results[1].matched);

        data_p->URL = match_results[1].str ();

        break;
      } // end IF
      attributes_p = &attributes_p[2];
    } // end WHILE

  } // end ELSE IF
}

void
test_i_libxml2_sax_end_element_cb (void* userData_in,
                                   const xmlChar* name_in)
{
  NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_end_element_cb"));

  // sanity check(s)
  struct Test_I_SAXParserContext* data_p =
    static_cast<struct Test_I_SAXParserContext*> (userData_in);
  ACE_ASSERT (data_p);
  //ACE_ASSERT (data_p->URL.empty ());

  bool done = false;
  switch (data_p->state)
  {
    case SAXPARSER_STATE_READ_ONCLICK:
      done = true;
      break;
    default:
      break;
  } // end SWITCH

  // clean up
  //data_p->accumulate = false;
  data_p->characters.clear ();

  if (done)
    return;

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

////////////////////////////////////////////////////////////////////////////////

void
test_i_libxml2_sax_characters_2 (void* userData_in,
                                 const xmlChar* string_in,
                                 int length_in)
{
  NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_characters_2"));

  ACE_UNUSED_ARG (length_in);

  // sanity check(s)
  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);
  ACE_ASSERT (data_p);

  if (likely (data_p->accumulate))
    data_p->characters = reinterpret_cast<const char*> (string_in);

  if ((data_p->state_2 == SAXPARSER_STATE_2_IN_SPAN_PRICE_CONTENT) &&
      data_p->accumulate)
    data_p->accumulate = false;
}

void
test_i_libxml2_sax_start_element_2 (void* userData_in,
                                    const xmlChar* name_in,
                                    const xmlChar** attributes_in)
{
  NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_start_element_2"));

  // sanity check(s)
  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);
  ACE_ASSERT (data_p);

  const xmlChar** attributes_p = attributes_in;

  // ------------------------------- html --------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state_2 == SAXPARSER_STATE_2_INVALID);

    data_p->state_2 = SAXPARSER_STATE_2_IN_HTML;

    return;
  } // end IF

  // ------------------------------- head/body ---------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state_2 == SAXPARSER_STATE_2_IN_HTML);

    data_p->state_2 = SAXPARSER_STATE_2_IN_HEAD;

    return;
  } // end IF
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state_2 == SAXPARSER_STATE_2_IN_HTML);

    data_p->state_2 = SAXPARSER_STATE_2_IN_BODY;

    return;
  } // end IF

  switch (data_p->state_2)
  {
    case SAXPARSER_STATE_2_IN_HTML:
      goto html;
    case SAXPARSER_STATE_2_IN_HEAD:
      goto head;
    case SAXPARSER_STATE_2_IN_BODY:
    //////////////////////////////////////
    case SAXPARSER_STATE_2_IN_BODY_DIV_CONTENT:
    case SAXPARSER_STATE_2_IN_SPAN_PRICE_CONTENT:
    case SAXPARSER_STATE_2_IN_SPAN_CHANGE_CONTENT:
    case SAXPARSER_STATE_2_IN_SPAN_DATE_CONTENT:
    //////////////////////////////////////
    case SAXPARSER_STATE_2_READ_PRICE:
    case SAXPARSER_STATE_2_READ_CHANGE:
    case SAXPARSER_STATE_2_READ_DATE:
      goto body;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown parser state (was: %d), returning\n"),
                  data_p->state_2));
      return;
    }
  } // end SWITCH

html:
  // -------------------------------- html -------------------------------------
  if (!(data_p->state_2 == SAXPARSER_STATE_2_IN_HTML))
    return;

  return;

head:
  // -------------------------------- head -------------------------------------
  if (!(data_p->state_2 == SAXPARSER_STATE_2_IN_HEAD))
    return;

  return;

body:
  // -------------------------------- body -------------------------------------
  //if (!(data_p->state_2 == SAXPARSER_STATE_2_IN_BODY))
  //  return;

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("div"))))
  {
    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        ACE_ASSERT (attributes_p[1]);

        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("prices "))))
        {
          data_p->state_2 = SAXPARSER_STATE_2_IN_BODY_DIV_CONTENT;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("span"))))
  {
    if (data_p->state_2 != SAXPARSER_STATE_2_IN_BODY_DIV_CONTENT)
      return;

    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        ACE_ASSERT (attributes_p[1]);

        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("price"))))
        {
          data_p->state_2 = SAXPARSER_STATE_2_IN_SPAN_PRICE_CONTENT;
          break;
        } // end IF
        else if (xmlStrEqual (attributes_p[1],
                              BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("change"))))
        {
          data_p->state_2 = SAXPARSER_STATE_2_IN_SPAN_CHANGE_CONTENT;
          data_p->accumulate = true;
          break;
        } // end IF
        else if (xmlStrEqual (attributes_p[1],
                              BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("date"))))
        {
          data_p->state_2 = SAXPARSER_STATE_2_IN_SPAN_DATE_CONTENT;
          data_p->accumulate = true;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end ELSE IF
}

void
test_i_libxml2_sax_end_element_2 (void* userData_in,
                                  const xmlChar* name_in)
{
  NETWORK_TRACE (ACE_TEXT ("::test_i_libxml2_sax_end_element_2"));

  // sanity check(s)
  struct Test_I_SAXParserContext* data_p =
    static_cast<struct Test_I_SAXParserContext*> (userData_in);
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->record);

  bool done = true;
  std::string regex_string;
  std::regex::flag_type flags = std::regex_constants::ECMAScript;
  std::regex regex;
  std::smatch match_results;
  std::istringstream converter;

  switch (data_p->state_2)
  {
    case SAXPARSER_STATE_2_IN_SPAN_PRICE_CONTENT:
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

      data_p->state_2 = SAXPARSER_STATE_2_IN_BODY_DIV_CONTENT;
      break;
    }
    case SAXPARSER_STATE_2_IN_SPAN_CHANGE_CONTENT:
    {
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([+\\-]{1})([[:digit:]]+),([[:digit:]]+) \\((.+)%\\)$");
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
      ACE_ASSERT (match_results[4].matched); // %

      std::string value_string_2 = match_results[2].str ();
      value_string_2 += '.';
      value_string_2 += match_results[3].str ();
      converter.str (value_string_2);
      converter >> data_p->record->change;
      value_string_2 = match_results[1].str ();
      if (value_string_2[0] == '-')
        data_p->record->change = -data_p->record->change;

      data_p->state_2 = SAXPARSER_STATE_2_IN_BODY_DIV_CONTENT;
      break;
    }
    case SAXPARSER_STATE_2_IN_SPAN_DATE_CONTENT:
    {
      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{2}).([[:digit:]]{2}).([[:digit:]]{4}), ([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2}) Uhr$");
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
      ACE_ASSERT (match_results[3].matched);
      ACE_ASSERT (match_results[4].matched);
      ACE_ASSERT (match_results[5].matched);
      ACE_ASSERT (match_results[6].matched);

      long value;
      struct tm tm_time;
      ACE_OS::memset (&tm_time, 0, sizeof (struct tm));

      converter.clear ();
      converter.str (match_results[3].str ());
      converter >> value;
      tm_time.tm_year = value - 1900;
      converter.clear ();
      converter.str (match_results[2].str ());
      converter >> value;
      tm_time.tm_mon = value - 1;
      converter.clear ();
      converter.str (match_results[1].str ());
      converter >> value;
      tm_time.tm_mday = value;

      converter.clear ();
      converter.str (match_results[4].str ());
      converter >> value;
      tm_time.tm_hour = value;
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> value;
      tm_time.tm_min = value;
      converter.clear ();
      converter.str (match_results[6].str ());
      converter >> value;
      tm_time.tm_sec = value;

      time_t time_seconds = ACE_OS::mktime (&tm_time);
      data_p->record->timeStamp.set (time_seconds, 0);

      data_p->state_2 = SAXPARSER_STATE_2_IN_HTML;
      break;
    }
    default:
      done = false;
      break;
  } // end SWITCH

  // clean up
  //data_p->accumulate = false;
  data_p->characters.clear ();

  if (done)
    return;

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  { ACE_ASSERT (data_p->state_2 == SAXPARSER_STATE_2_IN_HTML);
    data_p->state_2 = SAXPARSER_STATE_2_INVALID;
    return;
  } // end IF

  // ------------------------------- head --------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  { ACE_ASSERT (data_p->state_2 == SAXPARSER_STATE_2_IN_HEAD);
    data_p->state_2 = SAXPARSER_STATE_2_IN_HTML;
    return;
  } // end IF

  // -------------------------------- body -------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    data_p->state_2 = SAXPARSER_STATE_2_IN_HTML;
    return;
  } // end IF
}
