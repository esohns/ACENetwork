/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2010 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */
/* "%code top" blocks.  */


#include "stdafx.h"





/* First part of user declarations.  */




#include "http_parser.h"

/* User implementation prologue.  */


/* Unqualified %code blocks.  */


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

#include <ace/Log_Msg.h>
#include <ace/OS.h>

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
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {

#if YYERROR_VERBOSE

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
              /* Fall through.  */
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

#endif

  /// Build a parser object.
  HTTP_Parser::HTTP_Parser (HTTP_IParser* iparser_p_yyarg, yyscan_t yyscanner_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      iparser_p (iparser_p_yyarg),
      yyscanner (yyscanner_yyarg)
  {
  }

  HTTP_Parser::~HTTP_Parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  HTTP_Parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    switch (yytype)
      {
        case 0: /* "\"end\"" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 3: /* "\"method\"" */

	{ debug_stream () << *(yyvaluep->sval); };

	break;
      case 4: /* "\"uri\"" */

	{ debug_stream () << *(yyvaluep->sval); };

	break;
      case 5: /* "\"version\"" */

	{ debug_stream () << *(yyvaluep->sval); };

	break;
      case 6: /* "\"header\"" */

	{ debug_stream () << *(yyvaluep->sval); };

	break;
      case 7: /* "\"delimiter\"" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 8: /* "\"status\"" */

	{ debug_stream () << *(yyvaluep->sval); };

	break;
      case 9: /* "\"reason\"" */

	{ debug_stream () << *(yyvaluep->sval); };

	break;
      case 10: /* "\"body\"" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 11: /* "\"chunk\"" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 12: /* "\"end_of_fragment\"" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 14: /* "message" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 15: /* "head" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 16: /* "head_rest1" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 17: /* "request_line_rest1" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 18: /* "request_line_rest2" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 19: /* "head_rest2" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 20: /* "status_line_rest1" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 21: /* "status_line_rest2" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 22: /* "headers" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 23: /* "body" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 24: /* "chunked_body" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
      case 25: /* "chunks" */

	{ debug_stream () << (yyvaluep->ival); };

	break;
       default:
	  break;
      }
  }


  void
  HTTP_Parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  HTTP_Parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
        case 0: /* "\"end\"" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 3: /* "\"method\"" */

	{ delete (yyvaluep->sval); (yyvaluep->sval) = NULL; };

	break;
      case 4: /* "\"uri\"" */

	{ delete (yyvaluep->sval); (yyvaluep->sval) = NULL; };

	break;
      case 5: /* "\"version\"" */

	{ delete (yyvaluep->sval); (yyvaluep->sval) = NULL; };

	break;
      case 6: /* "\"header\"" */

	{ delete (yyvaluep->sval); (yyvaluep->sval) = NULL; };

	break;
      case 7: /* "\"delimiter\"" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 8: /* "\"status\"" */

	{ delete (yyvaluep->sval); (yyvaluep->sval) = NULL; };

	break;
      case 9: /* "\"reason\"" */

	{ delete (yyvaluep->sval); (yyvaluep->sval) = NULL; };

	break;
      case 10: /* "\"body\"" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 11: /* "\"chunk\"" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 12: /* "\"end_of_fragment\"" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 14: /* "message" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 15: /* "head" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 16: /* "head_rest1" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 17: /* "request_line_rest1" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 18: /* "request_line_rest2" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 19: /* "head_rest2" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 20: /* "status_line_rest1" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 21: /* "status_line_rest2" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 22: /* "headers" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 23: /* "body" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 24: /* "chunked_body" */

	{ (yyvaluep->ival) = 0; };

	break;
      case 25: /* "chunks" */

	{ (yyvaluep->ival) = 0; };

	break;

	default:
	  break;
      }
  }

  void
  HTTP_Parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
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
#endif

  int
  HTTP_Parser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    semantic_type yylval;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[2];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* User initialization code.  */
    
