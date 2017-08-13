// A Bison parser, made by GNU Bison 3.0.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2013 Free Software Foundation, Inc.

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
// //                    "%code top" blocks.


#include "stdafx.h"




// First part of user declarations.



# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "ace/Synch.h"
#include "http_parser.h"

// User implementation prologue.


// Unqualified %code blocks.


// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif
#include <regex>
#include <sstream>
#include <string>

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_dec_common.h"

#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_iparser.h"
#include "http_scanner.h"
#include "http_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex HTTP_Scanner_lex

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
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

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
      *yycdebug_ << std::endl;                  \
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
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyempty = true)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  HTTP_Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
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
              // Fall through.
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


  /// Build a parser object.
  HTTP_Parser::HTTP_Parser (HTTP_IParser* iparser_p_yyarg, yyscan_t yyscanner_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      iparser_p (iparser_p_yyarg),
      yyscanner (yyscanner_yyarg)
  {}

  HTTP_Parser::~HTTP_Parser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/

  inline
  HTTP_Parser::syntax_error::syntax_error (const location_type& l, const std::string& m)
    : std::runtime_error (m)
    , location (l)
  {}

  // basic_symbol.
  template <typename Base>
  inline
  HTTP_Parser::basic_symbol<Base>::basic_symbol ()
    : value ()
  {}

  template <typename Base>
  inline
  HTTP_Parser::basic_symbol<Base>::basic_symbol (const basic_symbol& other)
    : Base (other)
    , value ()
    , location (other.location)
  {
    value = other.value;
  }


  template <typename Base>
  inline
  HTTP_Parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const semantic_type& v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  inline
  HTTP_Parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const location_type& l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  inline
  HTTP_Parser::basic_symbol<Base>::~basic_symbol ()
  {
  }

  template <typename Base>
  inline
  void
  HTTP_Parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move(s);
    value = s.value;
    location = s.location;
  }

  // by_type.
  inline
  HTTP_Parser::by_type::by_type ()
     : type (empty)
  {}

  inline
  HTTP_Parser::by_type::by_type (const by_type& other)
    : type (other.type)
  {}

  inline
  HTTP_Parser::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  inline
  void
  HTTP_Parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.type = empty;
  }

  inline
  int
  HTTP_Parser::by_type::type_get () const
  {
    return type;
  }


  // by_state.
  inline
  HTTP_Parser::by_state::by_state ()
    : state (empty)
  {}

  inline
  HTTP_Parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  HTTP_Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.state = empty;
  }

  inline
  HTTP_Parser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  HTTP_Parser::symbol_number_type
  HTTP_Parser::by_state::type_get () const
  {
    return state == empty ? 0 : yystos_[state];
  }

  inline
  HTTP_Parser::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  HTTP_Parser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
    value = that.value;
    // that is emptied.
    that.type = empty;
  }

  inline
  HTTP_Parser::stack_symbol_type&
  HTTP_Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  HTTP_Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    switch (yysym.type_get ())
    {
            case 0: // "end"


        { (yysym.value.ival) = 0; }

        break;

      case 3: // "method"


        { delete (yysym.value.sval); (yysym.value.sval) = NULL; }

        break;

      case 4: // "uri"


        { delete (yysym.value.sval); (yysym.value.sval) = NULL; }

        break;

      case 5: // "version"


        { delete (yysym.value.sval); (yysym.value.sval) = NULL; }

        break;

      case 6: // "header"


        { delete (yysym.value.sval); (yysym.value.sval) = NULL; }

        break;

      case 7: // "delimiter"


        { (yysym.value.ival) = 0; }

        break;

      case 8: // "status"


        { delete (yysym.value.sval); (yysym.value.sval) = NULL; }

        break;

      case 9: // "reason"


        { delete (yysym.value.sval); (yysym.value.sval) = NULL; }

        break;

      case 10: // "body"


        { (yysym.value.ival) = 0; }

        break;

      case 11: // "chunk"


        { (yysym.value.ival) = 0; }

        break;

      case 12: // "end_of_fragment"


        { (yysym.value.ival) = 0; }

        break;

      case 14: // message


        { (yysym.value.ival) = 0; }

        break;

      case 15: // head


        { (yysym.value.ival) = 0; }

        break;

      case 16: // head_rest1


        { (yysym.value.ival) = 0; }

        break;

      case 17: // request_line_rest1


        { (yysym.value.ival) = 0; }

        break;

      case 18: // request_line_rest2


        { (yysym.value.ival) = 0; }

        break;

      case 19: // head_rest2


        { (yysym.value.ival) = 0; }

        break;

      case 20: // status_line_rest1


        { (yysym.value.ival) = 0; }

        break;

      case 21: // status_line_rest2


        { (yysym.value.ival) = 0; }

        break;

      case 22: // headers


        { (yysym.value.ival) = 0; }

        break;

      case 23: // body


        { (yysym.value.ival) = 0; }

        break;

      case 24: // chunked_body


        { (yysym.value.ival) = 0; }

        break;

      case 25: // chunks


        { (yysym.value.ival) = 0; }

        break;


      default:
        break;
    }
  }

