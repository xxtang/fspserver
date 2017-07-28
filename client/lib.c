    /*********************************************************************\
    *  Copyright (c) 2004-2005 by Radim Kolar                                  *
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
#include "co_extern.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/time.h>


static int myfd;
static struct sockaddr_in server_addr;
static unsigned short myseq = 0;
static unsigned short key;

int client_trace = 0;
int client_intr_state = 0;
unsigned long target_delay = DEFAULT_DELAY;	/* expected max delay from server on good connection */

unsigned long target_maxdelay = DEFAULT_MAXDELAY; /* max resend timer */
unsigned long busy_delay   = DEFAULT_DELAY;	/* busy retransmit timer */
unsigned long idle_delay   = DEFAULT_DELAY;	/* idle retransmit timer */
unsigned long udp_sent_time;
unsigned long stat_resends, stat_iresends, stat_dupes, stat_bad, stat_ok;

UBUF *client_interact 	(unsigned char cmd, unsigned long pos,
			     unsigned int l1, unsigned const char * p1,
			     unsigned int l2, unsigned const char * p2)
{
  struct sockaddr_in from;
  UBUF sbuf;
  static UBUF rbuf;
  unsigned char *s, *t, *d, seq0, seq1;
  unsigned u, n, sum, mlen, rlen;
  fd_set mask;
  int retval, retry_send, retry_recv;
  socklen_t bytes;
  unsigned long w_delay;
  unsigned long total_delay;
  struct timeval start[8],stop;

  FD_ZERO(&mask);
  sbuf.cmd = cmd;

#ifdef DEBUG
  printf("sbuf.cmd = %u\n",sbuf.cmd);
#endif

  BB_WRITE2(sbuf.bb_len,l1);
  BB_WRITE4(sbuf.bb_pos,pos);

  client_intr_state = 1;
  total_delay = 0;
  w_delay = 0;

  for(u = l1, d = (unsigned char *) sbuf.buf; u--; *d++ = *p1++);
  for(u = l2; u--; *d++ = *p2++);
  mlen = d - (unsigned char *) &sbuf;

  key = client_get_key();
  u = random() & 0xfff8;
  if ( u == myseq )
  {
      myseq ^= 0x1080;
  }
  else myseq = u;

  for(retry_send = 0; ; retry_send++) {
    total_delay += w_delay;
    BB_WRITE2(sbuf.bb_key,key);
    sbuf.bb_seq[0] = seq0 = (myseq >> 8) & 0x00ff;
    sbuf.bb_seq[1] = seq1 = (myseq & 0x00f8) | (retry_send & 0x0007);
    sbuf.sum = 0;

    for(t = (unsigned char *) &sbuf, sum = n = mlen; n--; sum += *t++);
    sbuf.sum = sum + (sum >> 8);

    switch(retry_send) {	/* adaptive retry delay adjustments */
      case  0:
        if(target_delay>=busy_delay)
	    w_delay=target_delay;
        else
            w_delay=busy_delay+DEFAULT_DELAY;
	/* classic FSP retry code. Not suitable for CZFREE.NET
        busy_delay = (target_delay+ (busy_delay<<3)-busy_delay)>>3;
	w_delay = busy_delay;
	*/
	break;
      case  1:
	busy_delay = busy_delay * 3 / 2;
	w_delay = busy_delay;
	idle_delay = busy_delay;
	if(client_trace) write(2,"R",1);
	stat_resends++;
	break;

      default:
#ifdef CLIENT_TIMEOUT
	if (total_delay/1000 >= env_timeout ) {
	  fprintf(stderr, "\rRemote server not responding.\n");
	  exit(EX_UNAVAILABLE);
	}
#endif
	idle_delay = idle_delay * 4 / 3;
	if (idle_delay > target_maxdelay) idle_delay = target_maxdelay;
	w_delay = idle_delay;
	if(client_trace) write(2,"I",1);
	stat_iresends++;
	break;
    }

    if(sendto(myfd,(const char*)&sbuf,mlen,0,(struct sockaddr *)&server_addr,
	      sizeof(server_addr)) == -1) {
      switch(errno) {
	  case ENOBUFS:
	  case EHOSTUNREACH:
	  case ECONNREFUSED:
	  case EHOSTDOWN:
	  case ENETDOWN:
	  case EPIPE:
	       /* try to resend packet */
	       continue;
	  default:
               perror("sendto");
               exit(EX_IOERR);
      }
    }
    /* Check if w_delay is within limits */
    if(w_delay < MIN_DELAY)
	w_delay=MIN_DELAY;
    else
	if (w_delay > target_maxdelay)
	    w_delay = target_maxdelay;

#ifdef DEBUG
    printf("Waiting %lu ms for server response.\n",w_delay);
#endif
    udp_sent_time = time((time_t *) 0);
    gettimeofday(&start[retry_send & 0x7],NULL);
    FD_SET(myfd,&mask);

    for(retry_recv = 0; ; retry_recv++) {
      retval = _x_select(&mask, w_delay);
      if((retval == -1) && (errno == EINTR)) continue;
      if(retval == 1) { /* an incoming message is waiting */
	bytes = sizeof(from);
	if((bytes = recvfrom(myfd,(char*)&rbuf,sizeof(rbuf),0,
			     (struct sockaddr *)&from, &bytes)) < UBUF_HSIZE)
	{
	  /* too enough bytes for header */
          if (client_trace) write(2,"H",1);
	  stat_bad++;
	  continue;
	}

	rlen = BB_READ2(rbuf.bb_len);

	if( (rlen+UBUF_HSIZE)  > bytes)
	{
	    /* truncated. */
            if (client_trace) write(2,"T",1);
	    stat_bad++;
	    continue;
	}

	s = (unsigned char *) &rbuf;
	d = s + bytes;
	u = rbuf.sum; rbuf.sum = 0;
	for(t = s, sum = 0; t < d; sum += *t++);
	sum = (sum + (sum >> 8)) & 0xff;
	if(sum != u)
	{   
	    /* wrong check sum */
            if (client_trace) write(2,"C",1);
	    stat_bad++;
	    continue;
	}
        /* check seq. number */
	if( (rbuf.bb_seq[0] ^ seq0) ||
	   ((rbuf.bb_seq[1] ^ seq1)&0xf8)) 
	{
	    /* wrong seq # */
            if (client_trace) write(2,"S",1);
	    stat_dupes++;
	    continue;
	}
	/* check command */
        if (cmd != rbuf.cmd && rbuf.cmd != CC_ERR)
	{
            if (client_trace) write(2,"C",1);
	    stat_bad++;
	    continue;
	}
	/* check pos */
	if (BB_READ4(rbuf.bb_pos) != pos && (cmd == CC_GET_DIR ||
	    cmd == CC_GET_FILE || cmd == CC_UP_LOAD || cmd == CC_INFO ||
	    cmd == CC_GRAB_FILE))
	{
	    /* wrong pos */
            if (client_trace) write(2,"P",1);
	    stat_bad++;
	    continue;
	}
	key = BB_READ2(rbuf.bb_key); /* key for next request */
	/* calculate real busy delay */
        gettimeofday(&stop,NULL);
	busy_delay = 1000*(stop.tv_sec-start[rbuf.bb_seq[1] & 0x7].tv_sec);
	busy_delay += (stop.tv_usec-start[rbuf.bb_seq[1] & 0x7].tv_usec)/1000;
#ifdef DEBUG
    printf("Server reply RTT was %lu ms.\n",busy_delay);
#endif
	client_set_key(key);
	stat_ok++;

	if(client_intr_state == 2) {
	  if(!key_persists) client_done();
	  exit(EX_TEMPFAIL);
	}

#ifdef DEBUG
  printf("rbuf.cmd = %u\n",rbuf.cmd);
#endif

	return(&rbuf);

      } else break;   /* go back to re-transmit buffer again */
    }
  }
}