{
  // initialize the location
  yylloc.initialize (NULL);
  //@$.begin.filename = @$.end.filename = &iparser_p->file;
  //ACE_OS::memset (&@$, 0, sizeof (@$));

  // initialize the token value container
  yylval.ival = 0;
  yylval.sval = NULL;
}


    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yyn == yypact_ninf_)
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
	YYCDEBUG << "Reading a token: ";
	yychar = yylex (&yylval, &yylloc, iparser_p, yyscanner);
      }


    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yyn == 0 || yyn == yytable_ninf_)
	goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
	  case 2:

    { (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) + (yysemantic_stack_[(3) - (2)].ival) + (yysemantic_stack_[(3) - (3)].ival); }
    break;

  case 3:

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].sval)->size () + (yysemantic_stack_[(2) - (2)].ival) + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.method =
                                                         HTTP_Tools::Method2Type (*(yysemantic_stack_[(2) - (1)].sval));
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set method: \"%s\"\n"),
//                                                                   ACE_TEXT ($1->c_str ())));
                                                     }
    break;

  case 4:

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].sval)->size () + (yysemantic_stack_[(2) - (2)].ival) + 1;
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
                                                       if (!std::regex_match (*(yysemantic_stack_[(2) - (1)].sval),
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP version (was: \"%s\"), aborting\n"),
                                                                     ACE_TEXT ((yysemantic_stack_[(2) - (1)].sval)->c_str ())));
                                                         YYABORT;
                                                       } // end IF
                                                       ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                                       ACE_ASSERT (match_results[1].matched);

                                                       record_r.version =
                                                           HTTP_Tools::Version2Type (match_results[1].str ());
