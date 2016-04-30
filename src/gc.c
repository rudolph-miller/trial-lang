#include <stdlib.h>
#include <stdio.h>

#include "trial-lang.h"
#include "trial-lang/gc.h"
#include "trial-lang/irep.h"

void init_heap_page(struct heap_page *heap) {
  union header *base;
  union header *freep;
  void *p;

  p = (union header *)malloc(TL_HEAP_SIZE);

  heap->base = base =
      (union header *)(((unsigned long)p + sizeof(union header) - 1) &
                       ~(sizeof(union header) - 1));
  base->s.ptr = base + 1;
  base->s.size = 0;

  heap->freep = freep = base->s.ptr;
  freep->s.ptr = base;
  freep->s.size =
      ((char *)p + TL_HEAP_SIZE - (char *)freep) / sizeof(union header);

  heap->endp = freep + freep->s.size;
}

void *tl_alloc(tl_state *tl, size_t size) {
  void *ptr;

  ptr = malloc(size);
  if (ptr == NULL) tl_raise(tl, "memory exhausted");

  return ptr;
}

void tl_free(tl_state *tl, void *ptr) { free(ptr); }

static void gc_protect(tl_state *tl, struct tl_object *obj) {
  if (tl->arena_idx >= TL_ARENA_SIZE) tl_raise(tl, "arena has overflowed");

  tl->arena[tl->arena_idx++] = obj;
}

void tl_gc_protect(tl_state *tl, tl_value v) {
  struct tl_object *obj;

  if (v.type != TL_VTYPE_HEAP) return;

  obj = tl_object_ptr(v);
  gc_protect(tl, obj);
}

int tl_gc_arena_preserve(tl_state *tl) { return tl->arena_idx; }

void tl_gc_arena_restore(tl_state *tl, int state) { tl->arena_idx = state; }

static void *gc_alloc(tl_state *tl, size_t size) {
  union header *freep;
  union header *p;
  union header *prevp;
  size_t nunits;

  nunits = (size + sizeof(union header) - 1) / sizeof(union header) + 1;

  freep = tl->heap->freep;
  prevp = freep;
  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
    if (p->s.size >= nunits) break;
    if (p == freep) return 0;
  }

  if (p->s.size == nunits) {
    prevp->s.ptr = p->s.ptr;
  } else {
    p->s.size -= nunits;
    p += p->s.size;
    p->s.size = nunits;
  }
  tl->heap->freep = prevp;

  p->s.mark = TL_GC_UNMARK;
  return (void *)(p + 1);
}

static void gc_mark(tl_state *, tl_value);

static void gc_mark_object(tl_state *tl, struct tl_object *obj) {
  union header *p;

  p = (union header *)obj - 1;
  p->s.mark = TL_GC_MARK;

  switch (obj->tt) {
    case TL_TT_PAIR: {
      gc_mark(tl, ((struct tl_pair *)obj)->car);
      gc_mark(tl, ((struct tl_pair *)obj)->cdr);
      break;
    }
    case TL_TT_SYMBOL: {
      break;
    }
    case TL_TT_PROC: {
      break;
    }
    default:
      tl_raise(tl, "gc_mark_object logic has flawed");
  }
}

static void gc_mark(tl_state *tl, tl_value v) {
  struct tl_object *obj;

  if (v.type != TL_VTYPE_HEAP) return;

  obj = tl_object_ptr(v);

  gc_mark_object(tl, obj);
}

static void gc_mark_phase(tl_state *tl) {
  tl_value *stack;
  struct tl_env *env;
  int i;

  for (stack = tl->stbase; stack != tl->sp; ++stack) {
    gc_mark(tl, *stack);
  }
  gc_mark(tl, *stack);

  for (i = 0; i < tl->arena_idx; ++i) {
    gc_mark_object(tl, tl->arena[i]);
  }

  env = tl->global_env;
  do {
    gc_mark(tl, env->assoc);
  } while ((env = env->parent) != NULL);
}

static bool is_marked(union header *p) { return p->s.mark == TL_GC_MARK; }

static void gc_unmark(union header *p) { p->s.mark = TL_GC_UNMARK; }

static void gc_finalize_object(tl_state *tl, struct tl_object *obj) {
#if GC_DEBUG
  printf("finalizing object type %d\n", obj->tt);
#endif
  switch (obj->tt) {
    case TL_TT_SYMBOL: {
      char *name;
      name = ((struct tl_symbol *)obj)->name;
      tl_free(tl, name);
      break;
    }
    case TL_TT_PAIR: {
      break;
    }
    case TL_TT_PROC: {
      struct tl_proc *proc;

      proc = (struct tl_proc *)obj;

      tl_free(tl, proc->u.irep->code);
      tl_free(tl, proc->u.irep);
      break;
    }
    default:
      tl_raise(tl, "gc_finalize_object logic has flawed");
  }
}

static void gc_sweep_phase(tl_state *tl) {
  union header *base;
  union header *bp;
  union header *p;

#if GC_DEBUG
  puts("sweep start!");
#endif

  base = tl->heap->base;
  for (p = base->s.ptr; p != base; p = p->s.ptr) {
#if GC_DEBUG
    puts("sweeping block");
#endif

  retry:
    for (bp = p + p->s.size; bp != p->s.ptr; bp += bp->s.size) {
#if GC_DEBUG
      printf("  bp = %p\n  p  = %p\n  p->s.ptr = %p\n  endp = %p\n", bp, p,
             p->s.ptr, tl->heap->endp);
#endif

      if (p >= p->s.ptr && bp == tl->heap->endp) break;
      if (is_marked(bp)) {
#if GC_DEBUG
        printf("marked:\t\t\t");
        tl_debug(tl, tl_obj_value((struct tl_object *)(bp + 1)));
        printf("\n");
#endif
        gc_unmark(bp);
        continue;
      }

#if GC_DEBUG
      puts("unmarked");
#endif

      gc_finalize_object(tl, (struct tl_object *)(bp + 1));

      if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
      } else {
        bp->s.ptr = p->s.ptr;
      }

      if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
      } else {
        p->s.ptr = bp;
        p = p->s.ptr;
      }
      goto retry;
    }
  }
}

void tl_gc_run(tl_state *tl) {
#if GC_DEBUG
  puts("gc run!");
#endif

  gc_mark_phase(tl);
  gc_sweep_phase(tl);
}

void tl_gc_free(tl_state *tl, void *ptr) {
  union header *bp;
  union header *p;

  bp = (union header *)ptr - 1;

  for (p = tl->heap->freep; !(p < bp && bp < p->s.ptr); p = p->s.ptr) {
    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) break;
  }

  if (bp + bp->s.size == p->s.ptr) {
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else {
    bp->s.ptr = p->s.ptr;
  }

  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else {
    p->s.ptr = bp;
  }
  tl->heap->freep = p;
}

struct tl_object *tl_obj_alloc(tl_state *tl, size_t size, enum tl_tt tt) {
  struct tl_object *obj;

  obj = (struct tl_object *)gc_alloc(tl, size);
  if (obj == NULL) {
    tl_gc_run(tl);
    obj = (struct tl_object *)gc_alloc(tl, size);
    if (obj == NULL) tl_raise(tl, "GC memory exhausted");
  }
  obj->tt = tt;

#if GC_DEBUG
  printf("* alloced object type %d\n", tt);
#endif

  gc_protect(tl, obj);
  return obj;
}
