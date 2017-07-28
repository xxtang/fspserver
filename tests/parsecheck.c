#include "tweak.h"
#include "server_def.h"
#include "s_extern.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my-string.h"

int dbug=1;

const char *testcases[]={
    "", ".","/",
    "\nonlypwd",
    "filename","/filename","//filename",
    "dirname/filename","//dirname/filename","//dirname//filename",
    "dir1name/dir2name/","//dir1name//dir2name//",
    "filename\npasswd","filename\nsymlink\npasswd",
    "file/.dir","../updir","file/../dir",
    NULL};
PPATH testresults[]={
    {".",".",1,".",1,NULL}, {NULL}, {".",".",1,".",1,NULL},
    {".",".",1,".",1,"onlypwd"},
    {"filename","filename",8,".",1,NULL} , {"filename","filename",8,".",1,NULL} , {"filename","filename",8,".",1,NULL},
    {"dirname/filename","filename",8,"dirname",7,NULL} , {"dirname/filename","filename",8,"dirname",7,NULL} , {"dirname//filename","filename",8,"dirname",7,NULL},
    {"dir1name/dir2name/",".",1,"dir1name/dir2name",17}, {"dir1name//dir2name//",".",1,"dir1name//dir2name",18},
    {"filename","filename",8,".",1,"passwd"}, {"filename","filename",8,".",1,"passwd"},
    {NULL},{NULL},{NULL},


};

static void print_path(PPATH *pp)
{
    printf("fullpath: %s ",pp->fullp);
    if(strcmp(pp->fullp,pp->d_ptr))
    {
	printf("d_ptr: %s (%d) ",pp->d_ptr,pp->d_len);
    } else
	printf("(%d) ",pp->d_len);

    printf("f_ptr: %s (%d) ",pp->f_ptr,pp->f_len);
    if(pp->passwd)
	printf("passwd: %s",pp->passwd);
}

/* returns: 0 okay, 1 different */

static int compareresults(PPATH *p1,PPATH *p2)
{
    if(p1->fullp==NULL && p2->fullp==NULL)  return  0;
    if(p1->fullp==NULL || p2->fullp==NULL)  return  -1;
    if(strcmp(p1->fullp,p2->fullp)) return -1;
    if(p1->f_len!=p2->f_len) return -1;
    if(strcmp(p1->f_ptr,p2->f_ptr)) return -1;
    if(p1->d_len!=p2->d_len) return -1;
    if(strncmp(p1->d_ptr,p2->d_ptr,p1->d_len)) return -1;
    if(p1->passwd==NULL && p2->passwd==NULL)
	return 0;
    if(p1->passwd==NULL || p2->passwd==NULL) return -1;
    if(strcmp(p1->passwd,p2->passwd)) return -1;
    return 0;
}

static int runtestcase(void)
{
    int rc=0;
    int i=0;
    PPATH pp;
    const char *err;
    char *test;

    for(;testcases[i];i++)
    {
	test=strdup(testcases[i]);
	pp.fullp=NULL;
       	err=parse_path(test,strlen(test)+1,&pp);
	printf("parsing: '%s'",test);
	if(err)
	{
	    printf(" parse err: '%s'. ",err);
	    pp.fullp=NULL;
	} else
	    printf(" parsed okay. ");
	if(compareresults(&pp,&testresults[i]))
	{
	    printf("!!!TEST FAILED!!!\a\n");
	    rc=1;
	} else
	    printf(" Test passed.\n");
	if(!err)
	{
          printf("  ");
	  print_path(&pp);
          printf("\n");
	}
	free(test);
    }
    return rc;
}

int main(int argc,const char *argv[])
{
    return runtestcase();
}
