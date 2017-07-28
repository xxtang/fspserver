#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>  
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h>  
#include <unistd.h>  
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>  
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>

#include "app_log.h"

#include "udptoapp.h"

char REC_PASSWD_T[128];
char INVITE_CODE_T[128];
char P2P_FLAG[8];


/***define globle parameter***************************/
#define nFSP_UDP_PORT  (1444)
int udp_cpe_fd = 0;
char send_buff[5120];
char recv_buff[8192];        
unsigned int gcookie;
char WEB_PASSWD[128]="admin";






int rand_pass()
{
	int passwd=0;
	srand((unsigned)time(NULL));
	passwd= rand()%800000+100000;
	return passwd;
}



/*,1. rc send data to app*/
RET_CODE  rcToAppPrimitive(E_PRIMITIVE_CODE eCode, void *payload )  
{
    S_PRIMITIVE *primitive = NULL;
    int  rc = 0;
    struct sockaddr_in remote;
    unsigned char *ptr;
    S_CONFIG      *config;
    UINT32         payload_length = strlen(payload);  /* payload size*/

    gcookie = time(NULL);


    memset( &remote, 0, sizeof(remote) );
    remote.sin_family =  AF_INET;
    remote.sin_port        =  htons(1305);
    remote.sin_addr.s_addr =  htonl(0x7f000001);

    primitive = (S_PRIMITIVE *)send_buff;
    
    primitive->source.component_id = eCOMPONENT_BOA;    /* eSource;*/
    primitive->dest.component_id   = eCOMPONENT_CONFIG; /* eDest;*/
    primitive->eCode               = eCode;  
    primitive->status              = 0;
    primitive->payload_length      = payload_length +  sizeof(S_CONFIG) + 10;
    primitive->payload             = primitive->data;

    config = (S_CONFIG*)primitive->data;

    config->eCategory = eCFG_CAT_XML;
    config->num_variable = 0;
    config->flag_reset   = 0;
    config->cookie	= gcookie;
    config->remote_ip    = htonl(0x7f000001);
    
    config->num_variable = 1;
    ptr = (unsigned char *)&config->var_start.data;
    config->var_start.type   = htons(nFSP_UDP_PORT);
    config->var_start.length = payload_length + 1;

    memcpy( ptr, payload,  payload_length );
    ptr[payload_length] = 0;  /* NULL term*/


    rc = sendto( udp_cpe_fd,
                 (char *)primitive,
                 sizeof(S_PRIMITIVE)+payload_length+sizeof(S_CONFIG)+10-nMAX_INBAND_PRIM_PAYLOAD_SIZE,
                 0, 
                 (struct sockaddr*) &remote, sizeof(struct sockaddr_in) );
	
    return (rc);

}


/* parse app response data */
int parse_app_response_getid638(char* buf,int length)
{
	char *p1=NULL;
	char *p2=NULL;
	char *p3=NULL;
	char tmp[128];
	memset(tmp,0,128);
	p1=strstr(buf,"value");
	/*SNMP_BOOT_LOG("p1=%s\n",p1);*/
	if(p1)
	{
		p2=strchr(p1,'\"');
		p2=p2+1;
		p3=strchr(p2,'\"');
		strncpy(tmp,p2,p3-p2);
		/*SNMP_BOOT_LOG("id638,tmp=%s\n",tmp);
		//SNMP_BOOT_LOG("p2=%s\n",p2);
		//SNMP_BOOT_LOG("p3=%s\n",p3);*/
		if(strcmp(tmp,"")==0)
		{
			return 11;/*value =null*/
		}
		else
		{
			memset(REC_PASSWD_T,0,128);
			strcpy(REC_PASSWD_T,tmp);
			return 12;/*value !=null*/
		}
		
	}
 return 13; /* no value */
}

