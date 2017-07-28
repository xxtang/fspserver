/*
 * Simple FIFO generic cache. (c) Radim Kolar 2003, 2009
 * This file is copyrighted as LGPL v2.1
 *
 * When this file is used as part of FSP, it uses 2-term BSD license
 * (aka MIT/X11 License).
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fifocache.h"

/**
 * Create new FIFO cache.
 *
 * Allocates a memory for new cache stucture and initialise it.
 *
 * @param cachesize cache size in number of entries
 * @param entrysize size of one entry in bytes
 * @param edf  entry destroy function
 * @param kdf  key destroy function
 * @param kcf  key compare function, must return zero for equals keys
 * @return NULL or error, otherwise newly allocated fifocache structure
 */

struct FifoCache * f_cache_new(unsigned int cachesize,unsigned int entrysize,
    void (*edf) (void *),unsigned int keysize, void (*kdf) (void *), int (*kcf)(const void *,const void *))
{
    struct FifoCache *cache;
    if(cachesize==0) return NULL;
    if(entrysize==0 && keysize==0) return NULL;
    cache=calloc(1,sizeof(struct FifoCache));
    if(cache==NULL) return NULL;

    cache->cachesize=cachesize;
    cache->keysize=keysize;
    cache->entrysize=entrysize;

    if(entrysize==0)
        cache->e_head=calloc(1,1);
    else
	cache->e_head=calloc(cachesize,entrysize);

    if(keysize==0)
        cache->k_head=calloc(1,1);
    else
        cache->k_head=calloc(cachesize,keysize);

    if(!cache->e_head || !cache->k_head)
    {
	f_cache_destroy(cache);
	return NULL;
    }
    cache->k_next=cache->k_head;

    if(keysize==0)
	cache->k_stop=NULL;
    else
	cache->k_stop=cache->k_head+cache->keysize*cache->cachesize;

    cache->e_next=cache->e_head;
    if(entrysize==0)
	cache->e_stop=NULL;
    else
	cache->e_stop=cache->e_head+cache->entrysize*cache->cachesize;

    cache->k_destroy_func=kdf;
    cache->e_destroy_func=edf;
    cache->k_compare_func=kcf;

    return cache;
}

/**
 * Set memory profile functions.
 *
 * Memory profile functions returns sizes of data referenced
 * by keys or entries. In this cache entries and keys are fixed
 * size.
 *
 * @param cache cache to operate on
 * @param keysize returns size of key in bytes
 * @param entrysize returns size of entry in bytes
 */
void f_cache_set_memory_profilers(struct FifoCache *cache,unsigned int (*keysize) (const void *key),unsigned int (*entrysize) (const void *entry))
{
    cache->get_keysize=keysize;
    cache->get_entrysize=entrysize;
}

/**
 * Prints cache memory usage and hit/miss ratio.
 *
 * @param cache cache to operate on
 * @param f FILE to write results to
 */ 
void f_cache_stats(struct FifoCache *cache,FILE *f)
{
    unsigned int i;
    unsigned int used=0;
    unsigned int j;
    unsigned int dkey=0;
    unsigned int dentry=0;

    if(f==NULL) return;
    /* count used items */
    for(i=0;i<cache->cachesize;i++)
    {
        if(cache->keysize>0)
	{
	    for(j=0;j<cache->keysize;j++)
	    {
		if( *(const char*)(cache->k_head+i*cache->keysize+j) != 0)
		{
		    used++;
		    break;
		}
	    }
	}
    }

    /* calculate dynamic memory used */
    if(cache->get_keysize)
    {
	for(i=0;i<cache->cachesize;i++)
	{
	    dkey+=cache->get_keysize(cache->k_head+i*cache->keysize);
	}
    }
    if(cache->get_entrysize)
    {
	for(i=0;i<cache->cachesize;i++)
	{
	    dentry+=cache->get_entrysize(cache->e_head+i*cache->entrysize);
	}
    }
    fprintf(f,"%u entries (%u used). Memory: %u hdr, keys %u static + %u dynamic, entries %u static + %u dynamic. Hit ratio: %u hit, %u miss.\n",
	    cache->cachesize,used,(unsigned int)sizeof(struct FifoCache), cache->cachesize*cache->keysize,dkey,cache->cachesize*cache->entrysize,dentry,cache->hit,cache->miss);
}

/**
 * Empty memory profiler function.
 *
 * Intended use is passing this function as argument to
 * f_cache_set_memory_profilers.
 *
 * @return always 0
 */
unsigned int f_cache_void_profiler(void *anything)
{
    return 0;
}

/**
 * Destroys cache without calling destructor function on
 * keys or elements. 
 *
 * Keys and elements and cache control structure are zeroed and deallocated
 * but if they pointed to another structures via pointers, then these
 * structures are left intact.
 *
 * @param cache cache to be destroyed
 */
