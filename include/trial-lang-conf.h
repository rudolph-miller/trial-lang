#ifndef TLCONF_H__
#define TLCONF_H__

#define TL_ARENA_SIZE 100
#define TL_HEAP_SIZE 4096

#define DEBUG 1

#if DEBUG
# define GC_DEBUG 1
# define VM_DEBUG 1
#endif

#endif
