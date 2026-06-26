lexer grammar http_antlr_scanner;

tokens { METHOD, URI, VERSION, CODE, REASON, FIELD_KEY, COLON, FIELD_VALUE, CRLF_HEAD, CRLF, CHUNK }

@header {
#include <regex>
#include <sstream>
#include <string>

#include "common_string_tools.h"

#include "http_common.h"
#include "http_defines.h"

#include "http_antlr_iparser.h"
}

@members {
 public:
  bool                chunked;
  size_t              content_length;
  std::string         key;
  size_t              missing_body_or_chunk_bytes;
  size_t              offset;
  HTTP_ANTLR_IParser* parser;
  struct HTTP_Record  record;

  void reset_2 ()
  {
    chunked = false;
    content_length = 0;
    key.clear ();
    missing_body_or_chunk_bytes = 0;
    offset = 0;
    parser = NULL;
    record.reset ();
  }
}

/* *NOTE*: see RFC 1945 page 10 */
fragment OCTET  :          [\u0000-\u00FF];
/* *NOTE*: (US-)ASCII */
fragment CHAR   :          [\u0000-\u007F];
fragment UPALPHA:          [A-Z];
fragment LOALPHA:          [a-z];
fragment ALPHA  :          [a-zA-Z];
fragment DIGIT  :          [0-9];
fragment CTL    :          [\u0000-\u001F\u007F];
fragment CR     :          '\u000D';
fragment LF     :          '\u000A';
fragment SP     :          '\u0020';
/* *NOTE*: (horizontal) TAB */
fragment HT     :          '\u0009';
/* fragment DOUBLE_QUOTE   \u0022 */


CRLF         :             '\u000D\u000A';
fragment LWS :             ('\u000D\u000A'('\u0020'|'\u0009'));
/* *NOTE*: antlr 4.8 does not support class unions */
/*fragment TEXT:             ([\u0000-\u00FF]{-}[\u0000-\u001F]{+}[\u0009\u000A\u000D])*;*/
fragment HEX :             [a-fA-F0-9];

