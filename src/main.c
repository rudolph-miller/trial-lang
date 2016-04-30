#include <stdio.h>

#include "trial-lang.h"

#define LINE_MAX_LENGTH 256

int main() {
  tl_state *tl;
  char line[LINE_MAX_LENGTH];
  char last_char;
  int char_index;
  tl_value v;
  struct tl_proc *proc;
  int ai;

  tl = tl_open();

  ai = tl_gc_arena_preserve(tl);

  while (1) {
    printf("> ");
    char_index = 0;
    while ((last_char = getchar()) != '\n') {
      if (last_char == EOF) goto eof;
      if (char_index == LINE_MAX_LENGTH) goto overflow;
      line[char_index++] = last_char;
    }
    line[char_index] = '\0';

    v = tl_parse(tl, line);

    proc = tl_codegen(tl, v, tl->global_env);
    v = tl_run(tl, proc, tl_nil_value());
    tl_debug(tl, v);
    printf("\n");

    tl_gc_arena_restore(tl, ai);
  }

overflow:
  puts("Line has oveflowed.");
  goto exit;

eof:
  puts("");
  goto exit;

exit:
  tl_close(tl);

  return 0;
}
