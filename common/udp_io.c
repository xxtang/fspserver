    /*********************************************************************\
    *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
    *                                                                     *
    *  You may copy or modify this file in any manner you wish, provided  *
    *  that this notice is always included, and that you hold the author  *
    *  harmless for any loss or damage resulting from the installation or *
    *  use of this software.                                              *
    \*********************************************************************/

#include "tweak.h"
#include "common_def.h"
#include "co_extern.h"
#include "my-string.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#define DSIZE (sizeof(int)*8)
#define SAVE_ERRNO(A) { int sav; sav = errno; A; errno = sav; }

#ifndef EXOS_IPC
#include <netdb.h>
#include <netinet/in.h>

int _x_udp (const char * bindaddress, unsigned short * port)
{
  int f;
  socklen_t len;
  struct sockaddr_in me ;
  struct sockaddr_in myadr;
#ifdef REUSE_SOCKET  
  int zz=1;
#endif

  if(bindaddress == NULL || _x_adr (bindaddress, *port,&me ) != 0) {
     memset(&me,0,sizeof(me));
     me.sin_port = htons(*port);
     me.sin_family = AF_INET;
  };   

  len=sizeof(me);
  f=socket(AF_INET,SOCK_DGRAM,0);
  
  if(f == -1) return(-1);

  if(
#ifdef REUSE_SOCKET      
      setsockopt(f,SOL_SOCKET,SO_REUSEADDR,(char *)&zz,sizeof(zz)) < 0 ||
#endif      
     bind(f,(struct sockaddr *) &me,len) < 0 ||
     getsockname(f,(struct sockaddr *)&myadr,&len) < 0) {
    SAVE_ERRNO(((void) close(f)));
    return(-1);
  }
  if(!*port) *port = ntohs((unsigned short) myadr.sin_port);
  return(f);
}

int _x_adr (const char *host, int port, struct sockaddr_in * his)
{
  char myhost[128];
  struct hostent *H;
  int    i;
  char *s, *d;

  memset(his,0,sizeof(struct sockaddr_in));
  if(!host) {
	  gethostname(myhost,sizeof(myhost));
          host=myhost;
  }

  if(port <= 0)
      port=21;

  /* if((his->sin_addr.s_addr = inet_addr(host)) != -1) */
  if(inet_aton(host,&his->sin_addr)) {
    his->sin_family = AF_INET;
  } 
  else
    if( (H = gethostbyname(host))) {
      for(s = (char *)H->h_addr, d = (char *)&his->sin_addr, i = H->h_length;
	  i--; *d++ = *s++);
      his->sin_family = H->h_addrtype;
    } else return(-1);
  his->sin_port = htons((unsigned short) port);

  return(0);
}

int _x_select (fd_set * rf, long tt)  /* tt is in unit of ms */
{
  struct timeval timeout;

  if(tt != -1) {
    if(tt < MIN_DELAY) tt = MIN_DELAY;
    timeout.tv_sec  =  tt / 1000;
    timeout.tv_usec = (tt % 1000)*1000;
    return(select(DSIZE, rf, NULL, NULL, &timeout));
  }

  return(select(DSIZE, rf, NULL, NULL , (struct timeval *) 0));
}
#endif  /* not EXOS_IPC */

#ifdef EXOS_IPC

extern long rhost();

int _x_udp PROTO1(int *, port)
{
  struct sockaddr_in sin; int f;

  sin = INET_ZERO;
  sin.sin_family = AF_INET;
  sin.sin_port   = htons((unsigned short) *port);
  if((f = socket(SOCK_DGRAM, (struct sockproto *) 0, &sin,
		 SO_REUSEADDR)) == -1)
    return(-1);
  sin = INET_ZERO;
  if(socketaddr(f,&sin) == -1) {
    SAVE(((void) close(f)));
    return(-1);
  }
  if(!*port) *port = ntohs((unsigned short) sin.sin_port);
  return(f);
}

int _x_adr PROTO3(char *, host, int, port, struct sockaddr_in *, his)
{
  char myhost[128];
  int f;

  *his = INET_ZERO;
  if(!host) (void) gethostname(host = myhost,sizeof(myhost));

  his->sin_family = AF_INET;
  his->sin_port = htons((unsigned short) port);

  if((his->sin_addr.s_addr = rhost(&host)) == -1) return(-1);

  return(0);
}

int _x_select PROTO2(unsigned int *, readfds, long, tt)
{
  int  code;
  long mask = *readfds;

  if(tt & 0xc0000000) tt = 0x3fffffff;/* It does not like 0x7fffffff. */

  code = select(DSIZE, &mask, (long *) 0, tt);

  *readfds = mask;

  return(code);
}

int recvfrom PROTO6(int, s, char *, msg, int, len, int, flags,
		    struct sockaddr_in *, from, int *, fromlen)
{
  return(receive(s,from,msg,len));
}

int sendto PROTO6(int, s, char *, msg, int, len, int, flags,
		  struct sockaddr_in *, to, int *, tolen)
{
  to->sin_family = AF_INET;
  return(send(s,to,msg,len));
}

#endif /* EXOS_IPC */