fragment OBS_TEXT     :    [\u0080-\u00FF];
fragment OBS_FOLD     :    CRLF (SP|HT)+;
fragment TCHAR        :    ([!#$%&'*+-.^_|~]|DIGIT|ALPHA);
fragment VCHAR        :    [\u0020-\u007E]; /* visible ASCII */
fragment T_SPECIALS   :    [()<>@,;:\\"/[\]?={} \u0009];
fragment TOKEN        :    (TCHAR)+;

/*fragment C_TEXT       :    ([\u0020-\u00FF]|[\u0009\u000A\u000D\u0020]{-}[()])*;*/
fragment COMMENT      :    '\u0022'([\u0020-\u00FF]|[\u0009\u000A\u000D])*'\u0022';

/*fragment QD_TEXT      :    ([\u0000-\u00FF]{-}[\u0000-\u001F\u0022]{+}[\u0009\u000A\u000D\u0020])*;*/
fragment QUOTED_STRING:    '\u0022'([\u0020-\u00FF]|[\u0009\u000A\u000D])*'\u0022';

fragment WORD         :    (TOKEN|QUOTED_STRING);

/* HTTP Version */
/*VERSION:                   'HTTP/'[0-9]'.'[0-9];*/


/* ESCAPE         (%(HEX)(HEX)) */
fragment ESCAPE:           ('%'(HEX)(HEX));

fragment RESERVED:         [;/?:@&=+];
fragment EXTRA   :         [!*'(),];
fragment SAFE    :         [$\-_.];
/* (CTL|SP){+}["#%<>] */
fragment UNSAFE  :         [\u0000-\u001F\u007F]|[\u0020]|["#%<>];
/* (OCTET){-}([[:alnum:]]{+}(RESERVED|EXTRA|SAFE|UNSAFE)) */
/* NATIONAL       [^[:alnum:]]{-}[;/?:@&=+]{-}[!*'(),]{-}[$\-_.]{-}([\u0000-\u001F\u007F]{+}[\u0020]{+}["#%<>]) */
/*fragment NATIONAL:         [\u0000-\u00FF]{-}[a-zA-Z0-9]{-}[;/?:@&=+]{-}[!*'(),]{-}[$\-_.]{-}[\u0000-\u001F\u007F]{-}[\u0020]{-}["#%<>];*/

/* [[:alnum:]]{+}(SAFE|EXTRA|NATIONAL) */
/* UNRESERVED     [[:alnum:]]{+}[$\-_.]{+}[!*'(),]{+}{NATIONAL} */
fragment UNRESERVED:       [a-zA-Z0-9]|[$\-_.]|[!*'(),];
/* UNRESERVED     [[:alnum:]]{+}[$\-_.]{+}[!*'(),] */
/* (UNRESERVED|ESCAPE) */
/* U_CHAR         {UNRESERVED}|{ESCAPE} */
/* U_CHAR         [[:alnum:]]{+}{SAFE}{+}{EXTRA}{+}{NATIONAL} */
fragment U_CHAR    :       (UNRESERVED|ESCAPE);
/* (U_CHAR|:|@|&|=|+) */
fragment P_CHAR    :       (U_CHAR|[:@&=+]);


/* Uniform Resource Identifiers */
fragment NET_LOCATION :    (P_CHAR|';'|'?')*;
fragment F_SEGMENT    :    P_CHAR;
fragment SEGMENT      :    (P_CHAR)*;
fragment PATH         :    F_SEGMENT ('/' SEGMENT)*;
fragment PARAMETER    :    (P_CHAR|'/')*;
fragment PARAMETERS   :    PARAMETER(';'PARAMETER)*;
fragment QUERY        :    (U_CHAR|RESERVED)*;
/* *TODO*: specification is {PATH}{0,1}(;{PARAMETERS}){0,1}(?{QUERY_}){0,1} */
fragment RELATIVE_PATH:    (PATH)+;
fragment ABSOLUTE_PATH:    '/' RELATIVE_PATH;
fragment NET_PATH     :    '//' NET_LOCATION (ABSOLUTE_PATH)?;
fragment RELATIVE_URI :    (NET_PATH|ABSOLUTE_PATH|RELATIVE_PATH);
fragment SCHEME       :    [a-zA-Z0-9+\-.]+;
fragment ABSOLUTE_URI :    SCHEME ':' (U_CHAR|RESERVED)*;
fragment FRAGMENT     :    (P_CHAR|[/?])*;
fragment URI          :    (ABSOLUTE_URI|RELATIVE_URI) (FRAGMENT (',' FRAGMENT)*);


/* http URL */
fragment PORT         :    [0-9]+;
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
fragment HOST         :  ([a-zA-Z0-9.\-])+;
fragment URL          :  'http://'HOST(':'PORT)?ABSOLUTE_PATH;


/* Date/Time Formats */
fragment WKDAY       :  ('Mon'|'Tue'|'Wed'|'Thu'|'Fri'|'Sat'|'Sun');
fragment MONTH       :  ('Jan'|'Feb'|'Mar'|'Apr'|'May'|'Jun'|'Jul'|'Aug'|'Sep'|'Oct'|'Nov'|'Dec');
fragment DATE1       :  [0-9][0-9] SP MONTH SP [0-9][0-9][0-9][0-9];
fragment TIME        :  [0-9][0-9]':'[0-9][0-9]':'[0-9][0-9];
fragment RFC1123_DATE:  WKDAY ',' SP DATE1 SP TIME SP 'GMT';
fragment WEEKDAY     :  ('Monday'|'Tuesday'|'Wednesday'|'Thursday'|'Friday'|'Saturday'|'Sunday');
fragment DATE2       :  [0-9][0-9] '-' MONTH '-' [0-9][0-9];
fragment RFC850_DATE :  WEEKDAY ',' SP DATE2 SP TIME SP 'GMT';
fragment DATE3       :  [0-9][0-9] '-' MONTH '-' [0-9][0-9];
fragment ASCTIME_DATE:  WKDAY SP DATE3 SP TIME SP [0-9][0-9][0-9][0-9];
fragment DATE        :  (RFC1123_DATE|RFC850_DATE|ASCTIME_DATE);


/* Character Sets */
/* NOTE*: "...Applications should limit their use of character sets to those
          defined by the IANA registry." */
fragment CHARSET_TOKEN: TOKEN;
/* *NOTE*: "When no explicit charset parameter is provided by the sender, media
           subtypes of the "text" type are defined to have a default charset
           value of "ISO-8859-1" when received via HTTP" */
fragment CHARSET      : ('US-ASCII'|'ISO-8859-1'|'ISO-8859-2'|'ISO-8859-3'|'ISO-8859-4'|'ISO-8859-5'|'ISO-8859-6'|'ISO-8859-7'|'ISO-8859-8'|'ISO-8859-9'|'ISO-2022-JP'|'ISO-2022-JP-2'|'ISO-2022-KR'|'UNICODE-1-1'|'UNICODE-1-1-UTF-7'|'UNICODE-1-1-UTF-8'|CHARSET_TOKEN);


/* Content Coding */
fragment CODING_TOKEN  : TOKEN;
fragment CONTENT_CODING: ('x-gzip'|'x-compress'|CODING_TOKEN);


/* Media Types */
/* *NOTE*: "Media-type values are registered with the Internet Assigned Number
           Authority (IANA)" */
fragment TYPE          : TOKEN;
fragment SUB_TYPE      : TOKEN;
fragment ATTRIBUTE     : TOKEN;
fragment VALUE         : (TOKEN|QUOTED_STRING);
fragment TYPE_PARAMETER: ATTRIBUTE '=' VALUE;
fragment MEDIA_TYPE    : TYPE '/' SUB_TYPE (';'TYPE_PARAMETER)*;


/* Product Tokens */
fragment PRODUCT_TOKEN  : TOKEN;
fragment PRODUCT_VERSION: PRODUCT_TOKEN;
fragment PRODUCT        : PRODUCT_TOKEN ('/' PRODUCT_VERSION)?;


/* Message Headers (generic) */
fragment FIELD_NAME     : TOKEN;
fragment FIELD_VCHAR    : (VCHAR|OBS_TEXT);
fragment FIELD_CONTENT  : FIELD_VCHAR ((SP|HT)+ FIELD_VCHAR)?;
/* fragment FIELD_VALUE    : (FIELD_CONTENT|OBS_FOLD)*; */
fragment FIELD_VALUE    : (FIELD_CONTENT)+;
fragment HEADER         : FIELD_NAME ':' SP FIELD_VALUE;

/* General Headers */
fragment HEADER_DATE    : 'Date:' SP DATE;
fragment PRAGMA_TOKEN   : (TOKEN)+('=' WORD)?;
fragment PRAGMA_VALUE   : ('no-cache'|PRAGMA_TOKEN);
fragment HEADER_PRAGMA  : 'Pragma:' SP PRAGMA_VALUE;
fragment GENERAL_HEADER : (HEADER_DATE|HEADER_PRAGMA);

/* Request Headers */
/* *TODO*: see rfc1945 page 47 */
fragment CREDENTIALS           : TOKEN;
fragment HEADER_AUTHORIZATION  : 'Authorization:' SP CREDENTIALS;
/* *TODO*: e-mail address, see rfc822/1123 */
fragment MAILBOX               : TOKEN;
fragment HEADER_FROM           : 'From:' SP MAILBOX;
fragment HEADER_IFMODIFIEDSINCE: 'If-Modified-Since:' SP DATE;
fragment HEADER_REFERER        : 'Referer:' SP (ABSOLUTE_URI|RELATIVE_URI);
fragment HEADER_USERAGENT      : 'User-Agent:' SP (PRODUCT|COMMENT);
fragment REQUEST_HEADER        : (HEADER_AUTHORIZATION|HEADER_FROM|HEADER_IFMODIFIEDSINCE|HEADER_REFERER|HEADER_USERAGENT);

/* Request-Line */
fragment REQUEST_TOKEN         : (CHAR)+;
/* TODO*: allowing {REQUEST_TOKEN} breaks precedence in INITIAL state, as it
          gobbles the whole buffer */
/* METHOD          (GET|HEAD|POST|{REQUEST_TOKEN}) */
METHOD                         : ('GET'|'HEAD'|'POST');
fragment REQUEST_URI           : (ABSOLUTE_URI|ABSOLUTE_PATH);
fragment REQUEST               : METHOD SP REQUEST_URI SP VERSION;

/* Response Headers */
fragment LOCATION_HEADER       : 'Location:' SP ABSOLUTE_URI;
fragment SERVER_HEADER         : 'Server:' SP (PRODUCT|COMMENT);
fragment AUTH_SCHEME           : TOKEN;
fragment REALM_VALUE           : QUOTED_STRING;
fragment REALM                 : 'realm=' REALM_VALUE;
fragment AUTH_PARAMETER        : (TOKEN)+ '=' QUOTED_STRING;
fragment AUTH_PARAMETERS       : (',' AUTH_PARAMETER)*;
fragment CHALLENGE             : AUTH_SCHEME SP REALM AUTH_PARAMETERS;
fragment WWWAUTHENTICATE_HEADER: 'WWW-Authenticate:' SP CHALLENGE;
fragment RESPONSE_HEADER       : (LOCATION_HEADER|SERVER_HEADER|WWWAUTHENTICATE_HEADER);

/* Status-Line (Response-) */
/* ({TEXT}{-}{CRLF})* */
/* CODE                           : [0-9][0-9][0-9];*/
/* REASON                         : ([\u0020-\u00FF]|[\u0009])+;*/
/*STATUS                         : VERSION SP CODE SP REASON;*/

/* Entity Headers */
fragment ALLOW_HEADER          : 'Allow:' SP (METHOD)+;
fragment CONTENTENCODING_HEADER: 'Content-Encoding:' SP CONTENT_CODING;
fragment CONTENTLENGTH_HEADER  : 'Content-Length:' SP [0-9]+;
fragment CONTENTTYPE_HEADER    : 'Content-Type:' SP MEDIA_TYPE;
fragment EXPIRES_HEADER        : 'Expires:' SP DATE;
fragment LASTMODIFIED_HEADER   : 'Last-Modified:' SP DATE;
fragment EXTENSION_HEADER      : HEADER;
fragment ENTITY_HEADER         : (ALLOW_HEADER|CONTENTENCODING_HEADER|CONTENTLENGTH_HEADER|CONTENTTYPE_HEADER|EXPIRES_HEADER|LASTMODIFIED_HEADER|EXTENSION_HEADER);

/*BODY                           : (OCTET)+;*/

/* Chunks (*NOTE*: HTTP/1.1) */
fragment CHUNKSIZE             : HEX+;
fragment CHUNK_EXTENSION       : TOKEN ('=' TOKEN|QUOTED_STRING)?;
fragment CHUNK_EXTENSIONS      : (';' CHUNK_EXTENSION)*;
fragment CHUNK_LINE            : CHUNKSIZE CHUNK_EXTENSIONS;
/*fragment CHUNK_DATA            : (OCTET)+;*/
/* CHUNK                  {CHUNK_LINE}{CRLF}{CHUNK_DATA}{CRLF} */
fragment ZERO_ASCII            : '0';
fragment CHUNK_LINE_LAST       : ZERO_ASCII+ CHUNK_EXTENSIONS;
fragment CHUNK_TRAILER         : (ENTITY_HEADER CRLF)*;
/*CHUNK                          : CHUNK_DATA;*/

/*                                                                            */

VERSION                        : 'HTTP' '/' DIGIT '.' DIGIT {
                                   offset += 8;
                                 } -> type(VERSION), mode(RESPONSE_SP);

mode RESPONSE_SP;
SP_RESP                        : SP {
                                   ++offset;
                                 } -> skip, mode(STATUS_CODE);

mode STATUS_CODE;
CODE                           : DIGIT DIGIT DIGIT {
                                   offset += 3;
                                 } -> type(CODE);
SP_CODE                        : SP {
                                   ++offset;
                                 } -> skip, mode(REASON_STRING);

mode REASON_STRING;  /* *( HTAB / SP / VCHAR / obs-text ) */
REASON                         : (HT|SP|VCHAR|OBS_TEXT)+ {
                                   offset += getText ().size ();
                                 } -> type(REASON);
CRLF_REASON                    : CRLF {
                                   offset += 2;
                                 } -> type(CRLF), mode(HEADERS);

mode HEADERS;
KEY_HEAD                       : FIELD_NAME {
                                   key = getText ();
                                   offset += key.size ();
                                 } -> type(FIELD_KEY), pushMode(HEAD);
CRLF_HEADERS                   : CRLF {
                                 { offset += 2;
                                   HTTP_HeadersConstIterator_t iterator =
                                     record.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
                                   if (iterator != record.headers.end ())
                                   {
                                     std::istringstream converter;
                                     converter.str ((*iterator).second);
                                     converter >> content_length;
                                     missing_body_or_chunk_bytes = content_length;
                                     if (content_length)
                                       setMode (REGULAR_BODY);
                                     break;
                                   } // end IF
                                   iterator =
                                     record.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_TRANSFER_ENCODING_STRING)));
                                   if (iterator != record.headers.end ())
                                   {
                                     std::string value_string =
                                     Common_String_Tools::tolower (Common_String_Tools::strip ((*iterator).second));
                                     if (value_string == ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_TRANSFER_ENCODING_CHUNKED_STRING))
                                     {
                                       chunked = true;
                                       content_length = 0;
                                       setMode (CHUNKED_BODY);
                                     } // end IF
                                     else
                                     {
                                       ACE_DEBUG ((LM_ERROR,
                                                   ACE_TEXT ("invalid/unknown transfer encoding (was: \"%s\"), continuing\n"),
                                                   ACE_TEXT ((*iterator).second.c_str ())));
                                       setMode (DEFAULT_MODE);
                                     } // end ELSE
                                   } // end IF
                                   else
                                   {
                                     ACE_DEBUG ((LM_ERROR,
                                                 ACE_TEXT ("invalid/unknown transfer encoding (was: \"%s\"), continuing\n"),
                                                 ACE_TEXT ((*iterator).second.c_str ())));
                                     setMode (DEFAULT_MODE);
                                   } // end ELSE
                                 }
                                 } -> type(CRLF);

mode HEAD;
COLON_SP                       : ':' SP+ {
                                   offset += getText ().size ();
                                 } -> type(COLON), mode(HEAD_VALUE);

mode HEAD_VALUE;
VALUE_HEAD                     : FIELD_VALUE {
                                 {
                                   std::string value = getText ();
                                   offset += value.size ();
                                   record.headers.insert (std::make_pair (key, value));
                                 }
                                 } -> type(FIELD_VALUE), mode(HEAD_END);

mode HEAD_END;
CRLF_HEAD                      : CRLF {
                                   offset += 2;
                                 }-> type(CRLF), popMode;

mode REGULAR_BODY;
BODY                           : OCTET {
                                   ACE_ASSERT (missing_body_or_chunk_bytes);
                                   --missing_body_or_chunk_bytes;
                                   if (unlikely (!missing_body_or_chunk_bytes))
                                   {
                                     std::ostringstream converter;
                                     converter << content_length;
                                     setText (converter.str ());
                                     setType (BODY);
                                     break;
                                   } // end IF
                                   skip ();
                                 };

mode CHUNKED_BODY;
CHUNK_LAST                     : CHUNK_LINE_LAST CRLF {
                                   offset += getText ().size ();
                                   setText (ACE_TEXT_ALWAYS_CHAR ("0"));
                                   parser->chunk_2 (offset, 0);
                                 } -> type(CHUNK), mode(CHUNKED_BODY_END);
CHUNK                          : CHUNK_LINE CRLF {
                                 { // *TODO*: let the scanner parse this (it does it anyway)
                                   std::string input_string = getText ();
                                   offset += input_string.size ();
                                   std::smatch match_results;
                                   std::string regex_string = ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_REGEX_CHUNK_LINE);
                                   std::regex regex (regex_string.c_str ());
                                   if (!std::regex_match (input_string,
                                                          match_results,
                                                          regex,
                                                          std::regex_constants::match_default))
                                   {
                                     std::string error_message =
                                       ACE_TEXT_ALWAYS_CHAR ("invalid chunk line (was: \"");
                                     error_message += input_string;
                                     error_message += ACE_TEXT_ALWAYS_CHAR ("\"), throwing\n");
                                     ACE_DEBUG ((LM_ERROR,
                                                 ACE_TEXT (error_message.c_str ())));
                                     throw new ParseCancellationException (/*"line " + line + ":" + charPositionInLine + " " + */error_message);
                                   } // end IF
                                   ACE_ASSERT (match_results.ready () && !match_results.empty ());
                                   ACE_ASSERT (match_results[1].matched);
                                   std::stringstream converter;
                                   converter.setf (std::ios::hex,
                                                   std::ios::basefield);
                                   converter.clear ();
                                   converter.str (match_results[1].str ());
                                   size_t chunk_size;
                                   converter >> chunk_size;
                                   content_length += chunk_size;
                                   missing_body_or_chunk_bytes = chunk_size;
                                   converter.setf (std::ios::dec,
                                                   std::ios::basefield);
                                   converter.clear ();
                                   converter << chunk_size;
                                   setText (converter.str ());
                                   parser->chunk_2 (offset, static_cast<ACE_UINT32> (chunk_size));
                                   ACE_ASSERT (missing_body_or_chunk_bytes);
                                   pushMode(CHUNKED_DATA);
                                 }
                                 } -> type(CHUNK);

mode CHUNKED_BODY_END;
CRLF_CHUNKED_BODY              : CRLF {
                                   offset += 2;
                                 } -> type(CRLF), mode(DEFAULT_MODE);

mode CHUNKED_DATA;
CHUNK_DATA                     : OCTET {
                                   ++offset;
                                   ACE_ASSERT (missing_body_or_chunk_bytes);
                                   --missing_body_or_chunk_bytes;
                                   if (unlikely (!missing_body_or_chunk_bytes))
                                     setMode(CHUNKED_DATA_END);
                                 } -> skip;

mode CHUNKED_DATA_END;
CRLF_CHUNKED_DATA              : CRLF {
                                   offset += 2;
                                 } -> skip, popMode;
