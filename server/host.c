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
#include "server_def.h"
#include "s_extern.h"
#include <ctype.h>
#include <netdb.h>
#include <stdlib.h>
#include "my-string.h"

/****************************************************************************
* This file contains routines to maintain client database.
****************************************************************************/

static HTAB     *htab;		/* client data base.			*/
static unsigned  hcnt;		/* number of clients.			*/
static unsigned  htot = 0;	/* available entries in the data base.	*/
static HTAB     hzero;

#define HALLOC_SIZE 30

/****************************************************************************
 * Look up the hostname for an inet number.  Return NULL if not found.
 ****************************************************************************/

static char *find_hostname (unsigned long inet_num)
{
  struct hostent *he;
  char *hostname;

  if ((he = gethostbyaddr((char*)&inet_num, sizeof(inet_num), AF_INET))) {
    hostname = malloc(strlen(he->h_name)+1);
    strcpy(hostname, he->h_name);
  } else
    hostname = 0;

  return hostname;
}

/****************************************************************************
 * Returns an entry from the database corresponding to to the inet number.
 * A new entry is created is it is not found.
 * The database is a linear array of sorted structures.
 * Entries are searched using binary search on the array.
 ****************************************************************************/

HTAB *find_host (unsigned long inet_num)
{
  unsigned l, h, m, i;
  unsigned long inum;
  HTAB *hs, *hd;

  for(l = 0, h = hcnt-1; (m = (l + h) >> 1) != l; ) {	/* binary search */
    inum = htab[m].inet_num;
    if(inum > inet_num) h = m;
    else if(inum < inet_num) l = m;
    else {
      /* if we *need* reverse naming and we haven't already got one
	 then try looking it up again. */
      if (no_unnamed && !htab[m].hostname)
	htab[m].hostname = find_hostname(inum);
      htab[m].acc_cnt++;
      return(htab+m);
    }
  }

  if(htab[m].inet_num < inet_num) m++;  /* locate first entry that is > */

  if((hcnt+1) > htot) { /* need more space */
    htot += HALLOC_SIZE;		/* add HALLOC_SIZE entries at a time */

    if(!(htab = (HTAB *) realloc(htab,sizeof(HTAB)*htot))) {
      perror("grow_htab realloc");
      exit(5);
    }
  }

  for(i = hcnt-m, hs = htab+hcnt, hd=htab+hcnt+1; i--; *--hd = *--hs);

  htab[m]=hzero;
  htab[m].inet_num = inet_num;
  htab[m].hostname = find_hostname(inet_num);
  hcnt++;
  return(htab+m);
}

/****************************************************************************
 * Client database initialization routine.
 ****************************************************************************/

int init_htab (void) /* always have 2 entries -- 0, MAXINT */
{
  if(!(htab = (HTAB *) malloc(sizeof(HTAB)*HALLOC_SIZE))) {
    perror("grow_htab malloc");
    exit(5);
  }
  htab[0] = hzero;
  htab[1] = hzero;
  htab[1].inet_num = ~0U;
  hcnt = 2;
  htot = HALLOC_SIZE;
  return(0);
}

/****************************************************************************
 * Write out the client table in the .HTAB_DUMP file.
 ****************************************************************************/

int dump_htab (FILE *fp)
{
  int i;
  HTAB *hp;

  if( fp == NULL)
  {
      if(dbug)
	  fp=stdout;
      else
	  return(0);
  }

  fprintf(fp,"#FSP Server "PACKAGE_VERSION", dumping at %s\nHost table content:\n",ctime(&cur_time));

  fprintf(fp,"#IP address\tcount  last access date\n");
  for(i = hcnt-2, hp = htab+1; i--; hp++) {
    fprintf(fp,"%d.%d.%d.%d\t%5d  %s", ((unsigned char *)(&hp->inet_num))[0],
	    ((unsigned char *)(&hp->inet_num))[1],
	    ((unsigned char *)(&hp->inet_num))[2],
	    ((unsigned char *)(&hp->inet_num))[3],
	    hp->acc_cnt,
	    ctime((time_t *) &(hp->last_acc)));
  }
  fprintf(fp,"\n#END\n");
  return(0);
}
