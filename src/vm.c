#include <stdlib.h>
#include <stdio.h>

#include "trial-lang.h"

enum tl_instruction {
  OP_PUSHNIL,
  OP_PUSHI,
  OP_PUSHUNDEF,
  OP_GREF,
  OP_GSET,
  OP_CONS,
  OP_ADD,
  OP_STOP
};

struct tl_code {
  enum tl_instruction inst;
  union {
    int i;
    struct tl_pair *gvar;
  } u;
};

struct tl_irep {
  struct tl_code *code;
  size_t clen;
  size_t ccapa;
};

static tl_value tl_assq(tl_state *tl, tl_value key, tl_value assoc) {
  tl_value cell;

enter:
  if (tl_nil_p(assoc)) return assoc;

  cell = tl_car(tl, assoc);
  if (tl_eq_p(tl, key, tl_car(tl, cell))) return cell;

  assoc = tl_cdr(tl, assoc);
  goto enter;
}

static struct tl_pair *tl_env_lookup(tl_state *tl, tl_value sym,
                                     struct tl_env *env) {
  tl_value v;

enter:
  v = tl_assq(tl, sym, env->assoc);
  if (!tl_nil_p(v)) return tl_pair_ptr(v);

  if (env->parent) {
    env = env->parent;
    goto enter;
  }

  return NULL;
}

static struct tl_pair *tl_env_define(tl_state *tl, tl_value sym,
                                     struct tl_env *env) {
  tl_value cell;

  cell = tl_cons(tl, sym, tl_undef_value());
  env->assoc = tl_cons(tl, cell, env->assoc);

  return tl_pair_ptr(cell);
}

tl_value tl_run(tl_state *tl, struct tl_proc *proc, tl_value args) {
  struct tl_code *pc;
  tl_value *sp;

  pc = proc->u.irep->code;
  sp = tl->sp;

  while (1) {
    switch (pc->inst) {
      case OP_PUSHNIL: {
        *++sp = tl_nil_value();
        break;
      }
      case OP_PUSHI: {
        *++sp = tl_int_value(pc->u.i);
        break;
      }
      case OP_PUSHUNDEF: {
        *++sp = tl_undef_value();
        break;
      }
      case OP_GREF: {
        *++sp = pc->u.gvar->cdr;
        break;
      }
      case OP_GSET: {
        pc->u.gvar->cdr = *sp--;
        break;
      }
      case OP_CONS: {
        tl_value a;
        tl_value b;
        a = *sp--;
        b = *sp--;
        *++sp = tl_cons(tl, a, b);
        break;
      }
      case OP_ADD: {
        tl_value a;
        tl_value b;

        a = *sp--;
        b = *sp--;
        *++sp = tl_int_value(tl_int(a) + tl_int(b));
        break;
      }
      case OP_STOP: {
        goto STOP;
      }
    }
    pc++;
  }

STOP:
  return *sp;
}

void tl_gen(tl_state *tl, struct tl_irep *irep, tl_value obj,
            struct tl_env *env) {
  tl_value sDEFINE;
  tl_value sCONS;
  tl_value sADD;

  sDEFINE = tl_intern_cstr(tl, "define");
  sCONS = tl_intern_cstr(tl, "cons");
  sADD = tl_intern_cstr(tl, "add");

  switch (tl_type(obj)) {
    case TL_TT_SYMBOL: {
      struct tl_pair *gvar;

      gvar = tl_env_lookup(tl, obj, env);
      if (!gvar) tl_raise(tl, "unbound variable");

      irep->code[irep->clen].inst = OP_GREF;
      irep->code[irep->clen].u.gvar = gvar;
      irep->clen++;
      break;
    }
    case TL_TT_PAIR: {
      tl_value proc;

      proc = tl_car(tl, obj);
      if (tl_eq_p(tl, proc, sDEFINE)) {
        struct tl_pair *gvar;

        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, tl_cdr(tl, obj))), env);

        gvar = tl_env_define(tl, tl_car(tl, tl_cdr(tl, obj)), env);

        irep->code[irep->clen].inst = OP_GSET;
        irep->code[irep->clen].u.gvar = gvar;
        irep->clen++;
        irep->code[irep->clen].inst = OP_PUSHUNDEF;
        irep->clen++;
        break;
      } else if (tl_eq_p(tl, proc, sCONS)) {
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, tl_cdr(tl, obj))), env);
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, obj)), env);

        irep->code[irep->clen].inst = OP_CONS;
        irep->clen++;
        break;
      } else if (tl_eq_p(tl, proc, sADD)) {
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, tl_cdr(tl, obj))), env);
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, obj)), env);

        irep->code[irep->clen].inst = OP_ADD;
        irep->clen++;
        break;
      } else {
        break;
      }
    }
    case TL_TT_INT: {
      irep->code[irep->clen].inst = OP_PUSHI;
      irep->code[irep->clen].u.i = tl_int(obj);
      irep->clen++;
      break;
    }
    case TL_TT_NIL: {
      irep->code[irep->clen].inst = OP_PUSHNIL;
      irep->clen++;
      break;
    }
    case TL_TT_UNDEF: {
      irep->code[irep->clen].inst = OP_PUSHUNDEF;
      irep->clen++;
      break;
    }
  }
}

struct tl_proc *tl_codegen(tl_state *tl, tl_value obj, struct tl_env *env) {
  struct tl_proc *proc;
  struct tl_irep *irep;
  struct tl_code *code;

  proc = tl_alloc(tl, sizeof(struct tl_proc));

  proc->u.irep = irep = (struct tl_irep *)malloc(sizeof(struct tl_irep));
  irep->code = code = (struct tl_code *)malloc(sizeof(struct tl_code) * 1024);
  irep->clen = 0;
  irep->ccapa = 1024;

  tl_gen(tl, irep, obj, env);

  irep->code[irep->clen].inst = OP_STOP;
  irep->clen++;

  return proc;
}

void tl_raise(tl_state *tl, const char *str) {
  puts(str);
  abort();
}
