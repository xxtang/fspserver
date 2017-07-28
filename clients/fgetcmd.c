    /*********************************************************************\
    *  Copyright (c) 2003 by Radim Kolar (hsn@cybermail.net)              *
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
#include "bsd_extern.h"
#include "co_extern.h"
#include <signal.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_UTIME_H
#include <utime.h>
#endif
#include "my-string.h"
#include "merge.h"

#if !defined(COMMAND_GRAB) && !defined(COMMAND_GET)
#error "#define COMMAND_XXX to GET or GRAB when compiling this file"
#endif

#if defined(COMMAND_GRAB) && defined(COMMAND_GET)
#error "You must define COMMAND_GRAB -OR- COMMAND_GET"
#endif

static int clobbertype=C_UNIQUE;
static int preserve=0;
static char *fname;
static int optletter;
static int suffix;
static unsigned long start_from;

static int len;

static RETSIGTYPE fsp_cleanup (int signum)
{
  char filename[20];
  sprintf(filename,".fsp.%d",getpid());
  unlink(filename);
  exit(EX_TEMPFAIL);
}

static void
#ifdef COMMAND_GET
get_file
#else
grab_file
#endif
(char * path, struct stat * sbufp, int mode, int level)
{
  char *name = path + len;
  FILE *fp;
  struct stat statbuf;
#ifdef HAVE_UTIME_H
  struct utimbuf tb;
#endif

  start_from = 0;
 /* printf("Get function called!\n");
  if(sbufp)
  {
      printf("We have stat *!\n");
  }
  */

  if (clobbertype==C_NOCLOBBER) {
    if ( (fp=fopen(name,"r"))) {
      fclose(fp);
      fprintf(stderr,"Will not overwrite existing file %s\n",name);
      return;
    }
  }

  if (clobbertype==C_UNIQUE) {
    fname=name;
    name=(char *)malloc(strlen(fname)+5);
    strcpy(name,fname);
    for (suffix=0 ; (fp=fopen(name,"r")) ; suffix++) {
      fclose(fp);
      sprintf(name,"%s-%d",fname,suffix);
    }
  }

  if (clobbertype==C_TEMPNAME) {
    fname=name;
    name=(char*)malloc(20);
    sprintf(name,".fsp.%d",getpid());
  }

  if(clobbertype == C_APPEND)  {
    if(stat(name, &statbuf) == 0) {
      start_from = statbuf.st_size;
      if(start_from != 0)
         if((fp = fopen(name,"a")) == NULL) { perror(name);
                                             start_from = 0;}
    } else start_from = 0;
  } else start_from = 0;

  if(start_from == 0) {
    fp = fopen(name, "w");
    if (fp == NULL )
            fprintf(stderr,"Cannot create %s\n",name);
  }

  if(fp)
  {
    if(
#ifdef COMMAND_GET
	    util_download
#else
	    util_grab_file
#endif
	    (path,fp, start_from) == -1)
    {
      fclose(fp);
      if(clobbertype==C_TEMPNAME)
	 unlink(name);
    } else
      fclose(fp);
  } else fprintf(stderr,"Cannot write %s\n",name);

#ifdef HAVE_UTIME_H
  /* update last modified time stamp */
  if(preserve && sbufp)
  {
      tb.modtime=sbufp->st_mtime;
      tb.actime=sbufp->st_atime;
      utime(name,&tb);
  }
#endif
  if (clobbertype==C_TEMPNAME) {
    rename(name,fname);
    free(name);
  }

  if (clobbertype==C_UNIQUE) {
    free(name);
  }
  return;
}

static int make_dir (char * name, struct stat * sbufp, u_long * mode)
{
    struct stat sbuf;

    if (*mode == 0) return (-1);

    if (stat(name + len, &sbuf) == 0)
    {
        /* check if the directory already exists... */
        if (S_ISDIR(sbuf.st_mode))
            return (0);

        /* the directory doesn't exist, but *something* does... urgh! */
        fprintf(stderr,"local file `%s' is not a directory\n",
                 name + len);
        return (-1);
    }

    /* nothing exists by this name -- try to create it */
    if (mkdir(name + len, 0755) < 0)
    {
        perror (name + len);
        return(-1);
    }

    return (0);
}

