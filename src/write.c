#include <stdio.h>
#include <stdlib.h>

#include "trial-lang.h"

void tl_debug(tl_state *tl, tl_value obj) {
  switch (tl_type(obj)) {
    case TL_TT_NIL:
      printf("()");
      break;
    case TL_TT_PAIR:
      printf("(");
      tl_debug(tl, tl_car(tl, obj));
      printf(" . ");
      tl_debug(tl, tl_cdr(tl, obj));
      printf(")");
      break;
    case TL_TT_SYMBOL:
      printf("%s", tl_symbol_ptr(obj)->name);
      break;
    default:
      abort();
  }
}
