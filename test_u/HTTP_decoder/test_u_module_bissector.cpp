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

#include "stream_macros.h"

void
errorCallback (void* userData_in,
               const char* message_in,
               ...)
{
  STREAM_TRACE (ACE_TEXT ("::errorCallback"));

  Test_I_SAXParserContext* data_p =
      static_cast<Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_TCHAR buffer[BUFSIZ];
  va_list arguments;

  va_start (arguments, message_in);
  int length = ACE_OS::vsnprintf (buffer,
                                  sizeof (buffer),
//                                  sizeof (buffer) / sizeof (buffer[0]),
                                  message_in, arguments);
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
  STREAM_TRACE (ACE_TEXT ("::structuredErrorCallback"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("structuredErrorCallback: %s\n"),
              ACE_TEXT (error_in->message)));
}

Test_I_Stream_Module_HTMLParser::Test_I_Stream_Module_HTMLParser ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLParser::Test_I_Stream_Module_HTMLParser"));

}

Test_I_Stream_Module_HTMLParser::~Test_I_Stream_Module_HTMLParser ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLParser::~Test_I_Stream_Module_HTMLParser"));

}

void
Test_I_Stream_Module_HTMLParser::handleDataMessage (Test_I_Stream_Message*& message_inout,
                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLParser::handleDataMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::parserContext_.parserContext);

  result =
      htmlParseChunk (inherited::parserContext_.parserContext, // context
                      message_inout->rd_ptr (),                // chunk
                      message_inout->length (),                // size
                      0);                                      // terminate ?
  if (result)
  {
//    xmlParserErrors error = static_cast<xmlParserErrors> (result);
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("failed to htmlParseChunk() (status was: %d), continuing\n"),
//                error));

    xmlCtxtResetLastError (inherited::parserContext_.parserContext);
  } // end IF
//  result = htmlParseChunk(parserContext_, ACE_TEXT_ALWAYS_CHAR (""), 0, 1);
//  if (result)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to htmlParseChunk() (status was: %d), returning\n"),
//                result));
//    return;
//  } // end IF

//  if (!parserContext_->wellFormed)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("HTML document not well formed, continuing\n")));
//  if (parserContext_->errNo)
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("HTML document had errors (errno was: %d), continuing\n"),
//                parserContext_->errNo));
}

void
Test_I_Stream_Module_HTMLParser::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLParser::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // *TODO*: remove type inferences
  const Test_I_Stream_SessionData_t& session_data_container_r =
    message_inout->get ();
  const Test_I_Stream_SessionData& session_data_r =
    session_data_container_r.get ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
//      if (parserContext_)
//        htmlCtxtReset (parserContext_);

      // *TODO*: the upstream session data may not be the same as the downstream
      //         one...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (session_data_r.sessionID != reinterpret_cast<size_t> (ACE_INVALID_HANDLE))
#else
      if (session_data_r.sessionID != static_cast<size_t> (ACE_INVALID_HANDLE))
#endif
      {
        const_cast<Test_I_Stream_SessionData&> (session_data_r).parserContext =
            &(inherited::parserContext_);
        inherited::parserContext_.sessionData =
            &const_cast<Test_I_Stream_SessionData&> (session_data_r);
      } // end IF

      break;
    }
    case STREAM_SESSION_STEP:
    {
      if (inherited::parserContext_.parserContext)
        htmlCtxtReset (inherited::parserContext_.parserContext);

      break;
    }
    case STREAM_SESSION_END:
    {
//      // *TODO*: the upstream session data may not be the same as the downstream
//      //         one...
//      const_cast<Test_I_Stream_SessionData*> (session_data_p)->parserContext =
//          &(inherited::parserContext_);
//      inherited::parserContext_.sessionData =
//          const_cast<Test_I_Stream_SessionData*> (session_data_p);

      break;
    }
    default:
      break;
  } // end SWITCH
}

