#ifndef VALUE_H__
#define VALUE_H__

enum tl_vtype { TL_VTYPE_HEAP };

typedef struct {
  enum tl_vtype type;
  union {
    void *data;
  } u;
} tl_value;

enum tl_tt { TL_TT_PA_PAIR, TL_TT_SYMBOL };

#define TL_OBJECT_HEADER enum tl_tt tt;

struct tl_object {
  TL_OBJECT_HEADER
};

struct tl_pair {
  TL_OBJECT_HEADER
  tl_value car;
  tl_value cdr;
};

struct tl_symbol {
  TL_OBJECT_HEADER
  char *name;
};

tl_value tl_obj_value(struct tl_object *);

#endif
