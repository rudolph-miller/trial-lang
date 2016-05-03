#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "trial-lang.h"
#include "trial-lang/irep.h"
#include "trial-lang/proc.h"

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

void tl_defun(tl_state *tl, const char *name, tl_func_t cfunc) {
  struct tl_proc *proc;
  struct tl_pair *cell;

  proc = (struct tl_proc *)tl_obj_alloc(tl, sizeof(struct tl_proc), TL_TT_PROC);
  proc->u.cfunc = cfunc;
  proc->cfunc_p = true;
  cell = tl_env_define(tl, tl_intern_cstr(tl, name), tl->global_env);
  cell->cdr = tl_obj_value(proc);
}

void tl_get_args(tl_state *tl, const char *format, ...) {
  char c;
  int i = -1;
  va_list ap;

  va_start(ap, format);
  while ((c = *format++)) {
    switch (c) {
      case 'o': {
        tl_value *p;

        p = va_arg(ap, tl_value *);
        *p = tl->sp[i];
        i--;
        break;
      }
    }
  }
}

#define VM_LOOP \
  for (;;) {    \
    switch (pc->inst) {
#define CASE(x) case x:
#define NEXT \
  pc++;      \
  break
#define JUMP break
#define VM_LOOP_END \
  }                 \
  }

#define PUSH(v) (*tl->sp++ = (v))
#define POP() (*--tl->sp)

#define PUSHCI() (tl->ci++)
#define POPCI() (--tl->ci)

tl_value tl_run(tl_state *tl, struct tl_proc *proc, tl_value args) {
  struct tl_code *pc;
  tl_callinfo *ci;
  int ai = tl_gc_arena_preserve(tl);

  pc = proc->u.irep->code;

  ci = PUSHCI();
  ci->proc = proc;
  ci->argc = 0;

  VM_LOOP {
    CASE(OP_PUSHNIL) {
      PUSH(tl_nil_value());
      NEXT;
    }
    CASE(OP_PUSHNUM) {
      PUSH(tl_float_value(pc->u.f));
      NEXT;
    }
    CASE(OP_PUSHUNDEF) {
      PUSH(tl_undef_value());
      NEXT;
    }
    CASE(OP_GREF) {
      PUSH(pc->u.gvar->cdr);
      NEXT;
    }
    CASE(OP_GSET) {
      pc->u.gvar->cdr = POP();
      NEXT;
    }
    CASE(OP_CALL) {
      tl_value c;
      struct tl_proc *proc;
      tl_callinfo *ci;
      int ai = tl_gc_arena_preserve(tl);

      tl_gc_protect(tl, c = POP());
      proc = tl_proc_ptr(c);
      ci = PUSHCI();
      ci->proc = proc;
      ci->argc = pc->u.i;
      PUSH(proc->u.cfunc(tl));
      tl_gc_arena_restore(tl, ai);
      POPCI();
      NEXT;
    }
    CASE(OP_RET) { NEXT; }
    CASE(OP_LAMBDA) {
      struct tl_proc *proc;

      proc = (struct tl_proc *)tl_obj_alloc(tl, sizeof(struct tl_proc *),
                                            TL_TT_PROC);
      proc->cfunc_p = false;
      proc->u.irep = ci->proc->u.irep->proto[pc->u.i];
      PUSH(tl_obj_value(proc));
      tl_gc_arena_restore(tl, ai);
      NEXT;
    }
    CASE(OP_CONS) {
      tl_value a;
      tl_value b;
      tl_gc_protect(tl, a = POP());
      tl_gc_protect(tl, b = POP());
      PUSH(tl_cons(tl, a, b));
      tl_gc_arena_restore(tl, ai);
      NEXT;
    }
    CASE(OP_ADD) {
      tl_value a;
      tl_value b;
      a = POP();
      b = POP();
      PUSH(tl_float_value(tl_float(a) + tl_float(b)));
      NEXT;
    }
    CASE(OP_SUB) {
      tl_value a;
      tl_value b;
      a = POP();
      b = POP();
      PUSH(tl_float_value(tl_float(a) - tl_float(b)));
      NEXT;
    }
    CASE(OP_MUL) {
      tl_value a;
      tl_value b;
      a = POP();
      b = POP();
      PUSH(tl_float_value(tl_float(a) * tl_float(b)));
      NEXT;
    }
    CASE(OP_DIV) {
      tl_value a;
      tl_value b;
      a = POP();
      b = POP();
      PUSH(tl_float_value(tl_float(a) / tl_float(b)));
      NEXT;
    }
    CASE(OP_STOP) { goto STOP; }
  }
  VM_LOOP_END;

STOP:
  POPCI();
  return POP();
}

static void print_irep(tl_state *tl, struct tl_irep *irep) {
  int i;

  printf("## irep %p [clen = %zd, ccapa = %zd]\n", irep, irep->clen,
         irep->ccapa);
  for (i = 0; i < irep->clen; ++i) {
    switch (irep->code[i].inst) {
      case OP_PUSHNIL: {
        puts("OP_PUSHNIL");
        break;
      }
      case OP_PUSHNUM: {
        printf("OP_PUSHNUM\t%f\n", irep->code[i].u.f);
        break;
      }
      case OP_PUSHUNDEF: {
        puts("OP_PUSHUNDEF");
        break;
      }
      case OP_GREF: {
        printf("OP_GREF\t%p\n", irep->code[i].u.gvar);
        break;
      }
      case OP_GSET: {
        printf("OP_GSET\t%p\n", irep->code[i].u.gvar);
        break;
      }
      case OP_CALL: {
        printf("OP_CALL\t%d\n", irep->code[i].u.i);
        break;
      }
      case OP_RET: {
        puts("OP_RET");
        break;
      }
      case OP_LAMBDA: {
        printf("OP_LAMBDA\t%d\n", irep->code[i].u.i);
        break;
      }
      case OP_CONS: {
        puts("OP_CONS");
        break;
      }
      case OP_ADD: {
        puts("OP_ADD");
        break;
      }
      case OP_SUB: {
        puts("OP_SUB");
        break;
      }
      case OP_MUL: {
        puts("OP_MUL");
        break;
      }
      case OP_DIV: {
        puts("OP_DIV");
        break;
      }
      case OP_STOP: {
        puts("OP_STOP");
        break;
      }
    }
  }
}

