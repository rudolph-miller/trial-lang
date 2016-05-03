#ifndef PROC_H__
#define PROC_H__

struct tl_env {
  tl_value assoc;
  struct tl_env *parent;
};

struct tl_proc {
  TL_OBJECT_HEADER
  bool cfunc_p;
  union {
    tl_value (*cfunc)(tl_state *);
    struct tl_irep *irep;
  } u;
};

#define tl_proc_ptr(o) ((struct tl_proc *)o.u.data)

#endif
