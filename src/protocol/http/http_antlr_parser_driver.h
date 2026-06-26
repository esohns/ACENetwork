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

#ifndef HTTP_ANTLR_PARSER_DRIVER_T_H
#define HTTP_ANTLR_PARSER_DRIVER_T_H

#include <string>

#include "antlr4-runtime.h"

#include "ace/Global_Macros.h"

#include "common_parser_common.h"
#include "common_parser_defines.h"

#include "http_common.h"

#include "http_antlr_iparser.h"
#include "http_antlr_scanner.h"
#include "http_antlr_parser.h"
#include "http_antlr_parserBaseListener.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_ITask;

#define USE_UNBUFFERED 1

//////////////////////////////////////////

#if (USE_UNBUFFERED)
class HTTP_ANTLR_Streambuf
 : public std::streambuf
{
  typedef std::streambuf inherited;

 public:
  HTTP_ANTLR_Streambuf (HTTP_ANTLR_IParser* parser_in)
   : inherited ()
   , buffer_ ()
   // , leftover_bytes_ ()
   // , is_eof_ (false)
   // , state_ ()
   , parser_ (parser_in)
  {
    setg (NULL, NULL, NULL);
  }
  inline virtual ~HTTP_ANTLR_Streambuf () {}

  void reset ()
  {
    buffer_.clear ();

    setg (NULL, NULL, NULL);
  }

  // void append_chunk (const char* data_in, size_t size_in)
  // { ACE_ASSERT (data_in && size_in);
  //   if (!leftover_bytes_.empty ())
  //   {
  //     leftover_bytes_.insert (leftover_bytes_.end (), data_in, data_in + size_in);
  //     data_in = leftover_bytes_.data ();
  //     size_in = leftover_bytes_.size ();
  //   } // end IF

  //   std::vector<wchar_t> wide_chunk;
  //   const char* ptr = data_in;
  //   const char* end_ptr = data_in + size_in;

  //   while (ptr < end_ptr)
  //   {
  //     wchar_t wc;
  //     size_t max_bytes_to_read = static_cast<size_t> (end_ptr - ptr);

  //     size_t result = std::mbrtowc (&wc, ptr, max_bytes_to_read, &state_);
  //     if (result == static_cast<size_t> (-2))
  //       break;
  //     if (result == static_cast<size_t> (-1))
  //     {
  //       ptr++;
  //       std::mbrtowc (NULL, NULL, 0, &state_);
  //       continue;
  //     } // end IF
  //     if (result == 0)
  //     {
  //       wide_chunk.push_back (L'\0');
  //       ptr += 1;
  //       continue;
  //     } // end IF
  //     wide_chunk.push_back (wc);
  //     ptr += result;
  //   } // end WHILE
  //   if (ptr < end_ptr)
  //     leftover_bytes_.assign (ptr, end_ptr);
  //   else
  //     leftover_bytes_.clear ();
  //   if (wide_chunk.empty ())
  //     return;

  //   size_t current_read_offset = gptr () - eback ();
  //   if (current_read_offset > 0)
  //   {
  //     buffer_.erase (buffer_.begin (), buffer_.begin () + current_read_offset);
  //     current_read_offset = 0;
  //   } // end IF
  //   buffer_.insert (buffer_.end (), wide_chunk.begin (), wide_chunk.end ());

  //   // 3. Re-anchor standard stream pointers to the contiguous memory layout
  //   wchar_t* base = buffer_.data ();
  //   wchar_t* current_read_ptr = base + current_read_offset;
  //   wchar_t* end = base + buffer_.size ();

  //   setg (base, current_read_ptr, end);
  // }
  void append_chunk (const char* data_in, size_t size_in)
  { ACE_ASSERT (data_in && size_in);
    const char* ptr = data_in;
    const char* end_ptr = data_in + size_in;

    size_t current_read_offset = gptr () - eback ();
    if (current_read_offset > 0)
    {
      buffer_.erase (buffer_.begin (), buffer_.begin () + current_read_offset);
      current_read_offset = 0;
    } // end IF
    buffer_.insert (buffer_.end (), ptr, end_ptr);

    // 3. Re-anchor standard stream pointers to the contiguous memory layout
    char* base = buffer_.data ();
    char* current_read_ptr = base + current_read_offset;
    char* end = base + buffer_.size ();

    setg (base, current_read_ptr, end);
  }

  // void signal_eof ()
  // {
  //   is_eof_ = true;
  // }

 protected:
  virtual int_type underflow ()
  {
retry:
    if (gptr () < egptr ())
      return traits_type::to_int_type (*gptr ());
    if (parser_->hasFinished ())
      return traits_type::eof ();
    if (parser_->switchBuffer (true))
      goto retry;

    return traits_type::eof ();
  }

 private:
  // std::vector<wchar_t> buffer_;
  std::vector<char>   buffer_;
  // std::vector<char>   leftover_bytes_;
  // bool              is_eof_;
  // std::mbstate_t    state_;
  HTTP_ANTLR_IParser* parser_;
};

