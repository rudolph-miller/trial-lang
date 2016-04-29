#include <stdlib.h>

#include "trial-lang.h"

void *tl_alloc(tl_state *tl, size_t size) { return malloc(size); }

struct tl_object *tl_gc_alloc(tl_state *tl, size_t size, enum tl_tt tt) {
  struct tl_object *obj;

  obj = (struct tl_object *)malloc(size);
  obj->tt = tt;

  return obj;
}

void tl_free(tl_state *tl, void *ptr) { free(ptr); }