int parse_app_response_setid638(char* buf,int length)
{
	char *ptr1=NULL;
	char *ptr2=NULL;
	char *ptr3=NULL;
	char tmp[128];
	memset(tmp,0,128);
	ptr1=strstr(buf,"stat");
	/*SNMP_BOOT_LOG("ptr1=%s\n",ptr1);*/
	if(ptr1)
	{
		ptr2=strchr(ptr1,'\"');
		ptr2=ptr2+1;
		ptr3=strchr(ptr2,'\"');
		strncpy(tmp,ptr2,ptr3-ptr2);
		/*SNMP_BOOT_LOG("id638,tmp=%s\n",tmp);
		//SNMP_BOOT_LOG("ptr2=%s\n",p2);
		//SNMP_BOOT_LOG("ptr3=%s\n",p3);*/
		if(strcmp(tmp,"ok")==0)
		{
			return 14; /*set ok*/
		}
		else
		{
			return 15; /*set failed*/
		}
		
	}
 return 16; /*set no response*/
}

int parse_app_response_getid1102(char* buf,int length)
{
	char *p1=NULL;
	char *p2=NULL;
	char *p3=NULL;
	char tmp[128];
	memset(tmp,0,128);
	
	p1=strstr(buf,"value");
	/*SNMP_BOOT_LOG("p1=%s\n",p1);*/
	if(p1)
	{
		p2=strchr(p1,'\"');
		p2=p2+1;
		p3=strchr(p2,'\"');
		strncpy(tmp,p2,p3-p2);
		/*SNMP_BOOT_LOG("id638,tmp=%s\n",tmp);
		//SNMP_BOOT_LOG("p2=%s\n",p2);
		//SNMP_BOOT_LOG("p3=%s\n",p3);*/
		if(strcmp(tmp,"")==0)
		{
			return 21;/*value =null*/
		}
		else if(strcmp(tmp,"on")==0)
		{
			/*strcpy(P2P_FLAG,tmp);*/
			return 22;/*value =on*/
		}
		else if(strcmp(tmp,"off")==0)
		{
			/*strcpy(P2P_FLAG,tmp);*/
			return 23;/*value =off*/
		}
		else 
		{
			/*strcpy(P2P_FLAG,tmp);*/
			return 24;/*value !=null*/
		}
		
	}
 return 25; /* no value */
}

int parse_app_response(char* buf,int length)
{
	char *p1=NULL;
	char *p2=NULL;
	char *p3=NULL;
	char tmp[128];
	memset(tmp,0,128);
	
	p1=strstr(buf,"value");
	/*SNMP_BOOT_LOG("p1=%s\n",p1);*/
	if(p1)
	{
		p2=strchr(p1,'\"');
		p2=p2+1;
		p3=strchr(p2,'\"');
		strncpy(tmp,p2,p3-p2);
		/*SNMP_BOOT_LOG("id638,tmp=%s\n",tmp);
		//SNMP_BOOT_LOG("p2=%s\n",p2);
		//SNMP_BOOT_LOG("p3=%s\n",p3);*/
		if(strcmp(tmp,"on")==0||strcmp(tmp,"1")==0)
		{
			return 1;
		}
		else 
		{
			return 0; 
		}
		
	}
 return 2; /* no value */
}



int parse_app_response_getid1103(char* buf,int length)
{
	char *p1=NULL;
	char *p2=NULL;
	char *p3=NULL;
	char tmp[128];
	memset(tmp,0,128);
	p1=strstr(buf,"value");
	/*SNMP_BOOT_LOG("p1=%s\n",p1);*/
	if(p1)
	{
		p2=strchr(p1,'\"');
		p2=p2+1;
		p3=strchr(p2,'\"');
		strncpy(tmp,p2,p3-p2);
		/*SNMP_BOOT_LOG("id1103,tmp=%s\n",tmp);
		//SNMP_BOOT_LOG("p2=%s\n",p2);
		//SNMP_BOOT_LOG("p3=%s\n",p3);*/
		if(strcmp(tmp,"")!=0)
		{
			memset(INVITE_CODE_T,0,128);
			strcpy(INVITE_CODE_T,tmp);
			return 21;/*value =null*/
		}
		else
		{
			memset(INVITE_CODE_T,0,128);
			strcpy(INVITE_CODE_T,"new");
			return 22;/*value !=null*/
		}
		
	}
 return 23; /* no value */
}





