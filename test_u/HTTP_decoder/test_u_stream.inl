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

#include "ace/Log_Msg.h"

#include "stream_file_defines.h"

#include "stream_net_defines.h"

#include "stream_stat_defines.h"

#include "net_macros.h"

#include "http_defines.h"

#include "test_u_common_modules.h"

template <typename TimerManagerType>
Test_U_Stream_T<TimerManagerType>::Test_U_Stream_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream_T::Test_U_Stream_T"));

}

template <typename TimerManagerType>
Test_U_Stream_T<TimerManagerType>::~Test_U_Stream_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream_T::~Test_U_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

template <typename TimerManagerType>
bool
Test_U_Stream_T<TimerManagerType>::load (Stream_ILayout* layout_inout,
                                         bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream_T::load"));

  // initialize return value(s)
  deleteModules_out = false;

  // modules
  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  IO_MODULE_T (this,
                               ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_Module_FileWriter_Module (this,
  //                                                 ACE_TEXT_ALWAYS_CHAR ("FileDump")),
  //                false);
  //modules_out.push_back (module_p);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_Marshal_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR (HTTP_DEFAULT_MODULE_MARSHAL_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_U_Module_StatisticReport_Module (this,
  //                                                      ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //modules_out.push_back (module_p);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_U_Module_FileWriter_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_FILE_SINK_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  deleteModules_out = true;

  return true;
}

template <typename TimerManagerType>
bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_Stream_T<TimerManagerType>::initialize (const CONFIGURATION_T& configuration_in,
#else
Test_U_Stream_T<TimerManagerType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in,
#endif
                                               ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!this->isRunning ());

  if (inherited::isInitialized_)
  {
    if (!inherited::finalize ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::finalize(): \"%m\", continuing\n"),
                  ACE_TEXT (stream_name_string_)));
  } // end IF

  bool result = false;
  bool setup_pipeline = configuration_in.configuration_.setupPipeline;
  bool reset_setup_pipeline = false;

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
    return false;
  } // end IF
  const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  struct Test_U_HTTPDecoder_SessionData& session_data_r =
      const_cast<struct Test_U_HTTPDecoder_SessionData&> (inherited::sessionData_->getR ());
  //session_data_r.sessionId = configuration_in.sessionId;
  // *TODO*: remove type inferences
  //session_data_r.targetFileName =
  //  configuration_in.moduleHandlerConfiguration->targetFileName;

  // *IMPORTANT NOTE*: a connection data processing stream may be appended
  //                   ('outbound' scenario) or prepended ('inbound' (e.g.
  //                   listener-based) scenario) to another stream. In the first
  //                   case, the net io (head) module behaves in a somewhat
  //                   particular manner, as it may be neither 'active' (run a
  //                   dedicated thread) nor 'passive' (borrow calling thread in
  //                   start()). Instead, it can behave as a regular
  //                   synchronous (i.e. passive) module; this reduces the
  //                   thread-count and generally improves efficiency
//  bool reset_configuration = false;
//  enum Stream_HeadModuleConcurrency concurrency_mode;
//  bool is_concurrent;
//  if (!configuration_in.moduleHandlerConfiguration->inbound)
//  {
//    concurrency_mode = configuration_in.moduleHandlerConfiguration->concurrency;
//    is_concurrent = configuration_in.moduleHandlerConfiguration->concurrent;

//    configuration_in.moduleHandlerConfiguration->concurrency =
//      STREAM_HEADMODULECONCURRENCY_CONCURRENT;
//    configuration_in.moduleHandlerConfiguration->concurrent = true;

//    reset_configuration = true;
//  } // end IF

  // ---------------------------------------------------------------------------
  typename inherited::MODULE_T* module_p = NULL;
  READER_T* IOReader_impl_p = NULL;
  WRITER_T* IOWriter_impl_p = NULL;
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
      const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  session_data_r.targetFileName = (*iterator).second.second.targetFileName;

  // ******************* IO ************************
  module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT (stream_name_string_),
                ACE_TEXT (MODULE_NET_IO_DEFAULT_NAME_STRING)));
    goto error;
  } // end IF
  IOWriter_impl_p = dynamic_cast<WRITER_T*> (module_p->writer ());
  if (!IOWriter_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Module_Net_IOWriter_T> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  IOWriter_impl_p->setP (&(inherited::state_));

  IOReader_impl_p = dynamic_cast<READER_T*> (module_p->reader ());
  if (!IOReader_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_Module_Net_IOReader_T> failed, aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  if (!IOReader_impl_p->initialize ((*iterator).second.second))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to initialize Stream_Module_Net_IOReader_T, aborting\n"),
                ACE_TEXT (stream_name_string_),
                module_p->name ()));
    goto error;
  } // end IF
  // *NOTE*: push()ing the module will open() it
  //         --> set the argument that is passed along (head module expects a
  //             handle to the session data)
  module_p->arg (inherited::sessionData_);

  if (configuration_in.configuration_.setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto error;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;
  //inherited::dump_state ();

  result = true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_.setupPipeline =
      setup_pipeline;
//  if (reset_configuration)
//  {
//    configuration_in.moduleHandlerConfiguration->concurrency = concurrency_mode;
//    configuration_in.moduleHandlerConfiguration->concurrent = is_concurrent;
//  } // end IF

  return result;
}
