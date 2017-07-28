/*********************************************************************\
 *  Copyright (c) 2003-5 by Radim Kolar (hsn@cybermail.net)            *
 *  Copyright (c) 1991 by Wen-King Su (wen-king@vlsi.cs.caltech.edu)   *
 *                                                                     *
 *  You may copy or modify this file in any manner you wish, provided  *
 *  that this notice is always included, and that you hold the author  *
 *  harmless for any loss or damage resulting from the installation or *
 *  use of this software.                                              *
 \*********************************************************************/

#include "tweak.h"
#include "server_def.h"
#include "s_extern.h"
#include "co_extern.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "my-string.h"
#include "fifocache.h"

/*for p2pnat*/
#include "p2p_api.h"
#define DEFAULT_P2PNAT_PORT 7042
int server_port = DEFAULT_P2PNAT_PORT;
char server_address[24] = "54.223.244.5";
/*? end for p2pnat*/

time_t cur_time;
int shutdowning = 0;
static int dump = 0;
static int thc[THCCOUNT];
static time_t thcbase;
static int myfd;

/* add by xxtang 2017.4.24*/
extern double ptime_count;
extern time_t ptime_start;
extern time_t ptime_end;


/*Add by xxfan 2016-6-15
/*

int update_password(char* new_password)
{

	char* cache_cont_buf=NULL;
	int cache_flag=0;;
	char passwd_file_url[256];
	char read_buf[256];
	char *p,*q;

	long file_size;
	long  reserve_cont_length=0;
	long cache_cont_length;
	int i=0;
	int fd;
	int rc;


	//write password file
	sprintf(passwd_file_url,"%s/%s",home_dir,FSP_PASSWORD);

	fd=open(passwd_file_url,O_CREAT|O_TRUNC|O_WRONLY);
	if(fd<=0)
	{
		printf("open file-%s error\n",passwd_file_url);
		return -1;
	}

	rc = write(fd,new_password,strlen(new_password));
	printf("write file-%s ,cont-%s\n",passwd_file_url,new_password);
	if(rc<0) 
	{
		printf("write error -%s\n",passwd_file_url);
		close(fd);
		return -2;
	}
	close(fd);

	//update config file	
	fd=open(CONF_FILE,O_RDWR);
	if(fd<0)
	{
		printf("open error -%s\n",CONF_FILE);
		return -3;
	}

	//get file size
	file_size=lseek( fd, 0, SEEK_END );
	lseek( fd, 0, SEEK_SET );
	printf("file_size-%d\n",file_size);

	//update password value
	cache_flag=0;
	reserve_cont_length=0;
	cache_cont_length=0;	

	i=0;
	while((rc=read(fd,read_buf+i,1))>0)
	{
		//read one line of the file
		if(i<256 && *(read_buf+i)!='\n')
		{
			i++;
			continue;
		}

		*(read_buf+i+1)='\0';
		rc=i+1;//the length of one line
		i=0;
		//printf("read_buf=%s\n",read_buf);

		if(cache_flag==1)
		{
			strcpy(cache_cont_buf+cache_cont_length,read_buf);
			cache_cont_length+=rc;	
			continue;
		}

		reserve_cont_length+=rc;
		if(*read_buf=='#'||*read_buf=='\n') 
		{
			continue;
		}

		p = read_buf; 
		while(*p && isspace(*p)) p++;
		q = p;
		while(*q && !isspace(*q)) q++;
		*q='\0';

		if(strcasecmp(p,"password") == 0)
		{
			*q=' ';//recover ' ' from '\0'

			cache_cont_buf=malloc(file_size-reserve_cont_length+10);
			reserve_cont_length-=rc;
			bzero(cache_cont_buf,sizeof(cache_cont_buf));
			cache_flag =1;
		}
	}	

	//insert
	lseek( fd, reserve_cont_length, SEEK_SET );
	rc=sprintf(read_buf,"password %s\n",new_password);
	write(fd,read_buf,strlen(read_buf));
	//
	write(fd,cache_cont_buf,cache_cont_length);
	free(cache_cont_buf);

	file_size=reserve_cont_length+cache_cont_length+rc;
	printf("new file-%s size %d\n",CONF_FILE,file_size);
	ftruncate(fd,file_size);
	close(fd);
	return 0;
}
*/
static void server_process_packet (unsigned, UBUF *, int, HTAB *,
		struct sockaddr_in *);

/* log 1 argument */
#define ACTIONLOG0(FLAG,X) \
	do{ if((logging & (FLAG)) && !old) { \
		fsplogs(); \
		fsploga("%s %s", inetstr, (X)); \
	} } while (0)

/* LOG ARG, L1, S1 */
#define ACTIONLOG1(FLAG,X) \
	do { if((logging & (FLAG)) && !old) { \
		fsplogs(); \
		fsploga("%s %-8s /%.*s", inetstr, (X), l1, s1); \
	} } while (0)

/* LOG ARG, L1, S1 */
#define ACTIONLOG2(FLAG,X) \
	do { if((logging & (FLAG)) && !old) { \
		fsplogs(); \
		fsploga("%s %-8s /%.*s /%.*s", inetstr, (X), l1, s1, l2, s2); \
	} } while (0)

#define ACTIONINFO(FLAG,F) \
	do { if((logging & (FLAG)) && !old) { \
		fsploga F; \
	} } while (0)

