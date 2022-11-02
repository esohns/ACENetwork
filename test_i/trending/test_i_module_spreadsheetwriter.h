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

#ifndef TEST_I_OO_SPREADSHEETWRITER_H
#define TEST_I_OO_SPREADSHEETWRITER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "sal/types.h"

#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/task/XInteractionRequestStringResolver.hpp"
#include "com/sun/star/sheet/XSpreadsheetDocument.hpp"
#include "com/sun/star/uno/Reference.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_libreoffice_document_handler.h"
#include "stream_module_libreoffice_document_writer.h"

#include "test_i_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_trending_common.h"

using namespace ::com::sun::star;

class Test_I_Stream_DocumentHandler
 : public Stream_Module_LibreOffice_Document_Handler
{
  typedef Stream_Module_LibreOffice_Document_Handler inherited;
  
 public:
  Test_I_Stream_DocumentHandler ();
  inline virtual ~Test_I_Stream_DocumentHandler () {}

  void initialize (uno::Reference<uno::XComponentContext>&);

  // implement XInteractionHandler
  virtual void SAL_CALL handle (const uno::Reference<task::XInteractionRequest>&) /* throw (uno::RuntimeException, ::std::exception) */;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_DocumentHandler (const Test_I_Stream_DocumentHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_DocumentHandler& operator= (const Test_I_Stream_DocumentHandler&))

  uno::Reference<task::XInteractionRequestStringResolver> resolver_;
};

//////////////////////////////////////////

class Test_I_Stream_SpreadsheetWriter
 : public Stream_Module_LibreOffice_Document_Writer_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Trending_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      struct Test_I_Trending_SessionData,
                                                      sheet::XSpreadsheetDocument>
{
  typedef Stream_Module_LibreOffice_Document_Writer_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Trending_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      struct Test_I_Trending_SessionData,
                                                      sheet::XSpreadsheetDocument> inherited;

 public:
  Test_I_Stream_SpreadsheetWriter (ISTREAM_T*); // stream handle
  virtual ~Test_I_Stream_SpreadsheetWriter ();

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (Test_I_Stream_SessionMessage*&, // session message handle
                                     bool&);                         // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_SpreadsheetWriter ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_SpreadsheetWriter (const Test_I_Stream_SpreadsheetWriter&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_SpreadsheetWriter& operator= (const Test_I_Stream_SpreadsheetWriter&))

  uno::Reference<sheet::XSpreadsheetDocument> document_;
  Test_I_Stream_DocumentHandler*              handler_2;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Trending_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_Trending_ModuleHandlerConfiguration,             // module handler configuration type
                              libacestream_default_doc_libreoffice_writer_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_Stream_SpreadsheetWriter);                              // writer type

#endif
