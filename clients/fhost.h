struct fsp_host {
    char *hostname;
    char *hostaddr; /* decimal ip address */
    char **alias;
    int port;
    int local_port;
    char *dir;
    char *local_dir;
    int delay;
    int timeout;
    int trace;
    char *password;
};

#define NUMBER 1
#define NAME 2
struct fsp_host * init_host(void);
void add_host(struct fsp_host *h);
void add_host_alias(struct fsp_host *h, const char *name);
struct fsp_host *find_host(const char *name);
void list_prof_file (void); /* list resource file */
int print_host_setup(struct fsp_host *setup,int csh,int lhost);
void list_sites_file(void); /* list resource file in fspsites format */
