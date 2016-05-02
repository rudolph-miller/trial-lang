#include <stdio.h>
#include <editline/readline.h>

#include "trial-lang.h"

#define LINE_MAX_LENGTH 256

int main() {
  tl_state *tl;
  char *line;
  int char_index;
  tl_value v;
  struct tl_proc *proc;
  int ai;

  tl = tl_open();

  ai = tl_gc_arena_preserve(tl);

  while (1) {
    char_index = 0;
    line = readline("> ");

    v = tl_parse(tl, line);

    proc = tl_codegen(tl, v, tl->global_env);
    v = tl_run(tl, proc, tl_nil_value());
    tl_debug(tl, v);
    printf("\n");

    tl_gc_arena_restore(tl, ai);
  }

eof:
  puts("");
  goto exit;

exit:
  tl_close(tl);

  return 0;
}