/*,2. app to rc recv data */
int appTorcPrimitive(int method)
{
	int rc;
	fd_set  netFD;
    struct timeval   timeout;
    int ret=0;
    FD_ZERO (&netFD);
    FD_SET  (udp_cpe_fd, &netFD);
    timeout.tv_sec  = 3;  /*,1s*/
    timeout.tv_usec = 0;
	if ( udp_cpe_fd <= 0 )
	{
		udp_cpe_fd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if ( udp_cpe_fd < 0 )
       	{
       		SNMP_BOOT_LOG("open socket error\n" );
						return -1;
     	}
		else
      	{
        	struct sockaddr_in local;
        	memset( &local, 0, sizeof(local) );
       		local.sin_family =  AF_INET;
       		local.sin_port   =  htons( nFSP_UDP_PORT );
        	/* local.sin_addr.s_addr = htonl(0x7f000001);*/
        	    
        	 rc = bind( udp_cpe_fd, (struct sockaddr*)&local, sizeof(local));
         	    if ( rc != 0 )
         	    {
            		/*SNMP_BOOT_LOG( "socket-%d binding error\n", udp_cpe_fd );*/
           	    close( udp_cpe_fd );
           	    udp_cpe_fd = 0;
                return -1;
          	   }
           	   /*SNMP_BOOT_LOG( "socket:%d open for sending cmd to app\n", udp_cpe_fd ); */    
       		  }
         }        

        
        /* 1) select - see is there any activity */
        rc = select (udp_cpe_fd + 1, &netFD, NULL, NULL, &timeout);
        if ( rc <= 0)
        {
            /* select error*/
            if ( rc < 0)
            {
							SNMP_BOOT_LOG( "ERROR: Receiver Select rc=%d\n", rc );
	    				return -1;
            }
            /* no data*/
	    			return 0;
        }
        else
        {
	    		char *xmldata;

            /* 2) get the data from the socket */
				struct sockaddr_in local;		
				memset( &local, 0, sizeof(local) );
				int len = sizeof(local);

				int recvlen = recvfrom(udp_cpe_fd,
                                   recv_buff,
                                   8192,
                                   0,
                                   (struct sockaddr*) &local,
                                   (socklen_t*) &len);
			
			if ( recvlen>0 )
			{
				S_PRIMITIVE *primitive = NULL;
    		S_CONFIG      *config  = NULL;
				primitive = (S_PRIMITIVE*)recv_buff;
				config	  = (S_CONFIG*)primitive->data;
				xmldata	  = (void*)&(config->var_start.data);

		/*		SNMP_BOOT_LOG( "recv msg from app, xml data len:%d\n",strlen(xmldata) );
				//SNMP_BOOT_LOG( "recv msg from app:%s\n",xmldata);*/
				if (gcookie != config->cookie){
					/*SNMP_BOOT_LOG( "recv msg from app, cookie is error:gcookie=%ul resp_cookie=%ul\n",gcookie,config->cookie );*/
					ret = 9;
					return ret;
				}
				ret = 1;
			    if (xmldata)
        		{
        			/*SNMP_BOOT_LOG( "recv data:\r%s",xmldata);*/
        			if(method==1)
        			{
		    	    	ret = parse_app_response_getid638(xmldata,strlen(xmldata));
        			}
					if(method==2)
					{
						ret = parse_app_response_setid638(xmldata,strlen(xmldata));
					}
					if(method==3)
					{
						ret = parse_app_response_getid1103(xmldata,strlen(xmldata));
					}
					if(method==1102)
					{
						ret = parse_app_response_getid1102(xmldata,strlen(xmldata));
					}
					if(method==1168)
					{
						ret = parse_app_response(xmldata,strlen(xmldata));
					}
				} 

			  }              
         
        }
    return ret;
}


