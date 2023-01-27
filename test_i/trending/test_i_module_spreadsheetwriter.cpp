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

#include "test_i_module_spreadsheetwriter.h"

#include "ace/Log_Msg.h"

#include "rtl/bootstrap.h"

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/document/MacroExecMode.hpp"
#include "com/sun/star/frame/Desktop.hpp"
#include "com/sun/star/frame/FrameSearchFlag.hpp"
#include "com/sun/star/frame/XComponentLoader.hpp"
#include "com/sun/star/frame/XDesktop2.hpp"
#include "com/sun/star/frame/XStorable.hpp"
#include "com/sun/star/lang/XComponent.hpp"
//#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include "com/sun/star/sheet/XCalculatable.hpp"
#include "com/sun/star/sheet/XSpreadsheet.hpp"
#include "com/sun/star/table/XCell.hpp"
#include "com/sun/star/table/XCellRange.hpp"

#include "common_file_tools.h"

#include "common_timer_tools.h"

#include "stream_macros.h"

#include "stream_document_defines.h"
#include "stream_document_tools.h"

#include "test_i_trending_defines.h"

Test_I_Stream_DocumentHandler::Test_I_Stream_DocumentHandler ()
 : inherited ()
 , resolver_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_DocumentHandler::Test_I_Stream_DocumentHandler"));

}

void
Test_I_Stream_DocumentHandler::initialize (uno::Reference<uno::XComponentContext>& context_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_DocumentHandler::initialize"));

  //// sanity check(s)
  //ACE_ASSERT (context_in.is ());

  //bool result = false;
  //uno::Reference<lang::XMultiComponentFactory> multi_component_factory_p =
  //  context_in->getServiceManager ();
  //ACE_ASSERT (multi_component_factory_p.is ());
  //result =
  //  resolver_.set (multi_component_factory_p->createInstanceWithContext (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.task.InteractionRequestStringResolver")),
  //                                                                       context_in),
  //                 uno::UNO_QUERY);
  //ACE_ASSERT (result);
  //ACE_ASSERT (resolver_.is ());
}

