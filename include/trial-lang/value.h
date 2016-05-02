#ifndef VALUE_H__
#define VALUE_H__

enum tl_vtype {
  TL_VTYPE_NIL,
  TL_VTYPE_FLOAT,
  TL_VTYPE_UNDEF,
  TL_VTYPE_HEAP
};

typedef struct {
  enum tl_vtype type;
  union {
    void *data;
    double f;
  } u;
} tl_value;

enum tl_tt {
  TL_TT_NIL,
  TL_TT_FLOAT,
  TL_TT_UNDEF,
  TL_TT_PAIR,
  TL_TT_SYMBOL,
  TL_TT_PROC
};

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

struct tl_proc;

#define tl_object_ptr(o) ((struct tl_object *)o.u.data)
#define tl_pair_ptr(o) ((struct tl_pair *)o.u.data)
#define tl_symbol_ptr(o) ((struct tl_symbol *)o.u.data)

enum tl_tt tl_type(tl_value);

tl_value tl_nil_value();
tl_value tl_undef_value();
tl_value tl_obj_value(void *);
tl_value tl_float_value(double);

#define tl_float(v) ((v).u.f)

#define tl_nil_p(v) (tl_type(v) == TL_TT_NIL)

#endif