#define ACTIONFAILED(FLAG,M) \
	do { if((logging & (FLAG)) && !old) { \
		fsploga(": ERROR %s\n", (M)); \
		fsplogf(); \
	} } while (0)

#define ACTIONOK(FLAG) \
	do { if((logging & (FLAG)) && !old) { \
		fsploga("\n"); \
		fsplogf(); \
	} } while (0)

#define CHECK_ACCESS_RIGHTS(RIGHT,LOGFLAG) \
	pe = require_access_rights(di,(RIGHT),inet_num,pp.passwd); \
switch(pe[0]) \
{ \
	case 'I': \
			  return; \
	case 'D':   \
				ACTIONFAILED((LOGFLAG)|L_ERR,pe+1); \
	send_error(from,ub,pe+1);  \
	return; \
} \

RETSIGTYPE server_interrupt (int signum)
{
	shutdowning = 1;
}

RETSIGTYPE server_dump (int signum)
{
	dump = 1;
#ifndef RELIABLE_SIGNALS
	signal(SIGUSR1,server_dump);
#endif
}

static const char * print_command(unsigned char cmd)
{
	switch(cmd)
	{
		case CC_BYE:
			return "BYE";
		case CC_VERSION:
			return "VER";
		case CC_INFO:
			return "INFO";    
		case CC_ERR:
			return "ERR";
		case CC_GET_DIR:
			return "GETDIR";
		case CC_GET_FILE:
			return "GETFILE";
		case CC_DEL_FILE:
			return "DELFILE";
		case CC_DEL_DIR:
			return "DELDIR";
		case CC_UP_LOAD:
			return "UPLOAD";
		case CC_INSTALL:
			return "INSTALL";
		case CC_MAKE_DIR:
			return "MKDIR";
		case CC_GET_PRO:
			return "GETPRO";
		case CC_SET_PRO:
			return "SETPRO";
		case CC_GRAB_FILE:
			return "GRAB";
		case CC_GRAB_DONE:
			return "GRABDONE";
		case CC_STAT:
			return "STAT";
		case CC_RENAME:
			return "RENAME";
		case CC_CH_PASSWD:
			return "CH_PASSWD";
		default:
			return "*UNKNOWN*";
	}
}

/****************************************************************************
 * Send an error string to client.
 ****************************************************************************/

static void send_error (struct sockaddr_in * from, UBUF * ub, const char * msg)
{
	size_t sz;

	sz=strlen(msg)+1;
	memcpy(ub->buf,msg,sz);
	ub->cmd = CC_ERR;
	BB_WRITE4(ub->bb_pos,0);

	server_reply(from,ub,sz,0);
}

/****************************************************************************
 *  This is the message filter.  It is called by main with a timeout value.
 *  If timeout is -1, it will never time out.  Otherwise, it waits for a
 *  message.  If timed out, it returns.  Otherwise it pass it through checks.
 *  Those message that passed get sent to the dispatch loop.
 ****************************************************************************/

