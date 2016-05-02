#include <stdio.h>
#include <stdlib.h>

#include "trial-lang.h"
#include "trial-lang/proc.h"

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
    case TL_TT_FLOAT:
      printf("%f", tl_float(obj));
      break;
    case TL_TT_UNDEF:
      printf("#<undef>");
      break;
    case TL_TT_PROC:
      printf("#<proc %p>", tl_proc_ptr(obj));
      break;
    default:
      printf("#<unknown type>");
      break;
  }
}

static tl_value tl_port_write(tl_state *tl) {
  tl_value v;

  tl_get_args(tl, "o", &v);
  tl_debug(tl, v);
  return tl_undef_value();
}

static tl_value tl_port_newline(tl_state *tl) {
  puts("");
  return tl_undef_value();
}

void tl_init_port(tl_state *tl) {
  tl_defun(tl, "write", tl_port_write);
  tl_defun(tl, "newline", tl_port_newline);
}