static struct tl_irep *new_irep(tl_state *tl) {
  struct tl_irep *irep;

  irep = (struct tl_irep *)tl_alloc(tl, sizeof(struct tl_irep));
  irep->code = (struct tl_code *)tl_alloc(tl, sizeof(struct tl_code) * 1024);
  irep->clen = 0;
  irep->ccapa = 1024;
  irep->proto = NULL;
  irep->plen = irep->pcapa = 0;

  return irep;
}

static void tl_gen_call(tl_state *, struct tl_irep *, tl_value,
                        struct tl_env *);
static struct tl_irep *tl_gen_lambda(tl_state *, tl_value, struct tl_env *);

void tl_gen(tl_state *tl, struct tl_irep *irep, tl_value obj,
            struct tl_env *env) {
  tl_value sDEFINE;
  tl_value sLAMBDA;
  tl_value sCONS;
  tl_value sADD;
  tl_value sSUB;
  tl_value sMUL;
  tl_value sDIV;

  sDEFINE = tl->sDEFINE;
  sLAMBDA = tl->sLAMBDA;
  sCONS = tl->sCONS;
  sADD = tl->sADD;
  sSUB = tl->sSUB;
  sMUL = tl->sMUL;
  sDIV = tl->sDIV;

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
      } else if (tl_eq_p(tl, proc, sLAMBDA)) {
        if (irep->proto == NULL) {
          irep->proto =
              (struct tl_irep **)tl_alloc(tl, sizeof(struct tl_irep **) * 5);
          irep->pcapa = 5;
        }
        if (irep->plen >= irep->pcapa) {
          irep->proto =
              (struct tl_irep **)tl_realloc(tl, irep->proto, irep->pcapa * 2);
          irep->pcapa *= 2;
        }
        irep->code[irep->clen].inst = OP_LAMBDA;
        irep->code[irep->clen].u.i = irep->plen;
        irep->clen++;

        irep->proto[irep->plen++] = tl_gen_lambda(tl, obj, env);
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
      } else if (tl_eq_p(tl, proc, sSUB)) {
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, tl_cdr(tl, obj))), env);
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, obj)), env);

        irep->code[irep->clen].inst = OP_SUB;
        irep->clen++;
        break;
      } else if (tl_eq_p(tl, proc, sMUL)) {
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, tl_cdr(tl, obj))), env);
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, obj)), env);

        irep->code[irep->clen].inst = OP_MUL;
        irep->clen++;
        break;
      } else if (tl_eq_p(tl, proc, sDIV)) {
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, tl_cdr(tl, obj))), env);
        tl_gen(tl, irep, tl_car(tl, tl_cdr(tl, obj)), env);

        irep->code[irep->clen].inst = OP_DIV;
        irep->clen++;
        break;
      } else {
        tl_gen_call(tl, irep, obj, env);
        break;
      }
    }
    case TL_TT_FLOAT: {
      irep->code[irep->clen].inst = OP_PUSHNUM;
      irep->code[irep->clen].u.f = tl_float(obj);
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
    case TL_TT_PROC: {
      break;
    }
  }
}

static tl_value reverse(tl_state *tl, tl_value list, tl_value acc) {
  if (tl_nil_p(list)) return acc;
  return reverse(tl, tl_cdr(tl, list), tl_cons(tl, tl_car(tl, list), acc));
}

static void tl_gen_call(tl_state *tl, struct tl_irep *irep, tl_value obj,
                        struct tl_env *env) {
  tl_value seq;
  int i = 0;

  seq = reverse(tl, obj, tl_nil_value());
  for (; !tl_nil_p(seq); seq = tl_cdr(tl, seq)) {
    tl_value v;

    v = tl_car(tl, seq);
    tl_gen(tl, irep, v, env);
    ++i;
  }
  irep->code[irep->clen].inst = OP_CALL;
  irep->code[irep->clen].u.i = i - 1;
  irep->clen++;
}

static struct tl_irep *tl_gen_lambda(tl_state *tl, tl_value obj,
                                     struct tl_env *env) {
  tl_value body;
  tl_value v;
  struct tl_irep *irep;

  irep = new_irep(tl);

  body = tl_cdr(tl, tl_cdr(tl, obj));
  for (v = body; !tl_nil_p(v); v = tl_cdr(tl, v)) {
    tl_gen(tl, irep, tl_car(tl, v), env);
  }
  irep->code[irep->clen].inst = OP_RET;
  irep->clen++;

  return irep;
}

struct tl_proc *tl_codegen(tl_state *tl, tl_value obj, struct tl_env *env) {
  struct tl_proc *proc;
  struct tl_irep *irep;

  proc = (struct tl_proc *)tl_obj_alloc(tl, sizeof(struct tl_proc), TL_TT_PROC);
  proc->u.irep = irep = new_irep(tl);
  proc->cfunc_p = false;

  tl_gen(tl, irep, obj, env);

  irep->code[irep->clen].inst = OP_STOP;
  irep->clen++;

#if VM_DEBUG
  print_irep(tl, irep);
#endif

  return proc;
}

void tl_raise(tl_state *tl, const char *str) {
  puts(str);
  abort();
}