int server_loop (int fd, time_t timeout)
{
	HTAB *hp;
	const char *ir;
	UBUF rbuf;
	struct sockaddr_in from;
	unsigned int u, sum, rlen, rkey;
	int retval, old;
	socklen_t bytes;
	unsigned char *s, *d, *t;
	fd_set mask;

	FD_ZERO(&mask);
	myfd=fd;

	/*for p2pnat by xxfan 16-03-08*/
	int rc;
	/*? end/

	/* init throughput control */
	if(maxthcallowed) {
		for(old = 0; old<THCCOUNT; thc[old++]=0);
		time(&thcbase);
	}

	while(1) {
		FD_SET(myfd,&mask);
		if(use_p2p)
		{
			rendezvous_status_handle();

			/*add by xxtang 2017.4.24*/
			if(ptime_count>=120) 			
			{ 				
				time(&ptime_start); 				
				time(&ptime_end); 				
				ptime_count = 120; 			
			} 			
			else 			
			{ 				
				time(&ptime_end); 				
				ptime_count = difftime(ptime_end,ptime_start); 			
			} 

		}
		if(dump) {
			FILE *fp;
			if(dbug) fprintf(stderr,"Got USR1, dumping tables...\n");
			if(dumpname)
				fp=fopen(dumpname,"a");
			else
				fp=NULL;
			dump_htab(fp);
			dump_iptab(iptab,fp);
			stat_caches(fp);
			if(fp)
			{
				fclose(fp);
			}
			dump = 0;
		} else
			if(shutdowning) return 1;

		retval = _x_select(&mask, DEFAULT_DELAY);
		/*retval = _x_select(&mask, timeout);*/

		if(retval == -1) {
			if(dbug) fprintf(stderr,"select error\n");

			if(errno == EINTR) continue;
			perror("select");
			exit(7);
		}

		if(retval == 1) {   /* an incoming message is waiting */

			bytes = sizeof(from);
			memset((char *)&rbuf, 0, 256);
			bytes = recvfrom(myfd,(char*)&rbuf,sizeof(rbuf),0,(struct sockaddr *)&from,&bytes); 

			if(use_p2p)
			{
			/*p2pnat add by xxfan 16-03-08
			// - by xxfan 2016-6-4
			// rc = decode_rendezvous_packet((char *)&rbuf,&from,&resp_packet);
			// +*/
			rc = handle_rendezvous_packet(myfd, (char*) &rbuf,&from);
			if(rc == 0)
			{
				/*- by xxfan 2016-6-4
				//rendezvous_message_handle(myfd,&resp_packet);*/
				continue;
			}
			/*?end p2pnat

			//add by xxtang 2017.4.24*/
			time(&ptime_start);				
			time(&ptime_end);				
			ptime_count = 0;
			
			}
			
			if(bytes < UBUF_HSIZE)
			{
				if(dbug) fprintf(stderr,"Header truncated.\n");
				continue;
			}

			rlen = BB_READ2(rbuf.bb_len);
			if((rlen+UBUF_HSIZE) > bytes)
			{
				if(dbug) fprintf(stderr,"Message truncated.\n");
				continue;	/* truncated.  */
			}

			if(!(ir = check_ip_table(from.sin_addr.s_addr,iptab)))
			{ /* host not found in table */
				ir = priv_mode ? "DFSP service not available": "N";
			}

			switch (*ir) {
				case 'D':	/* disabled host - return error message */
					if (rbuf.cmd == CC_BYE)
						break;
					send_error(&from,&rbuf,&ir[1]);
					continue;
				case 'I':	/* ignore the host */
					continue;
				case 'N':	/* normal host */
					break;
				default:
					fputs("check_ip() returned illegal host type\n",stderr);
					exit(99);
			}

			hp = find_host(from.sin_addr.s_addr);

			if(hp->hostname == 0 && no_unnamed) {
				send_error(&from,&rbuf, REVERSE_ERR_MSG);
				continue;
			}

			old = 0;
			cur_time = time((time_t *) 0);

			rkey = BB_READ2(rbuf.bb_key);
			if(hp->next_key != rkey) {
				if(!hp->active)
					hp->next_key = rkey;
				else {
					if(hp->last_key == rkey) {
						if(cur_time < hp->last_acc + retry_timeout) {
							if(dbug) fprintf(stderr,"Ignoring too early retry request (rtime=%ld,timeout=%d).\n",(long)cur_time-hp->last_acc,(int)retry_timeout);
							continue;
						}
						old = 1;
					} else {
						if(cur_time < hp->last_acc + session_timeout ) {
							if(dbug) fprintf(stderr,"Request with bad key (rtime=%ld,timeout=%d).\n",(long)cur_time-hp->last_acc, (int)session_timeout);
							continue;
						}
						hp->active = 0;
						hp->next_key = rkey;
					}
				}
			}

			if(dbug && hp->active == 0) fprintf(stderr,"\n");
			hp->active = 1;
			hp->last_acc = cur_time;

			/* check checksum */
			s = (unsigned char *) &rbuf;
			d = s + bytes;
			u = rbuf.sum; rbuf.sum = 0;
			for(t = s, sum = bytes; t < d; sum += *t++);
			sum = (sum + (sum >> 8)) & 0xff;
			if(sum != u)
			{
				if(dbug) fprintf(stderr,"Wrong checksum got %x, expected %x\n",u,sum);
				continue;			/* wrong check sum */
			}

			server_process_packet(bytes,&rbuf,old,hp,&from);
		} 
		else
		{
			return(0);				/* got a timeout */
		}
	}
}

/****************************************************************************
 * Generic routine for sending reply back to clients.
 *        from: client address structure.
 *          ub: pointer to the message buffer.
 *  len1, len2: lengths of the two data regions in the message buffer.
 ****************************************************************************/

int server_reply (struct sockaddr_in *from, UBUF * ub,
		unsigned int len1,unsigned int len2)
{
	unsigned char *s, *t, *d;
	unsigned sum;
	int i;
	unsigned int thcsum;

	if(dbug)
		fprintf(stderr,"snd (%s,key=0x%04X,seq=0x%04X,len=%d,len2=%d,pos=%u) ---> %d.%d.%d.%d\n",
				print_command(ub->cmd), BB_READ2(ub->bb_key), BB_READ2(ub->bb_seq),len1, len2, BB_READ4(ub->bb_pos),
				((unsigned char *)(&(from->sin_addr.s_addr)))[0],
				((unsigned char *)(&(from->sin_addr.s_addr)))[1],
				((unsigned char *)(&(from->sin_addr.s_addr)))[2],
				((unsigned char *)(&(from->sin_addr.s_addr)))[3]);

	BB_WRITE2(ub->bb_len,len1);

	ub->sum = 0;
	s = (unsigned char *) ub;
	d = s + (len1 + len2 + UBUF_HSIZE);
	for(t = s, sum = 0; t < d; sum += *t++);
	ub->sum = sum + (sum >> 8);

	/*
	 * Check that we do not exceed maximum throughput allowed before sending
	 */
	if(maxthcallowed)
		for(;;) {
			if(cur_time > thcbase) {
				if(cur_time>thcbase+THCCOUNT) {
					for(i = 0; i < THCCOUNT; thc[i++]=0);
					thcbase = cur_time;
				} else {
					while (cur_time > thcbase) {
						for(i = THCCOUNT-1; i>0; i--) thc[i] = thc[i-1];
						thc[0] = 0;
						thcbase++;
					}
				}
			}
			for(i = 0, thcsum = 0; i< THCCOUNT; thcsum+= thc[i++]);
			thcsum /= THCCOUNT;
			if(dbug)
			{
				fprintf(stderr, "Average throughput: %d bytes/s | ", thcsum);
				for (i= THCCOUNT-1;i>=0;i--) fprintf(stderr,"%5d ",thc[i]) ;
				fprintf(stderr,"\n") ;
			}
			if(thcsum <= maxthcallowed) {
				thc[0]+=(len1+len2+UBUF_HSIZE);
				break;
			}
			if(dbug) fprintf(stderr, "Throughput too high, waiting.\n");
			sleep(1);
		}

	if(sendto(myfd,(char *)ub,(len1 + len2 + UBUF_HSIZE),0,
				(struct sockaddr *)from,sizeof(struct sockaddr_in)) == -1) {
		perror("sendto");
		exit(7);
	}

	return(0);
}

