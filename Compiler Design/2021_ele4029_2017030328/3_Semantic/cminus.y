/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName[2048]; /* for use in assignments */
static int savedNamePos = 0;
static int savedLineNo[2048];
static int savedLineNoPos = 0;
static int FLineNo;
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yyerror(char * message);
static int yylex(void);
%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LCURLY RCURLY LBRACE RBRACE SEMI COMMA
%token ERROR 

%nonassoc NOELSE
%nonassoc ELSE
%% /* Grammar for TINY */

program     : D_L
                 { savedTree = $1;} 
            ;

D_L			: D_L D
				{ YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
			| D { $$ = $1; }
			;

D			: V_D { $$ = $1; }
			| F_D { $$ = $1; }
			;

V_D			: T IDS SEMI 
				{	$$ = $1;
					$$->attr.name = savedName[--savedNamePos];
					$$->lineno = savedLineNo[--savedLineNoPos];
					$$->kind.exp = VarDK;
				 }
			| T IDS LBRACE NUMS RBRACE SEMI
				{	$$ = $1;
					$$->attr.name = savedName[--savedNamePos];
					$$->lineno = savedLineNo[--savedLineNoPos];
					$$->kind.exp = VarDK;
					$$->type += 2;
					$$->child[0] = $4;
				 }
			;

IDS			: ID { savedName[savedNamePos++] = copyString(tokenString);
					savedLineNo[savedLineNoPos++] = lineno;}
			;

NUMS		: NUM 
				{	$$ = newExpNode(ConstK);
					$$->attr.val = atoi(tokenString);
				}
			;

T			: INT
				{	$$ = newExpNode(ParamK);
                   $$->type = Integer;
				 }
			| VOID
				{	$$ = newExpNode(ParamK);
                   $$->type = Void;
				 }
			;

F_D			: T IDS 
				{
					FLineNo = lineno;
				} 
			  LPAREN PS RPAREN C_SM
				{	$$->attr.name = savedName[--savedNamePos];
					$$->lineno = FLineNo;
					$$->kind.exp = FunDK;
					$$->child[0] = $5;
					$$->child[1] = $7;
				 }

PS			: P_L { $$ = $1; }
			;

P_L			: P_L COMMA P 
				{ YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
                 }
			| P { $$ = $1; }
			| VOID
				{ $$ = newExpNode(ParamK); 
				  $$->void_p = 1;
				  $$->type = Void;
				}
			;

P			: T IDS
				{	$$ = $1;
					$$->attr.name = savedName[--savedNamePos];
					$$->lineno = savedLineNo[--savedLineNoPos];
				}
			| T IDS LBRACE RBRACE
				{	$$ = $1;
					$$->attr.name = savedName[--savedNamePos];
					$$->lineno = savedLineNo[--savedLineNoPos];
					$$->type += 2;
				}
			;

C_SM		: LCURLY L_D S_L RCURLY
				{	$$ = newStmtNode(CompK);
					$$->child[0] = $2;
					$$->child[1] = $3;
				}
			;

L_D			: L_D V_D 
				{ YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
			| /* EMPTY */ { $$ = NULL; }
			;

S_L			: S_L SM
				{ YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
			| /* EMPTY */ { $$ = NULL; }
			;

SM			: E_SM { $$ = $1; }
			| C_SM { $$ = $1; }
			| S_SM { $$ = $1; }
			| I_SM { $$ = $1; }
			| RE_SM { $$ = $1; }
			;

E_SM		: E SEMI { $$ = $1; }
			| SEMI { $$ = NULL; }
			;

S_SM		: IF LPAREN E RPAREN SM 	%prec NOELSE
				{ 
					$$ = newStmtNode(IfK);
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = NULL;
				}
			| IF LPAREN E RPAREN SM ELSE SM 
				{	
					$$ = newStmtNode(IfK); 
					$$->child[0] = $3;
					$$->child[1] = $5;
					$$->child[2] = $7;
				}
			;
	
I_SM		: WHILE LPAREN E RPAREN SM
				{ $$ = newStmtNode(WhileK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
				}
            ;

RE_SM		: RETURN SEMI { $$ = newStmtNode(ReturnK); }
			| RETURN E SEMI
				{ $$ = newStmtNode(ReturnK); 
				   $$->child[0] = $2;
				 }
			;

E			: V ASSIGN E
				{ $$ = newStmtNode(AssignK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
			| S_E { $$ = $1; }
			;

V			: IDS 
				{ $$ = newExpNode(VarAccK);
                   $$->attr.name = savedName[--savedNamePos];
                 }
			| IDS LBRACE E RBRACE
				{ $$ = newExpNode(VarAccK);
                   $$->attr.name = savedName[--savedNamePos];
				   $$->child[0] = $3;
                 }
			;

S_E			: A_SM ROP A_SM 
				{ $$ = $2;
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
			| A_SM { $$ = $1; }
			;

ROP			: EQ 
				{ $$ = newExpNode(OpK);
                   $$->attr.op = EQ;
                 }
			| NE
				{ $$ = newExpNode(OpK);
                   $$->attr.op = NE;
                 }
			| GE
				{ $$ = newExpNode(OpK);
                   $$->attr.op = GE;
                 }
			| GT
				{ $$ = newExpNode(OpK);
                   $$->attr.op = GT;
                 }
			| LE 
				{ $$ = newExpNode(OpK);
                   $$->attr.op = LE;
                 }
			| LT
				{ $$ = newExpNode(OpK);
                   $$->attr.op = LT;
                 }
			;

A_SM		: A_SM AOP TERM 
				{ $$ = $2;
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
			| TERM { $$ = $1; }
			;

AOP			: PLUS
				{ $$ = newExpNode(OpK);
                   $$->attr.op = PLUS;
                 }
			| MINUS
				{ $$ = newExpNode(OpK);
                   $$->attr.op = MINUS;
                 }
			;

TERM		: TERM MOP FAC 
				{ $$ = $2;
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
			| FAC { $$ = $1; }
			;

MOP			: TIMES
				{ $$ = newExpNode(OpK);
                   $$->attr.op = TIMES;
                 }
			| OVER
				{ $$ = newExpNode(OpK);
                   $$->attr.op = OVER;
                 }
			;

FAC			: LPAREN E RPAREN { $$ = $2; }
			| V { $$ = $1; }
			| CAL { $$ = $1; }
			| NUM  { $$ = newExpNode(ConstK);
					$$->attr.val = atoi(tokenString);
				}
			;

CAL			: IDS LPAREN ARG RPAREN
				{	$$ = newExpNode(CallK);
					$$->attr.name = savedName[--savedNamePos];
					$$->child[0] = $3;
				 }
			;

ARG			: ARG_L { $$ = $1; }
			| /* EMPTY */ { $$ = NULL; }
			;

ARG_L		: ARG_L COMMA E
				{ YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
                 }
			| E { $$ = $1; }
			;


%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

