#ifndef __RS_MEMPOOL_H__
#define __RS_MEMPOOL_H__

/* Mempool - an uber simple, thread-unsafe, memory pool */
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

/* stateless allocation function for the pool */
typedef void *(*mempool_alloc_fn)();
/* free function for the pool */
typedef void (*mempool_free_fn)(void *);

/* mempool - the struct holding the memory pool */
#ifndef _RS_MEMPOOL_C_
typedef struct mempool_t mempool_t;
#else
struct mempool_t;
#endif

#define MEMPOOOL_STATIC_ALLOCATOR(name, sz) \
  void *name() {                            \
    return malloc(sz);                      \
  }
/* Create a new memory pool */
struct mempool_t *mempool_new(size_t cap, mempool_alloc_fn alloc, mempool_free_fn free);
struct mempool_t *mempool_new_limited(size_t cap, size_t max_cap, mempool_alloc_fn alloc,
                                      mempool_free_fn free);

/* Get an entry from the pool, allocating a new instance if unavailable */
void *mempool_get(struct mempool_t *p);

/* Release an allocated instance to the pool */
void mempool_release(struct mempool_t *p, void *ptr);

/* destroy the pool, releasing all entries in it and destroying its internal array */
void mempool_destroy(struct mempool_t *p);

/**
 * Declare a thread-specific thread pool. This transparently sets up the
 * creation and destruction of the pool. Call MEMPOOL_GET_THREADED to get the
 * actual pool back for allocations etc.
 */
#define MEMPOOL_DECLARE_THREADED(name, allocfn, freefn, initcap, maxcap) \
  static pthread_key_t name##__ptKey;                                    \
  static inline mempool_t *name##__getPool(void) {                       \
    mempool_t *pool = pthread_getspecific(name##__ptKey);                \
    if (pool) {                                                          \
      return pool;                                                       \
    }                                                                    \
    pool = mempool_new_limited(initcap, maxcap, allocfn, freefn);        \
    pthread_setspecific(name##__ptKey, pool);                            \
    return pool;                                                         \
  }                                                                      \
  static void __attribute__((constructor)) name##__initMempool() {       \
    pthread_key_create(&name##__ptKey, (void(*))mempool_destroy);        \
  }

#define MEMPOOL_GET_THREADED(name) name##__getPool()

#endif