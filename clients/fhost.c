    /*********************************************************************\
    *  Copyright (c) 1993 by Michael Meskes                               *
    *  (meskes@ulysses.informatik.rwth-aachen.de)                         *
    *  Copyright (c) 2003-2006 by Radim Kolar (hsn@cybermail.net)         *
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <assert.h>
#ifdef HOST_LOOKUP
#include <netdb.h>
#endif

#include "fhost.h"

static struct fsp_host *host;
static int hostsize=0;

/* allocate and init fsp_host structure */
struct fsp_host * init_host(void)
{
    struct fsp_host *h;

    h=malloc(sizeof(struct fsp_host));
    if(h==NULL)
    {
	perror("init_host");
	exit(EX_OSERR);
    }
	
    h->hostname=NULL;
    h->hostaddr=NULL;
    h->alias=calloc(1,sizeof(char *));
    h->port=-1;
    h->dir=NULL;
    h->delay=-1;
    h->local_port=-1;
    h->timeout=-1;
    h->trace=-1;
    h->local_dir=NULL;
    h->password=NULL;

    return h;
}

void add_host_alias(struct fsp_host *h, const char *name)
{
    int i=0;
    while(h->alias[i])
	i++;
    h->alias=realloc(h->alias,sizeof(char *)*(i+2));
    h->alias[i]=strdup(name);
    h->alias[i+1]=NULL;
}

void add_host(struct fsp_host *h)
{
    if (hostsize==0)
	host=NULL;
    if(h==NULL) return;
    if(h->port<=0) return;
    host=realloc(host,sizeof(struct  fsp_host)*(hostsize+1));
    if(host==NULL)
    {
	perror("host realloc");
	exit(EX_OSERR);
    }
    memcpy(host+hostsize,h,sizeof(struct fsp_host));
    hostsize++;
    return;
}

struct fsp_host *find_host(const char *name)
{
    int i,j;

    if(name==NULL || hostsize==0 ) return NULL;
    for(i=0;i<hostsize;i++)
    {
	if(host[i].hostname)
	    if(!strcmp(host[i].hostname,name)) return &host[i];
	if(host[i].hostaddr)
	    if(!strcmp(host[i].hostaddr,name)) return &host[i];
	j=0;
        while(host[i].alias[j])
	{
	    if(!strcmp(host[i].alias[j],name)) return &host[i];
	    j++;
	}
    }
    return NULL;
}

void list_prof_file (void) /* list resource file */
{
  int i;
  for(i=0;i<hostsize;i++)
  {
      printf("host: %s port: %d\n",(host[i].hostname?host[i].hostname  :  host[i].hostaddr),(host[i].port<=0? 21 : host[i].port));
  }

  return;
}

void list_sites_file (void) /* list resource file in fspsites format */
{
  int i,j;
  for(i=0;i<hostsize;i++)
  {
      printf("%-20s%6d %-20s",(host[i].hostname?host[i].hostname  :  host[i].hostaddr),(host[i].port<=0? 21 : host[i].port), host[i].dir != NULL? host[i].dir : "/");
      j=0;
      while(host[i].alias[j])
      {
	  printf("  %s",host[i].alias[j]);
	  j++;
      }
      printf("\n");
  }

  return;
}

