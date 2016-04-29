%{
#include <stdlib.h>
#include <stdio.h>

#include "trial-lang.h"

struct parser_control {
  tl_state *tl;
  tl_value value;
};

#define YYDEBUG 1

int yylex();
int yyerror(struct parser_control *, const char *);
char *yy_scan_string(const char *);
int yylex_destroy();
%}

%parse-param {struct parser_control *p}
%lex-param {struct parser_control *p}

%union {
  tl_value datum;
}

%token tLPAREN tRPAREN tDOT
%token <datum> tSYMBOL

%type <datum> datum simple_datum symbol compound_datum
%type <datum> list list_tail

%%

program
  :
  {
    p-> value = tl_nil_value(p->tl);
  }
  | datum
  {
    p->value = $1;
  }
;

datum
  : simple_datum
  | compound_datum
;

simple_datum
  : symbol
;

symbol
  : tSYMBOL
  {
    $$ = $1;
  }
;

compound_datum
  : list
;

list
  : tLPAREN list_tail
  {
    $$ = $2;
  }
;

list_tail
  : tRPAREN
  {
    $$ = tl_nil_value();
  }
  | datum tDOT datum tRPAREN
  {
    $$ = tl_cons(p->tl, $1, $3);
  }
  | datum list_tail
  {
    $$ = tl_cons(p->tl, $1, $2);
  }
;

%%

int yyerror(struct parser_control *p, const char *msg) {
  tl_debug(p->tl, yylval.datum);
  puts(msg);
  abort();
}

tl_value tl_parse(tl_state *tl, const char *str) {
  struct parser_control p;

  p.tl = tl;

  yy_scan_string(str);
  yyparse(&p);
  yylex_destroy();

  return p.value;
}
