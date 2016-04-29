#include <string.h>

#include "trial-lang.h"

bool tl_eq_p(tl_state *tl, tl_value x, tl_value y) {
  if (tl_type(x) != tl_type(y)) return false;

  switch (tl_type(x)) {
    case TL_TT_NIL:
      return true;
    case TL_TT_SYMBOL:
      return strcmp(tl_symbol_ptr(x)->name, tl_symbol_ptr(y)->name) == 0;
    default:
      return false;
  }
}
