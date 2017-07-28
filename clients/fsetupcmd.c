    /*********************************************************************\
    *  Copyright (c) 1993 by Michael Meskes                               *
    *  (meskes@ulysses.informatik.rwth-aachen.de)                         *
    *  Copyright (c) 2003-9by Radim Kolar (hsn@sendmail.cz)               *
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

#define FSP_STAT stat

#include "fhost.h"

static const char *home="/";
static int tryfile=0;


/* generated lex parser */
extern FILE *sitein;
int sitelex(void);
int sitewrap(void);

static void setup_usage (void) /* print usage message */
{
  fprintf(stderr,"Usage: fsetup -l | [ -b | -c ] host port [directory] | abbreviation \n");
  exit(EX_OK);
}

/* get data out of resource file */
static void parse_prof_file_new (void)
{
  int rc;

  rc=sitewrap();
  if(rc==0)
      sitelex();
}

int main (int argc, char ** argv)
{
  int optletter,csh,lhost=0;
  register char *p;
  char *log;
  struct passwd *pw=0L;
  struct fsp_host *setup=NULL;
  
  /* Determine user home directory, try to use password file first
     fallback to $HOME.
  */   
  log = (char *)getlogin();
  if (log) pw = getpwnam(log);
  if (!pw) pw = getpwuid(getuid());
  if (pw) {
    /* Load default shell type from password file. We will check
    for $SHELL later */  
    csh = !strcmp(pw->pw_shell + strlen(pw->pw_shell) - 3, "csh");
    home = pw->pw_dir;   /* for default search for file .fspsites */
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
  while ((optletter=getopt(argc, argv,"hbcl?")) != EOF)
    switch (optletter) {
      case '?':
      case 'h':
        setup_usage();
      case 'b':
	csh=0;
	break;
      case 'c':
	csh=1;
	break;
      case 'l':
        parse_prof_file_new();
	list_sites_file();
	exit(0);
      default:
	setup_usage();
	break;
    }

  if(argc > optind + 1) { 
    /* host and port given */
    for (p=argv[optind];!setup->hostname && *p && *p!='\n';p++)
      if (!isdigit(*p) && *p!='.') setup->hostname=argv[optind];
    if (!setup->hostname) setup->hostaddr=argv[optind];
    setup->port=atol(argv[optind+1]);
    if(setup->port==0 || setup->port>65535) {
        /* port is not number, clear hostname/address */
	setup=init_host();
    }
    if (argc > optind + 1) setup->dir=argv[optind+2]; /* directory given, too */
  } else if (argc > optind) { /* abbreviation given */
    parse_prof_file_new();
    setup=find_host(argv[optind]);
    if(!setup) setup=init_host();
  } else { /* list or set command-line options */
    if (argc==1) {  /* no arguments */
      setup_usage();
    }
  }
  if(setup->hostname==NULL && setup->hostaddr==NULL)
  {
	fprintf(stderr,"fsetup: No host given!\n");
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

int sitewrap(void)
{
  char *f2=NULL;
  int rc;

  if(sitein!=NULL)
  {
      fclose(sitein);
      sitein=NULL;
  }

  switch(tryfile)
  {
      case 0:
	     /* file in cur. dir */
             sitein=fopen(FSPSITES,"r");
	     break;
      case 1:
	     /* file in home dir */
	     f2=(char *)malloc (strlen(home) + strlen(FSPSITES) + 2);
	     if (!f2) {
		perror("malloc");
		return(1);
	     }
             sprintf (f2,"%s/%s",home,FSPSITES);
	     sitein=fopen(f2,"r");
	     free(f2);
	     break;
      case 2:
	     sitein=fopen(FSPSITESRC,"r");
	     break;
      default:
	     return 1;
  }
  tryfile++;
  if(sitein==NULL)
  {
      /* try next available */
      rc=sitewrap();
      return rc;
  }

  return 0;
}
