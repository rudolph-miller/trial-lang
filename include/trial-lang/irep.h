#ifndef IREP_H__
#define IREP_H__

enum tl_instruction {
  OP_PUSHNIL,
  OP_PUSHI,
  OP_PUSHUNDEF,
  OP_GREF,
  OP_GSET,
  OP_CONS,
  OP_ADD,
  OP_STOP
};

struct tl_code {
  enum tl_instruction inst;
  union {
    int i;
    struct tl_pair *gvar;
  } u;
};

struct tl_irep {
  struct tl_code *code;
  size_t clen;
  size_t ccapa;
};

#endif