bool
Test_I_Stream_Module_HTMLParser::initialize (const Test_I_Stream_ModuleHandlerConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLParser::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.mode == STREAM_MODULE_HTMLPARSER_SAX);

  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_HTMLParser_T::initialize(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

//  initGenericErrorDefaultFunc ((xmlGenericErrorFunc*)&::errorCallback);
//  xmlSetGenericErrorFunc (inherited::parserContext_, &::errorCallback);
//  xmlSetStructuredErrorFunc (inherited::parserContext_, &::structuredErrorCallback);

  return true;
}

bool
Test_I_Stream_Module_HTMLParser::initializeSAXParser ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLParser::initializeSAXParser"));

  // set necessary SAX parser callbacks
  // *IMPORTANT NOTE*: the default SAX callbacks expect xmlParserCtxtPtr as user
  //                   data. This implementation uses Test_I_SAXParserContext*.
  //                   --> the default callbacks will crash on invocation, so
  //                       disable them here...
  inherited::SAXHandler_.cdataBlock = NULL;
  inherited::SAXHandler_.comment = NULL;
  inherited::SAXHandler_.internalSubset = NULL;
  inherited::SAXHandler_.startDocument = NULL;

//  inherited::SAXHandler_.getEntity = getEntity;
//  inherited::SAXHandler_.startDocument = startDocument;
//  inherited::SAXHandler_.endDocument = endDocument;
  inherited::SAXHandler_.startElement = startElement;
  inherited::SAXHandler_.endElement = endElement;
  inherited::SAXHandler_.characters = characters;
  ///////////////////////////////////////
  inherited::SAXHandler_.warning = errorCallback;
  inherited::SAXHandler_.error = errorCallback;
  inherited::SAXHandler_.fatalError = errorCallback;

  ACE_ASSERT (inherited::SAXHandler_.initialized);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