void SAL_CALL
Test_I_Stream_DocumentHandler::handle (const uno::Reference<task::XInteractionRequest>& request_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_DocumentHandler::handle"));

  //// sanity check(s)
  //ACE_ASSERT (resolver_.is ());

  //beans::Optional<rtl::OUString> message_string =
  //  resolver_->getStringFromInformationalRequest (request_in);
  //if (message_string.IsPresent)
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("document handler: \"%s\"...\n"),
  //              ACE_TEXT (::rtl::OUStringToOString (message_string.Value,
  //                                                  RTL_TEXTENCODING_ASCII_US,
  //                                                  OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));

  uno::Sequence<uno::Reference<task::XInteractionContinuation> > continuations_sequence =
    request_in->getContinuations ();
  uno::Any request_any = request_in->getRequest ();
  ACE_ASSERT (request_any.hasValue ());
  const uno::Exception* exception_p =
    static_cast<const uno::Exception*> (request_any.getValue ());
  ACE_ASSERT (exception_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("document handler: \"%s\"...\n"),
              ACE_TEXT (::rtl::OUStringToOString (exception_p->Message,
                                                  RTL_TEXTENCODING_ASCII_US,
                                                  OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
}

//////////////////////////////////////////

Test_I_Stream_SpreadsheetWriter::Test_I_Stream_SpreadsheetWriter (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , document_ ()
 , handler_2 (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_SpreadsheetWriter::Test_I_Stream_SpreadsheetWriter"));

//  ACE_NEW_NORETURN (handler_,
//                    Test_I_Stream_DocumentHandler ());
  handler_2 = new Test_I_Stream_DocumentHandler ();
  if (!handler_2)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  handler_2->acquire ();
  bool result = inherited::interactionHandler_.set (*handler_2,
                                                    uno::UNO_QUERY);
  ACE_ASSERT (inherited::interactionHandler_.is ());
  ACE_ASSERT (result);

  inherited::releaseHandler_ = false;
}

Test_I_Stream_SpreadsheetWriter::~Test_I_Stream_SpreadsheetWriter ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_SpreadsheetWriter::~Test_I_Stream_SpreadsheetWriter"));

  if (document_.is ())
    document_.clear ();
  if (handler_2)
  {
    handler_2->release (); handler_2 = NULL;
//    delete handler_2;
  } // end IF
}

void
Test_I_Stream_SpreadsheetWriter::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_SpreadsheetWriter::handleSessionMessage"));

//  int result = -1;
  oslProcessError result_2 = osl_Process_E_InvalidError;
  ::osl::FileBase::RC result_3 = ::osl::FileBase::RC::E_invalidError;
  bool result_4 = false;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ::rtl::OUString filename, working_directory, filename_url;
  ::rtl::OUString absolute_filename_url;
  result_2 = osl_getProcessWorkingDir (&working_directory.pData);
  ACE_ASSERT (result_2 == osl_Process_E_None);
  uno::Sequence<beans::PropertyValue> document_properties;

  const Test_I_Trending_SessionData_t& session_data_container_r =
    message_inout->getR ();
  struct Test_I_Trending_SessionData& session_data_r =
    const_cast<struct Test_I_Trending_SessionData&> (session_data_container_r.getR ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      //// sanity check(s)
      //ACE_ASSERT (inherited::configuration_->socketConfigurations);
      //ACE_ASSERT (!inherited::configuration_->socketConfigurations->empty ());

      uno::Reference<lang::XMultiComponentFactory> multi_component_factory_p = NULL; // local
      std::string connection_string = ACE_TEXT_ALWAYS_CHAR ("uno:socket,host=");
      std::ostringstream converter;
      ::rtl::OUString connection_string_2;
      uno::Reference<bridge::XUnoUrlResolver> url_resolver_p = NULL;
      uno::Reference<frame::XDesktop2> desktop_p = NULL;
      uno::Reference<frame::XComponentLoader> component_loader_p = NULL;
      uno::Reference<beans::XPropertySet> property_set_p = NULL;
      uno::Reference<sheet::XSpreadsheets> spreadsheets_p = NULL;

      // --> create new frame (see below)
      ::rtl::OUString target_frame_name (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_FRAME_BLANK)));
      const char* result_p = NULL;
      sal_Int32 search_flags = frame::FrameSearchFlag::AUTO;
      document_properties.realloc (3);
      document_properties[0].Name =
          ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_HIDDEN)));
      document_properties[0].Value <<= true;
      document_properties[1].Name =
        ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_INTERACTIONHANDLER)));
      document_properties[1].Value = makeAny (inherited::interactionHandler_);
      document_properties[2].Name =
        ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_MACROEXECCUTIONMODE)));
      document_properties[2].Value <<=
        document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;


      std::string filename_string =
        Common_Tools::environment (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_ENV_UNO_HOME));
      filename_string += ACE_DIRECTORY_SEPARATOR_STR;
      filename_string +=
        ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_DEFAULT_LIBREOFFICE_FILE_UNO_INI);
      filename = ::rtl::OUString::createFromAscii (filename_string.c_str ());

      //filename = ::rtl::OUString::createFromAscii (inherited::configuration_->libreOfficeRc.c_str ());
      result_3 = ::osl::FileBase::getFileURLFromSystemPath (filename,
                                                            filename_url);
      ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);
      result_3 = ::osl::FileBase::getAbsoluteFileURL (working_directory,
                                                      filename_url,
                                                      absolute_filename_url);
      ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);
//      registry_p->open (absolute_filename_url,
//                        sal_True,
//                        sal_False);
//      ::rtl::Bootstrap::set (::rtl::OUString::createFromAscii (ACE_TEXT_ALWAYS_CHAR ("URE_MORE_TYPES")),
//                             ::rtl::Bootstrap::encode (absolute_filename_url));
      //rtl_bootstrap_setIniFileName (absolute_filename_url.pData);
  //    try {
  //      result_4 =
      inherited::componentContext_.set (::cppu::defaultBootstrap_InitialComponentContext (absolute_filename_url),
  ////          component_context_p.set (::cppu::bootstrap_InitialComponentContext (registry_p),
                                        uno::UNO_QUERY);
  //    } catch (::uno::Exception& exception_in) {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("%s: caught exception in ::cppu::defaultBootstrap_InitialComponentContext (): \"%s\", aborting\n"),
  //                  inherited::mod_->name (),
  //                  ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
  //                                                      RTL_TEXTENCODING_ASCII_US,
  //                                                      OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
  //      goto error;
  //    }
      ACE_ASSERT (inherited::componentContext_.is ());
