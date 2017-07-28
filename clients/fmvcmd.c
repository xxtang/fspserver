    /*********************************************************************\
    *  Copyright (c) 2004 by Radim Kolar (hsn@sendmail.cz)                *
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
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my-string.h"
#include "merge.h"

static unsigned int n;
static unsigned char buf[UBUF_SPACE];

static int append_to_buf(const char *what,size_t len)
{
    if(n+len  > UBUF_SPACE )
	return -1;
    memcpy(buf+n,what,len);	
    n+=len;
    return 0;
}    
        
static void rename_file (const char *fname,const char *target)
{
   char *fpath;
   UBUF *reply;
   unsigned int srclen,dstlen;
   
   /* reset buffer */
   n=0;
   /* append source file */
   fpath = util_abs_path(fname);
   srclen=strlen(fpath)+1;
   if(append_to_buf(fpath,srclen))
   {
       printf("path too long: %s.\n",fpath);
       free(fpath);
       return;
   }
   free(fpath);
   /* add dest */
   fpath=util_abs_path(target);
   dstlen=strlen(fpath)+1;
   if(append_to_buf(fpath,dstlen))
   {
       printf("path too long: %s.\n",fpath);
       free(fpath);
       return;
   }

   /* send our nicely crafted junk to the server */
   reply=client_interact (CC_RENAME,dstlen,srclen,buf,dstlen,buf+srclen);
  
   if(reply->cmd==CC_ERR) 
   {
       /* seems like we have really bad luck today */
       printf("mv %s %s: %s\n",fname,target,reply->buf);
   } 
}

int main (int argc, char ** argv)
{
  char **av, *av2[2];

  env_client();

  if(argc>1)
  {
    for( optind=1; argc-1>optind ; optind++)
    {
      if(!(av = bsdglob(argv[optind])))
      {
        av = av2;
        av2[0] = argv[optind];
        av2[1] = 0;
      }
      while(*av)
	  rename_file(*av++,argv[argc-1]);
    }
  }
  else
    {
      fprintf(stderr,"%s source target\n", argv[0]);
      exit(EX_USAGE);
    }

  client_done();

  exit(EX_OK);
}
