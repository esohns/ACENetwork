/* A Bison parser, made by GNU Bison 3.7.4.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

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

#ifndef YY_YY_PCP_PARSER_H_INCLUDED
# define YY_YY_PCP_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */

#include <cstdio>
#include <string>

class PCP_ParserDriver;
//class PCP_Scanner;
struct YYLTYPE;
//union YYSTYPE;

union yytoken
{
  ACE_UINT64 ival;
  ACE_UINT8* aval;
};
typedef yytoken yytoken_t;
#define YYSTYPE yytoken_t

typedef void* yyscan_t;

//#define YYERROR_VERBOSE
extern void yyerror (YYLTYPE*, PCP_ParserDriver*, yyscan_t, const char*);
extern int yyparse (PCP_ParserDriver*, yyscan_t);

#undef YYPRINT
//extern void yyprint (FILE*, yytokentype, YYSTYPE);


/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    END = 0,                       /* "end"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    VERSION = 258,                 /* "version"  */
    OPCODE = 259,                  /* "opcode"  */
    RESERVED = 260,                /* "reserved"  */
    RESULT_CODE = 261,             /* "result_code"  */
    LIFETIME = 262,                /* "lifetime"  */
    EPOCH_TIME = 263,              /* "epoch_time"  */
    RESERVED_2 = 264,              /* "reserved_2"  */
    OPTION_MAP_NONCE = 265,        /* "option_map_nonce"  */
    OPTION_MAP_PROTOCOL = 266,     /* "option_map_protocol"  */
    OPTION_MAP_RESERVED = 267,     /* "option_map_reserved"  */
    OPTION_MAP_INTERNAL_PORT = 268, /* "option_map_internal_port"  */
    OPTION_MAP_ASSIGNED_EXTERNAL_PORT = 269, /* "option_map_assigned_external_port"  */
    OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS = 270, /* "option_map_assigned_external_ip_address"  */
    OPTION_PEER_NONCE = 271,       /* "option_peer_nonce"  */
    OPTION_PEER_PROTOCOL = 272,    /* "option_peer_protocol"  */
    OPTION_PEER_RESERVED = 273,    /* "option_peer_reserved"  */
    OPTION_PEER_INTERNAL_PORT = 274, /* "option_peer_internal_port"  */
    OPTION_PEER_ASSIGNED_EXTERNAL_PORT = 275, /* "option_peer_assigned_external_port"  */
    OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS = 276, /* "option_peer_assigned_external_ip_address"  */
    OPTION_PEER_REMOTE_PEER_PORT = 277, /* "option_peer_remote_port"  */
    OPTION_PEER_RESERVED_2 = 278,  /* "option_peer_reserved_2"  */
    OPTION_PEER_REMOTE_PEER_IP_ADDRESS = 279, /* "option_peer_remote_peer_address"  */
    OPTION_AUTHENTICATION_SESSION_ID = 280, /* "option_authentication_session_id"  */
    OPTION_AUTHENTICATION_SEQUENCE_NUMBER = 281, /* "option_authentication_sequence_number"  */
    OPTION_CODE = 282,             /* "option_code"  */
    OPTION_RESERVED = 283,         /* "option_reserved"  */
    OPTION_LENGTH = 284,           /* "option_length"  */
    OPTION_THIRD_PARTY_ADDRESS = 285, /* "option_third_party_address"  */
    OPTION_FILTER_RESERVED = 286,  /* "option_filter_reserved"  */
    OPTION_FILTER_PREFIX_LENGTH = 287, /* "option_filter_prefix_length"  */
    OPTION_FILTER_REMOTE_PEER_PORT = 288, /* "option_filter_remote_peer_port"  */
    OPTION_FILTER_REMOTE_PEER_IP_ADDRESS = 289, /* "option_filter_remote_peer_address"  */
    OPTION_NONCE_NONCE = 290,      /* "option_nonce_nonce"  */
    OPTION_AUTHENTICATION_TAG_SESSION_ID = 291, /* "option_authentication_tag_session_id"  */
    OPTION_AUTHENTICATION_TAG_SEQUENCE_NUMBER = 292, /* "option_authentication_tag_sequence_number"  */
    OPTION_AUTHENTICATION_TAG_KEY_ID = 293, /* "option_authentication_tag_key_id"  */
    OPTION_AUTHENTICATION_TAG_DATA = 294, /* "option_authentication_tag_data"  */
    OPTION_PA_AUTHENTICATION_TAG_KEY_ID = 295, /* "option_pa_authentication_tag_key_id"  */
    OPTION_PA_AUTHENTICATION_TAG_DATA = 296, /* "option_pa_authentication_tag_data"  */
    OPTION_EAP_PAYLOAD_DATA = 297, /* "option_eap_payload_data"  */
    OPTION_PSEUDO_RANDOM_FUNCTION_ID = 298, /* "option_pseudo_random_function_id"  */
    OPTION_MAC_ALGORITHM_ID = 299, /* "option_mac_algorithm_id"  */
    OPTION_SESSION_LIFETIME_LIFETIME = 300, /* "option_session_lifetime_lifetime"  */
    OPTION_RECEIVED_PAK_SEQUENCE_NUMBER = 301, /* "option_received_pak_sequence_number"  */
    OPTION_ID_INDICATOR_DATA = 302 /* "option_id_indicator_data"  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

  ACE_UINT64 ival;
  ACE_UINT8* aval;


};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (PCP_ParserDriver* driver, yyscan_t yyscanner);

#endif /* !YY_YY_PCP_PARSER_H_INCLUDED  */
