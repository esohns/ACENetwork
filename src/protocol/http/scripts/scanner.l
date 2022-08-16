%top{
#include "http_common.h"
//#include "http_exports.h"
#include "http_iparser.h"
//#undef YYTOKENTYPE
#include "http_parser.h"

#if !defined (YY_DECL)
/*#define YY_DECL                                           \
yy::HTTP_Parser::token_type                               \
HTTP_Scanner_lex (yy::HTTP_Parser::semantic_type* yylval, \
                  yy::HTTP_Parser::location_type* yylloc, \
                  HTTP_IParser* iparser_p,                \
                  yyscan_t yyscanner)*/
#define YY_DECL                            \
enum yytokentype                           \
HTTP_Scanner_lex (YYSTYPE* yylval,         \
                  YYLTYPE* yylloc,         \
                  HTTP_IParser* iparser_p, \
                  yyscan_t yyscanner)
YY_DECL;
#endif

//using namespace yy;
/*#define YYLTYPE yy::location
#define YYSTYPE yy::HTTP_Parser::semantic_type*/

void HTTP_Scanner_set_column (int, yyscan_t);

//#define FLEXINT_H
}

%{
#include <cctype>
#include <regex>
#include <sstream>
#include <string>

#include "ace/ace_wchar.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "common_string_tools.h"
#include "common_tools.h"

#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"

/* *NOTE*: instead of the default (int), this yylex returns token_type.
           Unfortunately, yyterminate by default returns 0, which is not of
           token_type. This %define solves that issue. */
//#define yyterminate() return yy::HTTP_Parser::token::END
#define yyterminate() return yytokentype::END

/* *NOTE*: YY_DECL is defined elsewhere (needed so the scanner can support
           extra arguments) */
//#define YY_DECL
%}

%option prefix="HTTP_Scanner_"

%option header-file="http_scanner.h" outfile="http_scanner.cpp"
%option yylineno yywrap
%option nomain nounput noyymore noreject nodefault nostdinit noline
/* %option nostack noline nounistd */
%option nostack nounistd
/* %option c++ yyclass="HTTP_Scanner" */
/* *NOTE*: the '#line' directives confuse gdb */
/* *NOTE*: 'noline' not supported (on Linux, flex 2.5.39)
           --> use --noline and (manually) remove '#line's introduced by %top */
/* %option ansi-definitions ansi-prototypes */
/* *IMPORTANT NOTE*: do NOT mess with these (it's broken) */
/* %option bison-bridge bison-locations */
%option 8bit batch never-interactive
%option reentrant pointer
/* *IMPORTANT NOTE*: 'read' requires 'unistd'(.h) */
/* *TODO*: find out why 'read' does not compile (on Linux, flex 2.5.39) */
%option align read full
%option backup debug perf-report perf-report verbose warn
/* *IMPORTANT NOTE*: flex 2.5.4 does not recognize 'reentrant, nounistd,
                     ansi-definitions, ansi-prototypes, trace header-file extra-type'
*/
%option extra-type="HTTP_IParser*"
/* *NOTE*: option 'tables-file' breaks 'yylineno' (flex 2.6.3), which crashes
           the lexer when 'debug' is set (apparently, array yy_rule_linenum is
           not initialized in this case) */
/* %option tables-file="http_scanner.tab" */

/* *NOTE*: see RFC 1945 page 10 */
OCTET          [\x00-\xFF]
/* *NOTE*: (US-)ASCII */
CHAR           [\x00-\x7F]
UPALPHA        [A-Z]
LOALPHA        [a-z]
ALPHA          [[:alpha:]]
DIGIT          [[:digit:]]
CTL            [\x00-\x1F\x7F]
CR             \x0D
LF             \x0A
SP             \x20
/* *NOTE*: (horizontal) TAB */
HT             \x09
/* DOUBLE_QUOTE   \x22 */


CRLF           \x0D\x0A
LWS            (\x0D\x0A(\x20|\x09){1})
/* *NOTE*: flex 2.5.4 does not support class unions ({+}) */
TEXT           ([\x00-\xFF]{-}[\x00-\x1F]{+}[\x09\x0A\x0D])*
HEX            [a-fA-F[:digit:]]