#if YYDEBUG
  template <typename Base>
  void
  HTTP_Parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    switch (yytype)
    {
            case 0: // "end"


        { debug_stream () << (yysym.value.ival); }

        break;

      case 3: // "method"


        { debug_stream () << *(yysym.value.sval); }

        break;

      case 4: // "uri"


        { debug_stream () << *(yysym.value.sval); }

        break;

      case 5: // "version"


        { debug_stream () << *(yysym.value.sval); }

        break;

      case 6: // "header"


        { debug_stream () << *(yysym.value.sval); }

        break;

      case 7: // "delimiter"


        { debug_stream () << (yysym.value.ival); }

        break;

      case 8: // "status"


        { debug_stream () << *(yysym.value.sval); }

        break;

      case 9: // "reason"


        { debug_stream () << *(yysym.value.sval); }

        break;

      case 10: // "body"


        { debug_stream () << (yysym.value.ival); }

        break;

      case 11: // "chunk"


        { debug_stream () << (yysym.value.ival); }

        break;

      case 12: // "end_of_fragment"


        { debug_stream () << (yysym.value.ival); }

        break;

      case 14: // message


        { debug_stream () << (yysym.value.ival); }

        break;

      case 15: // head


        { debug_stream () << (yysym.value.ival); }

        break;

      case 16: // head_rest1


        { debug_stream () << (yysym.value.ival); }

        break;

      case 17: // request_line_rest1


        { debug_stream () << (yysym.value.ival); }

        break;

      case 18: // request_line_rest2


        { debug_stream () << (yysym.value.ival); }

        break;

      case 19: // head_rest2


        { debug_stream () << (yysym.value.ival); }

        break;

      case 20: // status_line_rest1


        { debug_stream () << (yysym.value.ival); }

        break;

      case 21: // status_line_rest2


        { debug_stream () << (yysym.value.ival); }

        break;

      case 22: // headers


        { debug_stream () << (yysym.value.ival); }

        break;

      case 23: // body


        { debug_stream () << (yysym.value.ival); }

        break;

      case 24: // chunked_body


        { debug_stream () << (yysym.value.ival); }

        break;

      case 25: // chunks


        { debug_stream () << (yysym.value.ival); }

        break;


      default:
        break;
    }
    yyo << ')';
  }
#endif

  inline
  void
  HTTP_Parser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  HTTP_Parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  HTTP_Parser::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  HTTP_Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  HTTP_Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  HTTP_Parser::debug_level_type
  HTTP_Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  HTTP_Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  inline HTTP_Parser::state_type
  HTTP_Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  HTTP_Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  HTTP_Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  HTTP_Parser::parse ()
  {
    /// Whether yyla contains a lookahead.
    bool yyempty = true;

    // State.
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

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    // User initialization code.
    
{
  // initialize the location
  yyla.location.initialize (NULL);
  //@$.begin.filename = @$.end.filename = &iparser_p->file;
  //ACE_OS::memset (&@$, 0, sizeof (@$));

  // initialize the token value container
  (yyla.value).ival = 0;
  (yyla.value).sval = NULL;
}



    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyempty)
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location, iparser_p, yyscanner));
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
        yyempty = false;
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Discard the token being shifted.
    yyempty = true;

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 2:

    { (yylhs.value.ival) = (yystack_[2].value.ival) + (yystack_[1].value.ival) + (yystack_[0].value.ival); }

    break;

  case 3:

    { (yylhs.value.ival) = (yystack_[1].value.sval)->size () + (yystack_[0].value.ival) + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.method =
                                                         HTTP_Tools::MethodToType (*(yystack_[1].value.sval));
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set method: \"%s\"\n"),
//                                                                   ACE_TEXT ($1->c_str ())));
                                                     }

    break;

  case 4:

    { (yylhs.value.ival) = (yystack_[1].value.sval)->size () + (yystack_[0].value.ival) + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^");
                                                       regex_string +=
                                                         ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_VERSION_STRING_PREFIX);
                                                       regex_string +=
                                                         ACE_TEXT_ALWAYS_CHAR ("([[:digit:]]{1}\\.[[:digit:]]{1})$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*(yystack_[1].value.sval),
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP version (was: \"%s\"), aborting\n"),
                                                                     ACE_TEXT ((yystack_[1].value.sval)->c_str ())));
                                                         YYABORT;
                                                       } // end IF
                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                                       ACE_ASSERT (match_results[1].matched);

                                                       record_r.version =
                                                           HTTP_Tools::VersionToType (match_results[1].str ());