int close_fsp_socket()
{
	if ( udp_cpe_fd )
            close( udp_cpe_fd );
	return 0;
}


int create_fsp_socket()
{
	struct sockaddr_in local;

	/*close_fsp_socket();
	//open socket*/
	udp_cpe_fd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( udp_cpe_fd < 0 )
    {
		SNMP_BOOT_LOG("error in create_fsp_socket\n");
        return -1;
    }

	/*set address*/
    memset( &local, 0, sizeof(local) );
    local.sin_family = AF_INET;
    /*local.sin_port   = htons(nFSP_UDP_PORT);*/
	local.sin_port   = 0;

	int buflen=65535;
	if (setsockopt(udp_cpe_fd, SOL_SOCKET, SO_RCVBUF, (char *)&buflen, sizeof(buflen)) < 0)
    {
		SNMP_BOOT_LOG("error in setsockopt\n");
    }

	if (setsockopt(udp_cpe_fd, SOL_SOCKET, SO_SNDBUF, (char *)&buflen, sizeof(buflen)) < 0)
    {
		SNMP_BOOT_LOG("error in setsockopt\n");
    }
    
	/*bind*/

	if (bind( udp_cpe_fd, (struct sockaddr*)&local, sizeof(local))==-1)
    {
        SNMP_BOOT_LOG("socket bind error errno:[%s]\n",strerror(errno));
        close( udp_cpe_fd );
        udp_cpe_fd = 0;
        return -2;
    }

    if ( udp_cpe_fd > 0 )
        /*SNMP_BOOT_LOG("socket-%d opened for msg from app\n", udp_cpe_fd );*/
    
    return 0;
}