//                                                         ACE_DEBUG ((LM_DEBUG,
//                                                                     ACE_TEXT ("set version: \"%s\"\n"),
//                                                                     ACE_TEXT (match_results[1].str ().c_str ())));
                                                     }
    break;

  case 5:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       YYACCEPT; }
    break;

  case 6:

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].ival) + (yysemantic_stack_[(2) - (2)].ival); }
    break;

  case 7:

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].sval)->size () + (yysemantic_stack_[(2) - (2)].ival) + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.URI = *(yysemantic_stack_[(2) - (1)].sval);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set URI: \"%s\"\n"),
//                                                                   ACE_TEXT ($1->c_str ())));
                                                     }
    break;

  case 8:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       YYACCEPT; }
    break;

  case 9:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].sval)->size () + 2;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.version =
                                                         HTTP_Tools::Version2Type (*(yysemantic_stack_[(1) - (1)].sval));
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set version: \"%s\"\n"),
//                                                                   ACE_TEXT ((*$1).c_str ())));
                                                     }
    break;

  case 10:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       YYACCEPT; }
    break;

  case 11:

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].ival) + (yysemantic_stack_[(2) - (2)].ival); }
    break;

  case 12:

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].sval)->size () + (yysemantic_stack_[(2) - (2)].ival) + 1;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       std::istringstream converter;
                                                       converter.str (*(yysemantic_stack_[(2) - (1)].sval));
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

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       YYACCEPT; }
    break;

  case 14:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].sval)->size () + 2;
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       record_r.reason = *(yysemantic_stack_[(1) - (1)].sval);
//                                                       ACE_DEBUG ((LM_DEBUG,
//                                                                   ACE_TEXT ("set reason: \"%s\"\n"),
//                                                                   ACE_TEXT ($1->c_str ())));
                                                     }
    break;

  case 15:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       YYACCEPT; }
    break;

  case 16:

    { /* NOTE*: use right-recursion here to force early state reductions
                                                                 (i.e. parse headers). This is required so the scanner can
                                                                 act on any set transfer encoding. */
                                                       (yyval.ival) = (yysemantic_stack_[(2) - (1)].ival) + (yysemantic_stack_[(2) - (2)].sval)->size ();
                                                       /* *TODO*: modify the scanner so it emits the proper fields itself */
                                                       std::string regex_string =
                                                         ACE_TEXT_ALWAYS_CHAR ("^([^:]+):\\s(.+)$");
                                                       std::regex regex (regex_string);
                                                       std::smatch match_results;
                                                       if (!std::regex_match (*(yysemantic_stack_[(2) - (2)].sval),
                                                                              match_results,
                                                                              regex,
                                                                              std::regex_constants::match_default))
                                                       {
                                                         ACE_DEBUG ((LM_ERROR,
                                                                     ACE_TEXT ("invalid HTTP header (was: \"%s\"), returning\n"),
                                                                     ACE_TEXT ((yysemantic_stack_[(2) - (2)].sval)->c_str ())));
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

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       yyclearin;
                                                       YYACCEPT; }
    break;

  case 18:

    { (yyval.ival) = 0; }
    break;

  case 19:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       struct HTTP_Record& record_r =
                                                         iparser_p->current ();
                                                       HTTP_HeadersIterator_t iterator =
                                                         record_r.headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING));
                                                       ACE_ASSERT (iterator != record_r.headers.end ());
                                                       std::istringstream converter;
                                                       converter.str ((*iterator).second);
                                                       unsigned int content_length = 0;
                                                       converter >> content_length;
                                                       if ((yysemantic_stack_[(1) - (1)].ival) == static_cast<int> (content_length))
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

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].ival) + (yysemantic_stack_[(2) - (2)].ival); }
    break;

  case 21:

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       yyclearin;
                                                       YYACCEPT; }
    break;

  case 22:

    { (yyval.ival) = (yysemantic_stack_[(3) - (1)].ival) + (yysemantic_stack_[(3) - (2)].ival) + (yysemantic_stack_[(3) - (3)].ival); // *TODO*: potential conflict here (i.e. incomplete chunk may be accepted)
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

    { (yyval.ival) = (yysemantic_stack_[(1) - (1)].ival);
                                                       yyclearin;
                                                       YYACCEPT; }
    break;

  case 24:

    { (yyval.ival) = 0;
                                                       YYACCEPT; }
    break;

  case 25:

    { (yyval.ival) = (yysemantic_stack_[(2) - (1)].ival) + (yysemantic_stack_[(2) - (2)].ival); }
    break;

  case 26:

    { (yyval.ival) = 0; }
    break;



	default:
          break;
      }
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[0] = yylloc;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

	if (yychar <= yyeof_)
	  {
	  /* Return failure if at end of input.  */
	  if (yychar == yyeof_)
	    YYABORT;
	  }
	else
	  {
	    yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
	    yychar = yyempty_;
	  }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
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

    yyerror_range[0] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (yyn != yypact_ninf_)
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	YYABORT;

	yyerror_range[0] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[1] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc);

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystate_stack_.height () != 1)
      {
	yydestruct_ ("Cleanup: popping",
		   yystos_[yystate_stack_[0]],
		   &yysemantic_stack_[0],
		   &yylocation_stack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  HTTP_Parser::yysyntax_error_ (int yystate, int tok)
  {
    std::string res;
    YYUSE (yystate);
#if YYERROR_VERBOSE
    int yyn = yypact_[yystate];
    if (yypact_ninf_ < yyn && yyn <= yylast_)
      {
	/* Start YYX at -YYN if negative to avoid negative indexes in
	   YYCHECK.  */
	int yyxbegin = yyn < 0 ? -yyn : 0;

	/* Stay within bounds of both yycheck and yytname.  */
	int yychecklim = yylast_ - yyn + 1;
	int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
	int count = 0;
	for (int x = yyxbegin; x < yyxend; ++x)
	  if (yycheck_[x + yyn] == x && x != yyterror_)
	    ++count;

	// FIXME: This method of building the message is not compatible
	// with internationalization.  It should work like yacc.c does it.
	// That is, first build a string that looks like this:
	// "syntax error, unexpected %s or %s or %s"
	// Then, invoke YY_ on this string.
	// Finally, use the string as a format to output
	// yytname_[tok], etc.
	// Until this gets fixed, this message appears in English only.
	res = "syntax error, unexpected ";
	res += yytnamerr_ (yytname_[tok]);
	if (count < 5)
	  {
	    count = 0;
	    for (int x = yyxbegin; x < yyxend; ++x)
	      if (yycheck_[x + yyn] == x && x != yyterror_)
		{
		  res += (!count++) ? ", expecting " : " or ";
		  res += yytnamerr_ (yytname_[x]);
		}
	  }
      }
    else
#endif
      res = YY_("syntax error");
    return res;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char HTTP_Parser::yypact_ninf_ = -14;
  const signed char
  HTTP_Parser::yypact_[] =
  {
        -1,     2,    -3,   -14,     7,    12,    -2,   -14,   -14,    -4,
       6,   -14,   -14,    -4,   -14,    11,   -14,   -14,   -14,   -14,
      18,   -14,   -14,   -14,    18,   -14,     1,   -14,   -14,   -14,
      14,   -14,   -14,    -4,   -14,    10,   -14
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  HTTP_Parser::yydefact_[] =
  {
         0,     0,     0,     5,     0,     0,     0,     8,     3,    18,
       0,    13,     4,    18,     1,     0,     9,    10,     7,    17,
       6,    14,    15,    12,    11,    19,    26,    21,     2,    16,
      26,    23,    20,    18,    25,     0,    22
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  HTTP_Parser::yypgoto_[] =
  {
       -14,   -14,   -14,   -14,   -14,   -14,   -14,   -14,   -14,   -13,
     -14,   -14,     0
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  HTTP_Parser::yydefgoto_[] =
  {
        -1,     4,     5,     8,     9,    18,    12,    13,    23,    20,
      28,    32,    33
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char HTTP_Parser::yytable_ninf_ = -25;
  const signed char
  HTTP_Parser::yytable_[] =
  {
        24,   -24,     1,    16,     2,    10,     6,    14,    19,    11,
      17,     3,    30,    31,     7,    21,    29,    36,    22,    15,
      35,    25,    26,    27,    29,    30,     0,     0,     0,     0,
      34
  };

  /* YYCHECK.  */
  const signed char
  HTTP_Parser::yycheck_[] =
  {
        13,     0,     3,     5,     5,     8,     4,     0,    12,    12,
      12,    12,    11,    12,    12,     9,     6,     7,    12,     7,
      33,    10,    11,    12,     6,    11,    -1,    -1,    -1,    -1,
      30
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  HTTP_Parser::yystos_[] =
  {
         0,     3,     5,    12,    14,    15,     4,    12,    16,    17,
       8,    12,    19,    20,     0,     7,     5,    12,    18,    12,
      22,     9,    12,    21,    22,    10,    11,    12,    23,     6,
      11,    12,    24,    25,    25,    22,     7
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  HTTP_Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  HTTP_Parser::yyr1_[] =
  {
         0,    13,    14,    15,    15,    15,    16,    17,    17,    18,
      18,    19,    20,    20,    21,    21,    22,    22,    22,    23,
      23,    23,    24,    24,    24,    25,    25
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  HTTP_Parser::yyr2_[] =
  {
         0,     2,     3,     2,     2,     1,     2,     2,     1,     1,
       1,     2,     2,     1,     1,     1,     2,     1,     0,     1,
       2,     1,     3,     1,     0,     2,     0
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const HTTP_Parser::yytname_[] =
  {
    "\"end\"", "error", "$undefined", "\"method\"", "\"uri\"",
  "\"version\"", "\"header\"", "\"delimiter\"", "\"status\"", "\"reason\"",
  "\"body\"", "\"chunk\"", "\"end_of_fragment\"", "$accept", "message",
  "head", "head_rest1", "request_line_rest1", "request_line_rest2",
  "head_rest2", "status_line_rest1", "status_line_rest2", "headers",
  "body", "chunked_body", "chunks", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const HTTP_Parser::rhs_number_type
  HTTP_Parser::yyrhs_[] =
  {
        14,     0,    -1,    15,     7,    23,    -1,     3,    16,    -1,
       5,    19,    -1,    12,    -1,    17,    22,    -1,     4,    18,
      -1,    12,    -1,     5,    -1,    12,    -1,    20,    22,    -1,
       8,    21,    -1,    12,    -1,     9,    -1,    12,    -1,    22,
       6,    -1,    12,    -1,    -1,    10,    -1,    11,    24,    -1,
      12,    -1,    25,    22,     7,    -1,    12,    -1,    -1,    11,
      25,    -1,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  HTTP_Parser::yyprhs_[] =
  {
         0,     0,     3,     7,    10,    13,    15,    18,    21,    23,
      25,    27,    30,    33,    35,    37,    39,    42,    44,    45,
      47,    50,    52,    56,    58,    59,    62
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
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
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  HTTP_Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
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
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int HTTP_Parser::yyeof_ = 0;
  const int HTTP_Parser::yylast_ = 30;
  const int HTTP_Parser::yynnts_ = 13;
  const int HTTP_Parser::yyempty_ = -2;
  const int HTTP_Parser::yyfinal_ = 14;
  const int HTTP_Parser::yyterror_ = 1;
  const int HTTP_Parser::yyerrcode_ = 256;
  const int HTTP_Parser::yyntokens_ = 13;

  const unsigned int HTTP_Parser::yyuser_token_number_max_ = 267;
  const HTTP_Parser::token_number_type HTTP_Parser::yyundef_token_ = 2;


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

