// A Bison parser, made by GNU Bison 3.7.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.

// "%code top" blocks.

#include "stdafx.h"

#include "ace/Synch.h"
#include "bittorrent_parser.h"





#include "bittorrent_parser.h"


// Unqualified %code blocks.

// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
/*#if defined (YYDEBUG)
#include <iostream>
#endif*/
/*#include <regex>*/
#include <sstream>
#include <string>

// *WORKAROUND*
/*using namespace std;*/
// *IMPORTANT NOTE*: several ACE headers include ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
/*#define ACE_IOSFWD_H*/

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_macros.h"

#include "bittorrent_defines.h"
/*#include "ace/Synch.h"*/
#include "bittorrent_parser_driver.h"
#include "bittorrent_scanner.h"
#include "bittorrent_tools.h"

// *TODO*: this shouldn't be necessary
/* #define yylex scanner->yylex */
#define yylex BitTorrent_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)



#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {

  /// Build a parser object.
  BitTorrent_Parser::BitTorrent_Parser (BitTorrent_IParser_t* parser_yyarg, yyscan_t scanner_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      parser (parser_yyarg),
      scanner (scanner_yyarg)
  {}

  BitTorrent_Parser::~BitTorrent_Parser ()
  {}

  BitTorrent_Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | symbol kinds.  |
  `---------------*/

  // basic_symbol.
  template <typename Base>
  BitTorrent_Parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  BitTorrent_Parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  BitTorrent_Parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (semantic_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}

  template <typename Base>
  BitTorrent_Parser::symbol_kind_type
  BitTorrent_Parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

  template <typename Base>
  bool
  BitTorrent_Parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  BitTorrent_Parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_kind.
  BitTorrent_Parser::by_kind::by_kind ()
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  BitTorrent_Parser::by_kind::by_kind (by_kind&& that)
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  BitTorrent_Parser::by_kind::by_kind (const by_kind& that)
    : kind_ (that.kind_)
  {}

  BitTorrent_Parser::by_kind::by_kind (token_kind_type t)
    : kind_ (yytranslate_ (t))
  {}

  void
  BitTorrent_Parser::by_kind::clear ()
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  BitTorrent_Parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  BitTorrent_Parser::symbol_kind_type
  BitTorrent_Parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

  BitTorrent_Parser::symbol_kind_type
  BitTorrent_Parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  // by_state.
  BitTorrent_Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  BitTorrent_Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  BitTorrent_Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  BitTorrent_Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  BitTorrent_Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  BitTorrent_Parser::symbol_kind_type
  BitTorrent_Parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  BitTorrent_Parser::stack_symbol_type::stack_symbol_type ()
  {}

  BitTorrent_Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  BitTorrent_Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  BitTorrent_Parser::stack_symbol_type&
  BitTorrent_Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }

  BitTorrent_Parser::stack_symbol_type&
  BitTorrent_Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  BitTorrent_Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    switch (yysym.kind ())
    {
      case symbol_kind::S_YYEOF: // "end"
                    { (yysym.value.size) = 0; }
        break;

      case symbol_kind::S_HANDSHAKE: // "handshake"
                    { (yysym.value.handshake) = NULL; }
        break;

      case symbol_kind::S_BITFIELD: // "bitfield"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_CANCEL: // "cancel"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_CHOKE: // "choke"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_HAVE: // "have"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_INTERESTED: // "interested"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_KEEP_ALIVE: // "keep-alive"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_NOT_INTERESTED: // "not_interested"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_PIECE: // "piece"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_PORT: // "port"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_REQUEST: // "request"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_UNCHOKE: // "unchoke"
                    { (yysym.value.record) = NULL; }
        break;

      case symbol_kind::S_END_OF_FRAGMENT: // "end_of_fragment"
                    { (yysym.value.size) = 0; }
        break;

      case symbol_kind::S_session: // session
                    { (yysym.value.size) = 0; }
        break;

      case symbol_kind::S_messages: // messages
                    { (yysym.value.size) = 0; }
        break;

      case symbol_kind::S_message: // message
                    { (yysym.value.size) = 0; }
        break;

      default:
        break;
    }
  }

#if YYDEBUG
  template <typename Base>
  void
  BitTorrent_Parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        switch (yykind)
    {
      case symbol_kind::S_YYEOF: // "end"
                    { debug_stream () << (yysym.value.size); }
        break;

      case symbol_kind::S_HANDSHAKE: // "handshake"
                    { debug_stream () << BitTorrent_Tools::HandShakeToString (*(yysym.value.handshake)); }
        break;

      case symbol_kind::S_BITFIELD: // "bitfield"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_CANCEL: // "cancel"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_CHOKE: // "choke"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_HAVE: // "have"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_INTERESTED: // "interested"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_KEEP_ALIVE: // "keep-alive"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_NOT_INTERESTED: // "not_interested"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_PIECE: // "piece"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_PORT: // "port"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_REQUEST: // "request"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_UNCHOKE: // "unchoke"
                    { debug_stream () << BitTorrent_Tools::RecordToString (*(yysym.value.record)); }
        break;

      case symbol_kind::S_END_OF_FRAGMENT: // "end_of_fragment"
                    { debug_stream () << (yysym.value.size); }
        break;

      case symbol_kind::S_session: // session
                    { debug_stream () << (yysym.value.size); }
        break;

      case symbol_kind::S_messages: // messages
                    { debug_stream () << (yysym.value.size); }
        break;

      case symbol_kind::S_message: // message
                    { debug_stream () << (yysym.value.size); }
        break;

      default:
        break;
    }
        yyo << ')';
      }
  }
#endif

  void
  BitTorrent_Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  BitTorrent_Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  BitTorrent_Parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  BitTorrent_Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  BitTorrent_Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  BitTorrent_Parser::debug_level_type
  BitTorrent_Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  BitTorrent_Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  BitTorrent_Parser::state_type
  BitTorrent_Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  BitTorrent_Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  BitTorrent_Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  BitTorrent_Parser::operator() ()
  {
    return parse ();
  }

  int
  BitTorrent_Parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
{
  // initialize the location
  yyla.location.initialize (NULL);
}



    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location, parser, scanner));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // $@1: %empty
                               { ACE_ASSERT ((yystack_[0].value.handshake));
                                 struct BitTorrent_PeerHandShake* handshake_p =
                                   const_cast<struct BitTorrent_PeerHandShake*> ((yystack_[0].value.handshake));
                                 try {
                                   parser->handshake (handshake_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser::handshake(), continuing\n")));
                                 } }
    break;

  case 3: // session: "handshake" $@1 messages
                               { (yylhs.value.size) = 67 + (yystack_[0].value.size); // 19 + 8 + 20 + 20
                                 YYACCEPT; }
    break;

  case 4: // session: messages
                               { (yylhs.value.size) = (yystack_[0].value.size);
                                 YYACCEPT; }
    break;

  case 5: // messages: messages message
                               { (yylhs.value.size) = (yystack_[1].value.size) + (yystack_[0].value.size); }
    break;

  case 6: // messages: %empty
                               { (yylhs.value.size) = 0; }
    break;

  case 7: // message: "bitfield"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 8: // message: "cancel"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 9: // message: "choke"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 10: // message: "have"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 11: // message: "interested"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 12: // message: "keep-alive"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 13: // message: "not_interested"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 14: // message: "piece"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 15: // message: "port"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 16: // message: "request"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 17: // message: "unchoke"
                               { (yylhs.value.size) = (yystack_[0].value.record)->length + 4;
                                 ACE_ASSERT ((yystack_[0].value.record));
                                 struct BitTorrent_PeerRecord* record_p =
                                   const_cast<struct BitTorrent_PeerRecord*> ((yystack_[0].value.record));
                                 try {
                                   parser->record (record_p);
                                 } catch (...) {
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("caught exception in BitTorrent_IParser_T::record(), continuing\n")));
                                 }
                                 YYACCEPT; }
    break;

  case 18: // message: "end_of_fragment"
                               { (yylhs.value.size) = (yystack_[0].value.size); }
    break;



            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  BitTorrent_Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  BitTorrent_Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
  BitTorrent_Parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  // BitTorrent_Parser::context.
  BitTorrent_Parser::context::context (const BitTorrent_Parser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  BitTorrent_Parser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        int yychecklim = yylast_ - yyn + 1;
        int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }



  int
  BitTorrent_Parser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  BitTorrent_Parser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char BitTorrent_Parser::yypact_ninf_ = -5;

  const signed char BitTorrent_Parser::yytable_ninf_ = -1;

  const signed char
  BitTorrent_Parser::yypact_[] =
  {
       9,    -5,    13,    -4,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -4
  };

  const signed char
  BitTorrent_Parser::yydefact_[] =
  {
       6,     2,     0,     4,     6,     1,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,     5,     3
  };

  const signed char
  BitTorrent_Parser::yypgoto_[] =
  {
      -5,    -5,    -5,    10,    -5
  };

  const signed char
  BitTorrent_Parser::yydefgoto_[] =
  {
      -1,     2,     4,     3,    18
  };

  const signed char
  BitTorrent_Parser::yytable_[] =
  {
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     1,     5,    19
  };

  const signed char
  BitTorrent_Parser::yycheck_[] =
  {
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,     3,     0,     4
  };

  const signed char
  BitTorrent_Parser::yystos_[] =
  {
       0,     3,    17,    19,    18,     0,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    20,    19
  };

  const signed char
  BitTorrent_Parser::yyr1_[] =
  {
       0,    16,    18,    17,    17,    19,    19,    20,    20,    20,
      20,    20,    20,    20,    20,    20,    20,    20,    20
  };

  const signed char
  BitTorrent_Parser::yyr2_[] =
  {
       0,     2,     0,     3,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const BitTorrent_Parser::yytname_[] =
  {
  "\"end\"", "error", "\"invalid token\"", "\"handshake\"",
  "\"bitfield\"", "\"cancel\"", "\"choke\"", "\"have\"", "\"interested\"",
  "\"keep-alive\"", "\"not_interested\"", "\"piece\"", "\"port\"",
  "\"request\"", "\"unchoke\"", "\"end_of_fragment\"", "$accept",
  "session", "$@1", "messages", "message", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  BitTorrent_Parser::yyrline_[] =
  {
       0,   223,   223,   223,   234,   236,   237,   239,   250,   261,
     272,   283,   294,   305,   316,   327,   338,   349,   360
  };

  void
  BitTorrent_Parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  BitTorrent_Parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  BitTorrent_Parser::symbol_kind_type
  BitTorrent_Parser::yytranslate_ (int t)
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15
    };
    // Last valid token kind.
    const int code_max = 270;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return YY_CAST (symbol_kind_type, translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

} // yy



void
yy::BitTorrent_Parser::error (const location_type& location_in,
                              const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Parser::error"));

  try {
    parser->error (location_in, message_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IParser::error(), continuing\n")));
  }
}

void
yy::BitTorrent_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Parser::set"));

  scanner = context_in;
}

/*void
yysetdebug (int debug_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yysetdebug"));

  yydebug = debug_in;
}

void
yyerror (YYLTYPE* location_in,
         BitTorrent_IParser_t* iparser_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

//  ACE_UNUSED_ARG (location_in);
  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (iparser_in);

  yy::location location;
  location.initialize (YY_NULLPTR,
                       location_in->first_line,
                       location_in->first_column);
  location.lines (location_in->last_line - location_in->first_line);
  location.columns (location_in->last_column - location_in->first_column);

  iparser_in->error (location, std::string (message_in));
//  iparser_in->error (std::string (message_in));
}

void
yyprint (FILE* file_in,
         yytokentype type_in,
         YYSTYPE value_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyprint"));

  int result = -1;

  std::string format_string;
  switch (type_in)
  {
    case HANDSHAKE:
    case BITFIELD:
    case CANCEL:
    case CHOKE:
    case HAVE:
    case INTERESTED:
    case KEEP_ALIVE:
    case NOT_INTERESTED:
    case PIECE:
    case PORT:
    case REQUEST:
    case UNCHOKE:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case END_OF_FRAGMENT:
    case END:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %d");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH

  result = ACE_OS::fprintf (file_in,
                            ACE_TEXT (format_string.c_str ()),
                            value_in);
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::fprintf(): \"%m\", returning\n")));
}*/
