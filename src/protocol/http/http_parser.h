/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison GLR parsers in C
   
      Copyright (C) 2002-2006, 2009-2010 Free Software Foundation, Inc.
   
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

/* "%code requires" blocks.  */


// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <cstdio>
#include <string>

#include "http_exports.h"

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
#undef YYTOKENTYPE
/* enum yytokentype; */
template <typename RecordType>
class Net_IParser;
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

#define YYDEBUG 1
extern int HTTP_Export yydebug;
//#define YYERROR_VERBOSE




/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 0,
     METHOD = 258,
     URI = 259,
     VERSION = 260,
     HEADER = 261,
     DELIMITER = 262,
     STATUS = 263,
     REASON = 264,
     BODY = 265,
     CHUNK = 266,
     END_OF_FRAGMENT = 267
   };
#endif


#ifndef YYSTYPE
typedef union YYSTYPE
{


  int          ival;
  std::string* sval;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{

  int first_line;
  int first_column;
  int last_line;
  int last_column;

} YYLTYPE;
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

/* "%code provides" blocks.  */


extern void yysetdebug (int);
void yyerror (YYLTYPE*, Net_IParser<HTTP_Record>*, yyscan_t, const char*);
int yyparse (Net_IParser<HTTP_Record>* driver, yyscan_t yyscanner);
void yyprint (FILE*, yytokentype, YYSTYPE);

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
#endif // HTTP_PARSER_H










