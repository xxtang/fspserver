 /*********************************************************************\
 *  Copyright (c) 2004-2009 by Radim Kolar                             *
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
#include <stdlib.h>
#include <stdio.h>
#include "my-string.h"

static int env_dir_malloced = 0;
const char *env_dir = "/";
const char *env_passwd = "\0";
const char *env_myport;
const char *env_host;
const char *env_port;
const char *env_local_dir;
const char *env_listen_on;
unsigned int env_timeout;
unsigned short client_buf_len;
unsigned short client_net_len;
int statworks=1;

#define TWOGIGS  0x7fffffffUL
#define FOURGIGS 0xffffffffUL

/* returns mallocated abs. path merged from s2 and env.dir */
#define APPEND_PASSWORD 	if(have_password) \
				{ \
 				  strcat(path,"\n"); \
                                  strcat(path,env_passwd); \
                                }
char *util_abs_path (const char * s2)
{
  char *path, *s, *d, *t;
  int have_password = 0;

  if(!env_dir) env_dir = "";
  if(!s2) s2 = "";
  if(strlen(env_passwd)>0)
      have_password=1;
  if(*s2 == '/') {
    path = malloc(strlen(s2)+2+strlen(env_passwd)+1);
    sprintf(path,"/%s",s2);
    if(have_password)
	util_junk_password(path);
  } else {
    path = malloc(strlen(env_dir)+strlen(s2)+3+strlen(env_passwd)+1);
    sprintf(path,"/%s/%s",env_dir,s2);
    if(have_password)
	util_junk_password(path);
  }

  for(t = path; *t; ) {
    if(t[0] == '/') {
      while(t[1] == '/') for(d = t, (s = t+1); (*d++ = *s++); );
      if(t != path && t[1] == 0) { t[0] = 0; return(path); }
    }
    if(t[0] == '.' && t[1] == '.') {
      if(t-1 == path && t[2] ==  0 ) {
	*t = 0;
	APPEND_PASSWORD;
	return(path);
      }
      if(t-1 == path && t[2] == '/') {
	for(d = t, s = t + 3; (*d++ = *s++); );
	continue;
      }
      if(t[-1] == '/' && (t[2] == '/' || t[2] ==  0)) {
	s = t + 2; /* point to either slash or nul */
	t -= 2;	/* guaranteed that t >= path here */
	while(t > path && t[0] != '/') t--;
	if(t != path || *s == '/') for(d = t; (*d++ = *s++); );
	else {
	  t[1] = 0;
	  APPEND_PASSWORD;
	  return(path);
	}
	continue;
      }
    }
    if(t[0] == '.') {
      if(t-1 == path && t[1] ==  0 ) {
	*t = 0;
	APPEND_PASSWORD;
	return(path);
      }
      if(t-1 == path && t[1] == '/') {
	for(d = t, s = t + 2; (*d++ = *s++); );
	continue;
      }
      if(t[-1] == '/' && (t[1] == '/' || t[1] ==  0)) {
	s = t + 1; /* point to either slash or nul */
	for(d = t-1; (*d++ = *s++); );
	t--;
	continue;
      }
    }
    t++;
  }
  APPEND_PASSWORD;
  return(path);
}

static void util_pktstats(void)
{
    fprintf(stderr,"Packets received: %lu total (%lu bad, %lu duplicate)\nPackets sent: %lu total (%lu first resends, %lu idle resends).\n",stat_ok+stat_bad+stat_dupes,stat_bad,stat_dupes,
    stat_ok+stat_resends+stat_iresends,stat_resends,stat_iresends);
    fprintf(stderr,"Line has %lu %% packet loss rate.\n",100*(stat_resends+stat_iresends)/(stat_ok+stat_dupes+stat_resends+stat_iresends));
}

static int util_split_path (char * path, char ** p1, char ** p2,
				  char ** p3)
{
  char *s;
  static char junk;

  *p1 = "/";
  if(*path == '/') {
    *p2 =  path;
    *p3 = path+1;
  } else {
    *p2 = &junk;
    *p3 = path;
  }

  for(s = *p3; *s; s++) {
    if(*s == '/') {
      *p1 = path;
      *p2 = s;
      *p3 = s+1;
    }
  }

  if (**p3 == '\0') *p3 = ".";
  return(1);
}

