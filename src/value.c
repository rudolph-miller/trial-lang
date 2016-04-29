#include "trial-lang.h"

enum tl_tt tl_type(tl_value v) {
  switch (v.type) {
    case TL_VTYPE_NIL:
      return TL_TT_NIL;
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
