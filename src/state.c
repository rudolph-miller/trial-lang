#include <stdlib.h>

#include "trial-lang.h"

tl_state *tl_open() {
  tl_state *tl;

  tl = (tl_state *)calloc(1, sizeof(tl_state));

  return tl;
}

void tl_close(tl_state *tl) { free(tl); }