/* return current working directory (from environment) */
char *util_getwd (char * p)
{
  if(p) strcpy(p,env_dir);
  return(p);
}

static RDIRENT **get_dir_blk (char * path)
{
  RDIRENT **dp;
  char *p1, *p2, *fpath, buf[NBSIZE];
  unsigned long pos;
  int cnt, k, len, rem, acc, at_eof, rlen;
  unsigned short dirblocksize;
  UBUF *ub;

  fpath = util_abs_path(path);
  
  dirblocksize = 0;

  for(pos = 0, at_eof = acc = cnt = 0; ; )
  {
    while((acc < UBUF_MAXSPACE) && !at_eof)
    {
      ub = client_interact(CC_GET_DIR,pos, strlen(fpath),
			   (unsigned char *)fpath+1, 2,
			   (unsigned char *)&client_net_len);
	
      if(ub->cmd == CC_ERR) {
	fprintf(stderr,"%s: %s\n",path, ub->buf);
	free(fpath);
	errno = EACCES;
	return((RDIRENT **) 0);
      }

      rlen = BB_READ2(ub->bb_len);
      if(dirblocksize == 0 )
	  dirblocksize = rlen;
      else
	  if (rlen < dirblocksize) at_eof = 1;	  
      memcpy(buf + acc, ub->buf, rlen);
      acc += rlen;
      pos += rlen;
    }

    if(acc >= dirblocksize) len = dirblocksize;
    else len = acc;

    for(p2 = buf, rem = len, k = 0; ; k++) {
      if(rem < RDHSIZE) break;
      if(((RDIRENT *) p2)->type == RDTYPE_SKIP) break;
      if(((RDIRENT *) p2)->type == RDTYPE_END ) { k++; break; }
      p2 += RDHSIZE; rem -= (RDHSIZE+1);
      while(*p2++) rem--;
      while((p2 - buf) & 3) {
	p2++;
	rem--;
      }
    }

    p1 = malloc(p2-buf);
    if(cnt) dp = (RDIRENT **) realloc(dp,(cnt+k+1)*sizeof(RDIRENT *));
    else dp = (RDIRENT **)  malloc((cnt+k+1)*sizeof(RDIRENT *));

    if(!p1 || !dp) {
      fputs("directory reading out of memory\n",stderr);
      free(fpath);
      return((RDIRENT **) 0);
    }

    for(p2 = buf, rem = len; ; cnt++) {
      if(rem < RDHSIZE) break;
      if(((RDIRENT *) p2)->type == RDTYPE_SKIP) break;
      if(((RDIRENT *) p2)->type == RDTYPE_END) {
	dp[cnt] = 0;
        free(fpath);
	return(dp);
      }
      dp[cnt] = (RDIRENT *) p1;
	
      for(k = RDHSIZE, rem -= (RDHSIZE+1); k--; *p1++ = *p2++);
      while( (*p1++ = *p2++) ) rem--;
      while((p2 - buf) & 3) {
	p2++;
	p1++;
	rem--;
      }
    }

    if(acc < dirblocksize) {
      dp[cnt] = 0;
      free(fpath);
      return(dp);
    }
    for(p1 = buf + dirblocksize, p2 = buf, k = (acc -= dirblocksize); k--;)
      *p2++ = *p1++;
  }
  free(fpath);
}

