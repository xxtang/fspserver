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
#include "merge.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static RETSIGTYPE dont_die (int signum)
{
#ifndef RELIABLE_SIGNALS	
  signal(signum,dont_die);
#endif
}

int main (int argc, char ** argv)
{
  char **av;

  env_client();

  signal(SIGPIPE,dont_die);
  if(isatty(1)) 
      client_trace = 0;
  else
      signal(SIGHUP,dont_die);    

  while(*++argv) {
    av = bsdglob(*argv);
    if(av)
      while(*av)
      {
         util_download(*av,stdout,0);
	 *av++;
      }
  }

  client_done();

  exit(EX_OK);
}