class UnbufferedByteCharStream
 : public antlr4::CharStream
{
 protected:
  std::istream&         _input;
  std::vector<char32_t> _data;
  size_t                _p;
  size_t                _numMarkers;
  size_t                _lastCharBufferStart;
  size_t                _currentCharIndex;

 public:
  UnbufferedByteCharStream (std::istream& input)
   : _input (input)
   , _data ()
   , _p (0)
   , _numMarkers (0)
   , _lastCharBufferStart (0)
   , _currentCharIndex (0)
  {}

  void reset ()
  {
    _data.clear ();
    _p = 0;
    _numMarkers = 0;
    _lastCharBufferStart = 0;
    _currentCharIndex = 0;
  }

  virtual void consume() override
  {
    if (LA(1) == antlr4::IntStream::EOF)
    {
      throw std::runtime_error("Cannot consume EOF");
    } // end IF
    _p++;
    _currentCharIndex++;
    if (_p == _data.size () && _numMarkers == 0)
    {
      // Buffer optimization: clear processed elements when lookahead pressure drops
      _lastCharBufferStart += _data.size ();
      _data.clear ();
      _p = 0;
    } // end IF
  }

  virtual size_t LA (ssize_t i) override
  {
    if (i == 0)
      return 0; // Undefined by ANTLR specification
    if (i < 0)
    {
      // Backward lookahead calculations
      ssize_t index = static_cast<ssize_t> (_p) + i;
      if (index < 0)
      {
        throw std::runtime_error("Backward lookahead went out of buffered window");
      }
      return _data[static_cast<size_t> (index)];
    }

    size_t index = _p + static_cast<size_t>(i) - 1;
    fill (i); // Lazily request streaming buffers to expand up to lookahead length
    if (index >= _data.size())
      return antlr4::IntStream::EOF;
    return _data[index];
  }

  virtual ssize_t mark () override
  {
    _numMarkers++;
    return _currentCharIndex;
  }

  virtual void release (ssize_t marker) override
  {
    if (_numMarkers > 0)
      _numMarkers--;
  }

  virtual size_t index () override
  {
    return _currentCharIndex;
  }

  virtual void seek (size_t index) override
  {
    if (index == _currentCharIndex)
      return;
    if (index < _lastCharBufferStart)
      throw std::runtime_error ("Cannot seek backward outside the sliding memory window.");

    size_t targetBufferPos = index - _lastCharBufferStart;
    if (targetBufferPos < _data.size ())
    {
      _p = targetBufferPos;
      _currentCharIndex = index;
    } // end IF
    else
    {
      // Fast-forward streaming chunks lazily to locate the target forward pointer
      fill (static_cast<ssize_t> (targetBufferPos - _p + 1));
      _p = std::min (targetBufferPos, _data.size ());
      _currentCharIndex = _lastCharBufferStart + _p;
    } // end ELSE
  }

  virtual size_t size () override
  {
    //throw std::runtime_error("Size verification is unavailable on unbuffered streams.");
    return 0;
  }

  virtual std::string getSourceName() const override
  {
    return "UnbufferedByteCharStream";
  }

  virtual std::string getText (const antlr4::misc::Interval& interval) override
  {
    if (interval.a < static_cast<ssize_t> (_lastCharBufferStart))
    {
      return "<DISCARDED_SLIDING_WINDOW>";
    } // end IF

    size_t start = static_cast<size_t> (interval.a) - _lastCharBufferStart;
    size_t stop = static_cast<size_t> (interval.b) - _lastCharBufferStart;

    if (start >= _data.size ())
      return "";
    stop = std::min (stop, _data.size () - 1);

    std::string result;
    for (size_t i = start; i <= stop; ++i)
    {
      // Narrow down internal UTF-32 points back to standard characters safely
      result += static_cast<char> (_data[i]);
    } // end IF
    return result;
  }

  virtual std::string toString () const override
  {
    return "UnbufferedByteCharStream(index=" + std::to_string(_currentCharIndex) + ")";
  }

 protected:
  void fill (ssize_t need)
  {
    ssize_t absoluteNeededPos = static_cast<ssize_t> (_p) + need;
    ssize_t toRead = absoluteNeededPos - static_cast<ssize_t> (_data.size ());

    for (ssize_t i = 0; i < toRead; ++i)
    {
      int byte = _input.get ();
      if (byte == EOF)
        break;
      _data.push_back (static_cast<char32_t> (byte));
    }
  }
};
#endif // USE_UNBUFFERED

