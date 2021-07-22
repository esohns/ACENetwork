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

#ifndef YY_YY_HTTP_PARSER_H_INCLUDED
# define YY_YY_HTTP_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef HTTP_PARSER_H
//#define HTTP_PARSER_H

#include <cstdio>
#include <string>

#include "common_iscanner.h"

/* enum yytokentype
{
  END = 0,
  METHOD = 258,
  URI = 259,
  VERSION = 260,
  HEADER = 261,
  DELIMITER = 262,
  STATUS = 263,
  REASON = 264,
  BODY = 265,
  CHUNK = 266
}; */
//#define YYTOKENTYPE
/*#undef YYTOKENTYPE*/
/* enum yytokentype; */
class HTTP_IParser;
struct HTTP_Record;
//class HTTP_Scanner;
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
extern int yydebug;
#define YYERROR_VERBOSE 1
#define YYLTYPE_IS_DECLARED 1

#undef YYTOKENTYPE


/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    END = 0,                       /* "end"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    METHOD = 258,                  /* "method"  */
    URI = 259,                     /* "uri"  */
    _VERSION = 260,                /* "version"  */
    HEADER = 261,                  /* "header"  */
    DELIMITER = 262,               /* "delimiter"  */
    STATUS = 263,                  /* "status"  */
    REASON = 264,                  /* "reason"  */
    BODY = 265,                    /* "body"  */
    CHUNK = 266,                   /* "chunk"  */
    END_OF_FRAGMENT = 267          /* "end_of_fragment"  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

  int          ival;
  std::string* sval;


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



int yyparse (HTTP_IParser* iparser_p, yyscan_t yyscanner);
/* "%code provides" blocks.  */

/*void yysetdebug (int);*/
void yyerror (YYLTYPE*, HTTP_IParser*, yyscan_t, const char*);
//void yyerror (HTTP_IParser*, yyscan_t, const char*);
/*int yyparse (HTTP_IParser*, yyscan_t);*/
/*void yyprint (FILE*, enum yytokentype, YYSTYPE);*/

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // HTTP_PARSER_H


#endif /* !YY_YY_HTTP_PARSER_H_INCLUDED  */
