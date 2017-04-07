/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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

#ifndef YY_YYJSIL_JSIL_Y_TAB_HPP_INCLUDED
# define YY_YYJSIL_JSIL_Y_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yyjsildebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_SCANNER_ERROR = 258,
    TOK_NEWLINE = 259,
    TOK_PROCEDURE = 260,
    TOK_RETURNS = 261,
    TOK_TO = 262,
    TOK_THROWS = 263,
    TOK_EVAL = 264,
    TOK_LABEL = 265,
    TOK_GOTO = 266,
    TOK_SKIP = 267,
    TOK_WITH = 268,
    TOK_NEW = 269,
    TOK_HAS_FIELD = 270,
    TOK_DELETE = 271,
    TOK_PROTO_FIELD = 272,
    TOK_PROTO_OBJ = 273,
    TOK_REF = 274,
    TOK_FIELD = 275,
    TOK_BASE = 276,
    TOK_TYPEOF = 277,
    TOK_NULL = 278,
    TOK_UNDEFINED = 279,
    TOK_EMPTY = 280,
    TOK_TRUE = 281,
    TOK_FALSE = 282,
    TOK_PROTO = 283,
    TOK_FID = 284,
    TOK_SCOPE = 285,
    TOK_CONSTRUCTID = 286,
    TOK_PRIMVALUE = 287,
    TOK_TARGETFUNCTION = 288,
    TOK_CLASS = 289,
    TOK_NUM_TO_STRING = 290,
    TOK_STRING_TO_NUM = 291,
    TOK_NUM_TO_INT32 = 292,
    TOK_NUM_TO_UINT32 = 293,
    TOK_MEMBER_REFERENCE = 294,
    TOK_VARIABLE_REFERENCE = 295,
    TOK_T_NULL = 296,
    TOK_T_UNDEFINED = 297,
    TOK_T_BOOLEAN = 298,
    TOK_T_STRING = 299,
    TOK_T_NUMBER = 300,
    TOK_T_BUILTIN_OBJECT = 301,
    TOK_T_USER_OBJECT = 302,
    TOK_T_OBJECT = 303,
    TOK_T_REFERENCE = 304,
    TOK_DEFEQ = 305,
    TOK_LEQ = 306,
    TOK_AND = 307,
    TOK_OR = 308,
    TOK_SUBTYPE_OF = 309,
    TOK_LEFT_SHIFT = 310,
    TOK_SIGNED_RIGHT_SHIFT = 311,
    TOK_UNSIGNED_RIGHT_SHIFT = 312,
    TOK_NOT = 313,
    TOK_IDENTIFIER = 314,
    TOK_FLOATING = 315,
    TOK_STRING = 316,
    TOK_BUILTIN_LOC = 317,
    TOK_BUILTIN_IDENTIFIER = 318,
    TOK_SPEC_IDENTIFIER = 319
  };
#endif
/* Tokens.  */
#define TOK_SCANNER_ERROR 258
#define TOK_NEWLINE 259
#define TOK_PROCEDURE 260
#define TOK_RETURNS 261
#define TOK_TO 262
#define TOK_THROWS 263
#define TOK_EVAL 264
#define TOK_LABEL 265
#define TOK_GOTO 266
#define TOK_SKIP 267
#define TOK_WITH 268
#define TOK_NEW 269
#define TOK_HAS_FIELD 270
#define TOK_DELETE 271
#define TOK_PROTO_FIELD 272
#define TOK_PROTO_OBJ 273
#define TOK_REF 274
#define TOK_FIELD 275
#define TOK_BASE 276
#define TOK_TYPEOF 277
#define TOK_NULL 278
#define TOK_UNDEFINED 279
#define TOK_EMPTY 280
#define TOK_TRUE 281
#define TOK_FALSE 282
#define TOK_PROTO 283
#define TOK_FID 284
#define TOK_SCOPE 285
#define TOK_CONSTRUCTID 286
#define TOK_PRIMVALUE 287
#define TOK_TARGETFUNCTION 288
#define TOK_CLASS 289
#define TOK_NUM_TO_STRING 290
#define TOK_STRING_TO_NUM 291
#define TOK_NUM_TO_INT32 292
#define TOK_NUM_TO_UINT32 293
#define TOK_MEMBER_REFERENCE 294
#define TOK_VARIABLE_REFERENCE 295
#define TOK_T_NULL 296
#define TOK_T_UNDEFINED 297
#define TOK_T_BOOLEAN 298
#define TOK_T_STRING 299
#define TOK_T_NUMBER 300
#define TOK_T_BUILTIN_OBJECT 301
#define TOK_T_USER_OBJECT 302
#define TOK_T_OBJECT 303
#define TOK_T_REFERENCE 304
#define TOK_DEFEQ 305
#define TOK_LEQ 306
#define TOK_AND 307
#define TOK_OR 308
#define TOK_SUBTYPE_OF 309
#define TOK_LEFT_SHIFT 310
#define TOK_SIGNED_RIGHT_SHIFT 311
#define TOK_UNSIGNED_RIGHT_SHIFT 312
#define TOK_NOT 313
#define TOK_IDENTIFIER 314
#define TOK_FLOATING 315
#define TOK_STRING 316
#define TOK_BUILTIN_LOC 317
#define TOK_BUILTIN_IDENTIFIER 318
#define TOK_SPEC_IDENTIFIER 319

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yyjsillval;

int yyjsilparse (void);

#endif /* !YY_YYJSIL_JSIL_Y_TAB_HPP_INCLUDED  */