T_SPECIALS     [()<>@,;:\\"/[\]?={} \x09]
TOKEN          [\x00-\x7F]{-}[\x00-\x1F\x7F]{+}[()<>@,;:\\"/[\]?={} \x09]

C_TEXT         ([\x00-\xFF]{-}[\x00-\x1F]{+}[\x09\x0A\x0D\x20]{-}[()])*
COMMENT        \x22([\x00-\xFF]{-}[\x00-\x1F]{+}[\x09\x0A\x0D\x20]{-}[()])*\x22

QD_TEXT        ([\x00-\xFF]{-}[\x00-\x1F\x22]{+}[\x09\x0A\x0D\x20])*
QUOTED_STRING  \x22([\x00-\xFF]{-}[\x00-\x1F\x22]{+}[\x09\x0A\x0D\x20])*\x22

WORD           ({TOKEN}|{QUOTED_STRING})

/* HTTP Version */
VERSION        HTTP\/[[:digit:]]{1}\.[[:digit:]]{1}


/* ESCAPE         (%(HEX)(HEX)) */
ESCAPE         (%({HEX}{1})({HEX}{1}))

RESERVED       [;/?:@&=+]
EXTRA          [!*'(),]
SAFE           [$\-_.]
/* (CTL|SP){+}["#%<>] */
UNSAFE         [\x00-\x1F\x7F]{+}[\x20]{+}["#%<>]
/* (OCTET){-}([[:alnum:]]{+}(RESERVED|EXTRA|SAFE|UNSAFE)) */
/* NATIONAL       [^[:alnum:]]{-}[;/?:@&=+]{-}[!*'(),]{-}[$\-_.]{-}([\x00-\x1F\x7F]{+}[\x20]{+}["#%<>]) */
NATIONAL       [\x00-\xFF]{-}[[:alnum:]]{-}[;/?:@&=+]{-}[!*'(),]{-}[$\-_.]{-}[\x00-\x1F\x7F]{-}[\x20]{-}["#%<>]

/* [[:alnum:]]{+}(SAFE|EXTRA|NATIONAL) */
/* UNRESERVED     [[:alnum:]]{+}[$\-_.]{+}[!*'(),]{+}{NATIONAL} */
UNRESERVED     [\x00-\xFF]{-}[[:alnum:]]{-}[;/?:@&=+]{-}[!*'(),]{-}[$\-_.]{-}[\x00-\x1F\x7F]{-}[\x20]{-}["#%<>]{+}[[:alnum:]]{+}[$\-_.]{+}[!*'(),]
/* UNRESERVED     [[:alnum:]]{+}[$\-_.]{+}[!*'(),] */
/* (UNRESERVED|ESCAPE) */
/* U_CHAR         {UNRESERVED}|{ESCAPE} */
/* U_CHAR         [[:alnum:]]{+}{SAFE}{+}{EXTRA}{+}{NATIONAL} */
U_CHAR         {UNRESERVED}|{ESCAPE}
/* (U_CHAR|:|@|&|=|+) */
P_CHAR         [[:alnum:]$\-_.!*'(),%a-fA-F:@&=+]{+}[^[:alnum:];/?:@&=+!*'(),$\-_.\x00-\x1F\x7F\x20"#%<>]


/* Uniform Resource Identifiers */
NET_LOCATION   ({P_CHAR}|;|?)*
F_SEGMENT      {P_CHAR}{1}
SEGMENT        {P_CHAR}*
PATH           {F_SEGMENT}(\/{SEGMENT})*
PARAMETER      ({P_CHAR}|\/)*
PARAMETERS     {PARAMETER}(;{PARAMETER})*
QUERY          ({U_CHAR}|{RESERVED})*
/* *TODO*: specification is {PATH}{0,1}(;{PARAMETERS}){0,1}(?{QUERY_}){0,1}
           however, flex (2.5.39) does not compile */
RELATIVE_PATH  {PATH}{0,1}
ABSOLUTE_PATH  \/{RELATIVE_PATH}
NET_PATH       \/\/{NET_LOCATION}({ABSOLUTE_PATH}){0,1}
RELATIVE_URI   ({NET_PATH}|{ABSOLUTE_PATH}|{RELATIVE_PATH})
SCHEME         [[:alpha:][:digit:]+\-.]+
ABSOLUTE_URI   {SCHEME}:({U_CHAR}|{RESERVED})*
FRAGMENT       ({U_CHAR}|{RESERVED})*
URI            ({ABSOLUTE_URI}|{RELATIVE_URI})({FRAGMENT}(,{FRAGMENT})*)


/* http URL */
PORT           [[:digit:]]+
/* *NOTE*: "see RFC 952 [DNS:4] for details on allowed hostnames..."
RFC 952
1. A "name" (Net, Host, Gateway, or Domain name) is a text string up
   to 24 characters drawn from the alphabet (A-Z), digits (0-9), minus
   sign (-), and period (.).  Note that periods are only allowed when
   they serve to delimit components of "domain style names".
RFC 1123
The syntax of a legal Internet host name was specified in RFC-952.
One aspect of host name syntax is hereby changed: the restriction on
the first character is relaxed to allow either a letter or a digit.

And so on...
--> [[:alpha:][:digit:].-] is just a somewhat informed and tolerant suggestion.
    A more CORRECT implementation would probably look something like this...
<hostname>    = <label> | <hostname>.<label>
<label>       = <let-dig> | <let-dig><let-dig> | <let-dig><ldh-string><let-dig>
<ldh-string>  = <let-dig-hyp> | <ldh-string><let-dig-hyp>
<let-dig-hyp> = <let-dig> | '-'
<let-dig>     = <letter> | <digit>
<digit>       = [[:digit:]]
<letter>      = [[:alpha:]]*/
HOST           ([[:alpha:][:digit:].\-])+
URL            http://{HOST}(:{PORT}){0,1}{ABSOLUTE_PATH}


/* Date/Time Formats */
WKDAY          (Mon|Tue|Wed|Thu|Fri|Sat|Sun)
MONTH          (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)
DATE1          [[:digit:]]{2}{SP}{MONTH}{SP}[[:digit:]]{4}
TIME           [[:digit:]]{2}:[[:digit:]]{2}:[[:digit:]]{2}
RFC1123_DATE   {WKDAY},{SP}{DATE1}{SP}{TIME}{SP}GMT
WEEKDAY        (Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday)
DATE2          [[:digit:]]{2}-{MONTH}-[[:digit:]]{2}
RFC850_DATE    {WEEKDAY},{SP}{DATE2}{SP}{TIME}{SP}GMT
DATE3          [[:digit:]]{2}-{MONTH}-[[:digit:]]{2}
ASCTIME_DATE   {WKDAY}{SP}{DATE3}{SP}{TIME}{SP}[[:digit:]]{4}
DATE           ({RFC1123_DATE}|{RFC850_DATE}|{ASCTIME_DATE})


/* Character Sets */
/* NOTE*: "...Applications should limit their use of character sets to those
          defined by the IANA registry." */
CHARSET_TOKEN  {TOKEN}+
/* *NOTE*: "When no explicit charset parameter is provided by the sender, media
           subtypes of the "text" type are defined to have a default charset
           value of "ISO-8859-1" when received via HTTP" */
CHARSET        (US-ASCII|ISO-8859-1|ISO-8859-2|ISO-8859-3|ISO-8859-4|ISO-8859-5|ISO-8859-6|ISO-8859-7|ISO-8859-8|ISO-8859-9|ISO-2022-JP|ISO-2022-JP-2|ISO-2022-KR|UNICODE-1-1|UNICODE-1-1-UTF-7|UNICODE-1-1-UTF-8|{CHARSET_TOKEN})


/* Content Coding */
CODING_TOKEN   {TOKEN}+
CONTENT_CODING (x-gzip|x-compress|{CODING_TOKEN})


/* Media Types */
/* *NOTE*: "Media-type values are registered with the Internet Assigned Number
           Authority (IANA)" */
TYPE           {TOKEN}+
SUB_TYPE       {TOKEN}+
ATTRIBUTE      {TOKEN}+
VALUE          ({TOKEN}+|{QUOTED_STRING})
TYPE_PARAMETER {ATTRIBUTE}={VALUE}
MEDIA_TYPE     {TYPE}\/{SUB_TYPE}(;{TYPE_PARAMETER})*


/* Product Tokens */
PRODUCT_TOKEN   {TOKEN}+
PRODUCT_VERSION {PRODUCT_TOKEN}
PRODUCT         {PRODUCT_TOKEN}(\/{PRODUCT_VERSION}){0,1}


/* Message Headers (generic) */
FIELD_NAME      {TOKEN}+
/* ({TEXT}|[{TOKEN}{T_SPECIALS}{QUOTED_STRING}]*|{LWS})* */
FIELD_VALUE     {TOKEN}+
HEADER          {FIELD_NAME}:({SP}{FIELD_VALUE}){0,1}

/* General Headers */
HEADER_DATE     Date:{SP}{DATE}
PRAGMA_TOKEN    {TOKEN}+(={WORD}){0,1}
PRAGMA_VALUE    (no-cache|{PRAGMA_TOKEN})
HEADER_PRAGMA   Pragma:{SP}{PRAGMA_VALUE}
GENERAL_HEADER  ({HEADER_DATE}|{HEADER_PRAGMA})

/* Request Headers */
/* *TODO*: see rfc1945 page 47 */
CREDENTIALS            {TOKEN}+
HEADER_AUTHORIZATION   Authorization:{SP}{CREDENTIALS}
/* *TODO*: e-mail address, see rfc822/1123 */
MAILBOX                {TOKEN}+
HEADER_FROM            From:{SP}{MAILBOX}
HEADER_IFMODIFIEDSINCE If-Modified-Since:{SP}{DATE}
HEADER_REFERER         Referer:{SP}(ABSOLUTE_URI|RELATIVE_URI)
HEADER_USERAGENT       User-Agent:{SP}({PRODUCT}|{COMMENT})
REQUEST_HEADER         ({HEADER_AUTHORIZATION}|{HEADER_FROM}|{HEADER_IFMODIFIEDSINCE}|{HEADER_REFERER}|{HEADER_USERAGENT})

/* Request-Line */
REQUEST_TOKEN   {CHAR}+
/* TODO*: allowing {REQUEST_TOKEN} breaks precedence in INITIAL state, as it
          gobbles the whole buffer */
/* METHOD          (GET|HEAD|POST|{REQUEST_TOKEN}) */
METHOD          (GET|HEAD|POST)
REQUEST_URI     ({ABSOLUTE_URI}|{ABSOLUTE_PATH})
REQUEST         {METHOD}{SP}{REQUEST_URI}{SP}{VERSION}

/* Response Headers */
LOCATION_HEADER        Location:{SP}{ABSOLUTE_URI}
SERVER_HEADER          Server:{SP}({PRODUCT}|{COMMENT})
AUTH_SCHEME            {TOKEN}+
REALM_VALUE            {QUOTED_STRING}
REALM                  realm={REALM_VALUE}
AUTH_PARAMETER         {TOKEN}+={QUOTED_STRING}
AUTH_PARAMETERS        (,{AUTH_PARAMETER})*
CHALLENGE              {AUTH_SCHEME}{SP}{REALM}{AUTH_PARAMETERS}
WWWAUTHENTICATE_HEADER WWW-Authenticate:{SP}{CHALLENGE}
RESPONSE_HEADER        ({LOCATION_HEADER}|{SERVER_HEADER}|{WWWAUTHENTICATE_HEADER})

/* Status-Line (Response-) */
/* ({TEXT}{-}{CRLF})* */
REASON          ([\x00-\xFF]{-}[\x00-\x1F]{+}[\x09])*
STATUS          {VERSION}{SP}[[:digit:]]{3}{SP}{REASON}

/* Entity Headers */
ALLOW_HEADER           Allow:{SP}{METHOD}+
CONTENTENCODING_HEADER Content-Encoding:{SP}{CONTENT_CODING}
CONTENTLENGTH_HEADER   Content-Length:{SP}[[:digit:]]+
CONTENTTYPE_HEADER     Content-Type:{SP}{MEDIA_TYPE}
EXPIRES_HEADER         Expires:{SP}{DATE}
LASTMODIFIED_HEADER    Last-Modified:{SP}{DATE}
EXTENSION_HEADER       {HEADER}
ENTITY_HEADER          ({ALLOW_HEADER}|{CONTENTENCODING_HEADER}|{CONTENTLENGTH_HEADER}|{CONTENTTYPE_HEADER}|{EXPIRES_HEADER}|{LASTMODIFIED_HEADER}|{EXTENSION_HEADER})

/* Chunks (*NOTE*: HTTP/1.1) */
CHUNKSIZE              {HEX}+
CHUNK_EXTENSION        {TOKEN}+(={TOKEN}+|{QUOTED_STRING}){0,1}
CHUNK_EXTENSIONS       (;{CHUNK_EXTENSION})*
CHUNK_LINE             {CHUNKSIZE}{CHUNK_EXTENSIONS}
CHUNK_DATA             {OCTET}+
/* CHUNK                  {CHUNK_LINE}{CRLF}{CHUNK_DATA}{CRLF} */
CHUNK_LINE_LAST        0{CHUNK_EXTENSIONS}
CHUNK_TRAILER          ({ENTITY_HEADER}{CRLF})*

%s             request
%s             method
%s             uri
%s             version
%s             request_headers
%s             response
%s             status
%s             reason
%s             response_headers
%s             body
%s             chunked_body
%s             chunk
%s             data
%s             data_2

%{
/* handle locations */
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
                       yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;
/*#define YY_USER_ACTION yylloc->begin.line = yylloc->end.line = yylineno; \
                       yylloc->begin.column = yycolumn; yylloc->end.column = yycolumn+yyleng-1; \
                       yycolumn += yyleng;*/
//#define YY_USER_ACTION yylloc->columns (yyleng);
%}

%%
%{
  std::string regex_string;
  std::regex::flag_type regex_flags = std::regex_constants::ECMAScript;
  std::regex regex;
  std::smatch match_results;
  std::istringstream converter;
  unsigned int content_length_or_chunk_size = 0;
  unsigned int missing_bytes = 0;
%}

<INITIAL>{
{VERSION}                  {
                             BEGIN (response);
                             yycolumn -= yyleng;
                             yyless (0); }
{METHOD}                   {
                             BEGIN (request);
                             yycolumn -= yyleng;
                             yyless (0); }
} // end <INITIAL>
<request>{
{METHOD}{SP}               { iparser_p->offset (yyleng);
                             BEGIN (uri);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 1);
/*                             return yy::HTTP_Parser::token::METHOD; } */
                             return yytokentype::METHOD; }
} // end <request>
<uri>{
{REQUEST_URI}{SP}          { iparser_p->offset (yyleng);
                             BEGIN (version);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 1);
/*                             return yy::HTTP_Parser::token::URI; } */
                             return yytokentype::URI; }
} // end <uri>
<version>{
{VERSION}{CRLF}            { iparser_p->offset (yyleng);
                             BEGIN (request_headers);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::_VERSION; } */
                             return yytokentype::_VERSION; }
} // end <version>
<request_headers>{
{GENERAL_HEADER}{CRLF}     { iparser_p->offset (yyleng);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::HEADER; } */
                             return yytokentype::HEADER; }
{REQUEST_HEADER}{CRLF}     { iparser_p->offset (yyleng);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::HEADER; } */
                             return yytokentype::HEADER; }
{ENTITY_HEADER}{CRLF}      { iparser_p->offset (yyleng);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::HEADER; } */
                             return yytokentype::HEADER; }
{CRLF}                     { iparser_p->offset (yyleng);
                             yylval->ival = 2;
                             BEGIN (body);
/*                             return yy::HTTP_Parser::token::DELIMITER; } */
                             return yytokentype::DELIMITER; }
} // end <request_headers>
<response>{
{VERSION}{SP}              { iparser_p->offset (yyleng);
                             BEGIN (status);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 1);
/*                             return yy::HTTP_Parser::token::_VERSION; } */
                             return yytokentype::_VERSION; }
} // end <response>
<status>{
[[:digit:]]{3}{SP}         { iparser_p->offset (yyleng);
                             BEGIN (reason);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 1);
/*                             return yy::HTTP_Parser::token::STATUS; } */
                             return yytokentype::STATUS; }
} // end <status>
<reason>{
{REASON}{CRLF}             { iparser_p->offset (yyleng);
                             BEGIN (response_headers);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::REASON; } */
                             return yytokentype::REASON; }
} // end <reason>
<response_headers>{
{GENERAL_HEADER}{CRLF}     { iparser_p->offset (yyleng);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::HEADER; } */
                             return yytokentype::HEADER; }
{RESPONSE_HEADER}{CRLF}    { iparser_p->offset (yyleng);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::HEADER; } */
                             return yytokentype::HEADER; }
{ENTITY_HEADER}{CRLF}      { iparser_p->offset (yyleng);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::HEADER; } */
                             return yytokentype::HEADER; }
{CRLF}                     { iparser_p->offset (yyleng);
                             struct HTTP_Record& record_r =
                               iparser_p->current ();
                             HTTP_HeadersIterator_t iterator =
                                 record_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
                             if (iterator != record_r.headers.end ())
                             {
                               converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                               converter.clear ();
                               converter.str ((*iterator).second);
                               converter >> content_length_or_chunk_size;
                               if (!content_length_or_chunk_size)
                                 BEGIN (INITIAL); // no data
                               else
                                 BEGIN (body);
                             } // end IF
                             else
                             {
                               iterator =
                                   record_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_TRANSFER_ENCODING_STRING)));
                               if (iterator != record_r.headers.end ())
                               {
                                 // *NOTE*: the default locale is the 'C' locale (as in:
                                 //         'std::setlocale(LC_ALL, "C")')
                                 //         --> replace with (C++-)US-ASCII
//                                 std::locale locale;
//                                 //  std::locale locale (ACE_TEXT_ALWAYS_CHAR (""));
//                                 try {
//                                   std::locale us_ascii_locale (ACE_TEXT_ALWAYS_CHAR (COMMON_LOCALE_EN_US_STRING));
//                                   locale = us_ascii_locale;
//                                 } catch (std::runtime_error exception_in) {
//                                   ACE_DEBUG ((LM_ERROR,
//                                               ACE_TEXT ("caught exception in std::locale(\"%s\"): \"%s\", aborting\n"),
//                                               ACE_TEXT (COMMON_LOCALE_EN_US_STRING),
//                                               ACE_TEXT (exception_in.what ())));
//                                   Common_Tools::printLocales ();
//                                   yyterminate ();
//                                 } catch (...) {
//                                   ACE_DEBUG ((LM_ERROR,
//                                               ACE_TEXT ("caught exception in std::locale(\"%s\"), aborting\n"),
//                                               ACE_TEXT (COMMON_LOCALE_EN_US_STRING)));
//                                   Common_Tools::printLocales ();
//                                   yyterminate ();
//                                 }
//                                 ACE_ASSERT (std::has_facet<std::ctype<char> >(locale));
                                 std::string value_string =
                                     Common_String_Tools::tolower (Common_String_Tools::strip ((*iterator).second));
                                 if (value_string == ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_TRANSFER_ENCODING_CHUNKED_STRING))
                                   BEGIN (chunked_body);
                                 else
                                   ACE_DEBUG ((LM_ERROR,
                                               ACE_TEXT ("invalid/unknown transfer encoding (was: \"%s\"), continuing\n"),
                                               ACE_TEXT ((*iterator).second.c_str ())));
                               } // end IF
                               else
                                 BEGIN (INITIAL); // no data
                             } // end ELSE
                             yylval->ival = 2;
