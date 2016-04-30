#ifndef GC_H__
#define GC_H__

union header {
  struct {
    union header *ptr;
    size_t size;
  } s;
  long alignment;
};

struct heap_page {
  union header *base;
  union header *freep;
  size_t heap_size;
};

void init_heap_page(struct heap_page *);

#endif
