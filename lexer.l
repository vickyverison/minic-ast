%option noyywrap

%{
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *copy_text(const char *text)
{
    size_t size = strlen(text) + 1;
    char *copy = malloc(size);
    if (!copy) {
        fprintf(stderr, "Out of memory\n");
        exit(2);
    }
    memcpy(copy, text, size);
    return copy;
}
%}

%%

"int"       { return INT; }
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"printf"    { return PRINTF; }
\"%d\\n\"    { return FORMAT; }

"=="        { return EQ; }
"!="        { return NE; }
"<="        { return LE; }
">="        { return GE; }

[0-9]+ {
    yylval.number = strtol(yytext, NULL, 10);
    return NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.text = copy_text(yytext);
    return IDENTIFIER;
}

"//"[^\r\n]* ;
[ \t\r\n]+  ;

[(){};,+*/=<>\-] {
    return yytext[0];
}

. {
    fprintf(stderr, "Unknown character: %s\n", yytext);
    return yytext[0];
}

%%