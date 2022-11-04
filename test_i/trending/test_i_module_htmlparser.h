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

#ifndef TEST_I_HTMLPARSER_H
#define TEST_I_HTMLPARSER_H

#include "libxml/xmlerror.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_htmlparser.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_trending_common.h"

// SAX callbacks
void test_i_libxml2_sax_characters_cb (void*,          // user data
                                       const xmlChar*, // string
                                       int);           // length
void test_i_libxml2_sax_start_element_cb (void*,            // user data
                                          const xmlChar*,   // name
                                          const xmlChar**); // attributes
void test_i_libxml2_sax_end_element_cb (void*,           // user data
                                        const xmlChar*); // name
//////////////////////////////////////////
void test_i_libxml2_sax_characters_2 (void*,          // user data
                                      const xmlChar*, // string
                                      int);           // length
void test_i_libxml2_sax_start_element_2 (void*,            // user data
                                         const xmlChar*,   // name
                                         const xmlChar**); // attributes
void test_i_libxml2_sax_end_element_2 (void*,           // user data
                                       const xmlChar*); // name
//////////////////////////////////////////
void
test_i_libxml2_sax_error_cb (void*,       // context
                             const char*, // message
                             ...);        // arguments
void
test_i_libxml2_sax_structured_error_cb (void*,        // user data
                                        xmlErrorPtr); // error

class Test_I_Stream_HTMLParser
 : public Stream_Module_HTMLParser_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_Trending_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_Stream_Message,
                                     Test_I_Stream_SessionMessage,
                                     Test_I_Trending_SessionData_t,
                                     struct Test_I_Trending_SessionData,
                                     struct Test_I_SAXParserContext>
{
  typedef Stream_Module_HTMLParser_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_Trending_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_I_Stream_Message,
                                     Test_I_Stream_SessionMessage,
                                     Test_I_Trending_SessionData_t,
                                     struct Test_I_Trending_SessionData,
                                     struct Test_I_SAXParserContext> inherited;

 public:
  Test_I_Stream_HTMLParser (ISTREAM_T*); // stream handle
  inline virtual ~Test_I_Stream_HTMLParser () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const struct Test_I_Trending_ModuleHandlerConfiguration&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (Test_I_Stream_Message*&, // data message handle
                                  bool&);                  // return value: pass message downstream ?
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_HTMLParser ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_HTMLParser (const Test_I_Stream_HTMLParser&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_HTMLParser& operator= (const Test_I_Stream_HTMLParser&))

  // helper methods
  virtual bool initializeSAXParser ();

  bool                          isInitial_; // retrieve request URL ?
  Test_I_StockRecordsIterator_t iterator_;
  Test_I_StockRecordsIterator_t iterator_2_;
  Test_I_SAXParserContext       context_; // retrieve request URL
  Test_I_SAXParserContext       context_2; // retrieve stock data
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Trending_SessionData,                 // session data type
                              enum Stream_SessionMessageType,                     // session event type
                              struct Test_I_Trending_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_html_parser_module_name_string,
                              Stream_INotify_t,                                   // stream notification interface type
                              Test_I_Stream_HTMLParser);                          // writer type

#endif
