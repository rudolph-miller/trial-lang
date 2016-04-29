#include <stdio.h>

#define LINE_MAX_LENGTH 256

int main() {
  char line[LINE_MAX_LENGTH];
  char last_char;
  int char_index;

  while (1) {
    printf("> ");
    char_index = 0;
    while ((last_char = getchar()) != '\n') {
      if (last_char == EOF) goto eof;
      if (char_index == LINE_MAX_LENGTH) goto overflow;
      line[char_index++] = last_char;
    }
    line[char_index] = '\0';

    printf("%s\n", line);
  }

overflow:
  puts("Line has oveflowed.");
  goto exit;

eof:
  puts("");
  goto exit;

exit:
  return 0;
}