void f_cache_destroy(struct FifoCache *cache)
{
    if(cache==NULL) return;
    if(cache->e_head)
    {
	memset(cache->e_head,0,cache->entrysize*cache->cachesize);
	free(cache->e_head);
    }
    if(cache->k_head) {
	memset(cache->k_head,0,cache->keysize*cache->cachesize);
	free(cache->k_head);
    }
    memset(cache,0,sizeof(struct FifoCache));
    free(cache);
}

/** 
 * Copy record into cache replacing existing record.
 *
 * Always creates new record even if we already have record for key in
 * cache.
 * @param cache cache to put record into
 * @param key key for record
 * @param data data for record
 * @return pointer to data entry in cache
 */
void * f_cache_put(struct FifoCache *cache,const void *key,const void *data)
{
   void *where;
   /* free place for new element */
   if(cache->k_destroy_func) cache->k_destroy_func(cache->k_next);
   if(cache->e_destroy_func) cache->e_destroy_func(cache->e_next);

   /* copy new element in */
   where=cache->e_next;
   memcpy(cache->k_next,key,cache->keysize);
   memcpy(cache->e_next,data,cache->entrysize);

   /* update next pos. */
   cache->e_next+=cache->entrysize;
   cache->k_next+=cache->keysize;

   /* roll over? */
   if(cache->e_next==cache->e_stop || cache->k_next==cache->k_stop)
   {
       cache->e_next=cache->e_head;
       cache->k_next=cache->k_head;
   }

   return where;
}

/**
 * Find element by key.
 *
 * Linear search is performed.
 * @param cache used cache
 * @param key key for finding data
 * @return cached data or NULL 
 */
void *f_cache_find(struct FifoCache *cache,const void *key)
{
    unsigned int i;

    if(!cache->k_compare_func) return NULL;
    if(cache->keysize==0) return NULL;

    for(i=0;i<cache->cachesize;i++)
    {
	if(!cache->k_compare_func(key,cache->k_head+i*cache->keysize))
	{
	    cache->hit++;
	    return cache->e_head+i*cache->entrysize;
	}
    }
    cache->miss++;
    return NULL;
}

/**
 *  clear all elements from the cache. 
 *
 *  Cache itself is not destroyed and destroy functions are called.
 *
 *  @param cache cache to be cleared
 */
void f_cache_clear(struct FifoCache *cache)
{
    unsigned int i;

    /* free entries */
    for(i=0;i<cache->cachesize;i++)
    {
       if(cache->k_destroy_func)
	   cache->k_destroy_func(cache->k_head+i*cache->keysize);
       if(cache->e_destroy_func)
	   cache->e_destroy_func(cache->e_head+i*cache->entrysize);
    }

    /* clear entries */
    memset(cache->k_head,0,cache->cachesize*cache->keysize);
    memset(cache->e_head,0,cache->cachesize*cache->entrysize);

    cache->k_next=cache->k_head;
    cache->e_next=cache->e_head;

    cache->hit=0;
    cache->miss=0;
}

/**
 *  find key for given entry.
 *
 *  Entry must been received from cache before. We also dont
 *  check if data have not been overwriten by new content after receiving.
 *
 *  @param cache cache to search
 *  @param entry entry to search
 *  @return key for entry or NULL
 */
void * f_cache_get_key(struct FifoCache *cache,const void *entry)
{
    unsigned int i;
    if(cache->entrysize==0 || cache->keysize==0) return NULL;

    /* check if pointer is good */
    if(entry<(const void *)cache->e_head || entry>=(const void *)cache->e_stop)
         return NULL;
    /* find cache index */
    i=((const int8_t *)entry-cache->e_head)/cache->entrysize;
    return cache->k_head+cache->keysize*i;
}

/** 
 * delete entry from cache. 
 *
 * Destroy function is called on entry.
 * @param cache from what cache to delete entry
 * @param entry entry to be deleted
 * @return one if object is deleted
 */
int f_cache_delete_entry(struct FifoCache *cache, void *entry)
{
    unsigned int i;

    if(cache->entrysize==0) return 0;
    /* check if pointer is good */
    if(entry<(const void *)cache->e_head || entry>=(const void *)cache->e_stop) return 0;
    /* find cache index */
    i=((int8_t *)entry-cache->e_head)/cache->entrysize;

    /* deallocate */
    if(cache->k_destroy_func) cache->k_destroy_func(cache->k_head+cache->keysize*i);
    if(cache->e_destroy_func) cache->e_destroy_func(entry);
    /* zero them */
    memset(entry,0,cache->entrysize);
    memset(cache->k_head+cache->keysize*i,0,cache->keysize);

    return 1;
}

/**
 * Perform searched delete
 *
 * @param cache cache to search
 * @param key key used for deleting entries
 * @return how many objects were deleted.
 */
int f_cache_delete_by_key(struct FifoCache *cache, void *key)
{
    unsigned int i;
    int deleted=0;

    if(!cache->k_compare_func) return 0;

    for(i=0;i<cache->cachesize;i++)
    {
	if(!cache->k_compare_func(key,cache->k_head+i*cache->keysize))
	{
	    deleted+=f_cache_delete_entry(cache,cache->e_head+i*cache->entrysize);
	}
    }

    return deleted;
}
