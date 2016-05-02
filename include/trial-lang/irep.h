#ifndef IREP_H__
#define IREP_H__

enum tl_instruction {
  OP_PUSHNIL,
  OP_PUSHNUM,
  OP_PUSHUNDEF,
  OP_GREF,
  OP_GSET,
  OP_CALL,
  OP_CONS,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_STOP
};

struct tl_code {
  enum tl_instruction inst;
  union {
    double f;
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
