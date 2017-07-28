/*
This is an FSP port scanner that I wrote in late '92.  It's not very fast,
but that was a limit of the original fver code (and of current versions
of fver).  Just type "fspscan -h" to get help on how it's used.
My appologies for the nasty code...I just wanted something that worked.

 -Cuda
*/
/*
I have ported it in September 2003 to modern C and added to FSP software
suite. Don't blame me for the code, i have not touched it.

 -Radim
*/
/*********************************************************************\
*  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
*                                                                     *
*  You may copy or modify this file in any manner you wish, provided  *
*  that this notice is always included, and that you hold the author  *
*  harmless for any loss or damage resulting from the installation or *
*  use of this software.                                              *
\*********************************************************************/


/****************************************************************
 * FindPort -- hack of fver to find valid ports on a FSP server *
 * by Cuda.  Yes, it's ugly, but it works for me.               *
 * Type "fspscan -h" to see how it is used.                     *
 ****************************************************************/

#include "fspscan.h"
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *host = NULL;
char *outputfile = NULL;
/* Default values */
int  localport = 9191;
int  remoteport = 21;
int  endport = 65535;
int  retries = 3;
int  client_trace = 0;

static int myfd;
static struct sockaddr_in server_addr;
static unsigned short myseq = 0;
static unsigned short key;
int client_intr_state = 0;

static struct sockaddr_in INET_ZERO;

#define DSIZE (sizeof(int)*8)
#define SAVE(A) { int sav; sav = errno; A; errno = sav; }

static void printhelp(int argc,char **argv)
{
    fprintf(stderr,"%s - an fsp port scanner\n",argv[0]);
    fprintf(stderr,"Usage: %s [-h]\n",argv[0]);
    fprintf(stderr,"       (prints this help screen)\n");
    fprintf(stderr,"       %s [-t][-o file][-r retries][-l local port][-i ip address][-p starting port][-e ending port]\n\n",argv[0]);
    fprintf(stderr,"Defaults: (-o) file = stdout\n");
    fprintf(stderr,"          (-l) local port = 9191\n");
    fprintf(stderr,"          (-p) starting port = 21\n");
    fprintf(stderr,"          (-e) ending port = 65535\n");
    fprintf(stderr,"          (-r) retries = 3\n");
    fprintf(stderr,"          (-t) trace = off\n");
    fprintf(stderr,"          (-i) host-ip = none (host MUST be specified)\n");
    exit(0);
}

static void fdclose(void)
{
    close(myfd);
}

static void client_done(void)
{
    (void) client_interact(CC_BYE,0L,0,NULLP,0,NULLP);
}

int main(int argc,char **argv)
{
    int  i;
    UBUF *ub;
    FILE *logfile;

    while ((i = getopt(argc, argv, "htp:o:r:l:i:e:")) != EOF) {
	switch(i) {
	case 'h':
	    printhelp(argc, argv);
	    break;
        case 't':
	    client_trace=1;
	    break;
        case 'r':
	    retries=atoi(optarg);
	    break;
	case 'p':
	    remoteport=atoi(optarg);
	    break;
	case 'e':
	    endport=atoi(optarg);
	    break;
	case 'l':
	    localport=atoi(optarg);
	    break;
	case 'i':
	    host = (char *) malloc(strlen(optarg)+1);
	    strcpy(host, optarg);
	    break;
        case 'o':
	    outputfile = (char *) malloc(strlen(optarg)+1);
	    strcpy(outputfile, optarg);
	    break;
	case '?':
	    printhelp(argc, argv);
	    break;
	default:
	    fprintf(stderr,"This error should not happen\n");
	}
    }

    if (host == NULL) {
        fprintf(stderr, "host/ip not specified, unable to continue\n");
	printhelp(argc, argv);
	exit(1);
    }

    if (outputfile == NULL) {
	logfile=stdout;
    } else {
        logfile=fopen(outputfile,"a+");
    }

    fprintf(logfile,"Scanning %s\n",host);

    for ( ; remoteport<=endport; ++remoteport) {
        init_client(host,remoteport,localport);
        signal(SIGINT,client_intr);

        ub = client_interact(CC_VERSION,0L, 0,NULLP, 0,NULLP);
	if (ub)
	    /* success! */
            fprintf(logfile,"Found FSP ver: %s on port %0d\n",
			    ub->buf,remoteport);
        else
	    fprintf(logfile,"%0d...nada\n",remoteport);
        fflush(logfile);
        fdclose();
    }
    return 0;
}


