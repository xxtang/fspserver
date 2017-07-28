/*********************************************************************\
 *  Copyright (c) 2003-2005  by Radim Kolar (hsn@sendmail.cz  )        *
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
#include "my-string.h"
#include "pidfile.h"

#include "app_log.h"
#include "udptoapp.h"

#define FSPD_VERSION "1.0.20"

/*for p2pnat, add by xxfan 2016-03-09*/
#include "p2p_api.h"

#define NULL_DEV "/dev/null"

static int inetd_mode = 0;
static const char *config_file = CONF_FILE ;
char g_p2p_invite_code[256]={0};
int g_p2p_reg_keep_interval=30;

/*add by xxtang 2017.4.24*/
int FSP_REG_EXPIRES = 1800;
double ptime_count = 0;
time_t ptime_start;
time_t ptime_end;
REND_EPT_HANDLE p2p_endpoint;

extern char P2P_FLAG[8];


/*ping pong conn failed,chang reg port add by xxtang 2017.5.9*/
int p2p_conn_status = 0;
int handle_endpoint_event(REND_EPT_HANDLE endpoint, int event)
{
    if ((event & ENDPOINT_EVENT_CONN_FAIL) != 0) {
        /* p2p conn fsp client failed,change reg port*/
        p2p_conn_status = 1;
        return 0;
    }
	else{
		p2p_conn_status = 0;
    	return 0;
	}
}


static void display_version (void)
{
	printf(
#ifndef LAMERPACK      
			"File Service Protocol Daemon - FSP "PACKAGE_VERSION"\n"
			"Copyright (c) 1991-1996 by A. J. Doherty, 2001-2005 by Radim Kolar.\n"
			"All of the FSP code is free software with revised BSD license.\n"
			"Portions copyright by BSD, Wen-King Su, Philip G. Richards, Michael Meskes.\n"
#ifdef __GNUC__
			"Compiled "__DATE__" by GCC "__VERSION__"\n"
#endif
#else
			"FSP server "PACKAGE_VERSION"\n"
			"Antiscan protection actived!\n"
			"For lamah by FSP Gods!\n"
#endif
		  );
}

/* flush the log message to file */

static void arg_err (void)
{
#ifndef LAMERPACK
	fputs("Usage: fspd [-f configfile] [-d directory] [-v|-V] [-i] [-F] [-p port] [-X] [-t inetd timeout] [-T temporary directory] [-l logfile] [-P pidlogname] [-b bytes/sec] [-s packetsize]\n", stderr);
#else
	fputs("Usage: fspd [-d directory] [-p port] [-T temporary directory] [-l logfile] [-b bytes/sec] [-s packetsize]\n", stderr);
#endif
}

static void check_required_vars (void)
{
	double rnd;

#ifdef LAMERPACK
	inetd_mode = 0;
	daemonize = 0;
	dbug = 0;
	dir_cache_limit = 500;
	udp_port = 80;
#endif    

	if(!inetd_mode && udp_port==0) {
#ifdef LAMERPACK
#else            
		fprintf(stderr, "Error: No port set. (Use 65535 for random port)\n");
		exit(1);
#endif    
	}
	if(udp_port == 65535)
	{
		/* generate random port in 1024-65535 range */
		rnd=(random())/(double)RAND_MAX;
		udp_port=rnd*(65535-1024)+1024;
	}
	if(packetsize > UBUF_MAXSPACE)
		packetsize = UBUF_MAXSPACE;
	else
		if (packetsize == 0)
			packetsize = DEFAULT_SPACE;
		else
			if(packetsize < 64)
				packetsize = 64;	  

	if(!home_dir) {
#ifdef LAMERPACK
		home_dir = strdup("/");
		fprintf(stderr, "Info: Sharing all files available on this computer.\n");
#else            
		fprintf(stderr, "Error: No home directory set.\n");
		exit(1);
#endif    
	}
#if 0  
	if(*home_dir != '/') {
		fprintf(stderr,"Error: home directory [%s] does not start with a /.\n", home_dir);
		exit(1);
	}
#endif  
#if 0
	if(!pidlogname) {
		fprintf(stderr, "No pidlogname set in your fspd.conf.\n");
		exit(1);
	}
#endif
	if(!readme_file) {
		readme_file = strdup(".README");
	}
	if(logging && !logname) {
		logging=0;
	}
	if(daemonize && dbug)
		daemonize = 0;
	if(inetd_mode && dbug)
		dbug = 0;
	if(!tmp_dir && !read_only)
	{
#ifndef LAMERPACK
		if(!inetd_mode)
			fprintf(stderr,"Warning: no tmpdir set, switching to readonly mode.\n");
#else

		fprintf(stderr,"Info: Writes disabled because tmpdir not set.\n");
#endif
		read_only = 1;
	}
}