int read_WEB_PASSWD()
{
	char line[256]; 
	char tmp[128];
	memset(line,0,256);
	memset(tmp,0,128);
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
		if (strstr(s,"WEB_PASSWORD")) 
		{
			p1=strstr(s,"=");
			if(p1)
			{ 
			  p1++;
			  while(p1 && isspace(*p1))p1++; 
			  p2=p1+1; 
			  while(p2 && !isspace(*p2))p2++; 
              len=p2-p1;

			  if(len>128)
			  {
				strncpy(tmp,p1,128);
			  }
			  else
			  {
				strncpy(tmp,p1,len);
			  } 

			  if(strcmp(tmp,"")!=0)
			  {
				memset(WEB_PASSWD,0,128);
				strncpy(WEB_PASSWD,tmp,len);
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


int GetApp_Id638()
{
	create_fsp_socket();
	char buf[256];
	memset(buf,0,256);
	sprintf(buf,"method=gw.config.get&id=638&pass=%s",WEB_PASSWD);

	if ((rcToAppPrimitive(ePRIM_CONFIG_GET,buf)) == -1)
 	{
		SNMP_BOOT_LOG("rcToAppPrimitive error");
		return -1;
	}

	int rc1=0;
	rc1=appTorcPrimitive(1);
	/*SNMP_BOOT_LOG("appTorcPrimitive(1)=%d",rc1);*/
	
	
	close_fsp_socket();
	return rc1;

}
	

int SetApp_Id638()
{
	create_fsp_socket();
	char buf[256];
	char tmp2[128];
	memset(buf,0,256);
	memset(tmp2,0,128);
	int rc_REC_PASSWD_T=0;
	rc_REC_PASSWD_T=rand_pass();
	memset(REC_PASSWD_T,0,128);
	sprintf(REC_PASSWD_T,"%d",rc_REC_PASSWD_T);
	
	sprintf(buf,"method=gw.config.set&id638=%d&pass=%s",rc_REC_PASSWD_T,WEB_PASSWD);

	if((rcToAppPrimitive(ePRIM_CONFIG_GET,buf)) == -1)
 	{
		SNMP_BOOT_LOG("rcToAppPrimitive error");
		return -1;
	}

	int rc_id638 = 0;
	rc_id638= appTorcPrimitive(2);	
	
	close_fsp_socket();
	
	return rc_id638;

}

int GetApp_Id1102()
{
	create_fsp_socket();
	char buf[256];
	memset(buf,0,256);
	sprintf(buf,"method=gw.config.get&id=1102&pass=%s",WEB_PASSWD);

	if ((rcToAppPrimitive(ePRIM_CONFIG_GET,buf)) == -1)
 	{
		SNMP_BOOT_LOG("rcToAppPrimitive error");
		close_fsp_socket();
		return -1;
	}
	int rc_id1102=0;
	
	rc_id1102=appTorcPrimitive(1102);
	
	close_fsp_socket();
	
	return rc_id1102;

}


int GetApp_Id1103()
{
	create_fsp_socket();
	char buf[256];
	memset(buf,0,256);
	sprintf(buf,"method=gw.config.get&id=1103&pass=%s",WEB_PASSWD);

	if ((rcToAppPrimitive(ePRIM_CONFIG_GET,buf)) == -1)
 	{
		SNMP_BOOT_LOG("rcToAppPrimitive error");
		return -1;
	}
	int rc_id1103=0;
	
	rc_id1103=appTorcPrimitive(3);
	
	close_fsp_socket();
	
	return rc_id1103;

}

int GetAppId(int id)
{
	create_fsp_socket();
	char buf[256];
	memset(buf,0,256);
	int app_id = 0;
	app_id = id;
	sprintf(buf,"method=gw.config.get&id=%d&pass=%s",app_id,WEB_PASSWD);

	if ((rcToAppPrimitive(ePRIM_CONFIG_GET,buf)) == -1)
 	{
		SNMP_BOOT_LOG("rcToAppPrimitive error");
		close_fsp_socket();
		return -1;
	}
	int rc_id=0;
	
	rc_id=appTorcPrimitive(app_id);
	
	close_fsp_socket();
	
	return rc_id;

}


/*

int update_REC_PASSWD()
{
    char line[256]; 
    char tmp[256];
    memset(line,0,256);
    memset(tmp,0,256);
    char *s = NULL;
	sprintf(tmp,"password  %s",REC_PASSWD_T);

    FILE *fp=NULL; 
    if((fp = fopen("/var/config/fspd.ini","r")) == NULL) 
    { 
        return -1; 
    } 

    while (!feof(fp)) 
    {
        fgets(line,256,fp);  //read one line
        if(line[0] == '\n' || line[0] == '#'|| line[1] == 'e' ) 
        continue;
        s = line;
        if (strstr(s,"password")) 
        {
			fwrite(tmp,strlen(tmp)+11,1,fp); 
     	}
    }

    fclose(fp);                  
    return 0; 
}

int update_INVITE_CODE()
{
    char line[256]; 
    char tmp[256];
    memset(line,0,256);
    memset(tmp,0,256);
    char *s = NULL;
	sprintf(tmp,"p2p_invite_code  %s",INVITE_CODE_T);

    FILE *fp=NULL; 
    if((fp = fopen("/var/config/fspd.ini","r")) == NULL) 
    { 
        return -1; 
    } 

    while (!feof(fp)) 
    {
        fgets(line,256,fp);  //read one line
        if(line[0] == '\n' || line[0] == '#'|| line[1] == 'e' ) 
        continue;
        s = line;
        if (strstr(s,"p2p_invite_code")) 
        {
			fwrite(tmp,strlen(tmp)+11,1,fp); 
     	}
    }

    fclose(fp);                  
    return 0; 
}

*/

