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

#include "bittorrent_client_stream.h"

BitTorrent_Client_Stream::BitTorrent_Client_Stream (const std::string& name_in)
 : inherited (name_in)
 //, marshal_ (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
 //            NULL,
 //            false)
 //, parser_ (ACE_TEXT_ALWAYS_CHAR ("Parser"),
 //           NULL,
 //           false)
 //, runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
 //                     NULL,
 //                     false)
//, handler_ (ACE_TEXT_ALWAYS_CHAR ("Handler"),
//            NULL,
//            false)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_Stream::BitTorrent_Client_Stream"));

  // remember the ones we "own"...
  // *TODO*: clean this up
  // *NOTE*: one problem is that we need to explicitly close() all
  // modules which we have NOT enqueued onto the stream (e.g. because init()
  // failed...)
  //inherited::availableModules_.push_front (&marshal_);
  //inherited::availableModules_.push_front (&parser_);
  //inherited::availableModules_.push_front (&runtimeStatistic_);

  // *TODO*: fix ACE bug: modules should initialize their "next" member to NULL
  //inherited::MODULE_T* module_p = NULL;
  //for (ACE_DLList_Iterator<inherited::MODULE_T> iterator (inherited::availableModules_);
  //     iterator.next (module_p);
  //     iterator.advance ())
  //  module_p->next (NULL);
  for (Stream_ModuleListIterator_t iterator = inherited::modules_.begin ();
       iterator != inherited::modules_.end ();
       iterator++)
    (*iterator)->next (NULL);
}

BitTorrent_Client_Stream::~BitTorrent_Client_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_Stream::~BitTorrent_Client_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction
  inherited::shutdown ();
}

bool
BitTorrent_Client_Stream::initialize (const struct BitTorrent_Client_StreamConfiguration& configuration_in,
                                      bool setupPipeline_in,
                                      bool resetSessionData_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);
  ACE_ASSERT (!isRunning ());

  // allocate a new session state, reset stream
  if (!inherited::initialize (configuration_in,
                              false,
                              resetSessionData_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
//  ACE_ASSERT (inherited::sessionData_);

  // things to be done here:
  // - create modules (done for the ones "owned" by the stream itself)
  // - initialize modules
  // - push them onto the stream (tail-first) !
//  BitTorrent_Client_SessionData& session_data_r =
//      const_cast<BitTorrent_Client_SessionData&> (inherited::sessionData_->get ());
//  session_data_r.sessionID = configuration_in.sessionID;

//  ACE_ASSERT (configuration_in.moduleConfiguration);
//  configuration_in.moduleConfiguration->streamState = &inherited::state_;

  // ---------------------------------------------------------------------------

  //// ******************* Runtime Statistics ************************
  //IRC_Module_Statistic_WriterTask_t* runtimeStatistic_impl_p =
  //  dynamic_cast<IRC_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  //if (!runtimeStatistic_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<IRC_Module_Statistic_WriterTask_t> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!runtimeStatistic_impl_p->initialize (configuration_in.statisticReportingInterval,
  //                                          configuration_in.messageAllocator))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              runtimeStatistic_.name ()));
  //  return false;
  //} // end IF

//   // ******************* IRC Handler ************************
//   IRC_Module_IRCHandler* handler_impl =
//     dynamic_cast<IRC_Module_IRCHandler*> (handler_.writer());
//   if (!handler_impl)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("dynamic_cast<IRC_Module_IRCHandler> failed, aborting\n")));
//     return false;
//   } // end IF
//   if (!IRCHandler_impl->init(configuration_in.messageAllocator,
//                              (configuration_in.clientPingInterval ? false // servers shouldn't receive "pings" in the first place
//                                                                   : NET_DEF_CLIENT_PING_PONG), // auto-answer "ping" as a client ?...
//                              (configuration_in.clientPingInterval == 0))) // clients print ('.') dots for received "pings"...
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to initialize module: \"%s\", aborting\n"),
//                handler_.name()));
//     return false;
//   } // end IF

  //// ******************* Parser ************************
  //IRC_Module_Parser* parser_impl_p =
  //  dynamic_cast<IRC_Module_Parser*> (parser_.writer ());
  //if (!parser_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<IRC_Module_IRCParser> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!parser_impl_p->initialize (configuration_in.messageAllocator,                            // message allocator
  //                                configuration_in.moduleHandlerConfiguration_2.crunchMessages, // "crunch" messages ?
  //                                configuration_in.moduleHandlerConfiguration_2.traceScanning,  // debug scanner ?
  //                                configuration_in.moduleHandlerConfiguration_2.traceParsing))  // debug parser ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              parser_.name ()));
  //  return false;
  //} // end IF

  //// ******************* Marshal ************************
  //IRC_Module_Bisector* bisector_impl_p =
  //  dynamic_cast<IRC_Module_Bisector*> (marshal_.writer ());
  //if (!bisector_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<IRC_Module_IRCSplitter> failed, aborting\n")));
  //  return false;
  //} // end IF
  //if (!bisector_impl_p->initialize (configuration_in.moduleHandlerConfiguration_2))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              marshal_.name ()));
  //  return false;
  //} // end IF
  //if (!bisector_impl_p->initialize (inherited::state_))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
  //              marshal_.name ()));
  //  return false;
  //} // end IF

  //// enqueue the module...
  //// *NOTE*: push()ing the module will open() it
  ////         --> set the argument that is passed along (head module expects a
  ////             handle to the session data)
  //marshal_.arg (inherited::sessionData_);

//  if (setupPipeline_in)
//    if (!inherited::setup ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
//      return false;
//    } // end IF

  // set (session) message allocator
  // *TODO*: clean this up ! --> sanity check
//  ACE_ASSERT (configuration_in.messageAllocator);
//  inherited::allocator_ = configuration_in.messageAllocator;

//  inherited::isInitialized_ = true;

  return true;
}

//bool
//BitTorrent_Client_Stream::collect (IRC_RuntimeStatistic_t& data_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_Stream::collect"));
//
//  IRC_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
//  runtimeStatistic_impl =
//    dynamic_cast<IRC_Module_Statistic_WriterTask_t*> (const_cast<IRC_Module_RuntimeStatistic_Module&> (runtimeStatistic_).writer ());
//  if (!runtimeStatistic_impl)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<IRC_Module_Statistic_WriterTask_t> failed, aborting\n")));
//    return false;
//  } // end IF
//
//  // delegate to this module...
//  return (runtimeStatistic_impl->collect (data_out));
//}
//
//void
//BitTorrent_Client_Stream::report () const
//{
//  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_Stream::report"));
//
////   Net_Module_RuntimeStatistic* runtimeStatistic_impl = NULL;
////   runtimeStatistic_impl = dynamic_cast<Net_Module_RuntimeStatistic*> (//                                            myRuntimeStatistic.writer());
////   if (!runtimeStatistic_impl)
////   {
////     ACE_DEBUG((LM_ERROR,
////                ACE_TEXT("dynamic_cast<Net_Module_RuntimeStatistic> failed, aborting\n")));
////
////     return;
////   } // end IF
////
////   // delegate to this module...
////   return (runtimeStatistic_impl->report());
//
//  // just a dummy
//  ACE_ASSERT (false);
//
//  ACE_NOTREACHED (return;)
//}