//      ACE_ASSERT (result_4);

      result_4 =
        multi_component_factory_p.set (inherited::componentContext_->getServiceManager (),
                                       uno::UNO_QUERY);
      ACE_ASSERT (multi_component_factory_p.is ());
      ACE_ASSERT (result_4);

      // debug info
      ACE_TCHAR host_address[BUFSIZ];
      ACE_OS::memset (host_address, 0, sizeof (host_address));
      result_p =
        inherited::configuration_->libreOfficeHost.get_host_addr (host_address,
                                                                  sizeof (host_address));
      if (!result_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_INET_Addr::get_host_addr(%s): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::configuration_->libreOfficeHost).c_str ())));
        notify (STREAM_SESSION_MESSAGE_ABORT);
        return;
      } // end IF
      connection_string += ACE_TEXT_ALWAYS_CHAR (host_address);
      connection_string += ACE_TEXT_ALWAYS_CHAR (",port=");
      converter <<
        inherited::configuration_->libreOfficeHost.get_port_number ();
      connection_string += converter.str ();
      connection_string +=
        ACE_TEXT_ALWAYS_CHAR (",tcpNoDelay=1;urp;StarOffice.ServiceManager");
      connection_string_2 =
        ::rtl::OUString::createFromAscii (ACE_TEXT_ALWAYS_CHAR (connection_string.c_str ()));
      result_4 =
        url_resolver_p.set (multi_component_factory_p->createInstanceWithContext (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR ("com.sun.star.bridge.UnoUrlResolver"))),
                                                                                  inherited::componentContext_),
                            uno::UNO_QUERY);
      ACE_ASSERT (url_resolver_p.is ());
      ACE_ASSERT (result_4);
      //uno::Reference<lang::XComponent>::query (multi_component_factory_p)->dispose ();
      try {
        result_4 =
          property_set_p.set (url_resolver_p->resolve (connection_string_2),
                              uno::UNO_QUERY);
      } catch (uno::Exception& exception_in) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in XUnoUrlResolver::resolve(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (::rtl::OUStringToOString (connection_string_2,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ()),
                    ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
        notify (STREAM_SESSION_MESSAGE_ABORT);
        return;
      }
      ACE_ASSERT (property_set_p.is ());
      ACE_ASSERT (result_4);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened LibreOffice connection (was: \"%s\")...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (::rtl::OUStringToOString (connection_string_2,
                                                      RTL_TEXTENCODING_ASCII_US,
                                                      OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
      property_set_p->getPropertyValue (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_DEFAULT_CONTEXT)))) >>=
                                        inherited::componentContext_;
      ACE_ASSERT (inherited::componentContext_.is ());
      ACE_ASSERT (handler_2);
      handler_2->initialize (inherited::componentContext_);

