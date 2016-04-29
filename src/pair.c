#include "trial-lang.h"

tl_value tl_cons(tl_state *tl, tl_value car, tl_value cdr) {
  struct tl_pair *pair;

  pair = (struct tl_pair *)tl_gc_alloc(tl, sizeof(struct tl_pair), TL_TT_PAIR);

  pair->car = car;
  pair->cdr = cdr;

  return tl_obj_value(pair);
}

tl_value tl_car(tl_state *tl, tl_value obj) {
  struct tl_pair *pair;

  pair = (struct tl_pair *)obj.u.data;

  return pair->car;
}

tl_value tl_cdr(tl_state *tl, tl_value obj) {
  struct tl_pair *pair;

  pair = (struct tl_pair *)obj.u.data;

  return pair->cdr;
}
