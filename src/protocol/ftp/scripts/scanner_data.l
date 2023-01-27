%top{
#undef YYTOKENTYPE
#include "ftp_parser_data.h"

/*
#define YY_DECL                                                         \
yy::FTP_Parser::token_type                                              \
FTP_Scanner_Data_lex (yy::FTP_Parser_Data::semantic_type* yylval_param, \
                      yy::FTP_Parser_Data::location_type* yylloc_param, \
                      FTP_IParserData* driver,                          \
                      yyscan_t yyscanner) */
/* yytokentype */
#define YY_DECL                                \
int                                            \
FTP_Scanner_Data_lex (YYSTYPE* yylval_param,   \
                      YYLTYPE* yylloc_param,   \
                      FTP_IParserData* driver, \
                      yyscan_t yyscanner)
// ... and declare it for the parser's sake
extern YY_DECL;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int FTP_Scanner_Data_reset (yyscan_t);
#ifdef __cplusplus
}
#endif /* __cplusplus */
//extern void FTP_Scanner_Data_set_column (int, yyscan_t);

//using namespace yy;
//#define YYLTYPE FTP_Parser_Data::location_type
//#define YYSTYPE FTP_Parser_Data::semantic_type
}

%{
#include <sstream>
#include <string>

#include "ace/OS_Memory.h"

#include "net_macros.h"

#include "ftp_common.h"
#include "ftp_iparser.h"
#include "ftp_tools.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
//#define yyterminate() return yy::FTP_Parser_Data::token::END
#define yyterminate() return yytokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option prefix="FTP_Scanner_Data_"

%option header-file="ftp_scanner_data.h" outfile="ftp_scanner_data.cpp"
%option batch 8bit never-interactive yylineno
%option nodefault nostdinit yywrap nomain nounput
/* %option c++ yyclass="FTP_Scanner_Data" */
/* %option ansi-definitions ansi-prototypes bison-bridge */
/* *NOTE*: the 'line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
%option bison-locations
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
%option reentrant pointer
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, header-file extra-type'
*/
%option extra-type="FTP_IParserData*"
/* %option nounput */

OCTET          [\x00-\xFF]
DIGIT          [[:digit:]]
MINUS          \x2D
SPACE          \x20
CRLF           \x0D\x0A

%s             list_directory
%s             list_file
%s             data

%{
/* handle locations */
/*#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;                        \
                         yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; \
                         yycolumn += yyleng; */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno;                        \
                       yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%%
%{
  //yylloc->step ();
  std::istringstream converter;
%}

<INITIAL>{
{OCTET}{1}                 {
                             switch (driver->state ())
                             {
                               case FTP_STATE_DATA_LIST_DIRECTORY:
                               {
                                 ACE_NEW_NORETURN (yylval->sval,
                                                   std::string ());
                                 ACE_ASSERT (yylval->sval);
                                 BEGIN(list_directory);
                                 break;
                               }
                               case FTP_STATE_DATA_LIST_FILE:
                               {
                                 ACE_NEW_NORETURN (yylval->sval,
                                                   std::string ());
                                 ACE_ASSERT (yylval->sval);
                                 BEGIN(list_file);
                                 break;
                               }
                               case FTP_STATE_DATA_DATA:
                                 BEGIN(data); break;
                               default:
                               {
                                 /* *NOTE*: should not be reached, unless something
                                            went terribly wrong */
                                 ACE_ASSERT (false);

                                 // debug info
                                 std::string error_message =
                                   ACE_TEXT_ALWAYS_CHAR ("invalid state: \"");
                                 error_message += FTP_Tools::DataStateToString (driver->state ());
                                 error_message += ACE_TEXT_ALWAYS_CHAR ("\", aborting");
                                 driver->error (*yylloc, error_message);
                                 yyterminate();
                               }
                             } // end SWITCH
                             yycolumn -= yyleng;
                             yyless (0); }
} // end <INITIAL>
<list_directory>{
{CRLF}                     { driver->offset (yyleng);
                             BEGIN(INITIAL);
                             return yytokentype::LIST_DIRECTORY;
                             /* return yy::FTP_Parser_Data::token::LIST_DIRECTORY; */
                           }
[^\r]+                     { driver->offset (yyleng);
                             yylval->sval->append (yytext);
                           }
} // end <list_directory>
<list_file>{
{OCTET}{1}                 { driver->offset (yyleng);
                             yylval->sval->append (yytext);
                             if (driver->offset () == driver->availableBytes ())
                             {
                               BEGIN(INITIAL);
                               return yytokentype::LIST_FILE;
                               /* return yy::FTP_Parser_Data::token::LIST_FILE; */
                             } // end IF
                           }
} // end <list_file>
<data>{
{OCTET}{1}                 { driver->offset (yyleng);
                             if (driver->offset () == driver->availableBytes ())
                             {
                               BEGIN(INITIAL);
                               return yytokentype::RAW_DATA;
                               /* return yy::FTP_Parser_Data::token::RAW_DATA; */
                             } // end IF
                           }
} // end <data>
<<EOF>>                    { yyterminate(); }
<*>{OCTET}{1}              { /* *NOTE*: should not be reached, unless something
                                        went terribly wrong */
                             ACE_ASSERT (false);

                             // debug info
                             std::string error_message =
                               ACE_TEXT_ALWAYS_CHAR ("invalid character: \"");
                             error_message += yytext[0];
                             error_message += ACE_TEXT_ALWAYS_CHAR ("\", aborting");
                             driver->error (*yylloc, error_message);
                             yyterminate(); }
%%

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
FTP_Scanner_Data_reset (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::FTP_Scanner_Data_reset"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  FTP_IParserData* driver_p = FTP_Scanner_Data_get_extra (yyscanner);
  ACE_ASSERT (driver_p);

  BEGIN(INITIAL);

  return 0;
}

int
FTP_Scanner_Data_wrap (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::FTP_Scanner_Data_wrap"));

  // sanity check(s)
  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  ACE_ASSERT (yyg);
  FTP_IParserData* driver_p = FTP_Scanner_Data_get_extra (yyscanner);
  ACE_ASSERT (driver_p);

  if (!driver_p->isBlocking ())
    return 1; // not enough data, cannot proceed
//  if (driver_p->hasFinished ())
//  {
//    BEGIN(INITIAL);
//    return 1; // done
//  } // end IF

  // *NOTE*: there is more data
  // 1. gobble/save the rest
  // 2. switch buffers
  // 3. unput the rest
  // 4. continue scanning

  // step1
  //std::string the_rest;
  //the_rest.append (yytext, yyleng);
//  for (char c = yyinput (yyscanner);
//       c != EOF;
//       c = yyinput (yyscanner));
  //yyg->yy_c_buf_p += yyleng;
  //yyg->yy_hold_char = *yyg->yy_c_buf_p;
  //if (yy_flex_debug)
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("the rest: \"%s\" (%d byte(s))\n"),
  //              ACE_TEXT (the_rest.c_str ()),
  //              the_rest.size ()));

  // step2
  if (!driver_p->switchBuffer ())
  {
    // *NOTE*: most probable reason: received session end message
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_IParser::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  // step3
  //for (std::string::reverse_iterator iterator = the_rest.rbegin ();
  //     iterator != the_rest.rend ();
  //     ++iterator)
  //  unput (*iterator);

  // step4
  //yyg->yy_did_buffer_switch_on_eof = 1;
  // yymore ();

  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