static RETSIGTYPE client_intr (int signum)
{
  switch(client_intr_state) {
    case 0: exit(EX_TEMPFAIL);
    case 1: client_intr_state = 2; break;
    case 2: exit(EX_TEMPFAIL);
  }
#ifndef RELIABLE_SIGNALS
  signal(SIGINT,client_intr);
#endif
}

void init_client (const char * host, unsigned short port, unsigned short myport)
{
  busy_delay = idle_delay = target_delay;
  stat_resends = stat_iresends = stat_dupes = stat_bad = stat_ok = 0;
#ifdef HAVE_SRANDOMDEV
  srandomdev();
#else
  srandom(getpid()*time(NULL));
#endif
  myseq = random() & 0xfff8;

  if((myfd = _x_udp(env_listen_on,&myport)) == -1) {
    perror("socket open");
    exit(EX_OSERR);
  }

  if(_x_adr(host,port,&server_addr) == -1) {
    perror("server addr");
    exit(EX_OSERR);
  }

  client_init_key(server_addr.sin_addr.s_addr,port,getpid());
  signal(SIGINT,client_intr);
}

void client_finish(void)
{
  env_timeout=10;
  (void) client_interact(CC_BYE, 0L, 0, (unsigned char *)NULLP, 0,
			 (unsigned char *)NULLP);
  client_destroy_key();
}

int client_done (void)
{
  if(!key_persists) 
      client_finish();  
  return(0);
}