class HTTP_ANTLRErrorHandler
 : public antlr4::DefaultErrorStrategy
{
  typedef antlr4::DefaultErrorStrategy inherited;

 public:
  HTTP_ANTLRErrorHandler ()
   : inherited ()
  {}
  inline virtual ~HTTP_ANTLRErrorHandler () {}
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
class HTTP_ANTLRParserDriver_T
 : public http_antlr_parserBaseListener
 , public antlr4::ANTLRErrorListener
 , public HTTP_ANTLR_IParser
{
  typedef http_antlr_parserBaseListener inherited;

 public:
  HTTP_ANTLRParserDriver_T (Stream_ITask*); // (parent-) stream task
  virtual ~HTTP_ANTLRParserDriver_T ();

  // override (part of) http_antlr_parserBaseListener
  virtual void exitBody (http_antlr_parser::BodyContext*);
  virtual void exitChunks (http_antlr_parser::ChunksContext*);
  virtual void enterEveryRule (antlr4::ParserRuleContext*);

  // implement antlr4::ANTLRErrorListener
  virtual void syntaxError (antlr4::Recognizer*,
                            antlr4::Token*,
                            size_t,
                            size_t,
                            const std::string&,
                            std::exception_ptr);
  virtual void reportAmbiguity (antlr4::Parser*,
                                const antlr4::dfa::DFA&,
                                size_t,
                                size_t,
                                bool,
                                const antlrcpp::BitSet&,
                                antlr4::atn::ATNConfigSet*);
  virtual void reportAttemptingFullContext (antlr4::Parser*,
                                            const antlr4::dfa::DFA&,
                                            size_t,
                                            size_t,
                                            const antlrcpp::BitSet&,
                                            antlr4::atn::ATNConfigSet*);
  virtual void reportContextSensitivity (antlr4::Parser*,
                                         const antlr4::dfa::DFA&,
                                         size_t,
                                         size_t,
                                         size_t,
                                         antlr4::atn::ATNConfigSet*);

  // implement (part of) HTTP_ANTLR_IParser
  virtual bool initialize (const struct HTTP_ParserConfiguration&);
  inline virtual ACE_Message_Block* buffer () { return fragment_; }
  inline virtual bool isBlocking () const { ACE_ASSERT (configuration_); return configuration_->block; }
  //inline virtual void offset (size_t offset_in) { lexer_.offset += offset_in; } // offset (increment)
  inline virtual size_t offset () const { return static_cast<unsigned int> (lexer_.offset); }
  virtual bool begin (const char*, // buffer handle
                      size_t);     // buffer size
  virtual void end ();
  virtual bool parse (ACE_Message_Block*); // data buffer handle
  virtual bool switchBuffer (bool = true); // begin() current fragment ?
  // *NOTE*: (waits for and) appends the next data chunk to fragment_;
  virtual void waitBuffer ();

  virtual bool hasFinished ();
  inline virtual bool headerOnly () { ACE_ASSERT (configuration_); return configuration_->headerOnly; } // returns: parse HTTP header only ?
  virtual void chunk_2 (ACE_UINT64, ACE_UINT32); // chunk offset, chunk size

  virtual void dump_state () const;

 protected:
  struct HTTP_ParserConfiguration* configuration_;
  bool                             finished_; // processed the whole entity ?
  ACE_Message_Block*               fragment_;
#if (USE_UNBUFFERED)
  HTTP_ANTLR_Streambuf             inputBuffer_;
  // std::wistream                    inputStream_;
  std::istream                     inputStream_;
  // antlr4::UnbufferedCharStream     input_;
  UnbufferedByteCharStream         input_;
  antlr4::CommonTokenFactory       tokenFactory_;
  http_antlr_scanner               lexer_;
  antlr4::UnbufferedTokenStream    tokens_;
#else
  antlr4::ANTLRInputStream         inputStream_;
  http_antlr_scanner               lexer_;
  antlr4::BufferedTokenStream      tokens_;
#endif // USE_UNBUFFERED
  Stream_ITask*                    itask_;
  http_antlr_parser                parser_;

 private:
  ACE_UNIMPLEMENTED_FUNC (HTTP_ANTLRParserDriver_T ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_ANTLRParserDriver_T (const HTTP_ANTLRParserDriver_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_ANTLRParserDriver_T& operator= (const HTTP_ANTLRParserDriver_T&))

  inline virtual const HTTP_ParserConfiguration& getR () const { ACE_ASSERT (configuration_); return *configuration_; }
  inline virtual const Common_FlexScannerState& getR_2 () const { static Common_FlexScannerState dummy; ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) }
  inline virtual void setP (HTTP_ANTLR_IParser*) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  bool                             isFirst_;
  bool                             isInitialized_;
  ACE_Message_Queue_Base*          messageQueue_;
};

// include template definition
#include "http_antlr_parser_driver.inl"

#endif
