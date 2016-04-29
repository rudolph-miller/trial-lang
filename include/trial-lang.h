#ifndef TL_H__
#define TL_H__

#include "trial-lang/value.h"

typedef struct {
} tl_state;

tl_state *tl_open();
void tl_close(tl_state *);

void *tl_alloc(tl_state *, size_t);
struct tl_object *tl_gc_alloc(tl_state *, size_t, enum tl_tt);
void tl_free(tl_state *, void *);

#endif
