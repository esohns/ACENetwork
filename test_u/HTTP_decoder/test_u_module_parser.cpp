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

#include "test_i_module_databasewriter.h"

#include "stream_macros.h"

#include "stream_module_db_tools.h"

Test_I_Stream_Module_DataBaseWriter::Test_I_Stream_Module_DataBaseWriter ()
 : inherited ()
 //, commit_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_DataBaseWriter::Test_I_Stream_Module_DataBaseWriter"));

}

Test_I_Stream_Module_DataBaseWriter::~Test_I_Stream_Module_DataBaseWriter ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_DataBaseWriter::~Test_I_Stream_Module_DataBaseWriter"));

}

void
Test_I_Stream_Module_DataBaseWriter::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_DataBaseWriter::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  const Test_I_Stream_SessionData_t& session_data_container_r =
    message_inout->get ();
  Test_I_Stream_SessionData& session_data_r =
    const_cast<Test_I_Stream_SessionData&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::state_);

      my_bool result_2 = mysql_thread_init ();
      if (result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_thread_init(), aborting\n")));

        session_data_r.aborted = true;

        return;
      } // end IF

      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result =
        inherited::configuration_.loginOptions.host.addr_to_string (buffer,
                                                                    sizeof (buffer));
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", aborting\n")));

        session_data_r.aborted = true;

        return;
      } // end IF
      ACE_TCHAR host_address[BUFSIZ];
      ACE_OS::memset (host_address, 0, sizeof (host_address));
      const char* result_p =
        inherited::configuration_.loginOptions.host.get_host_addr (host_address,
                                                                   sizeof (host_address));
      if (!result_p || (result_p != host_address))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::get_host_addr(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (buffer)));

        session_data_r.aborted = true;

        return;
      } // end IF

      unsigned long client_flags =
        (//CAN_HANDLE_EXPIRED_PASSWORDS | // handle expired passwords
         //CLIENT_COMPRESS              | // use compression protocol
         //CLIENT_FOUND_ROWS            | // return #found (matched) rows
         // instead of #changed rows
         CLIENT_IGNORE_SIGPIPE | // do not install a SIGPIPE handler
         CLIENT_IGNORE_SPACE | // permit spaces after function
                               // names
                               //CLIENT_INTERACTIVE           | // permit interactive_timeout
                               // seconds (instead of wait_timeout
                               // seconds) of inactivity
                               //CLIENT_LOCAL_FILES           | // enable LOAD_LOCAL_DATA handling
         CLIENT_MULTI_RESULTS | // client can handle multiple result
                                // sets from multiple-statement
                                // executions/stored procedures
                                //CLIENT_MULTI_STATEMENTS      | // client may send multiple
                                // statements in a single string
                                // (separated by ';')
                                //CLIENT_NO_SCHEMA             | // do not permit
                                // 'db_name.tbl_name.col_name'
                                // syntax (ODBC)
                                //CLIENT_ODBC                  | // unused
                                //CLIENT_SSL                   | // use SSL. Do NOT set this
                                // manually, use mysql_ssl_set()
                                // instead
         CLIENT_REMEMBER_OPTIONS);        // remember options specified by
                                          // calls to mysql_options()
      const char* host_p =
        (inherited::configuration_.loginOptions.host.is_loopback () ? NULL // localhost ([Unix]socket file/[Win32]shared memory : TCP) : options file (?)
                                                                    : ACE_TEXT_ALWAYS_CHAR (host_address));
        //(inherited::configuration_.loginOptions.host.is_loopback () ? NULL // localhost ([Unix]socket file/[Win32]shared memory : TCP) : options file (?)
        //                                                            : ACE_TEXT_ALWAYS_CHAR (host_address));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (inherited::configuration_.loginOptions.useNamedPipe)
        host_p = ACE_TEXT_ALWAYS_CHAR (".");