//                                                         ACE_DEBUG ((LM_DEBUG,
//                                                                     ACE_TEXT ("set version: \"%s\"\n"),
//                                                                     ACE_TEXT (match_results[1].str ().c_str ())));
                                                     }

    break;

  case 5:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       YYACCEPT; }

    break;

  case 6:

    { (yylhs.value.ival) = (yystack_[1].value.ival) + (yystack_[0].value.ival); }

    break;

  case 7:

    { (yylhs.value.ival) = (yystack_[1].value.sval)->size () + (yystack_[0].value.ival) + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.URI = *(yystack_[1].value.sval);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set URI: \"%s\"\n"),
//                                                                   ACE_TEXT ($1->c_str ())));
                                                     }

    break;

  case 8:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       YYACCEPT; }

    break;

  case 9:

    { (yylhs.value.ival) = (yystack_[0].value.sval)->size () + 2;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.version =
                                                         HTTP_Tools::VersionToType (*(yystack_[0].value.sval));
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set version: \"%s\"\n"),
//                                                                   ACE_TEXT ((*$1).c_str ())));
                                                     }

    break;

  case 10:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       YYACCEPT; }

    break;

  case 11:

    { (yylhs.value.ival) = (yystack_[1].value.ival) + (yystack_[0].value.ival); }

    break;

  case 12:

    { (yylhs.value.ival) = (yystack_[1].value.sval)->size () + (yystack_[0].value.ival) + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       std::istringstream converter;
                                                       converter.str (*(yystack_[1].value.sval));
                                                       int status = -1;
                                                       converter >> status;
                                                       record_r.status =
                                                           static_cast<HTTP_Status_t> (status);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set status: %d\n"),
//                                                                   status));
                                                     }

    break;

  case 13:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       YYACCEPT; }

    break;

  case 14:

    { (yylhs.value.ival) = (yystack_[0].value.sval)->size () + 2;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.reason = *(yystack_[0].value.sval);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set reason: \"%s\"\n"),
//                                                                   ACE_TEXT ($1->c_str ())));
                                                     }

    break;

  case 15:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       YYACCEPT; }

    break;

  case 16:

    { /* NOTE*: use right-recursion here to force early state reductions
                                                                 (i.e. parse headers). This is required so the scanner can
                                                                 act on any set transfer encoding. */
                                                       (yylhs.value.ival) = (yystack_[1].value.ival) + (yystack_[0].value.sval)->size ();
                                                       /* *TODO*: modify the scanner so it emits the proper fields itself */
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^([^:]+):\\s(.+)$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*(yystack_[0].value.sval),
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP header (was: \"%s\"), returning\n"),
                                                                     ACE_TEXT ((yystack_[0].value.sval)->c_str ())));
                                                         break;
                                                       } // end IF
                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                                       ACE_ASSERT (match_results[1].matched);
                                                       ACE_ASSERT (match_results[2].matched);
                                                       ACE_ASSERT (!match_results[2].str ().empty ());

                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
//                                                     HTTP_HeadersIterator_t iterator =
//                                                       record_r.headers.find (match_results[1]);
//                                                     if (iterator != record_r.headers.end ())
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("duplicate HTTP header (was: \"%s\"), continuing\n"),
//                                                                   ACE_TEXT (match_results[1].str ().c_str ())));

                                                       record_r.headers[match_results[1]] =
                                                         match_results[2];
