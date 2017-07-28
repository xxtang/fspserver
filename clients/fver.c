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
#include "merge.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int main (int argc, char ** argv)
{
  UBUF *ub;
  unsigned int len, tput = 0, len2;
  char *v1, *v2;

  if(argc == 1)	{ /* no arg supplied, get version string of server */
    env_client();
    ub = client_interact(CC_VERSION,0L, 0, (unsigned char *)NULLP, 0,
			 (unsigned char *)NULLP);
    len = BB_READ2(ub->bb_len);

    len2 = BB_READ4(ub->bb_pos);
    /* is the above still causing problems under Solaris?  */

    v1 = ub->buf; v2 = ub->buf+len;
    printf("Remote FSP version: %s\n",v1);

    /* we have a new type of server */
    if(len2) {
      if(*v2 & VER_LOG)
	printf("\tLogging of all server transactions is ENABLED.\n");
      else
	printf("\tLogging of all server transactions is DISABLED.\n");
      if(*v2 & VER_READONLY)
	printf("\tRemote server is run in READONLY mode.\n");
      else
	printf("\tRemote server is run in READ/WRITE mode.\n");
      if(*v2 & VER_REVNAME)
	printf("\tServer REQUIRES connections to reverse name.\n");
      else
	printf("\tServer DOESN'T REQUIRE connections to reverse.\n");
      if(*v2 & VER_PRIVMODE)
	printf("\tRemote server is run in PRIVATE mode.\n");
      else
	printf("\tRemote server is run in PUBLIC mode.\n");
      if(*v2 & VER_THRUPUT) {
	printf("\tRemote server throughput control is ENABLED.");
	tput |= (((unsigned)*(++v2) << 24) & 0xff000000);
	tput |= (((unsigned)*(++v2) << 16) & 0x00ff0000);
	tput |= (((unsigned)*(++v2) << 8) & 0x0000ff00);
	tput |= ((unsigned)*(++v2) & 0x000000ff);
	printf(" (max %d bytes/sec)\n", tput);
      } else
	printf("\tRemote server throughput control is DISABLED.\n");
      if (*v2 & VER_XTRADATA)
        printf("\tServer can process extra data in input packets.\n");

      /* check for optional max. packet size block */
      if(++v2 < ub->buf+len+len2)
      {
	  tput=0;
          tput = BB_READ2(v2);
	  if (tput > 1024)
	       printf("\tMax. payload size supported by server is %d bytes.\n",tput);
	  else
	       printf("\tPayload size prefered by server is %d bytes.\n",tput);
      }
    }
    else
	printf("\tRemote server do not send extended info.\n");
    client_done();
  } else
  {
    printf("Local FSP version: %s\n\n",PACKAGE_VERSION);
    printf("Max. packet size supported by client: %d\n",UBUF_MAXSPACE);
    printf("System startup file: %s\n",FSPRC);
    printf("Local startup file: %s\n",FSPPROF);
    printf("Locking method is: ");
#if defined(FSP_USE_SHAREMEM_AND_LOCKF)
    printf("SHAREMEM_AND_LOCKF");
#elif defined(FSP_USE_FLOCK)
    printf("FLOCK");
#elif defined(FSP_USE_LOCKF)
    printf("LOCKF");
#elif defined(FSP_NOLOCKING)
    printf("None");
#elif defined(FSP_USE_SHAREMEM_AND_SEMOP)
    printf("SHAREMEM_AND_SEMOP");
#else
#error "We do not have any locking method defined!"
#endif
    printf("\n");
#ifndef FSP_NOLOCKING
    printf("Lock prefix is: %s\n",FSP_KEY_PREFIX);
#endif
    printf("Timestamping supported: ");
#ifdef HAVE_UTIME_H
    printf("yes\n");
#else
    printf("no\n");
#endif
    printf("Large file support: ");
#ifdef NATIVE_LARGEFILES
    printf("Yes, native\n");
#else
    printf("No\n");
#endif
    printf("Maximum file size: ");
#ifdef NATIVE_LARGEFILES
    printf("4 GB (FSP limit)\n");
#else
    printf("2 GB (No large file support)\n");
#endif
    printf("Host lookup: ");
#ifdef HOST_LOOKUP
    printf("yes\n");
#else
    printf("no\n");
#endif
    printf("Timeouts enabled: ");
#ifdef CLIENT_TIMEOUT
    printf("yes\n");
#else
    printf("no\n");
#endif
  }
  exit(EX_OK);
}