static void usage (void)
{

#ifdef COMMAND_GET
    printf("fgetcmd");
#else
    printf("fgrabcmd");
#endif
    printf(" [options] [filename] ...\n");
    printf("Options:\n");
    printf("-f,-o\toverwrite existing files\n");
    printf("-u\tuse unique names\n");
    printf("-t\tdownload into temporary file\n");
    printf("-n\tnever overwrite existing files\n");
    printf("-a,-c\tappend to files\n");
    printf("-p\tpreserve timestamp of remote files\n");
    printf("-r\trecursively get directories\n");
}
	
  /* Parse options
   *   -f forces overwrite                   (clobber)
   *   -u forces unique names                (unique)
   *   -t uses temporary file to download    (tempname)
   *   -n forces noclobber, never overwrite  (noclober)
   *   -a append to files if they exist      (append)
   *   -c same as -a
   *   -o same as -f
   *   -r recursively get directories
   *   -p preserve date/times of original file on downloaded copy
   */
int main (int argc, char ** argv)
{
  char **av, *av2[2], n[1024];
  int prompt, mode = 0;

  signal(SIGHUP,SIG_IGN);
  signal(SIGINT,fsp_cleanup);
  signal(SIGQUIT,fsp_cleanup);
  signal(SIGILL,fsp_cleanup);
  signal(SIGTRAP,fsp_cleanup);
  signal(SIGFPE,fsp_cleanup);
  signal(SIGSEGV,fsp_cleanup);
#ifndef __linux__
  signal(SIGEMT,fsp_cleanup);
  signal(SIGBUS,fsp_cleanup);
  signal(SIGSYS,fsp_cleanup);
#endif
  signal(SIGPIPE,fsp_cleanup);
  signal(SIGTERM,fsp_cleanup);

  env_client();
  if (strcmp(env_local_dir,".") && chdir(env_local_dir)) {
    perror("chdir");
    exit(EX_NOINPUT);
  }

  while ((optletter=getopt(argc, argv,"ofutnacrph?")) != EOF)
    switch (optletter) {
      case 'o':
      case 'f':
        clobbertype=C_CLOBBER;
        break;
      case 'u':
        clobbertype=C_UNIQUE;
        break;
      case 't':
        clobbertype=C_TEMPNAME;
        break;
      case 'n':
        clobbertype=C_NOCLOBBER;
        break;
      case 'a':
      case 'c':
        clobbertype = C_APPEND;
        break;
      case 'r':
        mode=1;
        break;
      case 'p':
        preserve=1;
        break;
      case 'h':
      case '?':
	usage();
	exit(EX_OK);
    }

  if(argc > optind) {
    for( ; argc>optind ; optind++) {
      if(!(av = bsdglob(argv[optind])))  {
        av = av2;
        av2[0] = argv[optind];
        av2[1] = 0;
      }
      while(*av)
      {
         for(len = strlen(*av); len >= 0 && (*av)[len] != '/'; len--);
         len++;
         util_process_file(*av, mode,
#ifdef COMMAND_GET
		 get_file,
#else
		 grab_file,
#endif
		 make_dir, 0L, 0);
         av++;
      }
    }
  } else {
    prompt = isatty(0);
    while(1) {
      if(prompt) {
        fputs(
#ifdef COMMAND_GET
		"fget: ",
#else
		"fgrab: ",
#endif
		stdout);
        fflush(stdout);
      }
      if(!getsl(n,1024)) break;
      if(!*n) break;
      if(!(av = bsdglob(n))) {
        av = av2;
        av2[0] = n;
        av2[1] = 0;
      }
      while(*av)
      {
         for(len = strlen(*av); len >= 0 && (*av)[len] != '/'; len--);
         len++;
         util_process_file(*av, mode,
#ifdef COMMAND_GET
		 get_file,
#else
		 grab_file,
#endif
		 make_dir, 0L, 0);
         av++;
      }
    }
  }

  client_done();

  exit(EX_OK);
}