/* lhost: type of FSP_HOST address NUMBER or NAME */ 
int print_host_setup(struct fsp_host *setup,int csh,int lhost)
{
  struct hostent *hp;
  long addr;
  register char *p;

  if (setup->hostname || setup->hostaddr) {
    if (csh) printf("setenv FSP_HOST ");
    else printf("FSP_HOST=");
    /* kill trailing junk from hostname and hostaddr */
    if(setup->hostname) {
      for(p = setup->hostname; *p && *p!='\n' && *p!= ' '; p++);
      *p = 0;
    }
    if(setup->hostaddr) {
      for(p=setup->hostaddr;*p && *p !='\n' && *p!=' ';p++);
      *p = 0;
    }
    if(lhost==NAME && !setup->hostname) {
      /* look for name */
#if HOST_LOOKUP
      addr=inet_addr(setup->hostaddr);
      if ( (hp=gethostbyaddr((char *) &addr, sizeof(addr), AF_INET)))
	setup->hostname= (char *)hp->h_name;
#endif
      if (!setup->hostname) lhost=NUMBER;
    }
    if ((lhost==NUMBER || lhost==0)  && !setup->hostaddr) {
      /* look for IP number */
#if HOST_LOOKUP
      if ( (hp=gethostbyname(setup->hostname)))
	setup->hostaddr=(char *)inet_ntoa(*(struct in_addr *) * hp->h_addr_list);
#endif
      if (!setup->hostaddr) lhost=NAME;
    }
    /* setup lhost to correct displaying type */
    if (lhost==0) 
    {
        if (setup->hostaddr) 
	    lhost=NUMBER;
        else 
	if(setup->hostname)
	    lhost=NAME;
	assert(lhost != 0 );    
    } else
    {
	if (lhost==NUMBER && !setup->hostaddr)
	    lhost=NAME;
	else
	if (lhost==NAME && !setup->hostname)
	    lhost=NUMBER;
    }
    printf("%s", (lhost==NAME)? setup->hostname : setup->hostaddr);
    if (csh) printf(";\n");
    else printf(";\nexport FSP_HOST;\n");
    if (!setup->dir) setup->dir="/"; /* if host is set we need this */
  }
  if (setup->delay>=0) {
    if (csh) printf("setenv FSP_DELAY %d;\n",setup->delay);
    else printf("FSP_DELAY=%d;\nexport FSP_DELAY;\n",setup->delay);
  }
  if (setup->local_port>=0) {
    if (csh) printf("setenv FSP_LOCALPORT %d;\n",setup->local_port);
    else printf("FSP_LOCALPORT=%d;\nexport FSP_LOCALPORT;\n",setup->local_port);
  }
  if (setup->trace>=0) {
    if (csh) {
      if (setup->trace) printf("setenv FSP_TRACE;\n");
      else printf("unsetenv FSP_TRACE;\n");
    } else {
      if (setup->trace) printf("FSP_TRACE=on;\nexport FSP_TRACE;\n");
      else printf("unset FSP_TRACE;\n");
    }
  }
  if (setup->timeout>=0) {
    if (csh) printf("setenv FSP_TIMEOUT %d;\n",setup->timeout);
    else printf("FSP_TIMEOUT=%d;\nexport FSP_TIMEOUT;\n",setup->timeout);
  }
  if (setup->port>=0) {
    if (csh) printf("setenv FSP_PORT %d;\n",setup->port);
    else printf("FSP_PORT=%d;\nexport FSP_PORT;\n",setup->port);
  }
  if (setup->local_dir) {
    if (csh) printf("setenv FSP_LOCAL_DIR ");
    else printf("FSP_LOCAL_DIR=");
    for (p=setup->local_dir;*p && *p!='\n' && *p!=' ';p++) printf("%c",*p);
    if (csh) printf(";\n");
    else printf(";\nexport FSP_LOCAL_DIR;\n");
  }
  if (setup->password) {
    if (csh) printf("setenv FSP_PASSWORD ");
    else printf("FSP_PASSWORD=");
    for (p=setup->password;*p && *p!='\n' && *p!=' ';p++) printf("%c",*p);
    if (csh) printf(";\n");
    else printf(";\nexport FSP_PASSWORD;\n");
  }
  if (setup->dir) {
    if (csh) printf("setenv FSP_DIR ");
    else printf("FSP_DIR=");
    for (p=setup->dir;*p && *p!='\n' && *p!=' ';p++) printf("%c",*p);
    if (csh) printf(";\n");
    else printf(";\nexport FSP_DIR;\n");
  }

  if (csh) printf("setenv FSP_NAME \"");
  else printf("FSP_NAME=\"");
  if (setup->hostname)
    for (p=setup->hostname;*p && *p!='\n' && *p!=' ';p++) printf("%c",*p);
  if (csh) printf("\";\n");
  else printf("\";\nexport FSP_NAME;\n");
  return (0);
}
