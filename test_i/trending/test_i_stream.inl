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

#include "stream_macros.h"

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include "test_i_module_msoffice_spreadsheetwriter.h"
//#endif // ACE_WIN32 || ACE_WIN64

template <typename ConnectorType>
Test_I_Trending_Stream_T<ConnectorType>::Test_I_Trending_Stream_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Trending_Stream_T::Test_I_Trending_Stream_T"));

}

template <typename ConnectorType>
bool
Test_I_Trending_Stream_T<ConnectorType>::load (Stream_ILayout* layout_in,
                                              bool& delete_out)

{
  STREAM_TRACE (ACE_TEXT ("Test_I_Trending_Stream_T::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_HTTPMarshal_Module (this,
                                             ACE_TEXT_ALWAYS_CHAR ("HTTPMarshal")),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                Test_I_StatisticReport_Module (this,
  //                                               ACE_TEXT_ALWAYS_CHAR ("StatisticReport")),
  //                false);
  //modules_out.push_back (module_p);
  //module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_ZIPDecompressor_Module (this,
                                                 ACE_TEXT_ALWAYS_CHAR ("ZIPDecompressor")),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  SOURCE_MODULE_T (this,
                                   ACE_TEXT_ALWAYS_CHAR ("NetSource")),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_HTTPGet_Module (this,
                                                ACE_TEXT_ALWAYS_CHAR ("HTTPGet")),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_HTMLParser_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR ("HTMLParser")),
                  false);
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_SpreadsheetWriter_Module (this,
                //Test_I_MSOffice_SpreadsheetWriter_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR ("SpreadsheetWriter")),
                  false);
#else
  ACE_NEW_RETURN (module_p,
                  Test_I_Stream_SpreadsheetWriter_Module (this,
                                                          ACE_TEXT_ALWAYS_CHAR ("SpreadsheetWriter")),
                  false);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (module_p);
  layout_in->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

template <typename ConnectorType>
bool
Test_I_Trending_Stream_T<ConnectorType>::initialize (const Test_I_Trending_StreamConfiguration_t& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Trending_Stream_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isRunning ());

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;

  if (inherited::isInitialized_)
  {
    if (!inherited::reset ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                  ACE_TEXT (stream_name_string_)));
  } // end IF

  // allocate a new session state, reset stream
  const_cast<Test_I_Trending_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    return false;
  } // end IF
  const_cast<Test_I_Trending_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;
  ACE_ASSERT (inherited::sessionData_);
  struct Test_I_Trending_SessionData& session_data_r =
      const_cast<struct Test_I_Trending_SessionData&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  typename inherited::CONFIGURATION_T::ITERATOR_T iterator =
      const_cast<Test_I_Trending_StreamConfiguration_t&> (configuration_in).find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.end ());
  session_data_r.targetFileName = (*iterator).second.second->fileIdentifier.identifier;

  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration_->setupPipeline)
    if (!inherited::setup ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to set up pipeline, aborting\n"),
                  ACE_TEXT (stream_name_string_)));
      goto failed;
    } // end IF

  // -------------------------------------------------------------

  inherited::isInitialized_ = true;

  return true;

failed:
  if (reset_setup_pipeline)
    const_cast<Test_I_Trending_StreamConfiguration_t&> (configuration_in).configuration_->setupPipeline =
      setup_pipeline;
  if (!inherited::reset ())
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::reset(): \"%m\", continuing\n"),
                ACE_TEXT (stream_name_string_)));

  return false;
}

template <typename ConnectorType>
bool
Test_I_Trending_Stream_T<ConnectorType>::collect (struct Stream_Statistic& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Trending_Stream_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  Test_I_Trending_SessionData& session_data_r =
      const_cast<Test_I_Trending_SessionData&> (inherited::sessionData_->getR ());

  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("StatisticReport")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("StatisticReport")));
    return false;
  } // end IF
  Test_I_Statistic_WriterTask_t* statistic_report_impl_p =
    dynamic_cast<Test_I_Statistic_WriterTask_t*> (module_p->writer ());
  if (!statistic_report_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_I_Statistic_WriterTask_t*> failed, aborting\n")));
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

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_report_impl_p->collect (data_out);
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

template <typename ConnectorType>
void
Test_I_Trending_Stream_T<ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Trending_Stream_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
