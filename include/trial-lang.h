#ifndef TL_H__
#define TL_H__

typedef struct {
} tl_state;

tl_state *tl_open();
void tl_close(tl_state *);

#endif
