#include <stdlib.h>

#include "trial-lang.h"
#include "trial-lang/gc.h"

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
}

void *tl_alloc(tl_state *tl, size_t size) {
  void *ptr;

  ptr = malloc(size);
  if (ptr == NULL) tl_raise(tl, "memory exhausted");

  return ptr;
}

void tl_free(tl_state *tl, void *ptr) { free(ptr); }

static void *tl_gc_alloc(tl_state *tl, size_t size) {
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

  return (void *)(p + 1);
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

  obj = (struct tl_object *)malloc(size);
  obj->tt = tt;

  return obj;
}