/****************************************************************************
 * Send a block of data read from the file 'fp'.  Offset information is
 * contained in the input ub message buffer, which also doubles as the output
 * message buffer.
 ****************************************************************************/

void serve_file (struct sockaddr_in * from, UBUF * ub, FILE * fp,
		unsigned int has_len, unsigned char * lp)
{
	size_t bytes;
	unsigned len;
	unsigned long pos;

	if(has_len == 2) {	/* recover length field if it exists */
		len=lp[0] << 8;
		len  = len + lp[1];
		printf("preferred size-%d\n",len);
		if(len > packetsize )
			len = packetsize;
		else	
			if(len <= 0)
				len = UBUF_SPACE;
	} else 
	{
		/* use default if it doesn't exist */
		len  = packetsize;
		if( len > UBUF_SPACE )
			len = UBUF_SPACE;
	}

		pos = BB_READ4(ub->bb_pos);

#ifndef NATIVE_LARGEFILES
#if SIZEOF_OFF_T == 4
		if(pos>TWOGIGS)
			len=0;
#endif
#endif

		if(fseeko(fp,pos,SEEK_SET))
		{
			/* seek failed, do not send any more data */
			/* TODO: or can we return error instead?  */
			len=0;
		}

		bytes = fread(ub->buf, 1, len, fp);

		server_reply(from,ub,bytes,0);
}

/****************************************************************************
 * Send version information.
 * Note: no bounds checking is currently performed.  As version information
 *       grows, this will become required.
 ****************************************************************************/
#ifndef LAMERPACK
static void server_show_version (struct sockaddr_in * from, UBUF * ub)
{
	char buf[UBUF_SPACE], verflags = 0;
	unsigned int xtra = VER_BYTES;
	strcpy(buf, "fspd " PACKAGE_VERSION);

	if (logging)       verflags |= VER_LOG;
	if (read_only)     verflags |= VER_READONLY;
	if (no_unnamed)    verflags |= VER_REVNAME;
	if (priv_mode)     verflags |= VER_PRIVMODE;
	if (maxthcallowed) verflags |= VER_THRUPUT;

	/* we are accepting xtra data on input */
	/* because UDP tranport provides data size */

	verflags |= VER_XTRADATA;

	strcpy(ub->buf, buf);
	ub->buf[strlen(ub->buf)] = '\0';
	ub->buf[strlen(ub->buf)+1] = verflags;
	/* add optional thruput data */
	if(maxthcallowed) {
		BB_WRITE4(ub->buf+strlen(ub->buf)+2,maxthcallowed);
		xtra+=4;
	}
	/* Add packetsize - 2.8.1 b20 extension */
	BB_WRITE2(ub->buf+strlen(ub->buf)+xtra+1,packetsize);
	xtra+=2;
	BB_WRITE4(ub->bb_pos,xtra);
	server_reply(from, ub, strlen(ub->buf)+1, xtra);
}
#endif
/****************************************************************************
 *  This is the dispatch loop for message that has been accepted.
 *  Serves command in message and sends out a reply.
 *    bytes: size of the message received.
 *       ub: pointer to the message buffer.
 *      old: true if this message contains old sequence number (retransmit).
 *       hp: pointer to the entry for the client host who sent this message.
 *     from: pointer to the socket address structure of the client host.
 ****************************************************************************/