#endif
      const char* user_name_string_p =
        (inherited::configuration_.loginOptions.user.empty () ? NULL // current user (Unix) : options file (?)
                                                              : inherited::configuration_.loginOptions.user.c_str ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *NOTE*: passing NULL for host crashes the program
      //         --> pass root instead
      if (!user_name_string_p)
        user_name_string_p = ACE_TEXT_ALWAYS_CHAR (MODULE_DB_MYSQL_DEFAULT_USER);
#endif
      const char* password_string_p =
        (inherited::configuration_.loginOptions.password.empty () ? NULL // (user table ?, options file (?))
                                                                  : inherited::configuration_.loginOptions.password.c_str ());
      const char* database_name_string_p =
        (inherited::configuration_.loginOptions.database.empty () ? NULL // default database : options file (?)
                                                                  : inherited::configuration_.loginOptions.database.c_str ());
      MYSQL* result_3 =
        mysql_real_connect (inherited::state_,      // state handle
                            host_p,                 // host name/address
                            user_name_string_p,     // db user
                            password_string_p,      // db password (non-encrypted)
                            database_name_string_p, // db database
                            inherited::configuration_.loginOptions.host.get_port_number (), // port
                            NULL,                   // (UNIX) socket
                            client_flags);          // client flags
      if (result_3 != inherited::state_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_real_connect(\"%s\",\"%s\",\"%s\",\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT (host_p),
                    ACE_TEXT (user_name_string_p),
                    ACE_TEXT (password_string_p),
                    ACE_TEXT (database_name_string_p),
                    ACE_TEXT (mysql_error (inherited::state_))));

        session_data_r.aborted = true;

        return;
      } // end IF
        //      result = mysql_ping ();
        //      if (result)
        //      {
        //        ACE_DEBUG ((LM_ERROR,
        //                    ACE_TEXT ("failed to mysql_ping(): \"%s\", aborting\n"),
        //                    ACE_TEXT (mysql_error (&mysql))));

        //        session_data_r.aborted = true;

        //        return;
        //      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("opened db connection to \"%s\"...\n"),
                  ACE_TEXT (buffer)));

      result =
        mysql_set_character_set (inherited::state_,
                                 ACE_TEXT_ALWAYS_CHAR (MODULE_DB_MYSQL_DEFAULT_CHARACTER_SET));
      if (result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_set_character_set(\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT (MODULE_DB_MYSQL_DEFAULT_CHARACTER_SET),
                    ACE_TEXT (mysql_error (inherited::state_))));

        session_data_r.aborted = true;

        return;
      } // end IF

      //      // enable debug messages ?
      //      if (configuration_.debug)
      //      {
      //        /*
      //        Activate debug trace of the MySQL client library (C API)
      //        Only available with a debug build of the MySQL client library!
      //        */
      //        connection_->setClientOption ("libmysql_debug", "d:t:O,client.trace");

      //        /*
      //        Connector/C++ tracing is available if you have compiled the
      //        driver using cmake -DMYSQLCPPCONN_TRACE_ENABLE:BOOL=1
      //        */
      //        int on_off = 1;
      //        connection_->setClientOption ("clientTrace", &on_off);
      //      } // end IF

      break;
    }
    case STREAM_SESSION_END:
    {
      // sanity check(s)
      if (!inherited::state_)
        return; // nothing to do

      my_bool result_2 = mysql_thread_init ();
      if (result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_thread_init(), aborting\n")));

        session_data_r.aborted = true;

        return;
      } // end IF

      std::string query_string = ACE_TEXT_ALWAYS_CHAR ("INSERT INTO ");
      query_string += inherited::configuration_.dataBaseTable;
      query_string += ACE_TEXT_ALWAYS_CHAR (" VALUES (");
      std::string timestamp_string;
      char buffer[BUFSIZ];
      unsigned int number_of_records = 0;
      for (Test_I_PageDataIterator_t iterator = session_data_r.data.pageData.begin ();
           iterator != session_data_r.data.pageData.end ();
           ++iterator)
        for (Test_I_DataItemsIterator_t iterator_2 = (*iterator).second.begin ();
             iterator_2 != (*iterator).second.end ();
             ++iterator_2)
        {
          query_string += ACE_TEXT_ALWAYS_CHAR ("DEFAULT"); // id
          query_string += ACE_TEXT_ALWAYS_CHAR (",'");
          timestamp_string =
            Stream_Module_DataBase_Tools::timeStamp2DataBaseString ((*iterator).first);
          query_string +=
            ACE_TEXT_ALWAYS_CHAR (timestamp_string.c_str ()); // date
          query_string += ACE_TEXT_ALWAYS_CHAR ("','");
          //mysql_real_escape_string_quote (inherited::state_,
          //                                buffer,
          //                                (*iterator_2).URI.c_str (),
          //                                (*iterator_2).URI.size (),
          //                                '\'');
          mysql_escape_string (buffer,
                               (*iterator_2).URI.c_str (),
                               (*iterator_2).URI.size ());
          query_string += buffer; // URI
          query_string += ACE_TEXT_ALWAYS_CHAR ("','");
          //mysql_real_escape_string_quote (inherited::state_,
          //                                buffer,
          //                                (*iterator_2).description.c_str (),
          //                                (*iterator_2).description.size (),
          //                                '\'');
          mysql_escape_string (buffer,
                               (*iterator_2).description.c_str (),
                               (*iterator_2).description.size ());
          query_string += buffer; // description
          query_string += ACE_TEXT_ALWAYS_CHAR ("',");
          query_string += ACE_TEXT_ALWAYS_CHAR ("DEFAULT"); // last_valid

          query_string += ACE_TEXT_ALWAYS_CHAR ("),(");
          ++number_of_records;
        } // end FOR
      query_string.resize (query_string.size () - 2);

      result = mysql_real_query (state_,
                                 query_string.c_str (),
                                 query_string.size ());
      if (result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to mysql_real_query(): \"%s\", aborting\n"),
                    ACE_TEXT (mysql_error (state_))));

        session_data_r.aborted = true;

        goto close;
      } // end IF
      my_ulonglong result_3 = mysql_affected_rows (state_);
      if (result_3 != number_of_records)
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("failed to store %u data record(s) (result was: %u), continuing\n"),
                    number_of_records, result_2));
        goto commit;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": stored %u data record(s)...\n"),
                  ACE_TEXT (session_data_r.data.title.c_str ()),
                  result_3));

commit:
      //my_bool result_3 = mysql_commit (state_);
      //if (result_3)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to mysql_commit(): \"%s\", aborting\n"),
      //              ACE_TEXT (mysql_error (state_))));

      //  session_data_r.aborted = true;

      //  goto close;
      //} // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("committed %u data record(s)...\n"),
      //            session_data_r.data.pageData.size (), result_2));

close:
      mysql_close (inherited::state_);
      inherited::state_ = NULL;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed db connection...\n")));

      break;
    }
    default:
      break;
  } // end SWITCH
}
