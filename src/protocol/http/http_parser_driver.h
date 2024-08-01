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

#ifndef HTTP_PARSER_DRIVER_T_H
#define HTTP_PARSER_DRIVER_T_H

#include <string>

#include "ace/Global_Macros.h"

#include "common_parser_common.h"
#include "common_parser_defines.h"

#include "http_iparser.h"
#include "http_parser.h"
#include "http_scanner.h"

#include "location.hh"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_ITask;
typedef void* yyscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
int HTTP_Scanner_get_debug  (yyscan_t);
void HTTP_Scanner_set_debug (int  _bdebug , yyscan_t);
void HTTP_Scanner_set_column (int, yyscan_t);
void HTTP_Scanner_set_lineno ( int _line_number , yyscan_t yyscanner );

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
class HTTP_ParserDriver_T
 : public HTTP_IParser
{
 public:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE, TimePolicyType> TASK_T;

  HTTP_ParserDriver_T (Stream_ITask*,
                       const std::string&); // scanner tables file (if any)
  virtual ~HTTP_ParserDriver_T ();

  // implement (part of) HTTP_IParser
  virtual bool initialize (const struct HTTP_ParserConfiguration&);
  inline virtual ACE_Message_Block* buffer () { return fragment_; }
  inline virtual bool debug () const { return HTTP_Scanner_get_debug (scannerState_); }
  inline virtual bool isBlocking () const { return blockInParse_; }
  virtual void error (const struct YYLTYPE&, // location
                      const std::string&); // message
  virtual void error (const yy::location&, // location
                      const std::string&); // message
  virtual void error (const std::string&); // message
  inline virtual void offset (unsigned int offset_in) { offset_ += offset_in; } // offset (increment)
  inline virtual unsigned int offset () const { return offset_; }
  virtual bool begin (const char*,   // buffer handle
                      unsigned int); // buffer size
  virtual void end ();
  inline virtual const HTTP_IParser* const getP_2 () const { return this; }
  virtual bool parse (ACE_Message_Block*); // data buffer handle
  virtual bool switchBuffer (bool = false); // unlink current fragment ?
  // *NOTE*: (waits for and) appends the next data chunk to fragment_;
  virtual void waitBuffer ();
  inline virtual struct HTTP_Record& current () { ACE_ASSERT (record_); return *record_; }
//  inline virtual void finished () { finished_ = true; };
  inline virtual bool hasFinished () const { return finished_; }

  virtual void dump_state () const;

 protected:
  struct HTTP_ParserConfiguration* configuration_;
  bool                             finished_; // processed the whole entity ?
  ACE_Message_Block*               fragment_;
  Stream_ITask*                    itask_;
  size_t                           offset_; // parsed entity bytes
  struct HTTP_Record*              record_;

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver_T ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver_T (const HTTP_ParserDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_ParserDriver_T& operator= (const HTTP_ParserDriver_T&))

  inline virtual const Common_FlexScannerState& getR () const { static Common_FlexScannerState dummy; ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) }
  inline virtual void setP (HTTP_IParser*) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void setDebug (yyscan_t state_in, bool toggle_in) { HTTP_Scanner_set_debug ((toggle_in ? 1 : 0), state_in); }
  inline virtual void reset () { HTTP_Scanner_set_lineno (1, scannerState_); HTTP_Scanner_set_column (1, scannerState_); }
  inline virtual bool initialize (yyscan_t&, HTTP_IParser* extra_in) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual void finalize (yyscan_t&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual YY_BUFFER_STATE create (yyscan_t, char*, size_t) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
  inline virtual void destroy (yyscan_t, struct yy_buffer_state*&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual bool lex (yyscan_t state_in) { ACE_ASSERT (false); return false; /*HTTP_Scanner_lex (NULL, NULL, this, state_in);*/ };

  bool                               blockInParse_;
  bool                               isFirst_;

  //// parser
  //yy::HTTP_Parser                    parser_;

  // scanner
  yyscan_t                           scannerState_;
  std::string                        scannerTables_;
  YY_BUFFER_STATE                    bufferState_;
  ACE_Message_Queue_Base*            messageQueue_;
  bool                               useYYScanBuffer_;

  bool                               isInitialized_;
};

// include template definition
#include "http_parser_driver.inl"

#endif
