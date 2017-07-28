    /*********************************************************************\
    *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
    *  Copyright (c) 2005 by Radim Kolar (hsn@cybermail.net)              *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "client_def.h"
#include "c_extern.h"
#include "bsd_extern.h"
#include "my-string.h"
#include "merge.h"
#include "printpro.h"

static int f_cd (char * p)
{
  UBUF *ub;
  
  ub = client_interact(CC_GET_PRO,0L, strlen(p), (unsigned const char *)p+1, 0,
		       (unsigned char *)NULLP);

  if(ub->cmd == CC_ERR) {
    fprintf(stderr, "ERR: %s\n",ub->buf);
    return(0);
  } else {
    util_junk_password(p);
    fprintf(stderr, "directory %s\nmode: ",p);
    print_pro(ub,stderr);
    return(1);
  }
}

int main (int argc, char ** argv)
{
  char *np;
  char **av, *av2[2];

  env_client();
  if(argc == 1) {
    np=util_abs_path("/");  
    f_cd(np);
    puts(np);
  } else {
    if(!(av = bsdglob(argv[1]))) {
      av = av2;
      av2[0] = *argv;
      av2[1] = 0;
    }
    np = util_abs_path(*av);
    if(f_cd(np))
    {
	puts(np);
    }
    else {
      puts(env_dir);
    }
  }
  client_done();
  exit(EX_OK);
}
