#ifndef GC_H__
#define GC_H__

enum tl_gc_mark {
  TL_GC_MARK,
  TL_GC_UNMARK
};

union header {
  struct {
    union header *ptr;
    size_t size;
    enum tl_gc_mark mark;
  } s;
  long alignment;
};

struct heap_page {
  union header *base;
  union header *freep;
  union header *endp;
  size_t heap_size;
};

void init_heap_page(struct heap_page *);

#endif