static int util_download_main (char * path, char * fpath, FILE * fp,
				     off_t start_from, int cmd)
{
  off_t pos, started_from = start_from, downloaded;
  unsigned int tmax, wrote, sent_time, rlen;
  UBUF *ub;
  time_t t = time(NULL);

  for(tmax = 1, pos = start_from, sent_time = 0; ;) {
    ub = client_interact(cmd,pos,strlen(fpath),(unsigned char *)fpath+1, 2,
			 (unsigned char *)&client_net_len);

    if(client_trace && (udp_sent_time != sent_time)) {
      sent_time = udp_sent_time;
      if(client_buf_len >= UBUF_SPACE) fprintf(stderr,"\r%luk  ",(long unsigned int)(1+(pos>>10)));
      else fprintf(stderr,"\r%lu   ", (long unsigned int) pos);
      fflush(stderr);
    }

    if(ub->cmd == CC_ERR) {
      fprintf(stderr,"downloading %s: %s\n",path,ub->buf);
      return(-1);
    }

    rlen = BB_READ2(ub->bb_len);
    wrote = fwrite(ub->buf,1,rlen,fp);
    /* check for long integer pos overflow */
#if SIZEOF_OFF_T > 4
    if(pos+wrote>FOURGIGS)
	break;
#else
    if(pos+wrote<pos)
	break;
#endif
    pos += wrote;

    if(rlen == 0)
	break;  /* end of file */
    if(rlen != wrote)
	return -1; /* write to local file failed */
  }

  t = time(NULL) - t;
  if (t == 0) t = 1;
  downloaded = pos - started_from;
  if(client_trace)
  {
    fprintf(stderr,"\r%luk : %s [%ldB/s] \n", (long unsigned int)(1+(pos>>10)), path, (long int)(downloaded/t));
    util_pktstats();
    fflush(stderr);
  }

  return(0);
}

int util_download (char * path, FILE * fp, unsigned long start_from)
{
  int code, len;
  char *fpath;

  fpath = util_abs_path(path);
  if(*env_passwd) {
    strcat(fpath, "\n");
    strcat(fpath, env_passwd);
    len = strlen(fpath);
  }
  code = util_download_main(path, fpath, fp, start_from, CC_GET_FILE);
  free(fpath);
  return(code);
}

int util_grab_file (char * path, FILE * fp, unsigned long start_from)
{
  int code, len;
  char *fpath;
  UBUF *ub;

  fpath = util_abs_path(path);
  if(*env_passwd) {
    strcat(fpath, "\n");
    strcat(fpath, env_passwd);
    len = strlen(fpath);
  }
  code = util_download_main(path, fpath, fp, start_from, CC_GRAB_FILE);
  if(code) {
    free(fpath);
    return(code);
  }

  ub = client_interact(CC_GRAB_DONE, 0L, len, (unsigned char *)fpath+1, 0,
		       (unsigned char *)NULLP);

  if(ub->cmd == CC_ERR) {
    fprintf(stderr,"Warning, unexpected grab error: %s\n",ub->buf);
  }

  free(fpath);
  return(code);
}

