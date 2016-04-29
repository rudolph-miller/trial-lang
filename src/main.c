#include <stdio.h>

#include "trial-lang.h"

#define LINE_MAX_LENGTH 256

void test(tl_state *tl) {
  tl_value v;

  {
    v = tl_intern_cstr(tl, "symbol");
    tl_debug(tl, v);
    puts(" sohuld be `symbol`.");
  }

  {
    v = tl_nil_value();
    tl_debug(tl, v);
    puts(" should be `()`.");
  }

  {
    v = tl_cons(tl, tl_intern_cstr(tl, "car"), tl_intern_cstr(tl, "cdr"));
    tl_debug(tl, v);
    puts(" should be `(car . cdr)`.");
  }
}

int main() {
  tl_state *tl;
  char line[LINE_MAX_LENGTH];
  char last_char;
  int char_index;
  tl_value v;

  tl = tl_open();

  test(tl);

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

    tl_debug(tl, tl_eval(tl, v, tl->global_env));
    printf("\n");
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