//                             return yy::HTTP_Parser::token::DELIMITER;
                             return yytokentype::DELIMITER;
                           }
} // end <response_headers>
<body>{
{OCTET}{1}                 { ACE_ASSERT (!content_length_or_chunk_size);
                             struct HTTP_Record& record_r =
                               iparser_p->current ();
                             HTTP_HeadersIterator_t iterator =
                                 record_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
                             ACE_ASSERT (iterator != record_r.headers.end ());
                             converter.str (ACE_TEXT_ALWAYS_CHAR (""));
                             converter.clear ();
                             converter.str ((*iterator).second);
                             converter >> content_length_or_chunk_size;
                             missing_bytes = content_length_or_chunk_size;
                             if (yy_flex_debug)
                               ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("found body @%d of %d byte(s)\n"),
                                           iparser_p->offset (),
                                           content_length_or_chunk_size));
                             --missing_bytes;
/*                             char c = 0;
                             for (unsigned int i = 0;
                                  i < missing_bytes;
                                  ++i)
                             {
                               c = yyinput (yyscanner);
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;
                             } // end FOR */
                             BEGIN (data);
                           }
} // end <body>
<data>{
{OCTET}{1}                 { ACE_ASSERT (content_length_or_chunk_size);
                             ACE_ASSERT (missing_bytes);
                             --missing_bytes;
                             if (unlikely (!missing_bytes))
                             {
                               BEGIN (INITIAL);
                               yylval->ival = content_length_or_chunk_size;
//                               return yy::HTTP_Parser::token::BODY;
                               return yytokentype::BODY;
                             } // end IF
                           }
} // end <data>
<chunked_body>{
{CHUNK_LINE}{CRLF}         { if (iparser_p->currentChunkSize ()) // *IMPORTANT NOTE*: 0 the first time around
                               iparser_p->offset (iparser_p->currentChunkSize () + 2);
                             // *TODO*: let the scanner parse this (it does it anyway)
                             regex.assign (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_REGEX_CHUNK_LINE),
                                           regex_flags);
                             std::string input_string = yytext;
                             if (!std::regex_match (input_string,
                                                    match_results,
                                                    regex,
                                                    std::regex_constants::match_default))
                             {
                               ACE_DEBUG ((LM_ERROR,
                                           ACE_TEXT ("invalid chunk line (was: \"%s\"), returning\n"),
                                           ACE_TEXT (yytext)));
                               yyterminate();
                             } // end IF
                             ACE_ASSERT (match_results.ready () && !match_results.empty ());
                             ACE_ASSERT (match_results[1].matched);
                             converter.setf (std::ios::hex,
                                             std::ios::basefield);
                             converter.clear ();
                             converter.str (match_results[1].str ());
                             converter >> content_length_or_chunk_size;
                             converter.setf (std::ios::dec,
                                             std::ios::basefield);
                             BEGIN (chunk);
                             yycolumn -= yyleng;
                             yyless (0);
                           }
{ENTITY_HEADER}{CRLF}      { iparser_p->offset (yyleng);
                             /* *TODO*: error handling */
                             ACE_NEW_NORETURN (yylval->sval,
                                               std::string ());
                             ACE_ASSERT (yylval->sval);
                             yylval->sval->assign (yytext, yyleng - 2);
/*                             return yy::HTTP_Parser::token::HEADER; } */
                             return yytokentype::HEADER; }
{CRLF}                     { iparser_p->offset (yyleng);
                             try {
//                               iparser_p->finished ();
                             } catch (...) {
                               ACE_DEBUG ((LM_ERROR,
                                           ACE_TEXT ("caught exception in HTTP_IParser::finished(), continuing\n")));
                             }
                             BEGIN (INITIAL);
                             yylval->ival = 2;
/*                             return yy::HTTP_Parser::token::DELIMITER; } */
                             return yytokentype::DELIMITER;
                           }
} // end <chunked_body>
<chunk>{
{CHUNK_LINE}{CRLF}         { iparser_p->offset (yyleng);
                             if (content_length_or_chunk_size)
                             {
                               ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("found entity chunk @%d: %u byte(s)\n"),
                                           iparser_p->offset (),
                                           content_length_or_chunk_size));
/*                               char c = 0;
                               for (unsigned int i = 0;
                                    i < content_length_or_chunk_size;
                                    ++i)
                               {
                                 c = yyinput (yyscanner);
                                 // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                                 *(yyg->yy_c_buf_p - 1) = c;
                               } // end FOR
                               // gobble trailing delimiter (CRLF)
                               c = yyinput (yyscanner);
                               ACE_ASSERT (c == '\r');
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;
                               c = yyinput (yyscanner);
                               ACE_ASSERT (c == '\n');
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;*/
                               missing_bytes = content_length_or_chunk_size;
                               BEGIN (data_2);
                             } // end IF
                             else
                             {
                               ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("found last entity chunk @%d: 0 byte(s)\n"),
                                           iparser_p->offset ()));
                               // *NOTE*: do not consume the terminating CRLF
                               //         here; this happens in <chunked_body>
                               yylval->ival = content_length_or_chunk_size;
                               BEGIN (chunked_body);