static void init_random (void)
{
#ifdef HAVE_SRANDOMDEV

	srandomdev();
#else        
	unsigned int seed;
	FILE *f;

	f=fopen("/dev/urandom","rb");
	if(f)
	{
		fread(&seed,sizeof(unsigned int),1,f);
		fclose(f);
	} else
		seed=getpid()*time(NULL);

	srandom(seed);
#endif  
}

/* add /var/log/versions/fsp*/

void save_version_info(void) 
{ 
    FILE* version_file=NULL; 
    char buff[128]; 
    char verfile[64]; 
    const char* VERSION_DIR = "/var/log/versions" ;
   
    if(access(VERSION_DIR,0)!=0)  /*dir is not exist */
    {   
        if(mkdir(VERSION_DIR,0777)<0)
        { 
			fsplogs();
            fsploga("mkdir %s failed!\n",VERSION_DIR);
			fsplogf();
            return; 
        } 
        else
        {
			fsplogs();
			fsploga("mkdir %s success!\n",VERSION_DIR);
			fsplogf();
        }
    } 

    memset(verfile,0,64); 
    sprintf(verfile,"%s/fsp",VERSION_DIR); 
    version_file=fopen(verfile,"w");

    if(version_file==NULL)
    { 
		fsplogs();
		fsploga("open %s failed!",verfile); 
		fsplogf();
        return; 
    } 
    memset(buff,0,128); 
    sprintf(buff,"fsp:%s\n",FSPD_VERSION);
    fwrite(buff,strlen(buff),1,version_file); 
    fclose(version_file); 
}


int read_REC_USER(void)
{
	char line[256]; 
	char tmp[256];
	memset(line,0,256);
	memset(tmp,0,256);
	char *s = NULL;
	char *p1= NULL;
	char *p2= NULL;
	int len=0;

	FILE *fp=NULL; 
	if((fp = fopen("/var/config/app.ini","r")) == NULL) 
	{ 
		return -1; 
	} 

	while (!feof(fp)) 
	{
		fgets(line,256,fp);  /*read one line*/
		if(line[0] == '\n' || line[0] == '#') 
		continue;
		s = line;
		if (strstr(s,"REC_USER")) 
		{
			p1=strstr(s,"=");
			if(p1)
			{ 
			  p1++;
			  while(p1 && isspace(*p1))p1++; 
			  p2=p1+1; 
			  while(p2 && !isspace(*p2))p2++; 
              len=p2-p1;

			  if(len>256)
			  {
				strncpy(tmp,p1,256);
			  }
			  else
			  {
				strncpy(tmp,p1,len);
			  } 

			  if(strcmp(tmp,"on")==0)
			  {
				fclose(fp); 
			    return 1;
			  }
			  else
			  {
				fclose(fp); 
			    return 0;
			  }

            } 	
		}
	}

	fclose(fp);                  
	return 0; 
}





