#include <stdio.h>

#include "trial-lang.h"

static tl_value tl_assq(tl_state *tl, tl_value key, tl_value assoc) {
  tl_value cell;

enter:
  if (tl_nil_p(assoc)) return assoc;

  cell = tl_car(tl, assoc);
  if (tl_eq_p(tl, key, tl_car(tl, cell))) return cell;

  assoc = tl_cdr(tl, assoc);
  goto enter;
}

static tl_value tl_env_lookup(tl_state *tl, tl_value sym, struct tl_env *env) {
  tl_value v;

enter:
  v = tl_assq(tl, sym, env->assoc);
  if (!tl_nil_p(v)) return tl_cdr(tl, v);

  if (env->parent) {
    env = env->parent;
    goto enter;
  }

  return tl_nil_value();
}

static void tl_env_define(tl_state *tl, tl_value sym, tl_value obj,
                          struct tl_env *env) {
  env->assoc = tl_cons(tl, tl_cons(tl, sym, obj), env->assoc);
}

tl_value tl_eval(tl_state *tl, tl_value obj, struct tl_env *env) {
  tl_value sDEFINE = tl_intern_cstr(tl, "define");
  tl_value sQUOTE = tl_intern_cstr(tl, "quote");

  while (1) {
    switch (tl_type(obj)) {
      case TL_TT_SYMBOL:
        return tl_env_lookup(tl, obj, env);
      case TL_TT_PAIR: {
        tl_value proc;

        proc = tl_car(tl, obj);
        if (tl_eq_p(tl, proc, sQUOTE)) {
          return tl_car(tl, tl_cdr(tl, obj));
        } else if (tl_eq_p(tl, proc, sDEFINE)) {
          tl_value sym;
          tl_value data;

          sym = tl_car(tl, tl_cdr(tl, obj));
          data = tl_car(tl, tl_cdr(tl, tl_cdr(tl, obj)));
          tl_env_define(tl, sym, tl_eval(tl, data, env), env);

          return tl_nil_value();
        } else {
          /* not implemented */
        }
      }
      case TL_TT_NIL:
        return obj;
      case TL_TT_INT:
        return obj;
      default:
        return tl_nil_value();
    }
  }
}