/*                             return yy::HTTP_Parser::token::CHUNK; */
                               return yytokentype::CHUNK;
                             } // end ELSE
                           }
} // end <chunk>
<data_2>{
{OCTET}{1}                 { ACE_ASSERT (missing_bytes);
                             --missing_bytes;
                             if (unlikely (!missing_bytes))
                             {
                               // gobble trailing delimiter (CRLF)
                               char c = yyinput (yyscanner);
                               ACE_ASSERT (c == '\r');
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;
                               c = yyinput (yyscanner);
                               ACE_ASSERT (c == '\n');
                               // *IMPORTANT NOTE*: yyinput() zeroes the buffer --> put the data back
                               *(yyg->yy_c_buf_p - 1) = c;
                               BEGIN (chunked_body);
                               yylval->ival = content_length_or_chunk_size;
/*                             return yy::HTTP_Parser::token::CHUNK; } */
                               return yytokentype::CHUNK;
                             } // end IF
                           }
} // end <data_2>
<<EOF>>                    { yyterminate(); } // *NOTE*: yywrap returned non-zero
<*>{OCTET}                 { /* *TODO*: use (?s:.) ? */
                             if (!iparser_p->isBlocking ())
                               yyterminate (); // not enough data, cannot proceed
//                               return yy::HTTP_Parser::token::END_OF_FRAGMENT;

                             // wait for more data fragment(s)
                             if (!iparser_p->switchBuffer ())
                             { // *NOTE*: most probable reason: connection
                               //         has been closed --> session end
                               ACE_DEBUG ((LM_DEBUG,
                                           ACE_TEXT ("failed to Net_IParser::switchBuffer(), returning\n")));
                               yyterminate (); // not enough data, cannot proceed
                             } // end IF
                             yyless (0); }
%%
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
int
HTTP_Scanner_wrap (yyscan_t yyscanner)
{
  NETWORK_TRACE (ACE_TEXT ("::HTTP_Scanner_wrap"));

  struct yyguts_t* yyg = static_cast<struct yyguts_t*> (yyscanner);
  HTTP_IParser* iparser_p = HTTP_Scanner_get_extra (yyscanner);

  // sanity check(s)
  ACE_ASSERT (iparser_p);
  if (!iparser_p->isBlocking ())
    return 1; // not enough data, cannot proceed
//  if (iparser_p->hasFinished ())
//  {
//    BEGIN (INITIAL);
//    return 1; // done
//  } // end IF

  if (!iparser_p->switchBuffer ())
  {
    // *NOTE*: most probable reason: received session end message
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_IParser::switchBuffer(), aborting\n")));
    return 1;
  } // end IF

  return 0;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
