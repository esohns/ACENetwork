%{
#undef YYTOKENTYPE
#include "test_i_av_stream_parser.h"
#include "test_i_av_stream_parser_driver.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
#define yyterminate() return END
%}

%option 8bit align backup batch
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option debug nodefault fast never-interactive perf-report noread
/* %option c++ yyclass="AVStream_Scanner" stack noline bison-bridge */
/*         bison-locations */
%option bison-bridge bison-locations
%option verbose warn
%option pointer
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd, */
/*                   ansi-definitions, ansi-prototypes, header-file' */
%option reentrant nounistd
%option header-file="test_i_av_stream_scanner.h"
/* *IMPORTANT NOTE*: 'yylineno' is not compatible with 'read' */
%option nomain nostdinit
%option outfile="test_i_av_stream_scanner.cpp" prefix="AVStream_Scanner_"
%option nounput
%option extra-type="AVStream_ParserDriver*"

CHAR           [\x00-\xFF]

%s             header
%s             length
%s             body

%{
//#define YY_USER_ACTION yylloc->columns(yyleng);
%}

%%
%{
%}

<INITIAL>{
{CHAR}{1}                  { BEGIN(header);
                             yyless(0);
                           }
} // end <INITIAL>
<header>{
{CHAR}{1}                  { //yylloc->step();
                             BEGIN(length);
                             yylval->ival = static_cast<ACE_UINT32> (yytext[0]);
                             return TYPE; }
} // end <header>
<length>{
{CHAR}{4}                  { //yylloc->step();
                             BEGIN(body);
                             yylval->ival = *reinterpret_cast<ACE_UINT32*> (yytext);
                             return LENGTH; }
} // end <length>
<body>{
{CHAR}{1}                  { //yylloc->step();
                             char c = 0;
                             for (unsigned int i = 0;
                                  i < (driver->missing_ - 1);
                                  ++i)
                             {
                               c = yyinput (yyscanner);
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;
                             } // end FOR
                             yylval->ival = driver->missing_;
                             BEGIN(header);
                             return BODY; }
} // end <body>
<<EOF>>                    { yyterminate(); }
<*>{CHAR}                  { /* *TODO*: use (?s:.) ? */
                             //yylloc->step();

                             // debug info
                             std::string error_message = ACE_TEXT("invalid character: \"");
                             error_message += yytext[0];
                             error_message += ACE_TEXT("\", aborting");
                             driver->error(*yylloc, error_message);

                             yyterminate(); }
%%

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
AVStream_Scanner_wrap (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::AVStream_Scanner_wrap"));

  // sanity check(s)
  ACE_ASSERT (yyscanner);
  AVStream_ParserDriver* driver = AVStream_Scanner_get_extra (yyscanner);
  ACE_ASSERT (driver);

  if (!driver->switchBuffer())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to AVStream_ParserDriver::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
