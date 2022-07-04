/****************************************************/
/* File: tiny.l                                     */
/* Lex specification for TINY                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
%}

digit       [0-9]
letter      [a-zA-Z]
number      {digit}+
identifier  {letter}({digit}|{letter})*
newline     \n
whitespace  [ \t]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"			    {return WHILE;}
"return"        {return RETURN;}
"int"			      {return INT;}
"void"          {return VOID;}
"="             {return ASSIGN;}
"=="            {return EQ;}
"!="			      {return NE;}
"<"             {return LT;}
"<="			      {return LE;}
">"				      {return GT;}
">="			      {return GE;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"{"				      {return LCURLY;}
"}"				      {return RCURLY;}
"["				      {return LBRACE;}
"]"				      {return RBRACE;}
";"             {return SEMI;}
","				      {return COMMA;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"             { char c;
					int a = 0;
                  while(1 == 1)
                  { c = input();
                    if (c == EOF) break;
					if (a == 1){
						if (c == '/') break;
						else a = 0;
					}
                    if (c == '\n') lineno++;
					if (c == '*') a = 1;
                  }
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

