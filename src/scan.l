%{
#include "trial-lang.h"
#include "y.tab.h"

struct parser_control {
  tl_state *tl;
  tl_value value;
};

#define YY_DECL int yylex (struct parser_control *p)
%}

%%

"(" return tLPAREN;
")" return tRPAREN;
"." return tDOT;
[a-z]+ { yylval.datum = tl_intern_cstr(p->tl, yytext); return tSYMBOL; }

%%

int yywrap() {
  return 1;
}
