    /*********************************************************************\
    *  Copyright (c) 1993 by Michael Meskes                               *
    *  (meskes@ulysses.informatik.rwth-aachen.de)                         *
    *  Copyright (c) 2003,2006 by Radim Kolar (hsn@cybermail.net)         *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include "client_def.h"
#include "c_extern.h"
#include <stdio.h>
#include "my-string.h"
#include "merge.h"
#include <pwd.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#ifdef HOST_LOOKUP
#include <netdb.h>
#endif

#define FSP_STAT stat


#include "fhost.h"

static const char *home="/";
static int tryfile=0;


/* generated lex parser */
extern FILE *yyin;
int yylex(void);
int yywrap(void);

static void host_usage (void) /* print usage message */
{
  fprintf(stderr,"Usage: fhost [-d delay] [-p local port] [-l local dir]\n");
  fprintf(stderr,"             [-o timeout] [-t trace] [-w password]\n");
  fprintf(stderr,"             [-f filename] [-h [number|name]] [-c | -b]\n");
  fprintf(stderr,"             [host port [directory] | abbreviation]\n");
  exit(EX_OK);
}

/* get data out of resource file */
static void parse_prof_file_new (const char * filename)
{

  FILE *input=NULL;
  int rc;

  if(filename)
  {
      input=fopen(filename,"r");
  }

  if(input)
  {
      yyin=input;
      rc=0;
  } else
      rc=yywrap();

  if(rc==0)
      yylex();

  if(input)
      fclose(input);
}

int main (int argc, char ** argv)
{
  int optletter,csh,lhost=0;
  register char *p;
  const char *filename=NULL;
  char *log;
  struct passwd *pw=0L;
  struct fsp_host *setup=NULL;

  log = (char *)getlogin();
  if (log) pw = getpwnam(log);
  if (!pw) pw = getpwuid(getuid());
  if (pw) {
    csh = !strcmp(pw->pw_shell + strlen(pw->pw_shell) - 3, "csh");
    home = pw->pw_dir;   /* for default search for file .fsp_prof*/
  } else
      home=getenv("HOME");

  /*
   * Figure out what shell we're using.  A hack, we look for a shell
   * ending in "csh".
   */
  log=getenv("SHELL");
  if(log)
  {
    csh = !strcmp(log + strlen(log) - 3, "csh");
  }

  setup=init_host();
  while ((optletter=getopt(argc, argv,"d:p:l:t:o:f:h:w:bc?")) != EOF)
    switch (optletter) {
      case '?':
        host_usage();
      case 'd':
        setup->delay=atol(optarg); /* FSP_DELAY */
	break;
      case 'p':
	setup->local_port=atol(optarg); /* FSP_LOCAL_PORT */
	break;
      case 'b':
	csh=0;
	break;
      case 'c':
	csh=1;
	break;
      case 'w':
	setup->password=optarg; /* FSP_PASSWORD */
	break;
      case 'l':
	setup->local_dir=optarg; /* FSP_LOCAL_DIR */
	break;
      case 'o':
	setup->timeout=atol(optarg); /* FSP_TIMEOUT */
	break;
      case 't':
	if (!strcmp(optarg,"on")) setup->trace=1;  /* FSP_TRACE */
	else if (!strcmp(optarg,"off")) setup->trace=0;
	else host_usage();
	break;
      case 'f':
	filename=optarg; /* file name */
	break;
      case 'h':
	if (!strcmp(optarg,"number")) lhost=NUMBER; /* host mode */
	else if (!strcmp(optarg,"name")) lhost=NAME;
	else host_usage();
	break;
      default:
	host_usage();
	break;
    }

  if(argc > optind+1 && !filename) { /* host and port and no filename given */
    for (p=argv[optind];!setup->hostname && *p && *p!='\n';p++)
      if (!isdigit(*p) && *p!='.') setup->hostname=argv[optind];
    if (!setup->hostname) setup->hostaddr=argv[optind];
    setup->port=atol(argv[optind+1]);
    if(setup->port==0 || setup->port > 65535)
	setup=init_host();
    if (argc > optind + 1) setup->dir=argv[optind+2]; /* directory given, too */
  } else if (argc > optind) { /* abbreviation given */
    parse_prof_file_new(filename);
    setup=find_host(argv[optind]);
    if(!setup) setup=init_host();
  } else { /* list or set command-line options */
    if (filename || argc==1) {  /* list only */
      parse_prof_file_new(filename);
      list_prof_file();
      exit(EX_OK);
    }
  }
  if(setup->hostname==NULL && setup->hostaddr==NULL)
  {
	fprintf(stderr,"fhost: No host given!\n");
	exit(EX_USAGE);
  }
  print_host_setup(setup,csh,lhost);
  exit(EX_OK);
}

/*
 *      search order: curdir, homedir, sysdir
 *
 * Returns: 1 for terminating scanner or 0 for switching
 */

int yywrap(void)
{
  char *f2=NULL;
  int rc;

  if(yyin!=NULL)
  {
      fclose(yyin);
      yyin=NULL;
  }

  switch(tryfile)
  {
      case 0:
	     /* file in cur. dir */
             yyin=fopen(FSPPROF,"r");
	     break;
      case 1:
	     /* file in home dir */
	     f2=(char *)malloc (strlen(home) + strlen(FSPPROF) + 2);
	     if (!f2) {
		perror("malloc");
		return(1);
	     }
             sprintf (f2,"%s/%s",home,FSPPROF);
	     yyin=fopen(f2,"r");
	     free(f2);
	     break;
      case 2:
	     yyin=fopen(FSPRC,"r");
	     break;
      default:
	     return 1;
  }
  tryfile++;
  if(yyin==NULL)
  {
      /* try next available */
      rc=yywrap();
      return rc;
  }

  return 0;
}
