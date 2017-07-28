/* check program for fifocache. This file is public domain.
 * made by radim kolar.
 */

#include "tweak.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "../server/fifocache.h"

static int intcompare(const void *vi1,const void *vi2)
{
    const int *i1,*i2;
    i1 = vi1;
    i2 = vi2;
    if(i1==NULL || i2==NULL) return 1;
    if(*i1==*i2) return 0;
    return 1;
}

static void string_free (void * entry)
{
    char **s=entry;
    if(*s!=NULL)
	free(*s);
}

static int string_compare (const void *e1,const void *e2)
{

    char *const *s1=e1;
    char *const *s2=e2;

    /* strcmp do not likes NULLs */
    if(*s1 && *s2)
    {
       return strcmp(*s1,*s2);
    }else
	return 1;
}

int main(int argv,char **argc)
{
    struct FifoCache * cache;
    char file[20];
    const char *s;
    int i;

    cache=f_cache_new(4,sizeof(file),NULL,sizeof(int),NULL,intcompare);
    assert(cache!=NULL);
    strcpy(file,"/jeden/soubor");
    i=1;
    printf("Puting key %d: %s\n",i,file);
    f_cache_put(cache,&i,file);
    strcpy(file,"/druhy");
    i=2;
    printf("Puting key %d: %s\n",i,file);
    f_cache_put(cache,&i,file);
    strcpy(file,"/treti");
    i=3;
    printf("Puting key %d: %s\n",i,file);
    f_cache_put(cache,&i,file);
    strcpy(file,"/ctvrty/soubor");
    i=4;
    printf("Puting key %d: %s\n",i,file);
    f_cache_put(cache,&i,file);

    for(i=0;i<=5;i++)
    {
       printf("Finding key %d: %s\n",i,(char *)f_cache_find(cache,&i));
    }
    f_cache_clear(cache);
    f_cache_destroy(cache);

    cache=f_cache_new(4,0,0,sizeof(char *),string_free,string_compare);
    assert(cache!=NULL);

    s="lamer1";
    f_cache_put(cache,&s,NULL);
    s="lamer2";
    f_cache_put(cache,&s,NULL);
    s="lamer co tu neni";
    printf("find2: %s\n",(char *)f_cache_find(cache,&s));
    s="lamer1";
    printf("find2: %s\n",(char *)f_cache_find(cache,&s));

    return 0;
}