//      result_4 =
//        multi_component_factory_p.set (inherited::componentContext_->getServiceManager (),
//                                       uno::UNO_QUERY);
//      ACE_ASSERT (multi_component_factory_p.is () && result_4);
//      desktop_p =
//          multi_component_factory_p->createInstanceWithContext (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR ("com.sun.star.frame.Desktop"))),
//                                                                inherited::componentContext_);
      desktop_p = ::com::sun::star::frame::Desktop::create (inherited::componentContext_);
      ACE_ASSERT (desktop_p.is ());
      result_4 =
        component_loader_p.set (desktop_p,
                                uno::UNO_QUERY);
      ACE_ASSERT (component_loader_p.is () && result_4);

      // generate document filename URL
      if (Common_File_Tools::isReadable (inherited::configuration_->fileName))
      {
        filename =
          ::rtl::OUString::createFromAscii (inherited::configuration_->fileName.c_str ());
        result_3 = ::osl::FileBase::getFileURLFromSystemPath (filename,
                                                              filename_url);
        ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);
        result_3 = ::osl::FileBase::getAbsoluteFileURL (working_directory,
                                                        filename_url,
                                                        absolute_filename_url);
        ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);
      } // end IF
      else
        absolute_filename_url =
          ::rtl::OUString::createFromAscii (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_FRAME_SPREADSHEET_NEW)); // <-- new file
      try {
        inherited::component_ =
            component_loader_p->loadComponentFromURL (absolute_filename_url, // URL
                                                      //rtl::OUString::createFromAscii("private:factory/scalc"),
                                                      target_frame_name,     // target frame name
                                                      search_flags,          // search flags
                                                      document_properties);  // properties
      } catch (uno::Exception& exception_in) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in XComponentLoader::loadComponentFromURL(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (::rtl::OUStringToOString (absolute_filename_url,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ()),
                    ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
        notify (STREAM_SESSION_MESSAGE_ABORT);
        return;
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in XComponentLoader::loadComponentFromURL(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (::rtl::OUStringToOString (absolute_filename_url,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
        notify (STREAM_SESSION_MESSAGE_ABORT);
        return;
      }
      ACE_ASSERT (inherited::component_.is ());
      uno::Reference<sheet::XSpreadsheetDocument> xSpreadsheetDocument (inherited::component_, uno::UNO_QUERY);
      result_4 = document_.set (inherited::component_,
                                uno::UNO_QUERY);
      ACE_ASSERT (document_.is () && result_4);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: loaded LibreOffice spreadsheet document (was: \"%s\")...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (::rtl::OUStringToOString (absolute_filename_url,
                                                      RTL_TEXTENCODING_ASCII_US,
                                                      OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
      try {
        spreadsheets_p = xSpreadsheetDocument->getSheets ();
      } catch (uno::Exception& exception_in) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in XSpreadSheetDocument::getSheets(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
        goto error;
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in XSpreadSheetDocument::getSheets(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      }
      ACE_ASSERT (spreadsheets_p.is () && result_4);

      break;

error:
      notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      uno::Reference<sheet::XSpreadsheets> spreadsheets_p;
      //uno::Reference<container::XIndexAccess> index_p;
      uno::Reference<container::XNameAccess> name_p;
      uno::Any any_p;
      uno::Reference<sheet::XSpreadsheet> sheet_p;
      uno::Reference<frame::XStorable> storable_p;
      uno::Reference<table::XCell> cell_p;
      ::rtl::OUString cell_value_string;
      unsigned int column;
      unsigned int row;
      uno::Reference<table::XCellRange> cell_range_p;
      Test_I_StockRecordsIterator_t iterator = session_data_r.data.begin ();
      uno::Reference<sheet::XCalculatable> calculatable_p;
      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      std::string timestamp_string;

      // sanity check(s)
      if (!document_.is ())
        goto continue_;

      // load worksheet collection
      try {
        spreadsheets_p = document_->getSheets ();
      }
      catch (com::sun::star::uno::Exception& exception_in) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in XSpreadsheetDocument::getSheets(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
                                                        RTL_TEXTENCODING_ASCII_US,
                                                        OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
        goto error_2;
      }
      ACE_ASSERT (spreadsheets_p.is ());
      //result_4 = index_p.set (spreadsheets_p,
      //                        uno::UNO_QUERY);
      result_4 = name_p.set (spreadsheets_p,
                             uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      //ACE_ASSERT (index_p.is ());
      ACE_ASSERT (name_p.is ());

      // load first worksheet
      //any_p = index_p->getByIndex (0);
      any_p =
        name_p->getByName (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (ACE_TEXT_ALWAYS_CHAR (STREAM_DOCUMENT_LIBREOFFICE_CALC_DEFAULT_TABLE_NAME))));
      ACE_ASSERT (any_p.hasValue ());
      any_p >>= sheet_p;
      ACE_ASSERT (sheet_p.is ());

      // retrieve a cell range interface handle
      result_4 = cell_range_p.set (sheet_p,
                                   uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      ACE_ASSERT (cell_range_p.is ());

      // set index data
      timestamp_string =
        Common_Timer_Tools::timestampToString ((*iterator).timeStamp,
                                               false);
      ACE_ASSERT (!timestamp_string.empty ());

      column = 0;
      row = inherited::configuration_->libreOfficeSheetStartRow;
      row =
        Stream_Document_Tools::firstFreeRow (cell_range_p,
                                             inherited::configuration_->libreOfficeSheetStartColumn + 1,
                                             row);
      for (Test_I_StockRecordsIterator_t iterator_2 = session_data_r.data.begin ();
           iterator_2 != session_data_r.data.end ();
           ++iterator_2)
      { ACE_ASSERT ((*iterator_2).item);
        column = inherited::configuration_->libreOfficeSheetStartColumn;
        cell_p =
          cell_range_p->getCellByPosition (column, row);
        ACE_ASSERT (cell_p.is ());
        cell_value_string =
            ::rtl::OUString::createFromAscii ((*iterator_2).item->symbol.c_str ());
        cell_p->setFormula (cell_value_string);

        ++column;
        cell_p =
          cell_range_p->getCellByPosition (column, row);
        ACE_ASSERT (cell_p.is ());
        cell_value_string =
          ::rtl::OUString::createFromAscii ((*iterator_2).item->ISIN.c_str ());
        cell_p->setFormula (cell_value_string);

        ++column;
        cell_p =
          cell_range_p->getCellByPosition (column, row);
        ACE_ASSERT (cell_p.is ());
        cell_p->setFormula (::rtl::OUString::createFromAscii ((*iterator_2).item->WKN.c_str ()));

        ++column;
        cell_p =
          cell_range_p->getCellByPosition (column, row);
        ACE_ASSERT (cell_p.is ());
        cell_p->setValue ((*iterator_2).value);

        ++column;
        cell_p =
          cell_range_p->getCellByPosition (column, row);
        ACE_ASSERT (cell_p.is ());
        cell_p->setValue ((*iterator_2).change);

        ++column;
        cell_p =
          cell_range_p->getCellByPosition (column, row);
        ACE_ASSERT (cell_p.is ());
        cell_p->setFormula (::rtl::OUString::createFromAscii (timestamp_string.c_str ()));

        ++row;
      } // end FOR

      // set date
      if (!session_data_r.data.empty ())
      {
        cell_p =
          cell_range_p->getCellByPosition (TEST_I_LIBREOFFICE_DATE_COLUMN - 1,
                                           TEST_I_LIBREOFFICE_DATE_ROW - 1);
        ACE_ASSERT (cell_p.is ());
        cell_p->setFormula (::rtl::OUString::createFromAscii (timestamp_string.c_str ()));
      } // end IF

      // update any 'dirty' cells
      result_4 = calculatable_p.set (document_,
                                     uno::UNO_QUERY);
      ACE_ASSERT (result_4);
      ACE_ASSERT (calculatable_p.is ());
      calculatable_p->calculate ();

      if (!session_data_r.data.empty ())
      {
        result_4 = storable_p.set (document_,
                                   uno::UNO_QUERY);
        ACE_ASSERT (result_4);
        ACE_ASSERT (storable_p.is ());
        //ACE_ASSERT (!storable_p->isReadonly ());

        bool save_as = true;
        if (Common_File_Tools::isValidFilename (inherited::configuration_->fileIdentifier.identifier))
        {
          filename =
            ::rtl::OUString::createFromAscii (inherited::configuration_->fileIdentifier.identifier.c_str ());
          result_3 = ::osl::FileBase::getFileURLFromSystemPath (filename,
                                                                filename_url);
          ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);
          result_3 = ::osl::FileBase::getAbsoluteFileURL (working_directory,
                                                          filename_url,
                                                          absolute_filename_url);
          ACE_ASSERT (result_3 == ::osl::FileBase::RC::E_None);

          document_properties.realloc (2);
          document_properties[0].Name =
            ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_OVERWRITE));
          document_properties[0].Value <<= true;
          document_properties[1].Name =
            ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (STREAM_DOCUMENT_LIBREOFFICE_PROPERTY_FILE_INTERACTIONHANDLER));
          document_properties[1].Value = makeAny (inherited::interactionHandler_);
        } // end IF
        else
          save_as = false;
        try {
          if (save_as)
            storable_p->storeToURL (absolute_filename_url,
                                    document_properties);
          else
            storable_p->store ();
        } catch (io::IOException exception_in) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in XStorable::store%s(): \"%s\", returning\n"),
                      inherited::mod_->name (),
                      (save_as ? ACE_TEXT ("ToURL") : ACE_TEXT ("")),
                      ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
                                                          RTL_TEXTENCODING_ASCII_US,
                                                          OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
          goto error_2;
        }
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: wrote %u record(s)...\n"),
                    inherited::mod_->name (),
                    session_data_r.data.size ()));
      } // end IF

      document_.clear ();
 
continue_:
error_2:
      if (inherited::component_.is ())
      {
        inherited::component_->dispose ();
        inherited::component_.clear ();
      } // end IF
//      if (inherited::componentContext_.is ())
//      {
//        uno::Reference<lang::XComponent> component_p = NULL;
//        try {
//          component_p =
//            uno::Reference<lang::XComponent>::query (inherited::componentContext_);
//        } catch (com::sun::star::uno::Exception& exception_in) {
//            ACE_DEBUG ((LM_ERROR,
//                        ACE_TEXT ("%s: caught exception in XComponentComtext::query(XComponent): \"%s\", continuing\n"),
//                        inherited::mod_->name (),
//                        ACE_TEXT (::rtl::OUStringToOString (exception_in.Message,
//                                                            RTL_TEXTENCODING_ASCII_US,
//                                                            OUSTRING_TO_OSTRING_CVTFLAGS).getStr ())));
//        }
//        if (component_p.is ())
//          component_p->dispose ();
//      } // end IF
      inherited::componentContext_.clear ();

      break;
    }
    default:
      break;
  } // end SWITCH
}