static void server_process_packet (unsigned bytes, UBUF * ub, int old,
		HTAB * hp, struct sockaddr_in * from)
{
	unsigned long  inet_num, pos;
	unsigned short port_num;
	unsigned l1, l2;
	char *s1, *s2, inetstr_buf[128], *inetstr;
	const char *pe;
	FILE *fp;
	PPATH pp;
	struct stat sd; /* for logging of filesize */
	DIRLISTING *dl;
	DIRINFO *di;
	unsigned len;

	pos = BB_READ4(ub->bb_pos);
	l1  = BB_READ2(ub->bb_len);
	l2 = bytes - l1 - UBUF_HSIZE;
	s1 = ub->buf;
	s2 = ub->buf + l1;





	/* put remote inet_number in a var, for logging purposes */
	if (dbug || logging) {
		if (hp->hostname)
			inetstr = hp->hostname;
		else {
			sprintf(inetstr_buf,"%d.%d.%d.%d",
					((unsigned char *)(&hp->inet_num))[0],
					((unsigned char *)(&hp->inet_num))[1],
					((unsigned char *)(&hp->inet_num))[2],
					((unsigned char *)(&hp->inet_num))[3]);
			inetstr = inetstr_buf;
		}
	}

	if(dbug) fprintf(stderr,"rcv (%s,key=0x%04X,seq=0x%04X,len=%d,len2=%d,pos=%lu) <--- %s\n", print_command(ub->cmd), BB_READ2(ub->bb_key),BB_READ2(ub->bb_seq),l1, l2,
			pos, inetstr);

	if(!old) {
		hp->last_key = hp->next_key;
		hp->next_key = gen_next_key();
	}

	BB_WRITE2(ub->bb_key,hp->next_key);
	inet_num = hp->inet_num;
	port_num = from->sin_port;

	switch(ub->cmd) {
		case CC_VERSION:
#ifdef LAMERPACK
			ACTIONLOG0(L_VER,"AVOIDED SCAN DETECTION");
			ACTIONOK(L_VER);
#else
			if(! ver_enabled) 
				return;
			ACTIONLOG0(L_VER,"VERSION");
			server_show_version(from, ub);
			ACTIONOK(L_VER);
#endif
			return;
		case CC_BYE:
			hp->active = 0; /* was: if(!old)   before, why? */
			server_reply(from,ub,0,0);
			return;
		case CC_GET_DIR :
			if (!pos) ACTIONLOG1(L_GETDIR,"GETDIR");
			if( (pe = validate_path(s1,l1,&pp,&di,1)) )
			{
				ACTIONLOG1(L_GETDIR|L_ERR,"GETDIR");
				ACTIONFAILED(L_GETDIR|L_ERR,pe);
				send_error(from, ub, pe);
				return;
			}
			CHECK_ACCESS_RIGHTS(DIR_LIST,L_GETDIR);
			
			/*Add by xxfan	*/
				if(l2 == 2) {	/* recover length field if it exists */
					len=(unsigned char)s2[0] << 8;
					len  = len + (unsigned char)s2[1];
					printf("perferred size-%d\n",len);	
					if(len > packetsize )
						len = packetsize;
					else	
						if(len <= 0)
							len = packetsize;
				} 
				else 
				{
					/* use default if it doesn't exist */
					len  = packetsize;
					if( len > UBUF_SPACE )
						len = UBUF_SPACE;
				}
				l2=len;
				
				/*end add*/


			if ( (pe = server_get_dir(&dl,di,len)) )
			{
				ACTIONLOG1(L_GETDIR|L_ERR,"GETDIR");
				ACTIONFAILED(L_GETDIR|L_ERR,pe);
				send_error(from, ub, pe);
				return;
			}
			/* copy directory listing to client buffer */
			if(pos>=dl->listing_size) 
				l1=0;
			else
			{
				l1=dl->listing_size-pos;
				if(l1>l2) l1=l2;

				memcpy( ub->buf, dl->listing+pos, l1);
			}
			if( (l1>0) && (pos % l2 != 0) )
			{
				send_error(from,ub,"Invalid directory seek offset");
			}
			else
			{
				server_reply(from,ub,l1,0);
			}
			if(!pos) ACTIONOK(L_GETDIR);
			return;
		case CC_GET_FILE:
			if (!pos) ACTIONLOG1(L_GETFILE,"GETFILE");
			pe = validate_path(s1,l1,&pp,&di,0);
			if(pe)
			{
				ACTIONLOG1(L_GETFILE|L_ERR,"GETFILE");
				ACTIONFAILED(L_GETFILE|L_ERR,pe);
				send_error(from, ub, pe);
				return;
			}
			CHECK_ACCESS_RIGHTS(DIR_GET,L_GETFILE);
			pe = server_get_file(&pp, &fp, inet_num, port_num,di);
			if(pe)
			{
				ACTIONLOG1(L_GETFILE|L_ERR,"GETFILE");
				ACTIONFAILED(L_GETFILE|L_ERR,pe);
				send_error(from, ub, pe);
				return;
			}
			if (!pos) {
				FSP_STAT(pp.fullp,&sd); /* log filesizes */
				ACTIONINFO(L_GETFILE,(" (%d)",sd.st_size));
				xferlog('o',pp.fullp,sd.st_size,inetstr);
			}
			serve_file(from,ub,fp,l2,s2);
			if (!pos) ACTIONOK(L_GETFILE);
			return;
		case CC_DEL_FILE:
			if (read_only) {
				ACTIONLOG1(L_RDONLY,"DELFILE");
				ACTIONFAILED(L_RDONLY,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode");
				return;
			}
			if(!old)
			{
				ACTIONLOG1(L_DELFILE,"DELFILE");
				pe = validate_path(s1,l1,&pp,&di,0);
				if(pe)
				{
					ACTIONLOG1(L_DELFILE|L_ERR,"DELFILE");
					ACTIONFAILED(L_DELFILE|L_ERR,pe);
					send_error(from, ub, pe);
					return;
				}
				CHECK_ACCESS_RIGHTS(DIR_DEL,L_DELFILE);
				pe = server_del_file(&pp,di);
				if(pe)
				{
					ACTIONLOG1(L_DELFILE|L_ERR,"DELFILE");
					ACTIONFAILED(L_DELFILE|L_ERR,pe);
					send_error(from, ub, pe);
					return;
				}
				ACTIONOK(L_DELFILE);
			}
			server_reply(from,ub,0,0);
			return;
		case CC_DEL_DIR :
			ACTIONLOG1(L_DELDIR,"DELDIR");
			if (read_only) {
				ACTIONLOG1(L_RDONLY,"DELDIR");
				ACTIONFAILED(L_RDONLY,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode") ;
				return;
			}
			if(!old)
			{
				pe = validate_path(s1,l1,&pp,&di,1);
				if(pe)
				{
					ACTIONLOG1(L_DELDIR|L_ERR,"DELDIR");
					ACTIONFAILED(L_DELDIR|L_ERR,pe);
					send_error(from, ub, pe);
					return;
				}
				CHECK_ACCESS_RIGHTS(DIR_DEL,L_DELDIR);
				pe = server_del_dir(&pp,di);
				if(pe)
				{
					ACTIONLOG1(L_DELDIR|L_ERR,"DELDIR");
					ACTIONFAILED(L_DELDIR|L_ERR,pe);
					send_error(from, ub, pe);
					return;
				}
			}
			server_reply(from,ub,0,0);
			ACTIONOK(L_DELDIR);
			return;
		case CC_UP_LOAD :
			if (!pos || read_only) {
				ACTIONLOG0(L_UPLOAD,"UPLOAD");
				if(read_only) {
					ACTIONLOG0(L_RDONLY,"UPLOAD");
					ACTIONFAILED(L_RDONLY,"Server is running in read-only mode");
					send_error(from, ub, "Server is running in read-only mode") ;
					return;
				}
			}
			if(!old)
			{
				pe = server_up_load(s1,l1,pos, inet_num,port_num);	
				if(pe)
				{
					ACTIONLOG0(L_UPLOAD|L_ERR,"UPLOAD");
					ACTIONFAILED(L_UPLOAD|L_ERR,pe);
					send_error(from, ub, pe) ;
					return;
				}
			}
			server_reply(from,ub,0,0);
			if(!pos) ACTIONOK(L_UPLOAD);
			return;
		case CC_INSTALL :
			if (read_only) {
				ACTIONLOG1(L_INSTALL|L_ERR,"INSTALL");
				ACTIONFAILED(L_INSTALL|L_ERR,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode") ;
				return;
			}
			if(!old)
			{
				ACTIONLOG1(L_INSTALL,"INSTALL");
				pe = validate_path(s1,l1,&pp,&di,0);
				if (pe)
				{
					ACTIONLOG1(L_INSTALL|L_ERR,"UPLOAD");
					ACTIONFAILED(L_INSTALL|L_ERR,pe);
					send_error(from, ub, pe) ;
					return;
				}
				if(l1>1) {
					CHECK_ACCESS_RIGHTS(DIR_ADD,L_INSTALL);
				}    
				pe = server_install(&pp,inet_num,port_num,pe,di,l2,s2);
				if(pe)
				{
					ACTIONLOG1(L_INSTALL|L_ERR,"UPLOAD");
					ACTIONFAILED(L_INSTALL|L_ERR,pe);
					send_error(from, ub, pe) ;
					return;
				}
				ACTIONOK(L_INSTALL);
				FSP_STAT(pp.fullp,&sd); /* log filesizes */
				xferlog('i',pp.fullp,sd.st_size,inetstr);
			}
			server_reply(from,ub,0,0);
			return;
		case CC_MAKE_DIR:
			ACTIONLOG1(L_MAKEDIR,"MAKEDIR");
			if (read_only) {
				ACTIONLOG1(L_MAKEDIR|L_ERR,"MAKEDIR");
				ACTIONFAILED(L_MAKEDIR|L_ERR,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode") ;
				return;
			}
			pe = validate_path(s1,l1,&pp,&di,0);
			if(pe)
			{
				ACTIONLOG1(L_MAKEDIR|L_ERR,"MAKEDIR");
				ACTIONFAILED(L_MAKEDIR|L_ERR,pe);
				send_error(from, ub, pe) ;
				return;
			}
			if(!old)
			{
				CHECK_ACCESS_RIGHTS(DIR_MKDIR,L_MAKEDIR);
				pe=server_make_dir(&pp,inet_num,&di);
				if(pe)
				{
					ACTIONLOG1(L_MAKEDIR|L_ERR,"MAKEDIR");
					ACTIONFAILED(L_MAKEDIR|L_ERR,pe);
					send_error(from, ub, pe) ;
					return;
				}
			}
			CHECK_ACCESS_RIGHTS(0,L_MAKEDIR);
			l1 = server_get_pro(di,ub->buf,pe);
			BB_WRITE4(ub->bb_pos,PRO_BYTES);
			server_reply(from,ub,l1,PRO_BYTES);
			ACTIONOK(L_MAKEDIR);
			return;
		case CC_GET_PRO :
			ACTIONLOG1(L_GETPRO,"GETPRO");
			pe=validate_path(s1,l1,&pp,&di,1);
			if(pe)
			{
				ACTIONLOG1(L_GETPRO|L_ERR,"GETPRO");
				ACTIONFAILED(L_GETPRO|L_ERR,pe);
				send_error(from, ub, pe) ;
				return;
			}
			CHECK_ACCESS_RIGHTS(0,L_GETPRO);
			l1=server_get_pro(di,ub->buf,pe);
			BB_WRITE4(ub->bb_pos,PRO_BYTES);
			server_reply(from,ub,l1,PRO_BYTES);
			ACTIONOK(L_GETPRO);
			return;
		case CC_SET_PRO :
			ACTIONLOG1(L_SETPRO,"SETPRO");
			if(read_only) {
				ACTIONLOG1(L_SETPRO|L_RDONLY,"SETPRO");
				ACTIONFAILED(L_SETPRO|L_ERR,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode") ;
				return;
			}
			pe = validate_path(s1,l1,&pp,&di,1);
			if(pe)
			{
				ACTIONLOG1(L_SETPRO|L_ERR,"SETPRO");
				ACTIONFAILED(L_SETPRO|L_ERR,pe);
				send_error(from, ub, pe) ;
				return;
			}
			if(!old)
			{
				CHECK_ACCESS_RIGHTS(DIR_OWNER,L_SETPRO);
				pe = server_set_pro(di,s2);
				if(pe)
				{
					ACTIONLOG1(L_SETPRO|L_ERR,"SETPRO");
					ACTIONFAILED(L_SETPRO|L_ERR,pe);
					send_error(from, ub, pe) ;
					return;
				}
			}
			l1 = server_get_pro(di,ub->buf,"O");
			BB_WRITE4(ub->bb_pos,PRO_BYTES);
			server_reply(from,ub,l1,PRO_BYTES);
			ACTIONOK(L_SETPRO);
			return;
		case CC_GRAB_FILE:
			if (read_only || !grab_enabled) {
				ACTIONLOG1(L_RDONLY,"GRABFILE");
				ACTIONFAILED(L_RDONLY,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode") ;
				return;
			}
			if (!pos) {
				ACTIONLOG1(L_GRABFILE,"GRABFILE");
			}
			pe = validate_path(s1,l1,&pp,&di,0);
			if(pe)
			{
				ACTIONLOG1(L_ERR|L_GRABFILE,"GRABFILE");
				ACTIONFAILED(L_ERR|L_GRABFILE,pe);
				send_error(from, ub, pe) ;
				return;
			}
			CHECK_ACCESS_RIGHTS(DIR_DEL,L_GRABFILE);
			if(!old && !pos)
			{
				pe=server_secure_file(&pp,inet_num,port_num,di);
				if(pe)
				{
					ACTIONLOG1(L_ERR|L_GRABFILE,"GRABFILE");
					ACTIONFAILED(L_ERR|L_GRABFILE,pe);
					send_error(from, ub, pe) ;
					return;
				}
			}
			pe = server_grab_file(&fp, inet_num, port_num);
			if(pe)
			{
				ACTIONLOG1(L_ERR|L_GRABFILE,"GRABFILE");
				ACTIONFAILED(L_ERR|L_GRABFILE,pe);
				send_error(from, ub, pe) ;
				return;
			}
			serve_file(from,ub,fp,l2,s2);
			fclose(fp);
			if (!pos) ACTIONOK(L_GRABFILE);
			return;
		case CC_GRAB_DONE:
			if (read_only || !grab_enabled) {
				ACTIONLOG1(L_RDONLY,"GRABFILE");
				ACTIONFAILED(L_RDONLY,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode") ;
				return;
			}
			ACTIONLOG1(L_GRABFILE,"GRABDONE");
			if(!old)
			{
				pe = validate_path(s1,l1,&pp,&di,0);
				if(pe)
				{
					ACTIONLOG1(L_ERR|L_GRABFILE,"GRABFILE");
					ACTIONFAILED(L_ERR|L_GRABFILE,pe);
					send_error(from, ub, pe) ;
					return;
				}
				CHECK_ACCESS_RIGHTS(DIR_DEL,L_GRABFILE);
				pe = server_grab_done(inet_num,port_num);
				if(pe)
				{
					ACTIONLOG1(L_ERR|L_GRABFILE,"GRABFILE");
					ACTIONFAILED(L_ERR|L_GRABFILE,pe);
					send_error(from, ub, pe) ;
					return;
				}
			}
			server_reply(from,ub,0,0);
			ACTIONOK(L_GRABFILE);
			return;
		case CC_STAT :
			ACTIONLOG1(L_STAT,"STAT");
			server_stat(ub);
			server_reply(from,ub,9,0);
			ACTIONOK(L_STAT);
			return;
			/*Add by xxfan 2016-03-25
/*
		case CC_CH_PASSWD :
			ACTIONLOG1(L_STAT,"CH_PASSWD");
			pe = validate_path(s1,l1,&pp,&di,0);
			if(pe)
			{
				ACTIONLOG1(L_ERR|L_CH_PASSWD,"CH_PASSWD");
				ACTIONFAILED(L_ERR|L_CH_PASSWD,pe);
				send_error(from, ub, pe) ;
				return;
			}
			char* ptmp = NULL;
			char passwd_file_url[256]={0};

			sprintf(passwd_file_url,"%s/%s",home_dir,FSP_PASSWORD);
			//CH_PASSWD METHOD ,data format:directory name\nold passwd\nnew passwd 
			if(pp.passwd!=NULL)
			{
				if (( ptmp = strchr(pp.passwd,'\n')) != NULL)
				{
					*ptmp = '\0';
					ptmp++;
				}
				else
				{
					send_error(from, ub, "format error") ;
					return;
				}
			}

			//
			   file_fd=open(passwd_file_url,O_RDONLY);
			   if(file_fd>0)
			   {
			   read(file_fd,old_passwd,256);
			   fprintf(stderr,"old_passwd-%s\n",old_passwd);
			   close(file_fd);
			   }
			   else 
			   {
			   fprintf(stderr,"open file-%s error\n",passwd_file_url);
			   }

			   fprintf(stderr,"msg_passwd-%s,old_passwd-%s\n",pp.passwd,old_passwd);

			//
			if(di->public_password!=NULL)
				fprintf(stderr,"di->public_password-%s\n",di->public_password);
			else fprintf(stderr,"di->public_password-NULL\n");

			int rc;

			if((di->public_password!=NULL&&strcmp(pp.passwd,di->public_password)==0)||di->public_password==NULL)
			{
				free(di->public_password);
				di->public_password=strdup(ptmp);

				rc=update_password(ptmp);
				if(rc<0) printf("update_password error\n");
			}
			else 
			{
				send_error(from,ub,"wrong password");
				return;
			}
			//
			   pe = server_grab_done(inet_num,port_num);
			   if(pe)
			   {
			   ACTIONLOG1(L_ERR|L_CH_PASSWD,"CH_PASSWD");
			   ACTIONFAILED(L_ERR|L_CH_PASSWD,pe);
			   send_error(from, ub, pe) ;
			   return;
			   }
			//
			server_reply(from,ub,0,0);

			return;
			//?end
*/			

		case CC_RENAME :
			if (read_only) {
				ACTIONLOG1(L_RDONLY,"RENAME");
				ACTIONFAILED(L_RDONLY,"Server is running in read-only mode");
				send_error(from, ub, "Server is running in read-only mode");
				return;
			}
			if(!old)
			{
				DIRINFO *srcdir;
				PPATH srcpath;
				int istargetdir;
				struct stat sb;

				ACTIONLOG2(L_RENAME,"RENAME");

				/* validate source file */
				pe = validate_path(s1,l1,&pp,&di,0);

				if (pe)
				{
					ACTIONLOG2(L_ERR|L_RENAME,"RENAME");
					ACTIONFAILED(L_ERR|L_RENAME,pe);
					send_error(from, ub, pe) ;
					return;
				}

				CHECK_ACCESS_RIGHTS(DIR_RENAME,L_RENAME);

				srcdir=di;
				srcpath=pp;

				/* validate target */
				pe = NULL;
				/* check if target path is zero terminated */
				if(s2[l2-1])
					pe = ("Target path is not zero terminated");
				else
				{
					if(FSP_STAT(s2,&sb))
						istargetdir=-1; /* non - existent! */
					else    
						if(S_ISDIR(sb.st_mode))
							istargetdir=1;
						else
							if(S_ISREG(sb.st_mode))
								istargetdir=0;
							else
								pe = ("Refusing to operate on special files");
				}

				if (!pe)
				{
					if (istargetdir == 1)
						pe = validate_path(s2,l2,&pp,&di,1);
					else
						pe = validate_path(s2,l2,&pp,&di,0);
				}

				if (pe)
				{
					ACTIONLOG2(L_ERR|L_RENAME,"RENAME");
					ACTIONFAILED(L_ERR|L_RENAME,pe);
					send_error(from, ub, pe) ;
					return;
				}

				/* check ACL in target directory */
				if (istargetdir == 0)
				{
					/* we need delete right in the target directory */
					pe = require_access_rights(di,DIR_DEL,inet_num,pp.passwd);
					if(pe[0]!='N' && pe[0]!='O')
						pe=("No permission for overwriting files");
					else
						pe=NULL;
				}

				/* execute rename */
				if (!pe)
					pe = server_rename(&srcpath,&pp,srcdir,di);
				if ( pe )
				{
					ACTIONLOG2(L_RENAME|L_ERR,"RENAME");
					ACTIONFAILED(L_RENAME|L_ERR,pe);
					send_error(from, ub, pe);
					return;
				}
			}
			/* clear position field */
			memset(ub->bb_pos,0,4);
			server_reply(from,ub,0,0);
			ACTIONOK(L_RENAME);
			return;
		default:
			if ((unsigned char)ub->cmd > CC_LIMIT) {	/* extended commands */
				ACTIONLOG0(L_ERR,"UNSUPPORTED");
				ACTIONINFO(L_ERR,(" (%d)", ub->cmd));
				ACTIONFAILED(L_ERR,"Unsupported FSP command");

				send_error(from, ub, "Unsupported command - this \
						server only supports version 2.8.1 or below of the FSP protocol") ;
			}
			else {
				ACTIONLOG0(L_ERR,"UNKNOWN");
				ACTIONINFO(L_ERR,(" (%d)", ub->cmd));
				ACTIONFAILED(L_ERR,"Unknown FSP command");
				send_error(from, ub, "Unknown FSP command") ;
			}

			return;
	}
}

/*Add by xxfan 2016-06-08*/
int create_dir_if_absent(char* url)
{
	int i;
	for(i=1;*(url+i)!='\0';i++)
	{
		if(*(url+i) == '/')
		{
			*(url+i) = '\0';

			if(access(url,F_OK) == -1)
			{
				if(mkdir(url,00700)==-1)
				{
					return -1;
				}
			}
			*(url+i) = '/';
		}
	}
	/*e.g. url=/var/tmp/FSP*/
	if(*(url+i-1)!='/')
	{
		if(access(url,F_OK) == -1)
		{
			if(mkdir(url,00700)==-1)
			{
				/*printf("mkdir error\n");*/
				return -1;
			}
		}

	}
	return 0;
}

