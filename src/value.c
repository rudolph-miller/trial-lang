#include "trial-lang.h"

tl_value tl_obj_value(struct tl_object *obj) {
  tl_value v;

  v.vtype = TL_VTYPE_HEAP;
  v.u.data = obj;
  return v;
}
