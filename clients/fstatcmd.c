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
#include "merge.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my-string.h"

static void stat_file (const char *fname)
{
   struct stat sb;
   struct tm *ftime;
   char buf[35];

   if(util_stat(fname,&sb))
   {
       printf("%s: stat error\n",fname);
   } else
   {
       if(S_ISREG(sb.st_mode))
	    printf("File");
       else
	  if(S_ISDIR(sb.st_mode))
	    printf("Directory");

       ftime=localtime(&sb.st_mtime);
       strftime(buf,35,"%Y-%m-%d %H:%M:%S",ftime);
#ifdef NATIVE_LARGEFILES
#define SFORM "%llu"
#else
#define SFORM "%lu"
#endif
       printf(": %s  Size: "SFORM" Time: %s\n",fname,sb.st_size,buf);
   }
	

}

int main (int argc, char ** argv)
{
  char n[1024];
  int prompt;
  char **av, *av2[2];

  env_client();

  if(argc>1)
  {
    for( optind=1; argc>optind ; optind++)
    {
      if(!(av = bsdglob(argv[optind])))
      {
        av = av2;
        av2[0] = argv[optind];
        av2[1] = 0;
      }
      while(*av)
	  stat_file(*av++);
    }
  } else {
    prompt = isatty(0);
    while(1) {
      if(prompt) {
	fputs("fstat: ",stdout);
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
	  stat_file(*av++);
    }
  }

  client_done();

  exit(EX_OK);
}
