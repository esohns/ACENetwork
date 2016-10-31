/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2013 Free Software Foundation, Inc.

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

#ifndef YY_YY_BITTORRENT_PARSER_H_INCLUDED
# define YY_YY_BITTORRENT_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
//extern int yydebug;
#endif
/* "%code requires" blocks.  */


// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef BITTORRENT_PARSER_H
//#define BITTORRENT_PARSER_H

#include <cstdio>
#include <string>

#include "bittorrent_exports.h"

/* enum yytokentype
{
  END = 0,
  HANDSHAKE = 258,
  MESSAGE_BITFIELD = 260,
  MESSAGE_CANCEL = 262,
  MESSAGE_HAVE = 259,
  MESSAGE_PIECE = 263,
  MESSAGE_PORT = 263,
  MESSAGE_REQUEST = 261,
}; */
//#define YYTOKENTYPE
#undef YYTOKENTYPE
/* enum yytokentype; */
class BitTorrent_IParser;
struct BitTorrent_PeerHandshake;
struct BitTorrent_Record;
//class BitTorrent_Scanner;
//struct YYLTYPE;

/* #define YYSTYPE
typedef union YYSTYPE
{
  int          ival;
  std::string* sval;
} YYSTYPE; */
#undef YYSTYPE
//union YYSTYPE;

typedef void* yyscan_t;

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually, as there is no way to add the export symbol to
//         the declaration
#define YYDEBUG 1
extern int BitTorrent_Export yydebug;
#define YYERROR_VERBOSE



/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    HANDSHAKE = 258,
    BITFIELD = 259,
    CANCEL = 260,
    CHOKE = 261,
    HAVE = 262,
    INTERESTED = 263,
    KEEP_ALIVE = 264,
    NOT_INTERESTED = 265,
    PIECE = 266,
    PORT = 267,
    REQUEST = 268,
    UNCHOKE = 269,
    END_OF_FRAGMENT = 270
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{


  struct BitTorrent_PeerHandshake* handshake;
  struct BitTorrent_Record*        record;
  unsigned int                     size;


};
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



int yyparse (BitTorrent_IParser* iparser_p, yyscan_t yyscanner);
/* "%code provides" blocks.  */


void BitTorrent_Export yysetdebug (int);
void BitTorrent_Export yyerror (YYLTYPE*, BitTorrent_IParser*, yyscan_t, const char*);
int BitTorrent_Export yyparse (BitTorrent_IParser*, yyscan_t);
void BitTorrent_Export yyprint (FILE*, yytokentype, YYSTYPE);

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // BITTORRENT_PARSER_H



#endif /* !YY_YY_BITTORRENT_PARSER_H_INCLUDED  */
