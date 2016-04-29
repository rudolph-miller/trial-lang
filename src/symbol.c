#include <string.h>

#include "trial-lang.h"

tl_vaule tl_intern_cstr(tl_state *tl, char *name) {
  struct tl_symbol *sym;
  size_t len;

  sym = (struct tl_symbol *)tl_gc_alloc(tl, sizeof(struct tl_symbol),
                                        TL_TT_SYMBOL);

  len = strlen(name);
  sym->name = (char *)tl_alloc(tl, len + 1);
  strncpy(sym->name, name, len + 1);

  return tl_obj_value(sym);
}
