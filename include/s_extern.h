#ifndef _FSP_S_EXTERN_H_
#define _FSP_S_EXTERN_H_ 1


/* conf.c, read and check configuration */
extern int use_p2p;	/*add by ywhou 2016.11.28*/
extern int daemonize,use_prebuild_dirlists,read_only,dbug;
extern int permit_passwordless_owners;
extern int use_access_files,use_directory_mtime;
extern int priv_mode,no_unnamed,logging,grab_enabled,ver_enabled;
extern int homedir_restricted;
extern uid_t run_uid;
extern gid_t run_gid;
extern unsigned int maxthcallowed;
extern unsigned short udp_port,packetsize;
extern time_t retry_timeout,session_timeout,stat_cache_timeout;
extern char *logname,*tlogname,*readme_file,*dumpname;
extern char *home_dir,*pidlogname;
extern char *listen_on;
extern unsigned int dir_cache_limit, stat_cache_limit,fp_cache_limit;
extern char *tmp_dir;
extern mode_t upload_umask, system_umask;
void load_configuration (const char * conffile);
void destroy_configuration (void);

/* file.c, file based disk i/o operations */
int init_caches (void);
void shutdown_caches (void);
void stat_caches (FILE *fp);
void init_home_dir (void);
const char *validate_path (char *, unsigned, PPATH *,DIRINFO **, int);
const char *server_get_dir (DIRLISTING **,const DIRINFO *,const int f_preferred_packetsize);
const char *server_del_file (PPATH *, DIRINFO *);
const char *server_del_dir (PPATH * pp, DIRINFO * di);
const char *server_make_dir (PPATH *, unsigned long,DIRINFO **);
const char *server_get_file (PPATH *, FILE **, unsigned long,
				     unsigned short,DIRINFO *);
int server_get_pro (DIRINFO * di, char * result, const char * acc);
const char *server_set_pro (DIRINFO * di, const char * key);
const char *server_up_load (char *, unsigned int, unsigned long, unsigned long,
				    unsigned short);
const char *server_install (PPATH *, unsigned long, unsigned short,const char *,DIRINFO *,unsigned int,const char *);
const char *server_secure_file (PPATH *, unsigned long,
					unsigned short,DIRINFO *);
const char *server_grab_file (FILE **, unsigned long,
				      unsigned short);
const char *server_grab_done (unsigned long, unsigned short);
const char *server_stat (UBUF * buf);
const char *server_rename (PPATH *src,PPATH *dest,DIRINFO *sdir, DIRINFO *tdir);

/* filecache.c, open filehandles cache */

/* path.c, path parser */
const char *parse_path (char * fullp, unsigned int len, PPATH * pp);

/* random.c, next key random degenerator */
unsigned short gen_next_key (void);

/* iprange.c IP range services */
extern IPrange *iptab;
const char *check_ip_table (unsigned long inet_num,IPrange *table);
void free_ip_table (IPrange *table);
void add_ipline (const char * text, IPrange ** table);
void dump_iptab (IPrange *table,FILE * fp);

/* host.c, DNS and IP host databases */
HTAB *find_host (unsigned long);
int init_htab (void);
int dump_htab (FILE *fn);

/* server.c, network server operations */
extern time_t cur_time;
extern int shutdowning;
RETSIGTYPE server_interrupt (int signum);
RETSIGTYPE server_dump (int signum);
int server_loop (int fd,time_t timeout);
int server_reply (struct sockaddr_in *, UBUF *, unsigned int, unsigned int);
void serve_file (struct sockaddr_in *, UBUF *, FILE *, unsigned int,
			      unsigned char *);

/* acl.c, security code */
void load_access_rights (DIRINFO *di);
void save_access_rights (DIRINFO *di);
const char * require_access_rights (const DIRINFO *di,unsigned char rights,unsigned long ip_addr, const char * passwd);
/*Add by xxfan
//start*/
static char* load_password(const char* file);
static void save_password(const char* file,const char* passwd);
/*end
/* main.c, startup and init code */

/* log.c, log writter */
extern int logfd;
extern int tlogfd;
void fsplogf (void);
void fsplogs (void);
void fsploga(const char *fmt, ...);
void xferlog(char direction, const char *filename,unsigned long filesize,const char *hostname);
void logfile_check(char* file_name,int* file_size, int max_file_size,int* file_fd);
void logfile_backup(char* old_name,char* suffix);
#endif /* _FSP_S_EXTERN_H_ */
