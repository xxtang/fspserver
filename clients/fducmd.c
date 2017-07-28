/******************************************************************************
* This file is Copyright 1992 by Philip G. Richards.  All Rights Reserved.
* See the file README that came with this distribution for permissions on
* code usage, copying, and distribution.  It comes with absolutely no warranty.
* email: <pgr@prg.ox.ac.uk>
******************************************************************************/

/*****************************************************************************
* reprogrammed as a stand alone client by Michael Meskes
* <meskes@ulysses.informatik.rwth-aachen.de>
******************************************************************************/

#include "tweak.h"
#include "client_def.h"
#include "c_extern.h"
#include "bsd_extern.h"
#include "my-string.h"
#include "merge.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*************************************************************************
* The following global variables are used by fdu		         *
* Added 5. Apr. 93 by M.Meskes                                           *
*************************************************************************/

#define RECURSIVE	1
#define SUM		2
#define EACH		4

static u_long total_file_size;

static void add_file_size (char * name, struct stat * sbufp,
				 int mode, int  level)
{
  register u_long file_size;

  file_size = (sbufp->st_size + 1023) / 1024;
  total_file_size += file_size;
  if(((mode & EACH) && (mode & RECURSIVE)) ||
     ((mode & EACH) && level < 2)) printf("%-7ld %s\n", file_size, name);
}

static int start_dir (char * name, struct stat * sbufp, u_long * sum)
{
  *sum = total_file_size;
  return(0);
}

static void end_dir (char * path, int mode, u_long sum, int level)
{
  /* directories are printed as default */
  /* but, check recursion level */
  if(((mode & RECURSIVE) && !(mode & SUM)) ||
     ((level == 1) && (!(mode & SUM) || (mode & EACH))) ||
     !level) {
    sum = total_file_size - sum; /* this is the real value */
    printf("%-7ld %s\n", sum, path);
  }
}

/* ARGSUSED */
int main (int argc, char ** argv)
{
  int mode=0;
  int filcnt = 0;
  static const char *wild[2] = { ".", 0 };
  char **files, *singlefile[2];
  int optletter;

  env_client();

  while ((optletter=getopt(argc, argv,"ras")) != EOF)
    switch (optletter) {
      case 'r':
        mode |= RECURSIVE; /* recursively read all subdirectories */
	break;
      case 's':
	mode |= SUM; /* print sums only */
	break;
      case 'a':
	mode |= EACH; /* print an entry for each file */
	break;
      default:
	fprintf(stderr,"Usage: du [-r|a|s] directory name.\n");
	exit(EX_USAGE);
    }

  /* special case `du' without file arguments -- becomes `du .' */
  if (argc == optind) {
    argv=(char **)wild;
    optind=0;
  }

  for ( ; argv[optind]; optind++) {
    if (!(files = bsdglob(argv[optind]))) {
      files = singlefile;
      singlefile[0] = argv[optind];
      singlefile[1] = 0;
    }

    for ( ; *files; files++) {
      util_process_file(*files, mode, add_file_size, start_dir, end_dir, 0);
      filcnt++;
    }
  }

  if (filcnt > 1) {
    fprintf(stdout, "--------:------\n");
    fprintf(stdout, "%-7ld   TOTAL\n", total_file_size);
  }

  client_done();

  return 0;
}
