#include "trial-lang.h"

void tl_init_port(tl_state *);

#define DONE tl_gc_arena_restore(tl, ai);

void tl_init_core(tl_state *tl) {
  int ai;

  ai = tl_gc_arena_preserve(tl);
  tl_init_port(tl);
  DONE;
}