int util_upload (char * path, FILE * fp, time_t stamp)
{
  unsigned long pos;
  unsigned bytes, first, tmax, sent_time;
  char *fpath, buf[UBUF_MAXSPACE];
  UBUF *ub;
  time_t t = time(NULL);
  char *dpath,*p1,*p2;
  unsigned char flags;
  struct stat sb;

  fpath = util_abs_path(path);
  util_split_path(fpath,&dpath,&p1,&p2);
  *p1='\0';

  /* check if we have enough rights to create file */
  ub = client_interact(CC_GET_PRO,0, strlen(dpath), (unsigned char *)dpath+1, 0,
			   (unsigned char *)NULLP);
  free(fpath);
  fpath = util_abs_path(path);
  if(ub->cmd == CC_ERR)
  {
     fprintf(stderr,"uploading %s: %s\n",path,ub->buf);
     free(fpath);
     return(1);
  }
  /* extract flags from server reply */
  bytes = BB_READ2(ub->bb_len);
  pos   = BB_READ4(ub->bb_pos);
  if(pos)
  {
      /* flags are in the reply */
      flags = *(ub->buf+bytes);
      /* check for required flags */
      if(! (flags & DIR_OWNER))
      {
	  if( ! (flags & DIR_ADD))
	  {
             fprintf(stderr,"No permission for adding files.\n");
             free(fpath);
             return(1);
	  }
      } else
      {
	  /* owner of directory */
	  flags|=DIR_DEL|DIR_ADD;
      }
      /* check if we can overwrite an existing file */
      if(! (flags & DIR_DEL))
      {
	  if(util_stat(fpath,&sb)==0)
	  {
             fprintf(stderr,"No permission for overwriting: %s\n",fpath);
             free(fpath);
             return(1);
	  }
      }
  }

  for(tmax = 1, sent_time = 0, pos = 0, first = 1; ; first = 0)
  {
    if((bytes = fread(buf,1,client_buf_len,fp)) || first)
    {
      ub = client_interact(CC_UP_LOAD,pos, bytes, (unsigned char *)buf, 0,
			   (unsigned char *)NULLP);
      if(client_trace && (udp_sent_time != sent_time))
      {
	sent_time = udp_sent_time;
	if(client_buf_len >= UBUF_SPACE)
	  fprintf(stderr,"\r%luk  ",1+(pos>>10));
	else fprintf(stderr,"\r%lu   ",   pos     );
	fflush(stderr);
      }
    }
    else
    {
      BB_WRITE4(buf,stamp);
      ub = client_interact(CC_INSTALL,stamp==0?0:4,strlen(fpath),
			   (unsigned char *)fpath+1, stamp==0?0:4,
			   (unsigned char *)buf);
    }
    if(ub->cmd == CC_ERR) {
      fprintf(stderr,"uploading %s: %s\n",path,ub->buf);
      free(fpath);
      return(1);
    }

    if(!bytes && !first) break;
#if SIZEOF_OFF_T > 4
    if(pos+bytes>FOURGIGS)
	break;
#else
    if(pos+bytes<pos)
	break;
#endif
    pos += bytes;
  }

  t = time(NULL) - t;
  if(t == 0) t = 1;
  if(client_trace)
  {
    fprintf(stderr,"\r%luk : %s [%ldB/s] \n", 1+(pos>>10), path, pos/t);
    util_pktstats();
    fflush(stderr);
  }
  free(fpath);
  return(0);
}

static void util_get_env (void)
{
  char *p;

  if(!(env_host = getenv("FSP_HOST"))) {
    fputs("No FSP_HOST specified.\n",stderr);
    exit(EX_CONFIG);
  }
  if(!(env_port = getenv("FSP_PORT"))) {
    fputs("No FSP_PORT specified.\n",stderr);
    exit(EX_CONFIG);
  }
  if(!(env_dir  = getenv("FSP_DIR"))) {
    env_dir = "/";
    env_dir_malloced = 0;
  }
  if(!(env_myport = getenv("FSP_LOCALPORT"))) env_myport = "0";
  if(!(env_listen_on = getenv("FSP_LOCALIP"))) env_listen_on = NULL;
  if(!(env_passwd = getenv("FSP_PASSWORD"))) env_passwd = "\0";
  client_trace  = !!getenv("FSP_TRACE");
  if( (p = getenv("FSP_BUF_SIZE")) ) client_buf_len = atoi(p);
  else client_buf_len = UBUF_SPACE;

  if(client_buf_len > UBUF_MAXSPACE) client_buf_len = UBUF_MAXSPACE;
  client_net_len = htons(client_buf_len);

  if( (p = getenv("FSP_DELAY")) ) target_delay = atol(p);
  if(target_delay < MIN_DELAY) target_delay = MIN_DELAY;
  if(target_delay > MAX_DELAY) target_delay = MAX_DELAY;
  
  if( (p = getenv("FSP_MAXDELAY")) ) target_maxdelay = atol(p);
  if(target_maxdelay < target_delay) target_maxdelay = target_delay;
  if(target_maxdelay > MAX_DELAY) target_maxdelay = MAX_DELAY;

  if(!(env_local_dir = getenv("FSP_LOCAL_DIR"))) env_local_dir=".";

  if(!(p = getenv("FSP_TIMEOUT"))) env_timeout = DEFAULT_TIMEOUT;
  else env_timeout = atoi(p);
  if ( env_timeout <= 0 ) env_timeout = 0x7fffffff;
}


