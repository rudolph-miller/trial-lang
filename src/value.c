#include "trial-lang.h"

enum tl_tt tl_type(tl_value v) {
  switch (v.type) {
    case TL_VTYPE_NIL:
      return TL_TT_NIL;
    case TL_VTYPE_INT:
      return TL_TT_INT;
    case TL_VTYPE_UNDEF:
      return TL_TT_UNDEF;
    case TL_VTYPE_HEAP:
      return ((struct tl_object *)v.u.data)->tt;
  }
}

tl_value tl_nil_value() {
  tl_value v;

  v.type = TL_VTYPE_NIL;
  v.u.data = NULL;
  return v;
}

tl_value tl_obj_value(void *obj) {
  tl_value v;

  v.type = TL_VTYPE_HEAP;
  v.u.data = obj;
  return v;
}

tl_value tl_int_value(int i) {
  tl_value v;

  v.type = TL_VTYPE_INT;
  v.u.i = i;
  return v;
}

tl_value tl_undef_value() {
  tl_value v;

  v.type = TL_VTYPE_UNDEF;
  v.u.data = NULL;
  return v;
}
