#include <string.h>
#include <stdlib.h>

#include "trial-lang.h"

tl_value tl_intern_cstr(tl_state *tl, const char *name) {
  struct tl_symbol *sym;
  size_t len;

  sym = (struct tl_symbol *)tl_obj_alloc(tl, sizeof(struct tl_symbol),
                                         TL_TT_SYMBOL);

  len = strlen(name);
  sym->name = (char *)malloc(len + 1);
  strncpy(sym->name, name, len + 1);

  return tl_obj_value(sym);
}