UBUF *client_interact(cmd,pos,l1,p1,l2,p2)
    unsigned cmd, l1, l2;
    unsigned long pos;
    unsigned char *p1, *p2;
{
    struct sockaddr_in from;
    UBUF sbuf;
    static UBUF rbuf;
    unsigned char *s, *t, *d;
    unsigned u, n, sum, mlen;
    fd_set mask;
    int retval, retry;
    socklen_t bytes;

    sbuf.cmd = cmd;
    sbuf.len = htons(l1);
    sbuf.pos = htonl(pos);

    client_intr_state = 1;
    FD_ZERO(&mask);

    for(u = l1, d = (unsigned char *) sbuf.buf; u--; *d++ = *p1++);
    for(u = l2				      ; u--; *d++ = *p2++);
    mlen = d - (unsigned char *) &sbuf;

    for(retry = 0; retry < retries; retry++) {
	sbuf.key = key;
	sbuf.seq = myseq;
	sbuf.sum = 0;

	for(t = (unsigned char *) &sbuf, sum = n = mlen; n--; sum += *t++);
	sbuf.sum = sum + (sum >> 8);
	if(client_trace && retry)
	    write(2,"R",1);

	if(sendto(myfd,&sbuf,mlen,0,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1)
						{ perror("sendto"); exit(1); }
	FD_SET(myfd,&mask);

	while(1) {
	    retval = _x_select(&mask, 3000L);

	    if((retval == -1) && (errno == EINTR)) continue;

	    if(retval == 1) {  /* an incoming message is waiting */
		bytes = sizeof(from);
		if((bytes = recvfrom(myfd,(char*)&rbuf,sizeof(rbuf),0,
	           (struct sockaddr*)&from,&bytes)) < UBUF_HSIZE) continue;

		s = (unsigned char *) &rbuf;
		d = s + bytes;
		u = rbuf.sum; rbuf.sum = 0;
		for(t = s, sum = 0; t < d; sum += *t++);
		sum = (sum + (sum >> 8)) & 0xff;
		if(sum != u) continue;  /* wrong check sum */

		rbuf.len = htons(rbuf.len);
		rbuf.pos = htonl(rbuf.pos);

		if(rbuf.seq 	      != myseq) continue;  /* wrong seq # */
		if(rbuf.len+UBUF_HSIZE > bytes) continue;  /* truncated.  */

		myseq++; key = rbuf.key;	/* update seq and keys.   */

		if(client_intr_state == 2) { client_done(); exit(1); }

		return(&rbuf);

	    } else break;   /* go back to re-transmit buffer again */
	}
    }
    /* return NULL if there have been too many retries - ffindport hack */
    return(NULL);
}


void init_client(char *myhost,int port,int myport)
{
    if((myfd = _x_udp(&myport)) == -1)
		{ perror("socket open"); exit(1); }

    if(_x_adr(myhost,port,&server_addr) == -1)
		{ perror("server addr"); exit(1); }
}


void client_intr(int sig)
{
    switch(client_intr_state) {
        case 0: exit(2);
        case 1: client_intr_state = 2; break;
        case 2: exit(3);
    }
}


int _x_udp(int *port)
{
    int f, zz;
    struct sockaddr_in me ;
    struct sockaddr_in sin;
    socklen_t len;

    me = sin = INET_ZERO;

    me.sin_port = htons((unsigned short) *port);
    me.sin_family = AF_INET;

    if((f=socket(AF_INET,SOCK_DGRAM,0)) == -1) return(-1);

    if( setsockopt(f,SOL_SOCKET,SO_REUSEADDR,&zz,sizeof(zz)) < 0 ||
        bind(f,(struct sockaddr *) &me,(len = sizeof(me))) < 0 ||
        getsockname(f,(struct sockaddr*)&sin,&len) < 0)
                                { SAVE(((void) close(f))); return(-1); }
    if(!*port) *port = ntohs((unsigned short) sin.sin_port); return(f);
}

int _x_adr(char *xhost,int port,struct sockaddr_in *his)
{
    char myhost[128];
    struct hostent *H;
    int    i;
    char *s, *d;

    *his = INET_ZERO;
    if(!xhost) (void) gethostname(xhost = myhost,sizeof(myhost));

    if((his->sin_addr.s_addr = inet_addr(xhost)) != INADDR_NONE) {
	his->sin_family = AF_INET;
    } else
    if((H = gethostbyname(xhost))) {
	for(s = (char *)H->h_addr,
	    d = (char *)&his->sin_addr,
	    i = H->h_length; i--; *d++ = *s++)
	      ;
        his->sin_family = H->h_addrtype;
    } else return(-1);
    his->sin_port = htons((unsigned short) port);

    return(0);
}

int _x_select(fd_set *rf, long tt)       /* tt is in unit of ms */
{
    struct timeval timeout;

    if(tt != -1)
    {
        timeout.tv_sec  =  tt / 1000;
        timeout.tv_usec = (tt % 1000)*1000;
        return(select(DSIZE, rf, NULL, NULL, &timeout));
    }

    return(select(DSIZE, rf, NULL, NULL, (struct timeval *) 0));
}