void env_client (void)
{
  util_get_env();
  init_client(env_host,atoi(env_port),atoi(env_myport));
}

static DDLIST *ddroot = 0;

RDIR *util_opendir (char * path)
{
  char *fpath;
  RDIRENT **dep;
  DDLIST *ddp;
  RDIR *rdirp;

  fpath = util_abs_path(path);

  for(ddp = ddroot; ddp; ddp = ddp->next)
    if(!strcmp(ddp->path,fpath)) break;

  if(!ddp) {
    if(!(dep = get_dir_blk(fpath)))
    {
	free(fpath);
	return((RDIR *) 0);
    }
    ddp = (DDLIST *) malloc(sizeof(DDLIST));
    ddp->dep_root = dep;
    ddp->path = fpath;
    ddp->ref_cnt = 0;
    ddp->next = ddroot;
    ddroot = ddp;
  } else free(fpath);

  ddp->ref_cnt++;

  rdirp = (RDIR *) malloc(sizeof(RDIR));
  rdirp->ddp = ddp;
  rdirp->dep = ddp->dep_root;
  return(rdirp);
}

void util_closedir (RDIR * rdirp)
{
  rdirp->ddp->ref_cnt--;
  free(rdirp);
}

rdirent *util_readdir (RDIR * rdirp)
{
  static rdirent rde;
  RDIRENT **dep;

  dep = rdirp->dep;

  if(!*dep) return((rdirent *) 0);

  rde.d_fileno = 10;
  rde.d_rcdlen = 10;
  rde.d_namlen = strlen((*dep)->name);
  rde.d_name   = (*dep)->name;
  rdirp->dep   = dep+1;

  return(&rde);
}

/*  Removes \npassword from input */
void util_junk_password(char *path)
{
    char *pos;

    pos=strchr(path,'\n');
    if(pos != NULL)
	/* terminate them! */
	*pos='\0';
}

int util_stat (char * path, struct stat * sbuf)
{
  RDIR *drp;
  RDIRENT **dep;
  DDLIST *ddp;
  UBUF *ub;
  char *fpath,*fpath2, *ppath, *p1, *pfile;
  int cached=0;

  fpath = util_abs_path(path);
  fpath2 = strdup(fpath);

  if(!strcmp(fpath,env_dir)) {
    ppath = fpath;
    pfile = ".";
  } else {
    util_split_path(fpath,&ppath,&p1,&pfile);
    *p1='\0';
  }
  /*  printf("ppath `%s` pfile '%s'\n",ppath,pfile); */

  /* check if we have ppath cached */
  /* printf("Finding `%s` in cache.\n",ppath); */
  for(ddp = ddroot; ddp; ddp = ddp->next)
  {
    /*  printf("  we have %s in cache.\n",ddp->path); */
    if(!strcmp(ddp->path,ppath))
    {
	cached=1;
	break;
    }
  }

  /*  if(cached) printf("Record found in cache.\n",ppath); */
  if(statworks && !cached)
  {
    /* send a new FSP_STAT command to server */
    ub = client_interact(CC_STAT,0L, strlen(fpath2),
			 (unsigned char *) fpath2+1, 0, 0);
    if(ub->cmd == CC_STAT)
    {
	sbuf->st_uid = 0;
	sbuf->st_gid = 0;
	sbuf->st_atime = sbuf->st_mtime =
			 sbuf->st_ctime = BB_READ4((ub->buf));
	sbuf->st_size  = BB_READ4((ub->buf+4));
	if((ub->buf[8]) == RDTYPE_DIR)
	{
	    sbuf->st_mode = 0777 | S_IFDIR;
	    sbuf->st_nlink  = 2;
	}
	else
	{
	    sbuf->st_mode = 0666 | S_IFREG;
	    sbuf->st_nlink  = 1;
	}

	free(fpath);
	free(fpath2);
	
	if(ub->buf[8]==0)
	{
                errno = ENOENT;
		return -1;
	}
	return 0;
    }
    else
    {
      statworks=0;
    }
  } /* CC_STAT */

  if( (drp = util_opendir(ppath)) )
  {
    for(dep = drp->dep; *dep; dep++)
    {
      if(!strcmp((*dep)->name,pfile))
      {
	if((*dep)->type == RDTYPE_DIR)
	    sbuf->st_mode = 0777 | S_IFDIR;
	else
	    sbuf->st_mode = 0666 | S_IFREG;
	
	if((*dep)->type == RDTYPE_DIR)
	    sbuf->st_nlink  = 2;
	else
	    sbuf->st_nlink  = 1;
	
	sbuf->st_uid = 0;
	sbuf->st_gid = 0;
	sbuf->st_size  = BB_READ4((*dep)->bb_size);
	sbuf->st_atime = sbuf->st_mtime =
			 sbuf->st_ctime = BB_READ4((*dep)->bb_time);
	util_closedir(drp);
	free(fpath);
	free(fpath2);
	return(0);
      }
    }
    util_closedir(drp);
  }

  free(fpath);
  free(fpath2);
  errno = ENOENT;
  return(-1);
}