//                                                     ACE_DEBUG ((LM_DEBUG,
//                                                                 ACE_TEXT ("set header: \"%s\" to \"%s\"\n"),
//                                                                 ACE_TEXT (match_results[1].str ().c_str ()),
//                                                                 ACE_TEXT (match_results[2].str ().c_str ())));

                                                       // upcall ?
                                                       if (match_results[1] == ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING))
                                                       {
                                                         try {
                                                           iparser_p->encoding (match_results[1]);
                                                         } catch (...) {
                                                           ACE_DEBUG ((LM_ERROR,
                                                                       ACE_TEXT ("caught exception in HTTP_IParser::encoding(), continuing\n")));
                                                         }
                                                       } }

    break;

  case 17:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       yyclearin;
                                                       YYACCEPT; }

    break;

  case 18:

    { (yylhs.value.ival) = 0; }

    break;

  case 19:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       HTTP_HeadersIterator_t iterator =
                                                         record_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
                                                       ACE_ASSERT (iterator != record_r.headers.end ());
                                                       std::istringstream converter;
                                                       converter.str ((*iterator).second);
                                                       unsigned int content_length = 0;
                                                       converter >> content_length;
                                                       if ((yystack_[0].value.ival) == static_cast<int> (content_length))
                                                       {
                                                         struct HTTP_Record* record_p =
                                                           &record_r;
                                                         try {
                                                           iparser_p->record (record_p);
                                                         } catch (...) {
                                                           ACE_DEBUG ((LM_ERROR,
                                                                       ACE_TEXT ("caught exception in HTTP_IParser::record(), continuing\n")));
                                                         }
                                                         YYACCEPT;
                                                       } }

    break;

  case 20:

    { (yylhs.value.ival) = (yystack_[1].value.ival) + (yystack_[0].value.ival); }

    break;

  case 21:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       yyclearin;
                                                       YYACCEPT; }

    break;

  case 22:

    { (yylhs.value.ival) = (yystack_[2].value.ival) + (yystack_[1].value.ival) + (yystack_[0].value.ival); // *TODO*: potential conflict here (i.e. incomplete chunk may be accepted)
                                                     struct HTTP_Record& record_r =
                                                       iparser_p->current ();
                                                     struct HTTP_Record* record_p =
                                                       &record_r;
                                                     try {
                                                       iparser_p->record (record_p);
                                                     } catch (...) {
                                                       ACE_DEBUG ((LM_ERROR,
                                                                   ACE_TEXT ("caught exception in HTTP_IParser::record(), continuing\n")));
                                                     }
                                                     YYACCEPT; }

    break;

  case 23:

    { (yylhs.value.ival) = (yystack_[0].value.ival);
                                                       yyclearin;
                                                       YYACCEPT; }

    break;

  case 24:

    { (yylhs.value.ival) = 0;
                                                       YYACCEPT; }

    break;

  case 25:

    { (yylhs.value.ival) = (yystack_[1].value.ival) + (yystack_[0].value.ival); }

    break;

  case 26:

    { (yylhs.value.ival) = 0; }

    break;



            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
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
        error (yyla.location, yysyntax_error_ (yystack_[0].state,
                                           yyempty ? yyempty_ : yyla.type_get ()));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyempty)
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyempty = true;
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
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

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyempty)
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyempty)
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  HTTP_Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  HTTP_Parser::yysyntax_error_ (state_type yystate, symbol_number_type yytoken) const
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
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
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char HTTP_Parser::yypact_ninf_ = -14;

  const signed char HTTP_Parser::yytable_ninf_ = -25;

  const signed char
  HTTP_Parser::yypact_[] =
  {
      -1,     2,    -3,   -14,     7,    12,    -2,   -14,   -14,    -4,
       6,   -14,   -14,    -4,   -14,    11,   -14,   -14,   -14,   -14,
      18,   -14,   -14,   -14,    18,   -14,     1,   -14,   -14,   -14,
      14,   -14,   -14,    -4,   -14,    10,   -14
  };

  const unsigned char
  HTTP_Parser::yydefact_[] =
  {
       0,     0,     0,     5,     0,     0,     0,     8,     3,    18,
       0,    13,     4,    18,     1,     0,     9,    10,     7,    17,
       6,    14,    15,    12,    11,    19,    26,    21,     2,    16,
      26,    23,    20,    18,    25,     0,    22
  };

  const signed char
  HTTP_Parser::yypgoto_[] =
  {
     -14,   -14,   -14,   -14,   -14,   -14,   -14,   -14,   -14,   -13,
     -14,   -14,     0
  };

  const signed char
  HTTP_Parser::yydefgoto_[] =
  {
      -1,     4,     5,     8,     9,    18,    12,    13,    23,    20,
      28,    32,    33
  };

  const signed char
  HTTP_Parser::yytable_[] =
  {
      24,   -24,     1,    16,     2,    10,     6,    14,    19,    11,
      17,     3,    30,    31,     7,    21,    29,    36,    22,    15,
      35,    25,    26,    27,    29,    30,     0,     0,     0,     0,
      34
  };

  const signed char
  HTTP_Parser::yycheck_[] =
  {
      13,     0,     3,     5,     5,     8,     4,     0,    12,    12,
      12,    12,    11,    12,    12,     9,     6,     7,    12,     7,
      33,    10,    11,    12,     6,    11,    -1,    -1,    -1,    -1,
      30
  };

  const unsigned char
  HTTP_Parser::yystos_[] =
  {
       0,     3,     5,    12,    14,    15,     4,    12,    16,    17,
       8,    12,    19,    20,     0,     7,     5,    12,    18,    12,
      22,     9,    12,    21,    22,    10,    11,    12,    23,     6,
      11,    12,    24,    25,    25,    22,     7
  };

  const unsigned char
  HTTP_Parser::yyr1_[] =
  {
       0,    13,    14,    15,    15,    15,    16,    17,    17,    18,
      18,    19,    20,    20,    21,    21,    22,    22,    22,    23,
      23,    23,    24,    24,    24,    25,    25
  };

  const unsigned char
  HTTP_Parser::yyr2_[] =
  {
       0,     2,     3,     2,     2,     1,     2,     2,     1,     1,
       1,     2,     2,     1,     1,     1,     2,     1,     0,     1,
       2,     1,     3,     1,     0,     2,     0
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const HTTP_Parser::yytname_[] =
  {
  "\"end\"", "error", "$undefined", "\"method\"", "\"uri\"",
  "\"version\"", "\"header\"", "\"delimiter\"", "\"status\"", "\"reason\"",
  "\"body\"", "\"chunk\"", "\"end_of_fragment\"", "$accept", "message",
  "head", "head_rest1", "request_line_rest1", "request_line_rest2",
  "head_rest2", "status_line_rest1", "status_line_rest2", "headers",
  "body", "chunked_body", "chunks", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned short int
  HTTP_Parser::yyrline_[] =
  {
       0,   217,   217,   218,   227,   257,   259,   260,   268,   270,
     279,   281,   282,   295,   297,   305,   307,   357,   360,   362,
     384,   385,   388,   400,   403,   407,   408
  };

  // Print the state stack on the debug stream.
  void
  HTTP_Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  HTTP_Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  // Symbol number corresponding to token number t.
  inline
  HTTP_Parser::token_number_type
  HTTP_Parser::yytranslate_ (int t)
  {
    static
    const token_number_type
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
       5,     6,     7,     8,     9,    10,    11,    12
    };
    const unsigned int user_token_number_max_ = 267;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} // yy




void
yy::HTTP_Parser::error (const location_type& location_in,
                        const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Parser::error"));

/*  ACE_UNUSED_ARG (location_in);*/

  iparser_p->error (location_in, message_in);
/*  iparser_p->error (message_in);*/
}

void
yy::HTTP_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Parser::set"));

  yyscanner = context_in;
}

/*void
yysetdebug (int debug_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yysetdebug"));

  yydebug = debug_in;
}

void
yyerror (YYLTYPE* location_in,
         HTTP_IParser* iparser_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (location_in);
  ACE_ASSERT (iparser_in);

  try {
    iparser_in->error (*location_in, std::string (message_in));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in HTTP_IParser::error(), continuing\n")));
  }
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
    case METHOD:
    case URI:
    case VERSION:
    case HEADER:
    case STATUS:
    case REASON:
    {
      format_string = ACE_TEXT_ALWAYS_CHAR (" %s");
      break;
    }
    case DELIMITER:
    case BODY:
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