startDocument (void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::startDocument"));

  ACE_UNUSED_ARG (userData_in);
}
void
endDocument (void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::endDocument"));

  ACE_UNUSED_ARG (userData_in);
}
void
characters (void* userData_in,
            const xmlChar* string_in,
            int length_in)
{
  STREAM_TRACE (ACE_TEXT ("::characters"));

  Test_I_SAXParserContext* data_p =
      static_cast<Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  switch (data_p->state)
  {
    case SAXPARSER_STATE_READ_DATE:
    {
      // parse date string
      std::string date_string = reinterpret_cast<const char*> (string_in);
      std::string regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{1,2})\\.([[:digit:]]{1,2})\\.([[:digit:]]{2,4})$");
      std::regex regex (regex_string);
      std::smatch match_results;
      if (!std::regex_match (date_string,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid date string (was: \"%s\"), returning\n"),
                    ACE_TEXT (date_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      ACE_ASSERT (match_results[1].matched);
      ACE_ASSERT (match_results[2].matched);
      ACE_ASSERT (match_results[3].matched);

      std::stringstream converter;
      long value;
      struct tm tm_time;
      ACE_OS::memset (&tm_time, 0, sizeof (struct tm));
      converter << match_results[1].str ();
      converter >> value;
      tm_time.tm_mday = value;
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter << match_results[2].str ();
      converter >> value;
      tm_time.tm_mon = value - 1; // months are 0-11
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter << match_results[3].str ();
      converter >> value;
      tm_time.tm_year = value - 1900; // years are since 1900

      time_t time_seconds = ACE_OS::mktime (&tm_time);
      data_p->timeStamp.set (time_seconds, 0);

      data_p->state = SAXPARSER_STATE_READ_ITEMS;

      break;
    }
    case SAXPARSER_STATE_READ_DESCRIPTION:
    {
      data_p->dataItem.description += (char*)string_in;

      break;
    }
    case SAXPARSER_STATE_READ_TITLE:
    {
      // sanity check(s)
      ACE_ASSERT (data_p->sessionData);

      data_p->sessionData->data.title = (char*)string_in;

      break;
    }
    default:
      break;
  } // end SWITCH
}
void
startElement (void* userData_in,
              const xmlChar* name_in,
              const xmlChar** attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("::startElement"));

  Test_I_SAXParserContext* data_p =
      static_cast<Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  const xmlChar** attributes_p = attributes_in;

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_INVALID);

    data_p->state = SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF

  // ------------------------------- header ------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_IN_HEAD;

    return;
  } // end IF

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("title"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HEAD);

    data_p->state = SAXPARSER_STATE_READ_TITLE;

    return;
  } // end IF

  // -------------------------------- body -------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_IN_BODY;

    return;
  } // end IF

  // <h4 class="published">DATE</h4>
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("h4"))))
  {
    // sanity check(s)
    if (data_p->state != SAXPARSER_STATE_READ_ITEMS)
      return; // done

    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("published"))))
        {
          data_p->state = SAXPARSER_STATE_READ_DATE;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("a"))))
  {
    if (data_p->state == SAXPARSER_STATE_READ_ITEM)
    {
      // save link URL
      while (NULL != attributes_p && NULL != attributes_p[0])
      {
        if (xmlStrEqual (attributes_p[0],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("href"))))
        {
          data_p->dataItem.URI = (char*)attributes_p[1];
          break;
        } // end IF

        attributes_p = &attributes_p[2];
      } // end WHILE

      data_p->state = SAXPARSER_STATE_READ_DESCRIPTION;
    } // end IF
  } // end IF
  ///////////////////////////////////////
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("div"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_BODY);

    const xmlChar** attributes_p = attributes_in;
    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("id"))))
      {
        ACE_ASSERT (attributes_p[1]);
        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("archiv_woche"))))
        {
          data_p->state = SAXPARSER_STATE_READ_ITEMS;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end ELSE IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("li"))))
  {
    if (data_p->state == SAXPARSER_STATE_READ_ITEMS)
      data_p->state = SAXPARSER_STATE_READ_ITEM;
  } // end ELSE IF
}
void
endElement (void* userData_in,
            const xmlChar* name_in)
{
  STREAM_TRACE (ACE_TEXT ("::endElement"));

  Test_I_SAXParserContext* data_p =
      static_cast<Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_INVALID;

    return;
  } // end IF

  // ------------------------------- header ------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HEAD);

    data_p->state = SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("title"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_READ_TITLE);

    data_p->state = SAXPARSER_STATE_IN_HEAD;

    return;
  } // end ELSE IF

  // -------------------------------- body -------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_BODY);

    data_p->state = SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("div"))))
  {
    if (data_p->state == SAXPARSER_STATE_READ_ITEMS)
      data_p->state = SAXPARSER_STATE_IN_BODY;
  } // end ELSE IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("a"))))
  {
    if (data_p->state != SAXPARSER_STATE_READ_DESCRIPTION)
      return; // nothing to do

    // sanity check(s)
    ACE_ASSERT (data_p->sessionData);

    Test_I_PageDataIterator_t iterator =
        data_p->sessionData->data.pageData.find (data_p->timeStamp);
    if (iterator == data_p->sessionData->data.pageData.end ())
    {
      Test_I_DataItems_t data_items;
      data_items.push_back (data_p->dataItem);
      data_p->sessionData->data.pageData[data_p->timeStamp] = data_items;
    } // end IF
    else
      (*iterator).second.push_back (data_p->dataItem);

    data_p->dataItem.description.clear ();

    data_p->state = SAXPARSER_STATE_READ_ITEMS;
  } // end IF
}
//xmlEntityPtr
//getEntity (void* userData_in,
//           const xmlChar* name_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::getEntity"));

//  ACE_UNUSED_ARG (userData_in);
//  ACE_UNUSED_ARG (name_in);

//  return NULL;
//}