int util_cd (char * p)
{
  char *fpath;
  UBUF *ub;
  DDLIST   *ddp;

  fpath = util_abs_path(p);
  for(ddp = ddroot; ddp; ddp = ddp->next)
    if(!strcmp(ddp->path,fpath)) break;

  if(!ddp && strcmp(p,".") && strcmp(p,"..")) {
    ub = client_interact(CC_GET_DIR,0L, strlen(fpath),
			 (unsigned char *) fpath+1, 2,
			 (unsigned char *)&client_net_len);
    if(ub->cmd == CC_ERR) {
      free(fpath);
      fprintf(stderr,"%s: %s\n",p, ub->buf);
      errno = EACCES;
      return(-1);
    }
  }

  if(env_dir_malloced) free(env_dir);
  env_dir_malloced = 1;
  env_dir = fpath;
  return(0);
}

/* Perform a cd, but don't verify path.  Assume the path has been
 * pre-verified
 */
int util_cd2 (char * p)
{
  char *fpath;

  fpath = util_abs_path(p);

  if(env_dir_malloced) free(env_dir);
  env_dir_malloced = 1;
  env_dir = fpath;
  return(0);
}

void util_process_file(char *path, int mode,
		void (*process_file)(char *,struct stat *, int, int),
		int (*process_start_dir)(char *,struct stat *,u_long *),
		void (*process_end_dir)(char *,int,u_long,int),
		       int level)
{
  struct stat sbuf;
  RDIR *rdir;
  struct rdirent *rde;
  int pathlen;
  char *newname;
  u_long sum;

  if (util_stat(path, &sbuf) < 0) {
    perror(path);
    return;
  }

  if (S_ISREG(sbuf.st_mode)) {
    if(process_file) (*process_file)(path, &sbuf, mode, level);
  } else if (S_ISDIR(sbuf.st_mode)) {
    sum = mode;
    if (process_start_dir && (*process_start_dir)(path, &sbuf, &sum) < 0)
      fprintf(stderr, "skipping remote directory `%s'\n", path);
    else {
      if ((rdir = util_opendir(path))) {
	pathlen = strlen(path);
	while ((rde = util_readdir(rdir))) {
	  /* skip over "." and ".." */
	  if (rde->d_name[0] == '.' &&
	      (rde->d_name[1] == '\0'  ||
	       (rde->d_name[1] == '.' && rde->d_name[2] == '\0')))
	    continue;
	  newname = malloc(pathlen + rde->d_namlen + 2);

	  strcpy(newname, path);
	  if(newname[pathlen-1] != '/') newname[pathlen] = '/';
	  else pathlen--;
	  strcpy(newname + pathlen + 1, rde->d_name);
	  util_process_file(newname, mode, process_file, process_start_dir,
			    process_end_dir, level + 1);
	  free(newname);
	}
	util_closedir(rdir);
      }
      if(process_end_dir) (*process_end_dir)(path, mode, sum, level);
    }
  } else
    fprintf(stderr, "remote file `%s' is not a file or directory!\n",path);
/*  free(path); */
}
