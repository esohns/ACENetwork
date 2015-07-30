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

#ifndef IRC_CLIENT_MODULE_IRCSPLITTER_H
#define IRC_CLIENT_MODULE_IRCSPLITTER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_istatistic.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_statistichandler.h"
#include "stream_streammodule_base.h"

#include "IRC_client_common.h"
#include "IRC_client_configuration.h"
#include "IRC_client_IRCbisect.h"
#include "IRC_client_message.h"
#include "IRC_client_sessionmessage.h"

// forward declaration(s)
class Stream_IAllocator;
//typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;

class IRC_Client_Module_IRCSplitter
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      IRC_Client_SessionMessage,
                                      IRC_Client_Message,
                                      ///
                                      IRC_Client_ModuleHandlerConfiguration,
                                      ///
                                      IRC_Client_StreamState,
                                      ///
                                      IRC_Client_StreamSessionData,
                                      IRC_Client_StreamSessionData_t>
   // implement this to have a generic (timed) event handler to trigger
   // periodic statistic collection
 , public Common_IStatistic_T<IRC_Client_RuntimeStatistic_t>
{
 public:
  IRC_Client_Module_IRCSplitter ();
  virtual ~IRC_Client_Module_IRCSplitter ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const IRC_Client_ModuleHandlerConfiguration&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (IRC_Client_Message*&, // data message handle
                                  bool&);               // return value: pass message downstream ?

  // catch the session ID...
  virtual void handleSessionMessage (IRC_Client_SessionMessage*&, // session message handle
                                     bool&);                      // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (IRC_Client_RuntimeStatistic_t&); // return value: (currently unused !)
  virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      IRC_Client_SessionMessage,
                                      IRC_Client_Message,
                                      ///
                                      IRC_Client_ModuleHandlerConfiguration,
                                      ///
                                      IRC_Client_StreamState,
                                      ///
                                      IRC_Client_StreamSessionData,
                                      IRC_Client_StreamSessionData_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Module_IRCSplitter (const IRC_Client_Module_IRCSplitter&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Module_IRCSplitter& operator= (const IRC_Client_Module_IRCSplitter&))

  // convenience types
  typedef Stream_StatisticHandler_Reactor_T<IRC_Client_RuntimeStatistic_t> STATISTICHANDLER_T;
  //typedef IRC_Client_SessionData SESSIONDATA_T;

  // helper methods
  bool putStatisticMessage (const IRC_Client_RuntimeStatistic_t&) const;

  // helper methods (to drive the scanner)
  bool scan_begin (char*,   // base address
                   size_t); // length of data block
  void scan_end ();

  // timer
  STATISTICHANDLER_T  statisticCollectHandler_;
  long                statisticCollectHandlerID_;

  // scanner
  YY_BUFFER_STATE     currentBufferState_;
  unsigned int        currentNumFrames_;
  yyscan_t            scannerContext_;

  // message buffers
  IRC_Client_Message* buffer_;
  IRC_Client_Message* currentMessage_;
  unsigned int        currentMessageLength_;

  bool                isInitialized_;
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                          // task synch type
                              Common_TimePolicy_t,                   // time policy
                              Stream_ModuleConfiguration,            // module configuration type
                              IRC_Client_ModuleHandlerConfiguration, // module handler configuration type
                              IRC_Client_Module_IRCSplitter);        // writer type

#endif
