#include <sys/types.h>

/* 
 * Simple FIFO generic cache. (c) Radim Kolar 2003, 2009
 * This file is copyrighted as LGPL v2.1
 *
 * When this file is used as part of FSP, it uses 2-term BSD license
 * (aka MIT/X11 License).
 */

struct FifoCache
{
    unsigned int cachesize; /* max. number of entries */
    int8_t *e_head;/* block of cache entries, every entry has entrysize bytes */
    int8_t *e_next; /* new intem will be placed there */
    const void *e_stop; /* stop mark for entries, never write here! */
    unsigned int entrysize; /* size of 1 entry in bytes */
    int8_t *k_head; /* block of keys starts there */
    int8_t *k_next; /* new item */
    const void *k_stop; /* stop mark for key entries */
    unsigned int keysize; /* size of 1 key in bytes */
    void (*k_destroy_func) (void *key); /* key destoy function */
    void (*e_destroy_func) (void *key); /* element destoy function */
    int (*k_compare_func) (const void *key1,const void *key2); /* element destoy function */
    unsigned int hit; /* cache search hits */
    unsigned int miss; /* cache search misses */
    unsigned int (*get_keysize) (const void *key); /* return dynamic memory used by key */
    unsigned int (*get_entrysize) (const void *entry);  /* return dynamic memory used by entry */
};

/* prototypes */
struct FifoCache * f_cache_new(unsigned int cachesize,unsigned int entrysize,void (*edf) (void *key),unsigned int keysize, void (*kdf) (void *key), int (*kcf)(const void *,const void *));
void f_cache_destroy(struct FifoCache *cache);
void * f_cache_put(struct FifoCache *cache,const void *key,const void *data);
void f_cache_clear(struct FifoCache *cache);
void *f_cache_find(struct FifoCache *cache,const void *key);
int f_cache_delete_entry(struct FifoCache *cache, void *entry);
void * f_cache_get_key(struct FifoCache *cache,const void *entry);
int f_cache_delete_by_key(struct FifoCache *cache, void *key);
/* utility functions */
unsigned int f_cache_void_profiler(void *anything);
void f_cache_set_memory_profilers(struct FifoCache *cache,unsigned int (*keysize) (const void *key),unsigned int (*entrysize) (const void *entry));
void f_cache_stats(struct FifoCache *cache,FILE *f);