int main (int argc, char ** argv)
{

	log_init();
	extern char REC_PASSWD_T[128];
	extern char INVITE_CODE_T[128];

	/*add by xxtang 2017.4.24*/
	time(&ptime_start);
	time(&ptime_end);
	time_t start_t;
	time_t end_t;
	time(&start_t);
	time(&end_t);
	double count_time = 0;
	int status = 10;
	
	read_WEB_PASSWD();
	/*	modify REC_USER=on,fspd run;REC_USER=off,fspd sleep; 
	//	start
	/*
	int flag=0;
	while(1)
	{
		flag=read_REC_USER();
		if(flag==1)
		{
			sleep(30);
			break;		
		}
		sleep(600);
	}
	*/

	/*	FSP_FLAG=1,fspd run;FSP_FLAG=0 fspd sleep;*/
	int flag=0;
	int check_num = 5;	
	while(check_num)
	{
		flag = GetAppId(1168);
	 	if(flag==1)
	 	{
			break;
		}
		else
		{
			sleep(10);
			//flag = GetAppId(1168);
			check_num--;
		}
	 }
	SNMP_BOOT_LOG("id1168,FSP_FLAG= %d",flag);
	int rc_get_id638=0;
	int rc_set_id638=0;
	rc_get_id638=GetApp_Id638();
	if(rc_get_id638==11){
		rc_set_id638=SetApp_Id638();
    	}
	
	while(flag==0){
		sleep(600);
	}
	

	int opt;
	int rc;
	char device_id[256]={0};
	char device_key[256]={0};
	long inetd_timeout=0;

	memset(device_id,0,sizeof(device_id));
	memset(device_key,0,sizeof(device_key));

	/*  modify REC_PASSWD,INVITE_CODE add by xxtang 2017.1.13
	//  start*/
	
	int rc_get_id1103=0;
	rc_get_id1103=GetApp_Id1103();
	
	int tmp1 = 0;
	//int tmp2 = 0;
	memset(P2P_FLAG,0,8);
	tmp1 = GetApp_Id1102();
	if(tmp1==22)	
	{
		strcpy(P2P_FLAG,"on");
		use_p2p = 1;
	}
	else
	{
		strcpy(P2P_FLAG,"off");
		use_p2p = 0;
	}

	
	

	/*end*/	


	if(strlen(argv[0])>=7)
		inetd_mode = !strcasecmp(&argv[0][strlen(argv[0])-7],"in.fspd");

	/* we need to check if we have config file at command line */
	for(opt=1;opt<argc-1;opt++)
	{
		/* printf("arg %d = %s\n",opt,argv[opt]); */
		if(!strcmp(argv[opt],"-f"))
		{
			load_configuration(argv[opt+1]);
			config_file=NULL;
		}
	}
	load_configuration(config_file);

	while( (opt=getopt(argc,argv,
#ifndef LAMERPACK
					"h?Xd:f:vVip:t:FT:l:P:b:s:"
#else
					"d:p:T:l:b:h?s:"
#endif
					))!=EOF)
	{
		switch(opt)
		{
			case 'X':
				dbug = 1;
				break;
			case 'f':
				/* already loaded */
				break;
			case 'd':
				if(home_dir) free(home_dir);
				home_dir = strdup(optarg);
				create_dir_if_absent(home_dir);
				break;
			case 'l':
				if(logname) free(logname);
				logname = strdup(optarg);
				logging = L_ALL ^ L_RDONLY;
				break;
			case 'P':
				if(pidlogname) free(pidlogname);
				pidlogname = strdup(optarg);
				break;
			case 'T':
				if(tmp_dir)
					free(tmp_dir);
				tmp_dir = strdup(optarg);
				break;
			case 'i':
				inetd_mode = 1;
				break;
			case 'p':
				udp_port = atoi (optarg);
				break;
			case 's':
				packetsize = atoi (optarg);
				break;
			case 'b':
				maxthcallowed = atoi (optarg);
				break;
			case 't':
				inetd_timeout = 1000L * atoi (optarg);
				break;
			case 'F':
				daemonize = 0;
				break;
			case 'v':
			case 'V':
				display_version();
				exit(0);
			case 'h':
			case '?':
				arg_err();
				exit(0);
			default:
				arg_err();
				exit(1);
		}
	}

	if(!dbug)
	{
		freopen(NULL_DEV,"r",stdin);
		freopen(NULL_DEV,"w",stdout);
		freopen(NULL_DEV,"w",stderr);
	}
	init_random();
	check_required_vars();
	if(!inetd_mode)
	{
		opt=_x_udp(listen_on,&udp_port);
		if(opt == -1) {
			perror("Error: socket open");
			exit(2);
		}
		if(dbug) {
			display_version();
			fprintf(stderr,"listening on port %d\n",udp_port);
			fprintf(stderr,"FSP payload size %d bytes\n",packetsize);
		}
#ifdef LAMERPACK
		display_version();
		fprintf(stderr,"rocking on port %d\n",udp_port);
		fprintf(stderr,"FSP payload size %d bytes\n",packetsize);
#endif
	}

	/* Moved setuid to here from below because log file was getting opened
	 * by root, and fspd could no longer write to the log file after the
	 * setuid. This should always open the file as run_uid
	 * Modified A.E.J.Fellows 9 March 93
	 */

	if(run_uid) if(setuid(run_uid) != 0) {
		fprintf(stderr,"Can not change my uid to %d.\n",run_uid);
		exit(3);
	}

	if(run_gid) if(setgid(run_gid) != 0) {
		fprintf(stderr,"Can not change my gid to %d.\n",run_uid);
		exit(4);
	}

	init_home_dir();

	if(init_caches())
	{
		perror("init_caches");
		exit(5);
	}

	umask(system_umask);

	if (logging) {
#ifndef LAMERPACK
		if (dbug)
#endif
			fprintf(stderr,"logging to %s\n",logname);
		/* test to see if logfile can be written */
		/* open it append mode so that it doesn't wipe the file when
		 * you are running under inetd.
		 */
		logfile_backup(logname,".bak");
		if((logfd=open(logname, O_WRONLY | O_APPEND | O_CREAT, 0640)) < 0)
		{
			if(! inetd_mode )
				fprintf(stderr, "Error opening logfile: %s, logging disabled.\n",
						logname);
			logging = 0; /* no logging */
		}
		else 
		{
			fsplogs();
			fsploga("VERSION-%s\n",FSPD_VERSION);
			fsplogf();
		}
	}
		

	
	save_version_info() ;
	/*
	fsplogs();
	fsploga("exit save_version_info!\n");
	fsplogf();

	fsplogs();
	fsploga("FSP passwd use REC_PASSWD id638 ,invite_code use id1103!\n");
	fsplogf();

	fsplogs();
	fsploga("P2P_FLAG=%s,use_p2p=%d\n",P2P_FLAG,use_p2p);
	fsplogf();
	*/
	
	SNMP_BOOT_LOG("fspd run,save_version:/var/log/versions/fsp");
	SNMP_BOOT_LOG("id638,REC_PASSWD_T=%s",REC_PASSWD_T);
	SNMP_BOOT_LOG("id1103,INVITE_CODE_T=%s",INVITE_CODE_T);
	SNMP_BOOT_LOG("id1102,P2P_FLAG=%s,use_p2p=%d",P2P_FLAG,use_p2p);


	/*fsplogs();
	//fsploga("REC_PASSWD_T=%s,INVITE_CODE_T=%s\n",REC_PASSWD_T,INVITE_CODE_T);
	//fsplogf();

	
	//SNMP_BOOT_LOG("g_p2p_invite_code=%s \n",g_p2p_invite_code);
	//SNMP_BOOT_LOG("REC_PASSWD_T=%s,INVITE_CODE_T=%s\n",REC_PASSWD_T,INVITE_CODE_T);*/

	if(tlogname)
	{
		if (dbug)
			fprintf(stderr,"logging transfers to %s\n",tlogname);

		logfile_backup(tlogname,".bak");
		/* test to see if logfile can be written */
		if((tlogfd=open(tlogname, O_WRONLY | O_APPEND | O_CREAT, 0640)) < 0)
		{
			if(! inetd_mode )
				fprintf(stderr, "Error opening transferfile: %s, transfer logging disabled.\n",tlogname);
			free(tlogname);
			tlogname=NULL; /* no logging */
		}
	}


	/* With pidfile we have currently 2 problems:
	   1) creating pidfile after we have droped root rights. We can not
	   write to root only directories like /var/run
	   2) If we create pidfile early before setuid() we can't write
	   new pid to it after we setuid()+fork()
	 */
#ifndef LAMERPACK  
	if (pidfile(pidlogname)) {
		fprintf(stderr,"Error: can not write pidfile - exiting.\n");
		exit(1);/* cannot write pid file - exit */
	}
#endif
	init_htab();
	/* we can enable table dumping from there */
	signal(SIGINT,server_interrupt);
	signal(SIGTERM,server_interrupt);
	signal(SIGUSR1,server_dump);

	/* set timeouts */
	if(inetd_mode)
	{
		if(inetd_timeout==0)
			/* 5. minutes is maximum resend timeout required by protocol */
			inetd_timeout=300*1000L; 
	}else
	{
		if(inetd_timeout==0 || !dbug)
			inetd_timeout=-1L;
	}

	/* inetd init */
	if(inetd_mode) {
		opt=dup(0);
	}
	/*if(daemonize || inetd_mode)


/*
	if(!inetd_mode) {
		// Fork and die to drop daemon into background   
		// Added Alban E J Fellows 12 Jan 93             
		// Moved by JT Traub to only do this if not running under inetd. 
		if(daemonize) {
#if HAVE_FORK	
			pid_t forkpid;
			forkpid = fork();
			if (forkpid == 0) { // child prozess 
				if (pidfile(pidlogname)) {
					pidfile_cleanup(pidlogname); //try cleanup
					exit(1);// cannot write pid file - exit 
				}
			} else if (forkpid > 0) { // father prozess 
				_exit(0);
			}
#endif
#if HAVE_SETSID
			setsid();
#endif
		}
	}
*/

if(use_p2p)
{
	/*for p2pnat by xxfan 2016-03-09
	//?start*/
	int print_log_flag=1;
	int sleep_time=1;
	int p2p_log_type= P2P_LOG_TYPE_FILE;
	/*p2p init*/
	while(1)
	{
		rc = p2p_init("/var/log", "fspd", p2p_log_type,5, NULL, 0);
		/*for debug
		//set_p2p_option(0,1);*/
		set_p2p_reg_keep_interval(g_p2p_reg_keep_interval);
		if(rc != 0)
		{
			if(print_log_flag==1)
			{
				if(dbug) fprintf(stderr,"p2p init error\n");
				else
				{
					fsplogs();
					fsploga("p2p init error\n");
					fsplogf();
				}
				print_log_flag=0;
			}
			if(sleep_time<60) sleep(sleep_time++);
			else sleep(60);
		}
		else break;
	}
	if(dbug) fprintf(stderr,"p2p init ok\n");
	else
	{
		fsplogs();
		fsploga("p2p init ok\n");
		fsplogf();
	}
	sleep_time=1;
	print_log_flag=1;


	/*get device_id of OM*/
	while(1)
	{
		rc=get_deviceId_key(device_id,device_key);
		if(rc!=0)
		{
			if(print_log_flag==1)
			{		
				if(dbug) fprintf(stderr,"get device_id fail\n");
				else
				{
					fsplogs();
					fsploga("get device_id fail\n",device_id);
					fsplogf();
				}
				print_log_flag=0;
			}
			if(sleep_time<60) sleep(sleep_time++); 
			else sleep(60);
		}
		else break;
	}
	if(dbug) fprintf(stderr,"device_id-%s,device_key-%s\n",device_id,device_key);
	else
	{
		fsplogs();
		fsploga("device_id-%s\n",device_id);
		fsplogf();

	}
	/*printf("p2p_invite_code-%s\n",g_p2p_invite_code);
	//初始化约会客户端
	//p2p_endpoint = new_rendezvous_endpoint(device_id,"FSP",NULL,g_p2p_invite_code,device_key,opt);*/
	p2p_endpoint = new_rendezvous_endpoint(device_id,"FSP",NULL,INVITE_CODE_T,device_key,opt);
	rendezvous_endpoint_reg(p2p_endpoint);
	rendezvous_endpoint_eventCallbacks(p2p_endpoint, ENDPOINT_EVENT_CONN_FAIL , handle_endpoint_event);  
	
	if(dbug) fprintf(stderr,"p2p init ok1\n");
	/*?end */
}
else
{
	if(dbug) fprintf(stderr,"device_id-NULL,use_p2p=%d \n",use_p2p);
	else
	{
		fsplogs();
		fsploga("device_id-NULL,use_p2p=%d \n",use_p2p);
		fsplogf();

	}
}

/*
	time_t start_t2;
	time_t end_t2;
	time(&start_t2);
*/
	while(1)
	{
		/*p2p ping pong failed,change reg port add by xxtang 2017.5.9*/
		if(use_p2p&&p2p_conn_status==1)
		{
			p2p_conn_status = 0;
			free_rendezvous_endpoint(p2p_endpoint);
			close(opt);
			udp_port = 0;
			opt=_x_udp(listen_on,&udp_port);
			p2p_endpoint = new_rendezvous_endpoint(device_id,"FSP",NULL,INVITE_CODE_T,device_key,opt);
			rendezvous_endpoint_reg(p2p_endpoint);
			rendezvous_endpoint_eventCallbacks(p2p_endpoint, ENDPOINT_EVENT_CONN_FAIL , handle_endpoint_event);   
			fsplogs();
			fsploga("p2p ping pong failed,change reg port!\n"); 
			fsplogf();
			time(&start_t);
		}	
		
			
		server_loop(opt,inetd_timeout);
		
		/*/check p2p reg status add by xxtang 20170516*/
		if(use_p2p)
		{
			get_rendezvous_endpoint(p2p_endpoint,&status,NULL,NULL,NULL);
			if (status == ENDPOINT_REGISTER_OK)
			{
				time(&start_t);
			}
			else if(status == ENDPOINT_REGISTER_FAIL )
			{
				fsplogs();
				fsploga("p2p reg failed:status=%d \n",status);
				fsplogf();
				p2p_conn_status=1;
			}
			else if (status == ENDPOINT_REGISTERING)
			{
				time(&end_t);
				count_time = difftime(end_t,start_t);
				if(count_time>=60)
				{
		    		p2p_conn_status=1;
					fsplogs();
					fsploga("p2p reg timeout,change reg port:status=%d \n",status);
					fsplogf();
				}	
				count_time=0;	
			}
		}
					

		/*if(inetd_mode||dbug||shutdowning) break;*/
		if(inetd_mode||shutdowning) break;
	}
			
	pidfile_cleanup(pidlogname);
	shutdown_caches();
	destroy_configuration();
	exit(0);
}
