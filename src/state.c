#include <stdlib.h>

#include "trial-lang.h"

static struct tl_env *tl_new_empty_env() {
  struct tl_env *env;

  env = (struct tl_env *)malloc(sizeof(struct tl_env));
  env->assoc = tl_nil_value();
  env->parent = NULL;

  return env;
}

tl_state *tl_open() {
  tl_state *tl;

  tl = (tl_state *)malloc(sizeof(tl_state));
  tl->stbase = tl->sp = (tl_value *)malloc(sizeof(tl_value) * 1024);
  tl->stend = tl->stbase + 1024;
  tl->global_env = tl_new_empty_env();

  return tl;
}

void tl_close(tl_state *tl) { free(tl); }
