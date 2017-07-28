#ifndef _FSP_C_EXTERN_H_
#define _FSP_C_EXTERN_H_ 1

/* lib.c */
extern int client_trace;
extern unsigned long udp_sent_time,target_delay,target_maxdelay;
UBUF *client_interact (unsigned char, unsigned long, unsigned int,
				     unsigned const char *, unsigned int,
				     unsigned const char *);
void init_client (const char *, unsigned short, unsigned short);
int client_done (void);
void client_finish(void);
extern unsigned long stat_resends, stat_iresends, stat_dupes, stat_bad, stat_ok;

/* lock.c */
extern int key_persists;
unsigned short client_get_key (void);
void client_set_key (unsigned short);
void client_destroy_key (void);
void client_init_key (unsigned long, unsigned long,
				    unsigned short);

/* util.c */
extern const char *env_dir,*env_passwd,*env_local_dir,*env_port,*env_myport,*env_host,*env_listen_on;
extern unsigned int env_timeout;
extern int statworks;
extern unsigned short client_buf_len,client_net_len;
char *util_abs_path (const char *);
void util_junk_password(char *path);
char *util_getwd (char *);
int util_download (char *, FILE *, unsigned long);
int util_grab_file (char *, FILE *, unsigned long);
int util_upload (char *, FILE *, time_t);
void env_client (void);
RDIR *util_opendir (char *);
void util_closedir (RDIR *);
rdirent *util_readdir (RDIR *);
int util_stat (char *, struct stat *);
int util_cd (char *);
int util_cd2 (char *);
void util_process_file (char *, int,
			void (*)(char *,struct stat *,int,int),
			int (*)(char *,struct stat *,u_long *),
			void (*)(char *,int,u_long,int),
			int);

#endif /* _FSP_C_EXTERN_H_ */
