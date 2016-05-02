#include <stdlib.h>

#include "trial-lang.h"
#include "trial-lang/gc.h"
#include "trial-lang/proc.h"

static struct tl_env *tl_new_empty_env() {
  struct tl_env *env;

  env = (struct tl_env *)malloc(sizeof(struct tl_env));
  env->assoc = tl_nil_value();
  env->parent = NULL;

  return env;
}

void tl_init_core(tl_state *);

tl_state *tl_open() {
  tl_state *tl;

  tl = (tl_state *)malloc(sizeof(tl_state));
  tl->stbase = tl->sp = (tl_value *)malloc(sizeof(tl_value) * TL_STACK_SIZE);
  tl->stend = tl->stbase + TL_STACK_SIZE;

  tl->cibase = tl->ci =
      (tl_callinfo *)malloc(sizeof(tl_callinfo) * TL_STACK_SIZE);
  tl->ciend = tl->cibase + TL_STACK_SIZE;

  tl->heap = (struct heap_page *)malloc(sizeof(struct heap_page));
  init_heap_page(tl->heap);

  tl->arena_idx = 0;

  tl->sDEFINE = tl_intern_cstr(tl, "define");
  tl->sCONS = tl_intern_cstr(tl, "cons");
  tl->sADD = tl_intern_cstr(tl, "+");
  tl->sSUB = tl_intern_cstr(tl, "-");
  tl->sMUL = tl_intern_cstr(tl, "*");
  tl->sDIV = tl_intern_cstr(tl, "/");

  tl->global_env = tl_new_empty_env();
  tl_init_core(tl);

  return tl;
}

void tl_close(tl_state *tl) { free(tl); }
