#include <stdlib.h>

#include "trial-lang.h"

enum tl_instruction {
  OP_PUSHNIL,
  OP_PUSHI,
  OP_PUSHUNDEF,
  OP_CONS,
  OP_ADD,
  OP_STOP
};

struct tl_code {
  enum tl_instruction inst;
  union {
    int i;
  } u;
};

struct tl_irep {
  struct tl_code *code;
  size_t clen;
  size_t ccapa;
};

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
  tl_value sCONS;
  tl_value sADD;

  sCONS = tl_intern_cstr(tl, "cons");
  sADD = tl_intern_cstr(tl, "add");

  switch (tl_type(obj)) {
    case TL_TT_SYMBOL: {
      break;
    }
    case TL_TT_PAIR: {
      tl_value proc;

      proc = tl_car(tl, obj);
      if (tl_eq_p(tl, proc, sCONS)) {
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
