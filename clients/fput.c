    /*********************************************************************\
    *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include "client_def.h"
#include "c_extern.h"
#include "co_extern.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my-string.h"
#include "merge.h"

static int timestamps=0;
static int optletter;

static void usage (void)
{
    printf("fput");
    printf(" [-p | -h | -? ] file ...\n");
    printf("-p\tPreserve file modification time\n");
}

static int put_file (char * path)
{
  struct stat sb;
  char *name, *t2;
  FILE *fp;

  if(stat(path,&sb) != 0) {
    perror(path);
    return(0);
  }
  if(!(S_ISREG(sb.st_mode))) {
    fprintf(stderr,"%s: not a file\n",path);
    return(0);
  }

  for(name = t2 = path; *t2; t2++)
    if(*t2 == '/') name = t2 + 1;

  if( (fp = fopen(path,"rb"))) {
    util_upload(name,fp,timestamps==1?sb.st_mtime:0L);
    fclose(fp);
  } else fprintf(stderr,"Cannot read %s\n",path);

  return(0);
}

static RETSIGTYPE upload_cleanup (int signum)
{
  UBUF *ub;

  env_timeout = 10;
  ub=client_interact(CC_INSTALL,0L, 1, "", 0, (unsigned char *)NULLP);
  if(ub->cmd==CC_UP_LOAD)
     ub=client_interact(CC_INSTALL,0L, 1, "", 0, (unsigned char *)NULLP);
  client_done();
  exit(EX_TEMPFAIL);
}

int main (int argc, char ** argv)
{
  char n[1024];
  int prompt;

  env_client();
  if (strcmp(env_local_dir,".") && chdir(env_local_dir)) {
    perror("chdir");
    exit(EX_NOINPUT);
  }

  signal(SIGHUP,SIG_IGN);
  signal(SIGINT,upload_cleanup);
  signal(SIGQUIT,upload_cleanup);
  signal(SIGILL,upload_cleanup);
  signal(SIGTRAP,upload_cleanup);
  signal(SIGFPE,upload_cleanup);
  signal(SIGSEGV,upload_cleanup);
#ifndef __linux__
  signal(SIGEMT,upload_cleanup);
  signal(SIGBUS,upload_cleanup);
  signal(SIGSYS,upload_cleanup);
#endif
  signal(SIGPIPE,upload_cleanup);
  signal(SIGTERM,upload_cleanup);

  while ((optletter=getopt(argc, argv,"ph?")) != EOF)
  {
    switch (optletter) {
      case 'h':
      case '?':
	       usage();
	       exit(EX_OK);
      case 'p':
	       timestamps=1;
    }
  }
  if(argc > optind)
  {
    while(argc > optind)
      put_file(argv[optind++]);
  }
  else {
    prompt = isatty(0);
    while(1) {
      if(prompt) {
	fputs("fput: ",stdout);
	fflush(stdout);
      }
      if(!getsl(n,1024)) break;
      if(!*n) continue;
      put_file(n);
    }
  }

  client_done();

  exit(EX_OK);
}
