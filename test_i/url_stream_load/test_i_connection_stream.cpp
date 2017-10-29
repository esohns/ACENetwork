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
#include "test_i_connection_stream.h"

#include "ace/Log_Msg.h"

#include "net_macros.h"

#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_common.h"
#include "test_i_common_modules.h"
#include "test_i_module_htmlparser.h"

Test_I_ConnectionStream::Test_I_ConnectionStream ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream::Test_I_ConnectionStream"));

}

bool
Test_I_ConnectionStream::load (Stream_ModuleList_t& modules_out,
                               bool& deleteModules_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_ConnectionStream::load"));

  Stream_Module_t* module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_Module_Dump_Module (this,
  //                                           ACE_TEXT_ALWAYS_CHAR ("Dump")),
  //                false);
  //modules_out.push_back (module_p);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Module_HTMLParser_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR ("HTMLParser")),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_StatisticReport_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
                  false);
  modules_out.push_back (module_p);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR ("Marshal")),
                  false);
  modules_out.push_back (module_p);
//  module_p = NULL;
//  ACE_NEW_RETURN (module_p,
//                  Test_I_Net_IO_Module (ACE_TEXT_ALWAYS_CHAR ("NetIO")),
//                  false);
//  modules_out.push_back (module_p);

  deleteModules_out = true;

  return true;
}

bool
Test_I_ConnectionStream::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
                                     ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isRunning ());

//  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;
  struct Test_I_URLStreamLoad_SessionData* session_data_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator;
  Stream_Module_t* module_p = NULL;
//  Test_I_Net_Writer_t* netIO_impl_p = NULL;
  Test_I_HTTPParser* parser_impl_p = NULL;

  // allocate a new session state, reset stream
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in,
                              handle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Net_IO_Stream_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto failed;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  session_data_p =
    &const_cast<struct Test_I_URLStreamLoad_SessionData&> (inherited::sessionData_->getR ());
  iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  // *TODO*: remove type inferences
  //session_data_p->sessionID = configuration_in.sessionID;
  session_data_p->targetFileName = (*iterator).second.targetFileName;

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  // ******************* Net IO ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("Marshal")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT ("Marshal")));
    goto failed;
  } // end IF
  parser_impl_p = dynamic_cast<Test_I_HTTPParser*> (module_p->writer ());
  if (!parser_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: dynamic_cast<HTTP_Module_ParserH_T> failed, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (module_p->name ())));
    goto failed;
  } // end IF
  parser_impl_p->setP (&(inherited::state_));

  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup (configuration_in.configuration_.notificationStrategy))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  //inherited::allocator_ = configuration_in.messageAllocator;

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}

bool
Test_I_ConnectionStream::collect (HTTP_Statistic_t& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  struct Test_I_URLStreamLoad_SessionData& session_data_r =
    const_cast<struct Test_I_URLStreamLoad_SessionData&> (inherited::sessionData_->getR ());

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("StatisticReport")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("StatisticReport")));
    return false;
  } // end IF
  Test_I_StatisticReport_WriterTask_t* statistic_impl_p =
    dynamic_cast<Test_I_StatisticReport_WriterTask_t*> (module_p->writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Stream_Module_StatisticReport_WriterTask_T> failed, aborting\n")));
    return false;
  } // end IF

    // synch access
  if (session_data_r.lock)
  {
    result = session_data_r.lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  session_data_r.statistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistics module
  bool result_2 = false;
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic_T::collect(), continuing\n")));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IStatistic_T::collect(), aborting\n")));
  else
    session_data_r.statistic = data_out;

  if (session_data_r.lock)
  {
    result = session_data_r.lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result_2;
}
