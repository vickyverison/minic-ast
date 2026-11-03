%code requires {
#include "ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *message);

AstNode *ast_root = NULL;
int parse_failed = 0;
%}

%define parse.error detailed

%union {
    long number;
    char *text;
    AstNode *node;
}

%token <number> NUMBER
%token <text> IDENTIFIER
%token INT IF ELSE WHILE PRINTF FORMAT
%token EQ NE LE GE

%type <node> program statement_list statement expression

%destructor { free($$); } <text>

%nonassoc IF_WITHOUT_ELSE
%nonassoc ELSE
%nonassoc EQ NE '<' '>' LE GE
%left '+' '-'
%left '*' '/'
%right UMINUS

%start program

%%

program:
    statement_list {
        ast_root = ast_block($1);
        $$ = ast_root;
    }
;

statement_list:
    %empty {
        $$ = NULL;
    }
  | statement_list statement {
        $$ = ast_append($1, $2);
    }
;

statement:
    INT IDENTIFIER ';' {
        $$ = ast_declaration($2, NULL);
        free($2);
    }
  | INT IDENTIFIER '=' expression ';' {
        $$ = ast_declaration($2, $4);
        free($2);
    }
  | IDENTIFIER '=' expression ';' {
        $$ = ast_assignment($1, $3);
        free($1);
    }
  | PRINTF '(' FORMAT ',' expression ')' ';' {
        $$ = ast_print($5);
    }
  | '{' statement_list '}' {
        $$ = ast_block($2);
    }
  | IF '(' expression ')' statement %prec IF_WITHOUT_ELSE {
        $$ = ast_if($3, $5, NULL);
    }
  | IF '(' expression ')' statement ELSE statement {
        $$ = ast_if($3, $5, $7);
    }
  | WHILE '(' expression ')' statement {
        $$ = ast_while($3, $5);
    }
;

expression:
    NUMBER {
        $$ = ast_number($1);
    }
  | IDENTIFIER {
        $$ = ast_variable($1);
        free($1);
    }
  | '(' expression ')' {
        $$ = $2;
    }
  | '-' expression %prec UMINUS {
        $$ = ast_unary("-", $2);
    }
  | expression '+' expression {
        $$ = ast_binary("+", $1, $3);
    }
  | expression '-' expression {
        $$ = ast_binary("-", $1, $3);
    }
  | expression '*' expression {
        $$ = ast_binary("*", $1, $3);
    }
  | expression '/' expression {
        $$ = ast_binary("/", $1, $3);
    }
  | expression EQ expression {
        $$ = ast_binary("==", $1, $3);
    }
  | expression NE expression {
        $$ = ast_binary("!=", $1, $3);
    }
  | expression '<' expression {
        $$ = ast_binary("<", $1, $3);
    }
  | expression '>' expression {
        $$ = ast_binary(">", $1, $3);
    }
  | expression LE expression {
        $$ = ast_binary("<=", $1, $3);
    }
  | expression GE expression {
        $$ = ast_binary(">=", $1, $3);
    }
;

%%

void yyerror(const char *message)
{
    parse_failed = 1;
    fprintf(stderr, "Syntax error: %s\n", message);
}